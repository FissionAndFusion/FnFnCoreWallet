#include "dbpserver.h"

#include <openssl/rand.h>
#include <boost/foreach.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>

#include "dbp.pb.h"
#include "lws.pb.h"
#include "dbputils.hpp"

using namespace walleve;

 CDbpClient::CDbpClient(CDbpServer *pServerIn,CDbpProfile *pProfileIn,
                CIOClient* pClientIn,uint64 nonce)
: pServer(pServerIn), pProfile(pProfileIn), pClient(pClientIn),nNonce(nonce)
  
{
}

CDbpClient::~CDbpClient()
{
    if(pClient) pClient->Close();
}

CDbpProfile *CDbpClient::GetProfile()
{
    return pProfile;
}

 uint64 CDbpClient::GetNonce()
 {
     return nNonce;
 }

bool CDbpClient::IsEventStream()
{
    return fEventStream;
}

void CDbpClient::SetEventStream()
{
    fEventStream = true;
}

void CDbpClient::Activate()
{
    fEventStream = false;
    ssRecv.Clear();
    ssSend.Clear();
    
    StartReadHeader();
}

void CDbpClient::SendResponse(CWalleveDbpConnected& body)
{
    ssSend.Clear();

    dbp::Base connectedMsgBase;
    connectedMsgBase.set_msg(dbp::Msg::CONNECTED);
    
    dbp::Connected connectedMsg;
    connectedMsg.set_session(body.session);

    google::protobuf::Any any;
    any.PackFrom(connectedMsg);

    connectedMsgBase.set_allocated_object(&any);
    
    int byteSize = connectedMsgBase.ByteSize();
    unsigned char byteBuf[byteSize];

    connectedMsgBase.SerializeToArray(byteBuf,byteSize);

    unsigned char msgLenBuf[4];
    CDbpUtils::writeLenToMsgHeader(byteSize,(char*)msgLenBuf,4);
    ssSend.Write((char*)msgLenBuf,4);
    ssSend.Write((char*)byteBuf,byteSize);

    pClient->Write(ssSend,boost::bind(&CDbpClient::HandleWritenResponse,this,_1));

}

void CDbpClient::SendResponse(CWalleveDbpFailed& body)
{
    ssSend.Clear();

    
    dbp::Base failedMsgBase;
    failedMsgBase.set_msg(dbp::Msg::FAILED);
    
    dbp::Failed failedMsg;

    for(const int32& version : body.versions)
    {
        failedMsg.add_version(version);
    }

    google::protobuf::Any any;
    any.PackFrom(failedMsg);

    failedMsgBase.set_allocated_object(&any);
 
    int byteSize = failedMsgBase.ByteSize();
    unsigned char byteBuf[byteSize];

    failedMsgBase.SerializeToArray(byteBuf,byteSize);

    unsigned char msgLenBuf[4];
    CDbpUtils::writeLenToMsgHeader(byteSize,(char*)msgLenBuf,4);
    ssSend.Write((char*)msgLenBuf,4);
    ssSend.Write((char*)byteBuf,byteSize);

    pClient->Write(ssSend,boost::bind(&CDbpClient::HandleWritenResponse,this,_1));
}

void CDbpClient::StartReadHeader()
{
    pClient->Read(ssRecv,4,
    boost::bind(&CDbpClient::HandleReadHeader,this,_1));
}

void CDbpClient::StartReadPayload(std::size_t nLength)
{
    pClient->Read(ssRecv,nLength,
        boost::bind(&CDbpClient::HandleReadPayload,this,_1));
}


void CDbpClient::HandleReadHeader(std::size_t nTransferred)
{
    if(nTransferred != 0)
    {
        uint32_t len = CDbpUtils::parseLenFromMsgHeader(ssRecv.GetData(),4);
        if(len == 0)
        {
            pServer->HandleClientError(this);
            return;
        }

        ssRecv.Clear();
        StartReadPayload(len);
    }
    else
    {
        pServer->HandleClientError(this);
    }
}

