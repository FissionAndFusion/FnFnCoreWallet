#include "dbpserver.h"

#include <memory>
#include <algorithm>
#include <openssl/rand.h>
#include <boost/foreach.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>

#include "dbp.pb.h"
#include "lws.pb.h"
#include "dbputils.hpp"

using namespace walleve;

static const std::size_t MSG_HEADER_LEN = 4;

 CDbpClient::CDbpClient(CDbpServer *pServerIn,CDbpProfile *pProfileIn,
                CIOClient* pClientIn,uint64 nonce)
: pServer(pServerIn), pProfile(pProfileIn), pClient(pClientIn),nNonce(nonce)
  
{
}

CDbpClient::~CDbpClient()
{
    if(pClient)
    { 
        pClient->Close();
    }
}

CDbpProfile *CDbpClient::GetProfile()
{
    return pProfile;
}

uint64 CDbpClient::GetNonce()
{
    return nNonce;
}

std::string CDbpClient::GetSession() const
{
    return session_;
}

void CDbpClient::SetSession(const std::string& session)
{
    session_ = session;
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

void CDbpClient::SendMessage(dbp::Base* pBaseMsg)
{
    ssSend.Clear();
    
    int byteSize = pBaseMsg->ByteSize();
    unsigned char byteBuf[byteSize];

    pBaseMsg->SerializeToArray(byteBuf,byteSize);

    unsigned char msgLenBuf[MSG_HEADER_LEN];
    CDbpUtils::writeLenToMsgHeader(byteSize,(char*)msgLenBuf,MSG_HEADER_LEN);
    ssSend.Write((char*)msgLenBuf,MSG_HEADER_LEN);
    ssSend.Write((char*)byteBuf,byteSize);

    pClient->Write(ssSend,boost::bind(&CDbpClient::HandleWritenResponse,this,_1));
}

void CDbpClient::SendPingNoActiveMessage(dbp::Base* pBaseMsg)
{
    ssPingSend.Clear();
    
    int byteSize = pBaseMsg->ByteSize();
    unsigned char byteBuf[byteSize];

    pBaseMsg->SerializeToArray(byteBuf,byteSize);

    unsigned char msgLenBuf[MSG_HEADER_LEN];
    CDbpUtils::writeLenToMsgHeader(byteSize,(char*)msgLenBuf,MSG_HEADER_LEN);
    ssPingSend.Write((char*)msgLenBuf,MSG_HEADER_LEN);
    ssPingSend.Write((char*)byteBuf,byteSize);

    pClient->Write(ssPingSend,boost::bind(&CDbpClient::HandleWritenResponse,this,_1,0));   
}

void CDbpClient::SendAddedNoActiveMessage(dbp::Base* pBaseMsg)
{
    ssAddedSend.Clear();
    
    int byteSize = pBaseMsg->ByteSize();
    unsigned char byteBuf[byteSize];

    pBaseMsg->SerializeToArray(byteBuf,byteSize);

    unsigned char msgLenBuf[MSG_HEADER_LEN];
    CDbpUtils::writeLenToMsgHeader(byteSize,(char*)msgLenBuf,MSG_HEADER_LEN);
    ssAddedSend.Write((char*)msgLenBuf,MSG_HEADER_LEN);
    ssAddedSend.Write((char*)byteBuf,byteSize);

    pClient->Write(ssAddedSend,boost::bind(&CDbpClient::HandleWritenResponse,this,_1,1));   
}

void CDbpClient::SendResponse(CWalleveDbpConnected& body)
{
  
    dbp::Base connectedMsgBase;
    connectedMsgBase.set_msg(dbp::Msg::CONNECTED);
    
    dbp::Connected connectedMsg;
    connectedMsg.set_session(body.session);

    google::protobuf::Any *any = new google::protobuf::Any();
    any->PackFrom(connectedMsg);

    connectedMsgBase.set_allocated_object(any);

    SendMessage(&connectedMsgBase);
}

void CDbpClient::SendResponse(CWalleveDbpFailed& body)
{
    dbp::Base failedMsgBase;
    failedMsgBase.set_msg(dbp::Msg::FAILED);
    
    dbp::Failed failedMsg;

    for(const int32& version : body.versions)
    {
        failedMsg.add_version(version);
    }

    failedMsg.set_reason(body.reason);
    failedMsg.set_session(body.session);

    google::protobuf::Any *any = new google::protobuf::Any();
    any->PackFrom(failedMsg);

    failedMsgBase.set_allocated_object(any);
 
    SendMessage(&failedMsgBase);
}

void CDbpClient::SendResponse(CWalleveDbpNoSub& body)
{
    dbp::Base noSubMsgBase;
    noSubMsgBase.set_msg(dbp::Msg::NOSUB);
    
    dbp::Nosub noSubMsg;
    noSubMsg.set_id(body.id);
    noSubMsg.set_error(body.error);

    google::protobuf::Any *any = new google::protobuf::Any();
    any->PackFrom(noSubMsg);

    noSubMsgBase.set_allocated_object(any);
 
    SendMessage(&noSubMsgBase);
}

void CDbpClient::SendResponse(CWalleveDbpReady& body)
{
    dbp::Base readyMsgBase;
    readyMsgBase.set_msg(dbp::Msg::READY);
    
    dbp::Ready readyMsg;
    readyMsg.set_id(body.id);

    google::protobuf::Any *any = new google::protobuf::Any();
    any->PackFrom(readyMsg);

    readyMsgBase.set_allocated_object(any);
 
    SendMessage(&readyMsgBase);
}

static void CreateLwsTransaction(const CWalleveDbpTransaction* dbptx,lws::Transaction* tx)
{
    tx->set_nversion(dbptx->nVersion);
    tx->set_ntype(dbptx->nType);
    tx->set_nlockuntil(dbptx->nLockUntil);
    
    std::string hashAnchor(dbptx->hashAnchor.begin(),dbptx->hashAnchor.end());
    tx->set_hashanchor(hashAnchor);

    for(const auto& input : dbptx->vInput)
    {
        std::string inputHash(input.hash.begin(),input.hash.end());
        tx->add_vinput()->set_hash(inputHash);
        tx->add_vinput()->set_n(input.n);
    }

    lws::Transaction::CDestination *dest = new lws::Transaction::CDestination();
    dest->set_prefix(dbptx->cDestination.prefix);
    dest->set_size(dbptx->cDestination.size);

    std::string destData(dbptx->cDestination.data.begin(),dbptx->cDestination.data.end());
    dest->set_data(destData);
    tx->set_allocated_cdestination(dest);

    tx->set_namount(dbptx->nAmount);
    tx->set_ntxfee(dbptx->nTxFee);
    
    std::string mintVchData(dbptx->vchData.begin(),dbptx->vchData.end());
    tx->set_vchdata(mintVchData);

    std::string mintVchSig(dbptx->vchData.begin(),dbptx->vchData.end());
    tx->set_vchsig(mintVchSig);

    std::string hash(dbptx->hash.begin(),dbptx->hash.end());
    tx->set_hash(hash);
}

static void CreateLwsBlock(CWalleveDbpBlock* pBlock,lws::Block& block)
{
    block.set_nversion(pBlock->nVersion);
    block.set_ntype(pBlock->nType);
    block.set_ntimestamp(pBlock->nTimeStamp);
    
    std::string hashPrev(pBlock->hashPrev.begin(),pBlock->hashPrev.end());
    block.set_hashprev(hashPrev);

    std::string hashMerkle(pBlock->hashMerkle.begin(),pBlock->hashMerkle.end());
    block.set_hashmerkle(hashMerkle);

    std::string vchproof(pBlock->vchProof.begin(),pBlock->vchProof.end());
    block.set_vchproof(vchproof);

    std::string vchSig(pBlock->vchSig.begin(),pBlock->vchSig.end());
    block.set_vchsig(vchSig);

    //txMint
    lws::Transaction *txMint = new lws::Transaction();
    CreateLwsTransaction(&(pBlock->txMint),txMint);
    block.set_allocated_txmint(txMint);
    
    //repeated vtx
    for(const auto& tx : pBlock->vtx)
    {
        CreateLwsTransaction(&tx,block.add_vtx());
    }

    block.set_nheight(pBlock->nHeight);
    std::string hash(pBlock->hash.begin(),pBlock->hash.end());
    block.set_hash(hash);
}


void CDbpClient::SendResponse(CWalleveDbpAdded& body)
{ 
    dbp::Base addedMsgBase;
    addedMsgBase.set_msg(dbp::Msg::ADDED);
    
    dbp::Added addedMsg;
    addedMsg.set_id(body.id);
    addedMsg.set_name(body.name);

    if(body.anyAddedObj.type() == typeid(CWalleveDbpBlock))
    {
        CWalleveDbpBlock tempBlock = boost::any_cast<CWalleveDbpBlock>(body.anyAddedObj);
        
        lws::Block block;
        CreateLwsBlock(&tempBlock,block);

        google::protobuf::Any *anyBlock = new google::protobuf::Any();
        anyBlock->PackFrom(block);
        addedMsg.set_allocated_object(anyBlock);

        std::cout << "Added Block: "  <<std::endl;

    }
    else if(body.anyAddedObj.type() == typeid(CWalleveDbpTransaction))
    {
        CWalleveDbpTransaction tempTx = boost::any_cast<CWalleveDbpTransaction>(body.anyAddedObj);

        std::unique_ptr<lws::Transaction> tx(std::make_unique<lws::Transaction>());
        CreateLwsTransaction(&tempTx,tx.get());

        google::protobuf::Any *anyTx = new google::protobuf::Any();
        anyTx->PackFrom(*tx);
        
        addedMsg.set_allocated_object(anyTx);
    }
    else
    {}
    
    google::protobuf::Any *anyAdded = new google::protobuf::Any();
    anyAdded->PackFrom(addedMsg);

    addedMsgBase.set_allocated_object(anyAdded);
 
    std::cout << "Send No ActiveMessage: "  <<std::endl;
    SendAddedNoActiveMessage(&addedMsgBase);
}

void CDbpClient::SendResponse(CWalleveDbpMethodResult& body)
{  
    dbp::Base resultMsgBase;
    resultMsgBase.set_msg(dbp::Msg::RESULT);
    
    dbp::Result resultMsg;
    resultMsg.set_id(body.id);
    resultMsg.set_error(body.error);

    auto dispatchHandler = [&](const boost::any& obj) -> void {
                      
        if(obj.type() == typeid(CWalleveDbpBlock))
        {
            CWalleveDbpBlock tempBlock = boost::any_cast<CWalleveDbpBlock>(obj);
            
            lws::Block block;
            CreateLwsBlock(&tempBlock,block);
            resultMsg.add_result()->PackFrom(block);
        }
        else if(obj.type() == typeid(CWalleveDbpTransaction))
        {
            CWalleveDbpTransaction tempTx = boost::any_cast<CWalleveDbpTransaction>(obj);
            
            std::unique_ptr<lws::Transaction> tx(std::make_unique<lws::Transaction>());
            CreateLwsTransaction(&tempTx,tx.get());
            resultMsg.add_result()->PackFrom(*tx);
        }
        else if(obj.type() == typeid(CWalleveDbpSendTxRet))
        {
            CWalleveDbpSendTxRet txret =  boost::any_cast<CWalleveDbpSendTxRet>(obj); 
            lws::SendTxRet sendTxRet;
           
            sendTxRet.set_hash(txret.hash);
            resultMsg.add_result()->PackFrom(sendTxRet);
        }
        else
        {

        }
    };  

    std::for_each(body.anyResultObjs.begin(),body.anyResultObjs.end(),dispatchHandler);

    google::protobuf::Any *anyResult = new google::protobuf::Any();
    anyResult->PackFrom(resultMsg);

    resultMsgBase.set_allocated_object(anyResult);
 
    SendMessage(&resultMsgBase);
}

void CDbpClient::SendPing(const std::string& id)
{
    dbp::Base pingMsgBase;
    pingMsgBase.set_msg(dbp::Msg::PING);
    
    dbp::Ping msg;
    msg.set_id(id);

    google::protobuf::Any *any = new google::protobuf::Any();
    any->PackFrom(msg);

    pingMsgBase.set_allocated_object(any);
 
    SendMessage(&pingMsgBase);
}

void CDbpClient::SendPong(const std::string& id)
{
    dbp::Base pongMsgBase;
    pongMsgBase.set_msg(dbp::Msg::PONG);
    
    dbp::Pong msg;
    msg.set_id(id);

    google::protobuf::Any *any = new google::protobuf::Any();
    any->PackFrom(msg);

    pongMsgBase.set_allocated_object(any);
 
    SendMessage(&pongMsgBase);
}

void CDbpClient::SendNocActivePing(const std::string& id)
{
    dbp::Base pingMsgBase;
    pingMsgBase.set_msg(dbp::Msg::PING);
    
    dbp::Ping msg;
    msg.set_id(id);

    google::protobuf::Any *any = new google::protobuf::Any();
    any->PackFrom(msg);

    pingMsgBase.set_allocated_object(any);

    SendPingNoActiveMessage(&pingMsgBase);
}

void CDbpClient::SendResponse(int statusCode,const std::string& description)
{
    dbp::Base errorMsgBase;
    errorMsgBase.set_msg(dbp::Msg::ERROR);
    
    dbp::Error errorMsg;
    errorMsg.set_reason(std::to_string(statusCode));
    errorMsg.set_explain(description);

    google::protobuf::Any *any = new google::protobuf::Any();
    any->PackFrom(errorMsg);

    errorMsgBase.set_allocated_object(any);
 
    SendMessage(&errorMsgBase);
}

void CDbpClient::StartReadHeader()
{
    pClient->Read(ssRecv,MSG_HEADER_LEN,
    boost::bind(&CDbpClient::HandleReadHeader,this, _1));
}

void CDbpClient::StartReadPayload(std::size_t nLength)
{
    pClient->Read(ssRecv,nLength,
        boost::bind(&CDbpClient::HandleReadPayload,this,_1,nLength));
}


void CDbpClient::HandleReadHeader(std::size_t nTransferred)
{ 
    std::cout << "[ReadHeaderHandler] transferred: " << nTransferred 
              << " StreamBuffer: " << ssRecv.GetSize()
              << std::endl ;

    if(nTransferred == MSG_HEADER_LEN)
    {
        
        std::string lenBuffer(MSG_HEADER_LEN,0);
        ssRecv.Read(&lenBuffer[0],MSG_HEADER_LEN);
        
        uint32_t nMsgHeaderLen = CDbpUtils::parseLenFromMsgHeader(&lenBuffer[0],MSG_HEADER_LEN);
        if(nMsgHeaderLen == 0)
        {
            std::cout << "Msg Base header length is 0" << std::endl;
            pServer->HandleClientError(this);
            return;
        }

        StartReadPayload(nMsgHeaderLen);
    }
    else
    {
        pServer->HandleClientError(this);
    }
}

void CDbpClient::HandleReadPayload(std::size_t nTransferred,uint32_t len)
{
    
    std::cout << "[ReadPayloadHandler] transferred: " << nTransferred 
                << " MessageLen: " << len
                << " StreamBuffer: " << ssRecv.GetSize()
               << std::endl ;
    
    if(nTransferred == len)
    {
        HandleReadCompleted(len);
    }
    else
    {
        pServer->HandleClientError(this);
    }
}

void CDbpClient::HandleReadCompleted(uint32_t len)
{
    // start parse msg body(payload) by protobuf
    dbp::Base msgBase;

    std::string payloadBuffer(len,0);
    ssRecv.Read(&payloadBuffer[0],len);
    
    if(!msgBase.ParseFromArray(&payloadBuffer[0],len))
    {        
        pServer->RespondError(this,400,"Parse Msg Base failed");
        pServer->HandleClientError(this);
        return;
    }

    dbp::Msg currentMsgType = msgBase.msg();
    google::protobuf::Any anyObj = msgBase.object();
    switch(currentMsgType)
    {
    case dbp::CONNECT:
        std::cout << "connect =========" << std::endl;
        pServer->HandleClientRecv(this,anyObj);
        break;
    case dbp::SUB:
        pServer->HandleClientRecv(this,anyObj);
        break;
    case dbp::UNSUB:
        pServer->HandleClientRecv(this,anyObj);
        break;
    case dbp::METHOD:
        pServer->HandleClientRecv(this,anyObj);
        break;
    case dbp::PONG:
        pServer->HandleClientRecv(this,anyObj);
        break;
    case dbp::PING:
        pServer->HandleClientRecv(this,anyObj);
        break;
    default:
        pServer->RespondError(this,400,"is not Message Base Type is unknown.");
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

void CDbpClient::HandleWritenResponse(std::size_t nTransferred, int type)
{
    if(nTransferred != 0)
    {
        if(type == 0)
        {
            std::cout << "ping transferred: " << nTransferred << std::endl;
        }
        else if(type == 1)
        {
            std::cout << "added transferred: " << nTransferred << std::endl;
        }
    }
    else
    {
        pServer->HandleClientError(this);
    }
}

CDbpServer::CDbpServer()
: CIOProc("dbpserver"),
pingTimerPtr_(std::make_shared<boost::asio::deadline_timer>(this->GetIoService(),
        boost::posix_time::seconds(5)))
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

void CDbpServer::HandleClientConnect(CDbpClient *pDbpClient,google::protobuf::Any* any)
{
    
    dbp::Connect connectMsg;
    any->UnpackTo(&connectMsg);

    std::string session = connectMsg.session();
    if(!IsSessionReconnect(session))
    { 
        session = GenerateSessionId();
        CreateSession(session,pDbpClient);

        CWalleveEventDbpConnect *pEventDbpConnect = new CWalleveEventDbpConnect(session);
        if(!pEventDbpConnect)
        {
            RespondError(pDbpClient,500,"dbp connect event create failed.");
            return;
        }


        CWalleveDbpConnect &connectBody = pEventDbpConnect->data;
        connectBody.isReconnect = false;
        connectBody.session = session;
        connectBody.version = connectMsg.version();
        connectBody.client  = connectMsg.client();
        
        pDbpClient->GetProfile()->pIOModule->PostEvent(pEventDbpConnect);
    }
    else
    {  
        if(IsSessionExist(session))
        { 
            UpdateSession(session,pDbpClient);

            CWalleveEventDbpConnect *pEventDbpConnect = new CWalleveEventDbpConnect(session);
            if(!pEventDbpConnect)
            {
                RespondError(pDbpClient,500,"dbp connect event create failed.");
                return;
            }


            CWalleveDbpConnect &connectBody = pEventDbpConnect->data;
            connectBody.isReconnect = true;
            connectBody.session = session;
            connectBody.version = connectMsg.version();
            connectBody.client  = connectMsg.client();
        
            pDbpClient->GetProfile()->pIOModule->PostEvent(pEventDbpConnect);
        }
        else
        {
            // if reconnect cannot find session,send failed msg
            CWalleveEventDbpFailed failedEvent(pDbpClient->GetNonce());
            failedEvent.data.session = session;
            failedEvent.data.reason =  "002";

            this->HandleEvent(failedEvent);
        }
        
    }     
}

void CDbpServer::HandleClientSub(CDbpClient *pDbpClient,google::protobuf::Any* any)
{
    if(IsSessionTimeOut(pDbpClient)) 
    {   
        RespondFailed(pDbpClient);
        return;
    }
        
    CWalleveEventDbpSub *pEventDbpSub = new CWalleveEventDbpSub(pDbpClient->GetSession());
    if(!pEventDbpSub)
    {
        RespondError(pDbpClient,500,"dbp sub event create failed");
        return;
    }
    
    dbp::Sub subMsg;
    any->UnpackTo(&subMsg);

    CWalleveDbpSub &subBody = pEventDbpSub->data;
    subBody.id = subMsg.id();
    subBody.name = subMsg.name();
    
    pDbpClient->GetProfile()->pIOModule->PostEvent(pEventDbpSub);
}

void CDbpServer::HandleClientUnSub(CDbpClient *pDbpClient,google::protobuf::Any* any)
{
    CWalleveEventDbpUnSub *pEventDbpUnSub = new CWalleveEventDbpUnSub(pDbpClient->GetSession());
    if(!pEventDbpUnSub)
    {
        RespondError(pDbpClient,500,"dbp unsub event create failed.");
        return;
    }

    dbp::Unsub unsubMsg;
    any->UnpackTo(&unsubMsg);

    CWalleveDbpUnSub &unsubBody = pEventDbpUnSub->data;
    unsubBody.id = unsubMsg.id();
    
    pDbpClient->GetProfile()->pIOModule->PostEvent(pEventDbpUnSub);
}

void CDbpServer::HandleClientMethod(CDbpClient *pDbpClient,google::protobuf::Any* any)
{
    CWalleveEventDbpMethod *pEventDbpMethod = new CWalleveEventDbpMethod(pDbpClient->GetSession());
    if(!pEventDbpMethod)
    {
        RespondError(pDbpClient,500,"dbp event method create failed.");
        return;
    }
    
    dbp::Method methodMsg;
    any->UnpackTo(&methodMsg);

    CWalleveDbpMethod &methodBody = pEventDbpMethod->data;
    methodBody.id = methodMsg.id();

    std::cout << "current id:" << methodBody.id << "current method name: " << methodMsg.method() 
        << std::endl;
    
    if(methodMsg.method() == "getblocks") methodBody.method = CWalleveDbpMethod::Method::GET_BLOCKS;
    if(methodMsg.method() == "gettransaction") methodBody.method = CWalleveDbpMethod::Method::GET_TX;
    if(methodMsg.method() == "sendtransaction") methodBody.method = CWalleveDbpMethod::Method::SEND_TX;

    if(methodBody.method == CWalleveDbpMethod::Method::GET_BLOCKS
        && methodMsg.params().Is<lws::GetBlocksArg>())
    {
        lws::GetBlocksArg args;
        methodMsg.params().UnpackTo(&args);
        
        if(args.hash().empty() || args.number() == 0)
        {
            delete pEventDbpMethod;
            RespondError(pDbpClient,400,"dbp method [getblocks] args hash is empty or number is 0.");
            return;
        }

        std::cout << "id: " << methodBody.id << "hash: " << 
            args.hash() << std::endl << "number: " << args.number() << std::endl;
        
        methodBody.params.insert(std::make_pair("hash",args.hash())); 
        methodBody.params.insert(std::make_pair("number",boost::lexical_cast<std::string>(args.number())));
    }
    else if(methodBody.method == CWalleveDbpMethod::Method::GET_TX 
        && methodMsg.params().Is<lws::GetTxArg>())
    {
        lws::GetTxArg args;
        methodMsg.params().UnpackTo(&args);
        
        if(args.hash().empty())
        {
            delete pEventDbpMethod;
            RespondError(pDbpClient,400,"dbp method [gettransaction] args hash is empty.");
            return;
        }
        
        methodBody.params.insert(std::make_pair("hash",args.hash()));
    }
    else if(methodBody.method == CWalleveDbpMethod::Method::SEND_TX 
        && methodMsg.params().Is<lws::SendTxArg>())
    {
        lws::SendTxArg args;
        methodMsg.params().UnpackTo(&args);
        
        if(args.hash().empty())
        {
            delete pEventDbpMethod;
            RespondError(pDbpClient,400,"dbp method [sendtransaction] args hash is empty.");
            return;
        }
        
        methodBody.params.insert(std::make_pair("hash",args.hash()));
    }   
    else
    {
        delete pEventDbpMethod;
        RespondError(pDbpClient,400,"dbp method name is empty,please specify a method name.");
        return;
    }
    
    pDbpClient->GetProfile()->pIOModule->PostEvent(pEventDbpMethod);
}

void CDbpServer::HandleClientPing(CDbpClient *pDbpClient,google::protobuf::Any* any)
{
    dbp::Ping pingMsg;
    any->UnpackTo(&pingMsg);

    std::cout << "recv ping" << pingMsg.id() << std::endl;
    pDbpClient->SendPong(pingMsg.id());
}

void CDbpServer::HandleClientPong(CDbpClient *pDbpClient,google::protobuf::Any* any)
{
    dbp::Pong pongMsg;
    any->UnpackTo(&pongMsg);

    std::cout << "recv pong" << pongMsg.id() << std::endl;

    if(HaveAssociatedSessionOf(pDbpClient))
    {
        std::string session = sessionClientBimap.right.at(pDbpClient);
        
        if(IsSessionExist(session))
        {
            sessionProfileMap[session].timestamp = CDbpUtils::currentUTC();
        }
    }

    this->HandleClientSent(pDbpClient);
}

void CDbpServer::HandleClientRecv(CDbpClient *pDbpClient, const boost::any& anyObj)
{
    if(anyObj.type() != typeid(google::protobuf::Any))
    {
        RespondError(pDbpClient,500,"protobuf msg base any object pointer is null.");
        return;
    }
    
    google::protobuf::Any any = boost::any_cast<google::protobuf::Any>(anyObj);

    if(any.Is<dbp::Connect>())
    {
        HandleClientConnect(pDbpClient,&any);
    }
    else if(any.Is<dbp::Sub>())
    {
        if(!HaveAssociatedSessionOf(pDbpClient))
        {
            RespondError(pDbpClient,400,"dbp sub message not had assiociated session,please connect first");
            return;
        }

        HandleClientSub(pDbpClient,&any);
    }
    else if(any.Is<dbp::Unsub>())
    {
        if(!HaveAssociatedSessionOf(pDbpClient))
        {
            RespondError(pDbpClient,400,"dbp unsub message not had assiociated session,please connect first");
            return;
        }

        if(IsSessionTimeOut(pDbpClient)) 
        {
            RespondFailed(pDbpClient);
            return;
        }
        
        HandleClientUnSub(pDbpClient,&any);
    }
    else if(any.Is<dbp::Method>())
    {
        if(!HaveAssociatedSessionOf(pDbpClient))
        {
            RespondError(pDbpClient,400,"dbp method message not had assiociated session,please connect first");
            return;
        }

        if(IsSessionTimeOut(pDbpClient)) 
        {
            RespondFailed(pDbpClient);
            return;
        }
              
        HandleClientMethod(pDbpClient,&any);
    }
    else if(any.Is<dbp::Ping>())
    {
        if(!HaveAssociatedSessionOf(pDbpClient))
        {
            RespondError(pDbpClient,400,"dbp ping message not had assiociated session,please connect first");
            return;
        }

        HandleClientPing(pDbpClient,&any);
    }
    else if(any.Is<dbp::Pong>())
    {
        if(!HaveAssociatedSessionOf(pDbpClient))
        {
            RespondError(pDbpClient,400,"dbp pong message not had assiociated session,please connect first");
            return;
        }

        HandleClientPong(pDbpClient,&any);     
    } 
    else
    {
        RespondError(pDbpClient,500,"unknown dbp message");
        return;
    }
}

void CDbpServer::HandleClientSent(CDbpClient *pDbpClient)
{
    // keep-alive connection,do not remove
    pDbpClient->Activate();
}

void CDbpServer::HandleClientError(CDbpClient *pDbpClient)
{
    WalleveLog("Client error\n");
    pingTimerPtr_->cancel();
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
    std::vector<CDbpClient*> vClient;
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
    profile.nSessionTimeout = confHost.nSessionTimeout;
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

    auto iter = sessionClientBimap.right.find(pDbpClient);
    if(iter != sessionClientBimap.right.end())
    {
        std::string assciatedSession = iter->second;
        sessionClientBimap.left.erase(assciatedSession);
        sessionClientBimap.right.erase(pDbpClient);
        sessionProfileMap.erase(assciatedSession);
    }
    
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
    pDbpClient->SendResponse(nStatusCode,strError);
}

void CDbpServer::RespondFailed(CDbpClient* pDbpClient)
{
    CWalleveEventDbpFailed failedEvent(pDbpClient->GetNonce());
    failedEvent.data.session = sessionClientBimap.right.at(pDbpClient);
    failedEvent.data.reason =  "400";

    this->HandleEvent(failedEvent);
}

void CDbpServer::SendPingHandler(const boost::system::error_code& err,CDbpClient *pDbpClient)
{ 
    if(err != boost::system::errc::success)
    {
        return;
    }
    
    std::string utc = std::to_string(CDbpUtils::currentUTC());
    pDbpClient->SendNocActivePing(utc);
    
    pingTimerPtr_->expires_at(pingTimerPtr_->expires_at() + boost::posix_time::seconds(5));
    pingTimerPtr_->async_wait(boost::bind(&CDbpServer::SendPingHandler, 
        this, boost::asio::placeholders::error, pDbpClient));
    
} 

bool CDbpServer::HandleEvent(CWalleveEventDbpConnected& event)
{
    auto it = sessionProfileMap.find(event.session_);
    if(it == sessionProfileMap.end())
    {
        std::cout << "cannot find session [Connected] " << event.session_ << std::endl;
        return false;
    }

    CDbpClient *pDbpClient = (*it).second.pDbpClient;
    CWalleveDbpConnected &connectedBody = event.data;

    pDbpClient->SendResponse(connectedBody);

    pingTimerPtr_->expires_at(pingTimerPtr_->expires_at() + boost::posix_time::seconds(5));    
    pingTimerPtr_->async_wait(boost::bind(&CDbpServer::SendPingHandler, 
        this, boost::asio::placeholders::error, pDbpClient));
    
    return true;
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
    
    RemoveClient(pDbpClient);
    
    return true;
}

bool CDbpServer::HandleEvent(CWalleveEventDbpNoSub& event)
{
    auto it = sessionProfileMap.find(event.session_);
    if(it == sessionProfileMap.end())
    {
        std::cout << "cannot find session [NoSub] " << event.session_ << std::endl;
        return false;
    }

    CDbpClient *pDbpClient = (*it).second.pDbpClient;
    CWalleveDbpNoSub &noSubBody = event.data;

    pDbpClient->SendResponse(noSubBody);
    
    return true;
}

bool CDbpServer::HandleEvent(CWalleveEventDbpReady& event)
{
    auto it = sessionProfileMap.find(event.session_);
    if(it == sessionProfileMap.end())
    {
        std::cout << "cannot find session [Ready] " << event.session_ << std::endl;
        return false;
    }

    CDbpClient *pDbpClient = (*it).second.pDbpClient;
    CWalleveDbpReady &readyBody = event.data;

    pDbpClient->SendResponse(readyBody);
    
    return true;
}

bool CDbpServer::HandleEvent(CWalleveEventDbpAdded& event)
{
    auto it = sessionProfileMap.find(event.session_);
    if(it == sessionProfileMap.end())
    {
        std::cout << "cannot find session [Added] " << event.session_ << std::endl;
        return false;
    }


    CDbpClient *pDbpClient = (*it).second.pDbpClient;
    CWalleveDbpAdded &addedBody = event.data;

    std::cout << "Added Send Response: "  <<std::endl;
    pDbpClient->SendResponse(addedBody);
    
    return true;
}

bool CDbpServer::HandleEvent(CWalleveEventDbpMethodResult& event)
{
    auto it = sessionProfileMap.find(event.session_);
    if(it == sessionProfileMap.end())
    {
        std::cout << "cannot find session [Method Result] " << event.session_ << std::endl;
        return false;
    }

    CDbpClient *pDbpClient = (*it).second.pDbpClient;
    CWalleveDbpMethodResult &resultBody = event.data;

    pDbpClient->SendResponse(resultBody);
    
    return true;
}

bool CDbpServer::IsSessionTimeOut(CDbpClient* pDbpClient)
{
    auto timeout = pDbpClient->GetProfile()->nSessionTimeout;
    auto iter = sessionClientBimap.right.find(pDbpClient);
    if(iter != sessionClientBimap.right.end())
    {
        std::string assciatedSession = iter->second;
        uint64 lastTimeStamp = sessionProfileMap[assciatedSession].timestamp;
        return (CDbpUtils::currentUTC() - lastTimeStamp > timeout) ? true : false;
    }
    else
    {
        return true;
    }
}

bool CDbpServer::IsSessionReconnect(const std::string & session)
{
    return !session.empty();
}

bool CDbpServer::IsSessionExist(const std::string& session)
{
    return sessionProfileMap.find(session) != sessionProfileMap.end();
}

bool CDbpServer::HaveAssociatedSessionOf(CDbpClient* pDbpClient)
{
    return sessionClientBimap.right.find(pDbpClient) != sessionClientBimap.right.end();
}

std::string CDbpServer::GenerateSessionId()
{
    std::string session = CDbpUtils::RandomString();    
    while(IsSessionExist(session))
    {
        session = CDbpUtils::RandomString();
    }
    
    return session; 
}

void CDbpServer::CreateSession(const std::string& session,CDbpClient* pDbpClient)
{
    CSessionProfile profile;
    profile.sessionId = session;
    profile.pDbpClient = pDbpClient;
    profile.timestamp = CDbpUtils::currentUTC();

    pDbpClient->SetSession(session);
          
    sessionProfileMap.insert(std::make_pair(session,profile));
    sessionClientBimap.insert(position_pair(session,pDbpClient));
}

void CDbpServer::UpdateSession(const std::string& session,CDbpClient* pDbpClient)
{
    if(sessionClientBimap.left.find(session) != sessionClientBimap.left.end())
    {          
        auto pDbplient = sessionClientBimap.left.at(session);
        sessionClientBimap.left.erase(session);
        sessionClientBimap.right.erase(pDbpClient);
    }
                
    sessionProfileMap[session].pDbpClient = pDbpClient;
    sessionProfileMap[session].timestamp = CDbpUtils::currentUTC();                
    sessionClientBimap.insert(position_pair(session,pDbpClient));
}