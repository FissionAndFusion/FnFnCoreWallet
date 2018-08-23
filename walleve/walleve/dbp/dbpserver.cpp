#include "dbpserver.h"

#include <openssl/rand.h>
#include <boost/foreach.hpp>
#include <boost/algorithm/string/trim.hpp>
#include "dbp.pb.h"

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

void CDbpClient::SendResponse(std::string& strResponse)
{

}

void CDbpClient::StartReadHeader()
{

}

void CDbpClient::StartReadPayload(std::size_t nLength)
{

}

void CDbpClient::HandleReadHeader(std::size_t nTransferred)
{

}

void CDbpClient::HandleReadPayload(std::size_t nTransferred)
{

}

void CDbpClient::HandleReadCompleted()
{

}

void CDbpClient::HandleWritenResponse(std::size_t nTransferred)
{

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

void CDbpServer::HandleClientRecv(CDbpClient *pDbpClient,
                          CWalleveBufStream& ssPayload)
{

}

void CDbpServer::HandleClientSent(CDbpClient *pDbpClient)
{

}

void CDbpServer::HandleClientError(CDbpClient *pDbpClient)
{

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

bool CDbpServer::HandleEvent(CWalleveEventDbpRespond& eventRsp)
{

}