void CDbpClient::HandleReadPayload(std::size_t nTransferred)
{
    if(nTransferred != 0)
    {
        HandleReadCompleted();
    }
    else
    {
        pServer->HandleClientError(this);
    }
}

void CDbpClient::HandleReadCompleted()
{
    // start parse msg body(payload) by protobuf
    dbp::Base msgBase;
    msgBase.ParseFromArray(ssRecv.GetData(),ssRecv.GetSize());

    dbp::Msg currentMsgType = msgBase.msg();
    google::protobuf::Any anyObj = msgBase.object();
    switch(currentMsgType)
    {
    case dbp::CONNECT:
        pServer->HandleClientRecv(this,&anyObj);
        break;
    case dbp::SUB:
        pServer->HandleClientRecv(this,&anyObj);
        break;
    case dbp::UNSUB:
        pServer->HandleClientRecv(this,&anyObj);
        break;
    case dbp::METHOD:
        pServer->HandleClientRecv(this,&anyObj);
        break;
    default:
        pServer->HandleClientError(this);
        break;
    }
}

void CDbpClient::HandleWritenResponse(std::size_t nTransferred)
{
    if(nTransferred != 0)
    {
        pServer->HandleClientSent(this);
    }
    else
    {
        pServer->HandleClientError(this);
    }
}

CDbpServer::CDbpServer()
: CIOProc("dbpserver")
{

}

CDbpServer::~CDbpServer()
{

}

CIOClient* CDbpServer::CreateIOClient(CIOContainer *pContainer)
{
    std::map<boost::asio::ip::tcp::endpoint,CDbpProfile>::iterator it;
    it = mapProfile.find(pContainer->GetServiceEndpoint());
    if (it != mapProfile.end() && (*it).second.pSSLContext != NULL)
    {
        return new CSSLClient(pContainer,GetIoService(),*(*it).second.pSSLContext);
    }
    return CIOProc::CreateIOClient(pContainer);
}

void CDbpServer::HandleClientRecv(CDbpClient *pDbpClient,void* anyObj)
{
    CDbpProfile *pDbpProfile = pDbpClient->GetProfile();

    google::protobuf::Any* any = static_cast<google::protobuf::Any*>(anyObj);
    if(!any)
    {
        RespondError(pDbpClient,500);
        return;
    }
    
    if(any->Is<dbp::Connect>())
    {
        
        CWalleveEventDbpConnect *pEventDbpConnect = new CWalleveEventDbpConnect(pDbpClient->GetNonce());
        if(!pEventDbpConnect)
        {
            RespondError(pDbpClient,500);
            return;
        }
        
        dbp::Connect connectMsg;
        any->UnpackTo(&connectMsg);
        
        CWalleveDbpConnect &connectBody = pEventDbpConnect->data;
        connectBody.session = connectMsg.session();
        connectBody.version = connectMsg.version();
        connectBody.client  = connectMsg.client();
        
        pDbpProfile->pIOModule->PostEvent(pEventDbpConnect);
    }
    else if(any->Is<dbp::Sub>())
    {
        CWalleveEventDbpSub *pEventDbpSub = new CWalleveEventDbpSub(pDbpClient->GetNonce());
        if(!pEventDbpSub)
        {
            RespondError(pDbpClient,500);
            return;
        }
        
        dbp::Sub subMsg;
        any->UnpackTo(&subMsg);

        CWalleveDbpSub &subBody = pEventDbpSub->data;
        subBody.id = subMsg.id();
        subBody.name = subMsg.name();
        
        pDbpProfile->pIOModule->PostEvent(pEventDbpSub);
    }
    else if(any->Is<dbp::Unsub>())
    {
        CWalleveEventDbpUnSub *pEventDbpUnSub = new CWalleveEventDbpUnSub(pDbpClient->GetNonce());
        if(!pEventDbpUnSub)
        {
            RespondError(pDbpClient,500);
            return;
        }

        dbp::Unsub unsubMsg;
        any->UnpackTo(&unsubMsg);

        CWalleveDbpUnSub &unsubBody = pEventDbpUnSub->data;
        unsubBody.id = unsubMsg.id();
        
        pDbpProfile->pIOModule->PostEvent(pEventDbpUnSub);
    }
    else if(any->Is<dbp::Method>())
    {
        CWalleveEventDbpMethod *pEventDbpMethod = new CWalleveEventDbpMethod(pDbpClient->GetNonce());
        if(!pEventDbpMethod)
        {
            RespondError(pDbpClient,500);
            return;
        }
        
        dbp::Method methodMsg;
        any->UnpackTo(&methodMsg);

        CWalleveDbpMethod &methodBody = pEventDbpMethod->data;
        methodBody.id = methodMsg.id();
        methodBody.method = methodMsg.method();

        if(methodBody.method == "getblocks" && methodMsg.params().Is<lws::GetBlocksArg>())
        {
            lws::GetBlocksArg args;
            methodMsg.params().UnpackTo(&args);
            methodBody.params.insert(std::make_pair("hash",args.hash())); 
            methodBody.params.insert(std::make_pair("number",boost::lexical_cast<std::string>(args.number())));
        }
        else if(methodBody.method == "gettransaction" && methodMsg.params().Is<lws::GetTxArg>())
        {
            lws::GetTxArg args;
            methodMsg.params().UnpackTo(&args);
            methodBody.params.insert(std::make_pair("hash",args.hash()));
        }
        else if(methodBody.method == "sendtransaction" && methodMsg.params().Is<lws::SendTxArg>())
        {
            lws::SendTxArg args;
            methodMsg.params().UnpackTo(&args);
            methodBody.params.insert(std::make_pair("hash",args.hash()));
        }   
        else
        {
            return;
        }
        
        pDbpProfile->pIOModule->PostEvent(pEventDbpMethod);
    }
    else
    {

    }
}

void CDbpServer::HandleClientSent(CDbpClient *pDbpClient)
{
    RemoveClient(pDbpClient);
}

void CDbpServer::HandleClientError(CDbpClient *pDbpClient)
{
    RemoveClient(pDbpClient);
}

void CDbpServer::AddNewHost(const CDbpHostConfig& confHost)
{
    vecHostConfig.push_back(confHost);
}

bool CDbpServer::WalleveHandleInitialize()
{
    BOOST_FOREACH(const CDbpHostConfig& confHost,vecHostConfig)
    {
        if (!CreateProfile(confHost))
        {
            return false;
        }
    }
    return true;
}

void CDbpServer::WalleveHandleDeinitialize()
{
    for (std::map<boost::asio::ip::tcp::endpoint,CDbpProfile>::iterator it = mapProfile.begin();
         it != mapProfile.end(); ++it)
    {
        delete (*it).second.pSSLContext;
    }
    mapProfile.clear();
}

void CDbpServer::EnterLoop()
{
    WalleveLog("Dbp Server starting:\n");

    for (std::map<boost::asio::ip::tcp::endpoint,CDbpProfile>::iterator it = mapProfile.begin();
         it != mapProfile.end(); ++it)
    {
        if(!StartService((*it).first,(*it).second.nMaxConnections))
        {
             WalleveLog("Setup service %s failed, listen port = %d, connection limit %d\n",
                    (*it).second.pIOModule->WalleveGetOwnKey().c_str(),
                    (*it).first.port(),(*it).second.nMaxConnections);
        }
        else
        {
             WalleveLog("Setup service %s success, listen port = %d, connection limit %d\n",
                    (*it).second.pIOModule->WalleveGetOwnKey().c_str(),
                    (*it).first.port(),(*it).second.nMaxConnections);
        } 
    }

}

void CDbpServer::LeaveLoop()
{
    std::vector<CDbpClient *>vClient;
    for (std::map<uint64,CDbpClient*>::iterator it = mapClient.begin();it != mapClient.end();++it)
    {
        vClient.push_back((*it).second);
    }
    
    BOOST_FOREACH(CDbpClient *pClient,vClient)
    {
        RemoveClient(pClient);
    }
    WalleveLog("Dbp Server stop\n");
}

bool CDbpServer::ClientAccepted(const boost::asio::ip::tcp::endpoint& epService,CIOClient *pClient)
{
    std::map<boost::asio::ip::tcp::endpoint,CDbpProfile>::iterator it = mapProfile.find(epService);
    if (it == mapProfile.end())
    {
        return false;
    }
    return AddNewClient(pClient,&(*it).second) != NULL;
}

bool CDbpServer::CreateProfile(const CDbpHostConfig& confHost)
{
    CDbpProfile profile;
    if (!WalleveGetObject(confHost.strIOModule,profile.pIOModule))
    {
        WalleveLog("Failed to request %s\n",confHost.strIOModule.c_str());
        return false;
    }

    if(confHost.optSSL.fEnable)
    {
        profile.pSSLContext = new boost::asio::ssl::context(boost::asio::ssl::context::sslv23);
        if (!profile.pSSLContext)
        {
            WalleveLog("Failed to alloc ssl context for %s:%u\n",
            confHost.epHost.address().to_string().c_str(),
            confHost.epHost.port());
            return false;
        }
        if (!confHost.optSSL.SetupSSLContext(*profile.pSSLContext))
        {
            WalleveLog("Failed to setup ssl context for %s:%u\n",
            confHost.epHost.address().to_string().c_str(),
            confHost.epHost.port());
            delete profile.pSSLContext;
            return false;
        }
    }

    profile.nMaxConnections = confHost.nMaxConnections;
    profile.vAllowMask = confHost.vAllowMask;
    mapProfile[confHost.epHost] = profile;

    return true;
}

CDbpClient* CDbpServer::AddNewClient(CIOClient *pClient,CDbpProfile *pDbpProfile)
{
    uint64 nNonce = 0;
    RAND_bytes((unsigned char*)&nNonce, sizeof(nNonce));
    while(mapClient.count(nNonce))
    {
        RAND_bytes((unsigned char*)&nNonce, sizeof(nNonce));
    }

    CDbpClient *pDbpClient = new CDbpClient(this,pDbpProfile,pClient,nNonce);
    if(pDbpClient)
    {
        mapClient.insert(std::make_pair(nNonce,pDbpClient));
        pDbpClient->Activate();
    }

    return pDbpClient;
}

void CDbpServer::RemoveClient(CDbpClient *pDbpClient)
{
    mapClient.erase(pDbpClient->GetNonce());
    
    CWalleveEventDbpBroken *pEventBroken = new CWalleveEventDbpBroken(pDbpClient->GetNonce());
    if (pEventBroken != NULL)
    {
        pEventBroken->data.fEventStream = pDbpClient->IsEventStream();
        pDbpClient->GetProfile()->pIOModule->PostEvent(pEventBroken);
    }
    delete pDbpClient;
}

void CDbpServer::RespondError(CDbpClient *pDbpClient,int nStatusCode,const std::string& strError)
{

}

bool CDbpServer::HandleEvent(CWalleveEventDbpConnected& event)
{
    std::map<uint64,CDbpClient*>::iterator it = mapClient.find(event.nNonce);
    if (it == mapClient.end())
    {
        return false;
    }

    CDbpClient *pDbpClient = (*it).second;
    CWalleveDbpConnected &connectedBody = event.data;

    pDbpClient->SendResponse(connectedBody);
}
    
bool CDbpServer::HandleEvent(CWalleveEventDbpFailed& event)
{
    std::map<uint64,CDbpClient*>::iterator it = mapClient.find(event.nNonce);
    if (it == mapClient.end())
    {
        return false;
    }

    CDbpClient *pDbpClient = (*it).second;
    CWalleveDbpFailed &failedBody = event.data;

    pDbpClient->SendResponse(failedBody);
}