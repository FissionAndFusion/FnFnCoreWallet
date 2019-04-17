// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "dbpserver.h"

#include <memory>
#include <algorithm>
#include <chrono>
#include <openssl/rand.h>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>

using namespace multiverse;
using namespace walleve;

static const std::size_t MSG_HEADER_LEN = 4;

CDbpServerSocket::CDbpServerSocket(CDbpServer* pServerIn, CDbpProfile* pProfileIn,
                       CIOClient* pClientIn, uint64 nonce)
    : pServer(pServerIn), pProfile(pProfileIn), pClient(pClientIn), nNonce(nonce)
    , IsReading(false)

{
    ssRecv.Clear();
}

CDbpServerSocket::~CDbpServerSocket()
{
    if (pClient)
    {
        pClient->Close();
    }
}

CDbpProfile* CDbpServerSocket::GetProfile()
{
    return pProfile;
}

uint64 CDbpServerSocket::GetNonce()
{
    return nNonce;
}

std::string CDbpServerSocket::GetSession() const
{
    return strSessionId;
}

void CDbpServerSocket::SetSession(const std::string& session)
{
    strSessionId = session;
}

void CDbpServerSocket::Activate()
{
   // ssRecv.Clear();
    IsReading = true;
    StartReadHeader();
}

void CDbpServerSocket::SendMessage(dbp::Msg type, google::protobuf::Any* any)
{
    dbp::Base base;
    base.set_msg(type);
    base.set_allocated_object(any);
    std::string bytes;
    base.SerializeToString(&bytes);

    uint32_t len;
    char len_buffer[4];
    len = bytes.size();
    len = htonl(len);
    std::memcpy(&len_buffer[0], &len, 4);
    bytes.insert(0, len_buffer, 4);

    if(!IsSentComplete())
    {
        queueMessage.push(std::make_pair(type,bytes));
        return;
    }
    
    ssSend.Write((char*)bytes.data(),bytes.size());
    pClient->Write(ssSend, boost::bind(&CDbpServerSocket::HandleWritenResponse, this, _1, type));
}

void CDbpServerSocket::SendResponse(CMvDbpConnected& body)
{
    dbp::Connected connectedMsg;
    connectedMsg.set_session(body.session);

    google::protobuf::Any* any = new google::protobuf::Any();
    any->PackFrom(connectedMsg);

    SendMessage(dbp::Msg::CONNECTED,any);
}

void CDbpServerSocket::SendResponse(CMvDbpFailed& body)
{
    dbp::Failed failedMsg;
    for (const int32& version : body.versions)
    {
        failedMsg.add_version(version);
    }

    failedMsg.set_reason(body.reason);
    failedMsg.set_session(body.session);

    google::protobuf::Any* any = new google::protobuf::Any();
    any->PackFrom(failedMsg);

    SendMessage(dbp::Msg::FAILED,any);
}

void CDbpServerSocket::SendResponse(CMvDbpNoSub& body)
{
    dbp::Nosub noSubMsg;
    noSubMsg.set_id(body.id);
    noSubMsg.set_error(body.error);

    google::protobuf::Any* any = new google::protobuf::Any();
    any->PackFrom(noSubMsg);

    SendMessage(dbp::Msg::NOSUB,any);
}

void CDbpServerSocket::SendResponse(CMvDbpReady& body)
{
    dbp::Ready readyMsg;
    readyMsg.set_id(body.id);

    google::protobuf::Any* any = new google::protobuf::Any();
    any->PackFrom(readyMsg);

    SendMessage(dbp::Msg::READY,any);
}

void CDbpServerSocket::SendResponse(const std::string& client, CMvDbpAdded& body)
{
    dbp::Added addedMsg;
    addedMsg.set_id(body.id);
    addedMsg.set_name(body.name);

    if (body.anyAddedObj.type() == typeid(CMvDbpBlock))
    {
        CMvDbpBlock tempBlock = boost::any_cast<CMvDbpBlock>(body.anyAddedObj);

        if(client != "supernode")
        {
            lws::Block block;
            CDbpUtils::DbpToLwsBlock(&tempBlock, block);
            google::protobuf::Any* anyBlock = new google::protobuf::Any();
            anyBlock->PackFrom(block);
            addedMsg.set_allocated_object(anyBlock);
        }
    }
    else if (body.anyAddedObj.type() == typeid(CMvDbpTransaction))
    {
        CMvDbpTransaction tempTx = boost::any_cast<CMvDbpTransaction>(body.anyAddedObj);

        if(client != "supernode")
        {
            std::unique_ptr<lws::Transaction> tx(new lws::Transaction());
            CDbpUtils::DbpToLwsTransaction(&tempTx, tx.get());
            google::protobuf::Any* anyTx = new google::protobuf::Any();
            anyTx->PackFrom(*tx);
            addedMsg.set_allocated_object(anyTx);
        }
    }
    else if(body.anyAddedObj.type() == typeid(CMvDbpVirtualPeerNetEvent))
    {
        CMvDbpVirtualPeerNetEvent tempEvent = boost::any_cast<CMvDbpVirtualPeerNetEvent>(body.anyAddedObj);
        if(client == "supernode")
        {
            std::unique_ptr<sn::VPeerNetEvent> event(new sn::VPeerNetEvent());
            event.get()->set_type(tempEvent.type); 
            event.get()->set_data(std::string(tempEvent.data.begin(), tempEvent.data.end()));
            google::protobuf::Any* anyEvent = new google::protobuf::Any();
            anyEvent->PackFrom(*event);
            addedMsg.set_allocated_object(anyEvent);
        }
    }
    else
    {
        std::cerr << "Unknown added type." << std::endl;
        return;
    }

    google::protobuf::Any* anyAdded = new google::protobuf::Any();
    anyAdded->PackFrom(addedMsg);

    SendMessage(dbp::Msg::ADDED,anyAdded);
}

void CDbpServerSocket::SendResponse(const std::string& client, CMvDbpMethodResult& body)
{
    dbp::Result resultMsg;
    resultMsg.set_id(body.id);
    resultMsg.set_error(body.error);

    auto dispatchHandler = [&](const boost::any& obj) -> void {
        if (obj.type() == typeid(CMvDbpBlock))
        {
            CMvDbpBlock tempBlock = boost::any_cast<CMvDbpBlock>(obj);
            if(client == "supernode")
            {

            }
            else
            {
                lws::Block block;
                CDbpUtils::DbpToLwsBlock(&tempBlock, block);
                resultMsg.add_result()->PackFrom(block);
            }
        }
        else if (obj.type() == typeid(CMvDbpTransaction))
        {
            CMvDbpTransaction tempTx = boost::any_cast<CMvDbpTransaction>(obj);
            if(client == "supernode")
            {

            }
            else
            {
                std::unique_ptr<lws::Transaction> tx(new lws::Transaction());
                CDbpUtils::DbpToLwsTransaction(&tempTx, tx.get());
                resultMsg.add_result()->PackFrom(*tx);
            }    
        }
        else if (obj.type() == typeid(CMvDbpSendTransactionRet))
        {
            CMvDbpSendTransactionRet txret = boost::any_cast<CMvDbpSendTransactionRet>(obj);
            
            lws::SendTxRet sendTxRet;
            sendTxRet.set_hash(txret.hash);
            sendTxRet.set_result(txret.result);
            sendTxRet.set_reason(txret.reason);
            resultMsg.add_result()->PackFrom(sendTxRet);
          
        }
        else
        {
        }
    };

    std::for_each(body.anyResultObjs.begin(), body.anyResultObjs.end(), dispatchHandler);

    google::protobuf::Any* anyResult = new google::protobuf::Any();
    anyResult->PackFrom(resultMsg);

    SendMessage(dbp::Msg::RESULT,anyResult);
}

void CDbpServerSocket::SendResponse(const std::string& client, CMvRPCRouteAdded& body)
{
    dbp::Added addedMsg;
    addedMsg.set_id(body.id);
    addedMsg.set_name(body.name);

    sn::RPCRouteEvent routeEvent;
    routeEvent.set_type(body.type);
    routeEvent.set_data(std::string(body.vData.begin(), body.vData.end()));

    google::protobuf::Any* anyEvent = new google::protobuf::Any();
    anyEvent->PackFrom(routeEvent);
    addedMsg.set_allocated_object(anyEvent);

    google::protobuf::Any* anyAdded = new google::protobuf::Any();
    anyAdded->PackFrom(addedMsg);

    SendMessage(dbp::Msg::ADDED, anyAdded);
}

void CDbpServerSocket::SendPong(const std::string& id)
{
    dbp::Pong msg;
    msg.set_id(id);

    google::protobuf::Any *any = new google::protobuf::Any();
    any->PackFrom(msg);

    SendMessage(dbp::Msg::PONG,any);
}

void CDbpServerSocket::SendPing(const std::string& id)
{
    dbp::Ping msg;
    msg.set_id(id);

    google::protobuf::Any* any = new google::protobuf::Any();
    any->PackFrom(msg);

    SendMessage(dbp::Msg::PING,any);
}

void CDbpServerSocket::SendResponse(const std::string& reason, const std::string& description)
{
    dbp::Error errorMsg;
    errorMsg.set_reason(reason);
    errorMsg.set_explain(description);

    google::protobuf::Any* any = new google::protobuf::Any();
    any->PackFrom(errorMsg);

    SendMessage(dbp::Msg::ERROR,any);
}

void CDbpServerSocket::StartReadHeader()
{
    pClient->Read(ssRecv, MSG_HEADER_LEN,
                  boost::bind(&CDbpServerSocket::HandleReadHeader, this, _1));
}

void CDbpServerSocket::StartReadPayload(std::size_t nLength)
{
    pClient->Read(ssRecv, nLength,
                  boost::bind(&CDbpServerSocket::HandleReadPayload, this, _1, nLength));
}

bool CDbpServerSocket::IsSentComplete()
{
    return (ssSend.GetSize() == 0 && queueMessage.empty());
}

void CDbpServerSocket::HandleReadHeader(std::size_t nTransferred)
{
    if(nTransferred == 0)
    {
        pServer->HandleClientError(this);
        return;
    }
    
    if (nTransferred == MSG_HEADER_LEN)
    {
        std::string lenBuffer(ssRecv.GetData(), ssRecv.GetData() + MSG_HEADER_LEN);
        uint32_t nMsgHeaderLen = CDbpUtils::ParseLenFromMsgHeader(&lenBuffer[0], MSG_HEADER_LEN);
        if (nMsgHeaderLen == 0)
        {
            std::cerr << "Msg Base header length is 0" << std::endl;
            pServer->HandleClientError(this);
            return;
        }

        StartReadPayload(nMsgHeaderLen);
    }
    else
    {
        std::cerr << "Msg Base header length is not 4 " << std::endl;
        pServer->HandleClientError(this);
    }
}

void CDbpServerSocket::HandleReadPayload(std::size_t nTransferred, uint32_t len)
{
    if(nTransferred == 0)
    {
        pServer->HandleClientError(this);
        return;
    }
    
    if (nTransferred == len)
    {
        HandleReadCompleted(len);
    }
    else
    {
        std::cerr << "pay load is not len. " << std::endl;
        pServer->HandleClientError(this);
    }
}

void CDbpServerSocket::HandleReadCompleted(uint32_t len)
{
    char head[4];
    ssRecv.Read(head,4);
    std::string payloadBuffer(len, 0);
    ssRecv.Read(&payloadBuffer[0], len);
    IsReading = false;
    dbp::Base msgBase;
    if (!msgBase.ParseFromString(payloadBuffer))
    {
        std::cerr << "#######parse payload failed. " << std::endl;
        pServer->RespondError(this, "002", "server recv invalid protobuf object");
        pServer->HandleClientError(this);
        return;
    }

    dbp::Msg currentMsgType = msgBase.msg();
    google::protobuf::Any anyObj = msgBase.object();
    switch (currentMsgType)
    {
    case dbp::CONNECT:
        pServer->HandleClientRecv(this, anyObj);
        break;
    case dbp::SUB:
        pServer->HandleClientRecv(this, anyObj);
        break;
    case dbp::UNSUB:
        pServer->HandleClientRecv(this, anyObj);
        break;
    case dbp::METHOD:
        pServer->HandleClientRecv(this, anyObj);
        break;
    case dbp::PONG:
        pServer->HandleClientRecv(this, anyObj);
        pServer->HandleClientSent(this);
        break;
    case dbp::PING:
        pServer->HandleClientRecv(this, anyObj);
        break;
    default:
        std::cerr << "is not Message Base Type is unknown." << std::endl;
        pServer->RespondError(this, "003", "is not Message Base Type is unknown.");
        pServer->HandleClientError(this);
        break;
    }
}

void CDbpServerSocket::HandleWritenResponse(std::size_t nTransferred, dbp::Msg type)
{
    if (nTransferred != 0)
    {
        if (ssSend.GetSize() != 0)
        {
            pClient->Write(ssSend, boost::bind(&CDbpServerSocket::HandleWritenResponse,
                                               this, _1, type));
            return;
        }

        if (ssSend.GetSize() == 0 && !queueMessage.empty())
        {
            
            auto messagePair = queueMessage.front();
            dbp::Msg messageType = messagePair.first;
            std::string bytes = messagePair.second;
            queueMessage.pop();
            ssSend.Write((char*)bytes.data(),bytes.size());
            pClient->Write(ssSend, boost::bind(&CDbpServerSocket::HandleWritenResponse, this, _1, messageType));
            return;
        }

        if(!IsReading)
        {
            pServer->HandleClientSent(this);
        }
       
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

CDbpServer::~CDbpServer() noexcept
{
}

CIOClient* CDbpServer::CreateIOClient(CIOContainer* pContainer)
{
    std::map<boost::asio::ip::tcp::endpoint, CDbpProfile>::iterator it;
    it = mapProfile.find(pContainer->GetServiceEndpoint());
    if (it != mapProfile.end() && (*it).second.pSSLContext != NULL)
    {
        return new CSSLClient(pContainer, GetIoService(), *(*it).second.pSSLContext);
    }
    return CIOProc::CreateIOClient(pContainer);
}

void CDbpServer::HandleClientConnect(CDbpServerSocket* pDbpClient, google::protobuf::Any* any)
{

    dbp::Connect connectMsg;
    any->UnpackTo(&connectMsg);

    std::string session = connectMsg.session();

    if (!IsSessionReconnect(session))
    {
        session = GenerateSessionId();
        std::string forkid = GetUdata(&connectMsg, "forkid");
        std::string client = connectMsg.client();
        CreateSession(session, client, forkid, pDbpClient);

        std::cout << "[#] Generate Session " << session << " For Connect Msg [dbpserver] " << std::endl;
        
        CMvEventDbpConnect *pEventDbpConnect = new CMvEventDbpConnect(session);
        if (!pEventDbpConnect)
        {
            return;
        }

        CMvDbpConnect& connectBody = pEventDbpConnect->data;
        connectBody.isReconnect = false;
        connectBody.session = session;
        connectBody.version = connectMsg.version();
        connectBody.client = connectMsg.client();

        pDbpClient->GetProfile()->pIOModule->PostEvent(pEventDbpConnect);
    }
    else
    {
        if (IsSessionExist(session))
        {
            UpdateSession(session, pDbpClient);

            CMvEventDbpConnect *pEventDbpConnect = new CMvEventDbpConnect(session);
            if (!pEventDbpConnect)
            {
                return;
            }

            CMvDbpConnect& connectBody = pEventDbpConnect->data;
            connectBody.isReconnect = true;
            connectBody.session = session;
            connectBody.version = connectMsg.version();
            connectBody.client = connectMsg.client();

            pDbpClient->GetProfile()->pIOModule->PostEvent(pEventDbpConnect);
        }
        else
        {
            RespondFailed(pDbpClient, "002");
        }
    }
}

void CDbpServer::HandleClientSub(CDbpServerSocket* pDbpClient, google::protobuf::Any* any)
{
    CMvEventDbpSub* pEventDbpSub = new CMvEventDbpSub(pDbpClient->GetSession());
    if (!pEventDbpSub)
    {
        return;
    }

    dbp::Sub subMsg;
    any->UnpackTo(&subMsg);

    CMvDbpSub& subBody = pEventDbpSub->data;
    subBody.id = subMsg.id();
    subBody.name = subMsg.name();

    pDbpClient->GetProfile()->pIOModule->PostEvent(pEventDbpSub);
}

void CDbpServer::HandleClientUnSub(CDbpServerSocket* pDbpClient, google::protobuf::Any* any)
{
    CMvEventDbpUnSub* pEventDbpUnSub = new CMvEventDbpUnSub(pDbpClient->GetSession());
    if (!pEventDbpUnSub)
    {
        return;
    }

    dbp::Unsub unsubMsg;
    any->UnpackTo(&unsubMsg);

    CMvDbpUnSub& unsubBody = pEventDbpUnSub->data;
    unsubBody.id = unsubMsg.id();

    pDbpClient->GetProfile()->pIOModule->PostEvent(pEventDbpUnSub);
}

void CDbpServer::HandleClientMethod(CDbpServerSocket* pDbpClient, google::protobuf::Any* any)
{
    CMvEventDbpMethod* pEventDbpMethod = new CMvEventDbpMethod(pDbpClient->GetSession());
    if (!pEventDbpMethod)
    {
        return;
    }

    dbp::Method methodMsg;
    any->UnpackTo(&methodMsg);

    CMvDbpMethod& methodBody = pEventDbpMethod->data;
    methodBody.id = methodMsg.id();
    
    if (methodMsg.method() == "getblocks" && 
        methodMsg.params().Is<lws::GetBlocksArg>())
    {
        lws::GetBlocksArg args;
        methodMsg.params().UnpackTo(&args);

        std::string forkid;
        GetSessionForkId(pDbpClient, forkid);

        methodBody.method = CMvDbpMethod::LwsMethod::GET_BLOCKS;
        methodBody.params.insert(std::make_pair("forkid", forkid));
        methodBody.params.insert(std::make_pair("hash", args.hash()));
        methodBody.params.insert(std::make_pair("number", boost::lexical_cast<std::string>(args.number())));
    }
    else if (methodMsg.method() == "gettransaction" && 
        methodMsg.params().Is<lws::GetTxArg>())
    {
        lws::GetTxArg args;
        methodMsg.params().UnpackTo(&args);
        
        methodBody.method = CMvDbpMethod::LwsMethod::GET_TRANSACTION;
        methodBody.params.insert(std::make_pair("hash", args.hash()));
    }
    else if (methodMsg.method() == "sendtransaction" && 
        methodMsg.params().Is<lws::SendTxArg>())
    {
        lws::SendTxArg args;
        methodMsg.params().UnpackTo(&args);
        
        methodBody.method = CMvDbpMethod::LwsMethod::SEND_TRANSACTION;
        methodBody.params.insert(std::make_pair("data", args.data()));
    }
    else if(methodMsg.method() == "sendevent" &&
        methodMsg.params().Is<sn::VPeerNetEvent>())
    {
        sn::VPeerNetEvent event;
        methodMsg.params().UnpackTo(&event);

        methodBody.method = CMvDbpMethod::SnMethod::SEND_EVENT;
        methodBody.params.insert(std::make_pair("type", event.type()));
        methodBody.params.insert(std::make_pair("data", event.data()));
    }
    else if (methodMsg.method() == "rpcroute" &&
             methodMsg.params().Is<sn::RPCRouteArgs>())
    {
        sn::RPCRouteArgs args;
        methodMsg.params().UnpackTo(&args);
        methodBody.method = CMvDbpMethod::SnMethod::RPC_ROUTE;
        methodBody.params.insert(std::make_pair("type", args.type()));
        methodBody.params.insert(std::make_pair("data", args.data()));
        methodBody.params.insert(std::make_pair("rawdata", args.rawdata()));
    }
    else
    {
        delete pEventDbpMethod;
        return;
    }

    pDbpClient->GetProfile()->pIOModule->PostEvent(pEventDbpMethod);
}

void CDbpServer::HandleClientPing(CDbpServerSocket* pDbpClient, google::protobuf::Any* any)
{
    dbp::Ping pingMsg;
    any->UnpackTo(&pingMsg);
    pDbpClient->SendPong(pingMsg.id());
}

void CDbpServer::HandleClientPong(CDbpServerSocket* pDbpClient, google::protobuf::Any* any)
{
    dbp::Pong pongMsg;
    any->UnpackTo(&pongMsg);
    std::string session = bimapSessionClient.right.at(pDbpClient);
    if (IsSessionExist(session))
    {
        mapSessionProfile[session].nTimeStamp = CDbpUtils::CurrentUTC();
    }
}

void CDbpServer::HandleClientRecv(CDbpServerSocket* pDbpClient, const boost::any& anyObj)
{
    if (anyObj.type() != typeid(google::protobuf::Any))
    {
        return;
    }

    google::protobuf::Any any = boost::any_cast<google::protobuf::Any>(anyObj);

    if (any.Is<dbp::Connect>())
    {
        HandleClientConnect(pDbpClient, &any);
    }
    else if (any.Is<dbp::Sub>())
    {
        if (!HaveAssociatedSessionOf(pDbpClient))
        {
            RespondError(pDbpClient, "001", "need to connect first. ");
            return;
        }

        if (IsSessionTimeOut(pDbpClient))
        {
            RespondFailed(pDbpClient, "002");
            return;
        }

        HandleClientSub(pDbpClient, &any);
    }
    else if (any.Is<dbp::Unsub>())
    {
        if (!HaveAssociatedSessionOf(pDbpClient))
        {
            RespondError(pDbpClient, "001", "need to connect first. ");
            return;
        }

        if (IsSessionTimeOut(pDbpClient))
        {
            RespondFailed(pDbpClient, "002");
            return;
        }

        HandleClientUnSub(pDbpClient, &any);
    }
    else if (any.Is<dbp::Method>())
    {
        if (!HaveAssociatedSessionOf(pDbpClient))
        {
            RespondError(pDbpClient, "001", "need to connect first. ");
            return;
        }

        if (IsSessionTimeOut(pDbpClient))
        {
            RespondFailed(pDbpClient, "002");
            return;
        }

        HandleClientMethod(pDbpClient, &any);
    }
    else if (any.Is<dbp::Ping>())
    {
        if (!HaveAssociatedSessionOf(pDbpClient))
        {
            RespondError(pDbpClient, "001", "need to connect first. ");
            return;
        }

        if (IsSessionTimeOut(pDbpClient))
        {
            RespondFailed(pDbpClient, "002");
            return;
        }

        HandleClientPing(pDbpClient, &any);
    }
    else if (any.Is<dbp::Pong>())
    {
        if (!HaveAssociatedSessionOf(pDbpClient))
        {
            RespondError(pDbpClient, "001", "need to connect first. ");
            return;
        }

        if (IsSessionTimeOut(pDbpClient))
        {
            RespondFailed(pDbpClient, "002");
            return;
        }

        HandleClientPong(pDbpClient, &any);
    }
    else
    {
        RespondError(pDbpClient, "003", "unknown dbp message");
        return;
    }
}

void CDbpServer::HandleClientSent(CDbpServerSocket* pDbpClient)
{
    // keep-alive connection,do not remove
    pDbpClient->Activate();
}

void CDbpServer::HandleClientError(CDbpServerSocket* pDbpClient)
{
    std::cerr << "Dbp Server Socket Error. " << std::endl;
    
    CMvEventDbpBroken *pEventDbpBroken = new CMvEventDbpBroken(pDbpClient->GetSession());
    if(pEventDbpBroken)
    {
        pEventDbpBroken->data.session = pDbpClient->GetSession();
        pEventDbpBroken->data.from = "dbpserver";
        pDbpClient->GetProfile()->pIOModule->PostEvent(pEventDbpBroken);
    }

    RemoveClient(pDbpClient);
}

void CDbpServer::AddNewHost(const CDbpHostConfig& confHost)
{
    vecHostConfig.push_back(confHost);
}

bool CDbpServer::WalleveHandleInitialize()
{
    for (const CDbpHostConfig& confHost : vecHostConfig)
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
    for (std::map<boost::asio::ip::tcp::endpoint, CDbpProfile>::iterator it = mapProfile.begin();
         it != mapProfile.end(); ++it)
    {
        delete (*it).second.pSSLContext;
    }
    mapProfile.clear();
}

void CDbpServer::EnterLoop()
{
    WalleveLog("Dbp Server starting:\n");

    for (std::map<boost::asio::ip::tcp::endpoint, CDbpProfile>::iterator it = mapProfile.begin();
         it != mapProfile.end(); ++it)
    {
        if (!StartService((*it).first, (*it).second.nMaxConnections, (*it).second.vAllowMask))
        {
            WalleveError("Setup service %s failed, listen port = %d, connection limit %d\n",
                       (*it).second.pIOModule->WalleveGetOwnKey().c_str(),
                       (*it).first.port(), (*it).second.nMaxConnections);
        }
        else
        {
            WalleveLog("Setup service %s success, listen port = %d, connection limit %d\n",
                       (*it).second.pIOModule->WalleveGetOwnKey().c_str(),
                       (*it).first.port(), (*it).second.nMaxConnections);
        }
    }
}

void CDbpServer::LeaveLoop()
{
    std::vector<CDbpServerSocket*> vClient;
    for (std::map<uint64, CDbpServerSocket*>::iterator it = mapClient.begin(); it != mapClient.end(); ++it)
    {
        vClient.push_back((*it).second);
    }

    for (CDbpServerSocket *pClient : vClient)
    {
        RemoveClient(pClient);
    }
    WalleveLog("Dbp Server stop\n");
}

bool CDbpServer::ClientAccepted(const boost::asio::ip::tcp::endpoint& epService, CIOClient* pClient)
{
    std::map<boost::asio::ip::tcp::endpoint, CDbpProfile>::iterator it = mapProfile.find(epService);
    if (it == mapProfile.end())
    {
        return false;
    }

    std::cout << "[<] #########Client Accepted Remote Address " << 
        pClient->GetRemote().address().to_string() << 
        " port " <<  pClient->GetRemote().port() << " [dbpserver]" << std::endl;

    return AddNewClient(pClient, &(*it).second) != NULL;
}

bool CDbpServer::CreateProfile(const CDbpHostConfig& confHost)
{
    CDbpProfile profile;
    if (!WalleveGetObject(confHost.strIOModule, profile.pIOModule))
    {
        WalleveError("Failed to request %s\n", confHost.strIOModule.c_str());
        return false;
    }

    if (confHost.optSSL.fEnable)
    {
        profile.pSSLContext = new boost::asio::ssl::context(boost::asio::ssl::context::sslv23);
        if (!profile.pSSLContext)
        {
            WalleveError("Failed to alloc ssl context for %s:%u\n",
                       confHost.epHost.address().to_string().c_str(),
                       confHost.epHost.port());
            return false;
        }
        if (!confHost.optSSL.SetupSSLContext(*profile.pSSLContext))
        {
            WalleveError("Failed to setup ssl context for %s:%u\n",
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

CDbpServerSocket *CDbpServer::AddNewClient(CIOClient* pClient, CDbpProfile* pDbpProfile)
{
    uint64 nNonce = 0;
    RAND_bytes((unsigned char *)&nNonce, sizeof(nNonce));
    while (mapClient.count(nNonce))
    {
        RAND_bytes((unsigned char *)&nNonce, sizeof(nNonce));
    }

    CDbpServerSocket* pDbpClient = new CDbpServerSocket(this, pDbpProfile, pClient, nNonce);
    if (pDbpClient)
    {
        mapClient.insert(std::make_pair(nNonce, pDbpClient));
        std::cout << "[rootnode] AddNewClient Activate " << nNonce << std::endl;
        pDbpClient->Activate();
    }

    return pDbpClient;
}

void CDbpServer::RemoveSession(CDbpServerSocket* pDbpClient)
{
    mapClient.erase(pDbpClient->GetNonce());

    if (HaveAssociatedSessionOf(pDbpClient))
    {
        std::string assciatedSession = bimapSessionClient.right.at(pDbpClient);
        bimapSessionClient.left.erase(assciatedSession);
        bimapSessionClient.right.erase(pDbpClient);
        mapSessionProfile[assciatedSession].ptrPingTimer->cancel();
        mapSessionProfile.erase(assciatedSession);
    }
}

void CDbpServer::RemoveClient(CDbpServerSocket* pDbpClient)
{
    RemoveSession(pDbpClient);
    delete pDbpClient;
}

void CDbpServer::RespondError(CDbpServerSocket* pDbpClient, const std::string& reason, const std::string& strError)
{
    pDbpClient->SendResponse(reason, strError);
}

void CDbpServer::RespondFailed(CDbpServerSocket* pDbpClient, const std::string& reason)
{
    CMvEventDbpFailed failedEvent(pDbpClient->GetNonce());

    std::string session =
        HaveAssociatedSessionOf(pDbpClient) ? bimapSessionClient.right.at(pDbpClient) : "";
    failedEvent.data.session = session;
    failedEvent.data.reason = reason;
    this->HandleEvent(failedEvent);
}

void CDbpServer::SendPingHandler(const boost::system::error_code& err, const CSessionProfile& sessionProfile)
{
    if (err != boost::system::errc::success)
    {
        return;
    }

    std::string utc = std::to_string(CDbpUtils::CurrentUTC());

    if(IsSessionTimeOut(sessionProfile.pDbpClient))
    {
        std::cerr << "######### dbp server session time out ############" << std::endl;
        HandleClientError(sessionProfile.pDbpClient);
        return;
    }

    sessionProfile.pDbpClient->SendPing(utc);

    sessionProfile.ptrPingTimer->expires_at(sessionProfile.ptrPingTimer->expires_at() + boost::posix_time::seconds(3));
    sessionProfile.ptrPingTimer->async_wait(boost::bind(&CDbpServer::SendPingHandler,
                                                        this, boost::asio::placeholders::error,
                                                        boost::ref(sessionProfile)));
}

bool CDbpServer::HandleEvent(CMvEventDbpConnected& event)
{
    auto it = mapSessionProfile.find(event.strSessionId);
    if (it == mapSessionProfile.end())
    {
        std::cerr << "cannot find session [Connected] " << event.strSessionId << std::endl;
        return false;
    }

    CDbpServerSocket* pDbpClient = (*it).second.pDbpClient;
    CMvDbpConnected& connectedBody = event.data;

    pDbpClient->SendResponse(connectedBody);

    it->second.ptrPingTimer =
        std::make_shared<boost::asio::deadline_timer>(this->GetIoService(),
                                                      boost::posix_time::seconds(3));

    it->second.ptrPingTimer->expires_at(it->second.ptrPingTimer->expires_at() +
                                       boost::posix_time::seconds(3));
    it->second.ptrPingTimer->async_wait(boost::bind(&CDbpServer::SendPingHandler,
                                                    this, boost::asio::placeholders::error,
                                                    boost::ref(it->second)));

    return true;
}

bool CDbpServer::HandleEvent(CMvEventDbpFailed& event)
{
    std::map<uint64, CDbpServerSocket *>::iterator it = mapClient.find(event.nNonce);
    if (it == mapClient.end())
    {
        std::cerr << "cannot find nonce [failed]" << std::endl;
        return false;
    }

    CDbpServerSocket* pDbpClient = (*it).second;
    CMvDbpFailed& failedBody = event.data;

    pDbpClient->SendResponse(failedBody);

    HandleClientError(pDbpClient);

    return true;
}

bool CDbpServer::HandleEvent(CMvEventDbpNoSub& event)
{
    auto it = mapSessionProfile.find(event.strSessionId);
    if (it == mapSessionProfile.end())
    {
        std::cerr << "cannot find session [NoSub] " << event.strSessionId << std::endl;
        return false;
    }

    CDbpServerSocket* pDbpClient = (*it).second.pDbpClient;
    CMvDbpNoSub& noSubBody = event.data;

    pDbpClient->SendResponse(noSubBody);

    return true;
}

bool CDbpServer::HandleEvent(CMvEventDbpReady& event)
{
    auto it = mapSessionProfile.find(event.strSessionId);
    if (it == mapSessionProfile.end())
    {
        std::cerr << "cannot find session [Ready] " << event.strSessionId << std::endl;
        return false;
    }

    CDbpServerSocket* pDbpClient = (*it).second.pDbpClient;
    CMvDbpReady& readyBody = event.data;

    pDbpClient->SendResponse(readyBody);

    return true;
}

bool CDbpServer::HandleEvent(CMvEventDbpAdded& event)
{
    if(!event.strSessionId.empty())
    {
        auto it = mapSessionProfile.find(event.strSessionId);
        if (it == mapSessionProfile.end())
        {
            std::cerr << "cannot find session [Added] " << event.strSessionId << std::endl;
            return false;
        }

        if(it->second.strClient != "supernode" && it->second.strForkId == event.data.forkid)
        {
            CDbpServerSocket* pDbpClient = (*it).second.pDbpClient;
            CMvDbpAdded& addedBody = event.data;
            pDbpClient->SendResponse(it->second.strClient,addedBody);
        }
        else
        {
            CDbpServerSocket* pDbpClient = (*it).second.pDbpClient;
            CMvDbpAdded& addedBody = event.data;
            pDbpClient->SendResponse("supernode",addedBody);
        }
    }
    else
    {
        
        if(mapSessionProfile.empty())
        {
            return false;
        }
        
        for(const auto& session : mapSessionProfile)
        {
            CDbpServerSocket* pDbpClient = session.second.pDbpClient;
            CMvDbpAdded& addedBody = event.data;
            pDbpClient->SendResponse("supernode",addedBody);
        }
    }
    
    return true;
}

bool CDbpServer::HandleEvent(CMvEventDbpMethodResult& event)
{
    auto it = mapSessionProfile.find(event.strSessionId);
    if (it == mapSessionProfile.end())
    {
        std::cerr << "cannot find session [Method Result] " << event.strSessionId << std::endl;
        return false;
    }

    CDbpServerSocket* pDbpClient = (*it).second.pDbpClient;
    CMvDbpMethodResult& resultBody = event.data;

    pDbpClient->SendResponse(it->second.strClient, resultBody);

    return true;
}

//rpc route

bool CDbpServer::HandleEvent(CMvEventRPCRouteAdded & event)
{
    if(!event.strSessionId.empty())
    {
        auto it = mapSessionProfile.find(event.strSessionId);
        if (it == mapSessionProfile.end())
        {
            std::cerr << "cannot find session [Added] " << event.strSessionId << std::endl;
            return false;
        }

        if(it->second.strClient == "supernode")
        {
            CDbpServerSocket* socket = (*it).second.pDbpClient;
            CMvRPCRouteAdded addedBody = event.data;
            socket->SendResponse(it->second.strClient, addedBody);
        }
    }

    return true;
}

//

bool CDbpServer::IsSessionTimeOut(CDbpServerSocket* pDbpClient)
{
    if (HaveAssociatedSessionOf(pDbpClient))
    {
        auto timeout = pDbpClient->GetProfile()->nSessionTimeout;
        std::string assciatedSession = bimapSessionClient.right.at(pDbpClient);
        uint64 lastTimeStamp = mapSessionProfile[assciatedSession].nTimeStamp;
        return (CDbpUtils::CurrentUTC() - lastTimeStamp > timeout) ? true : false;
    }
    else
    {
        return false;
    }
}

bool CDbpServer::GetSessionForkId(CDbpServerSocket* pDbpClient, std::string& forkid)
{
    if (HaveAssociatedSessionOf(pDbpClient))
    {
        std::string assciatedSession = bimapSessionClient.right.at(pDbpClient);
        forkid = mapSessionProfile[assciatedSession].strForkId;
        return true;
    }
    else
    {
        return false;
    }
}

bool CDbpServer::IsSessionReconnect(const std::string& session)
{
    return !session.empty();
}

bool CDbpServer::IsSessionExist(const std::string& session)
{
    return mapSessionProfile.find(session) != mapSessionProfile.end();
}

bool CDbpServer::HaveAssociatedSessionOf(CDbpServerSocket* pDbpClient)
{
    return bimapSessionClient.right.find(pDbpClient) != bimapSessionClient.right.end();
}

std::string CDbpServer::GetUdata(dbp::Connect* pConnect, const std::string& keyName)
{
    auto customParamsMap = pConnect->udata();
    google::protobuf::Any paramAny = customParamsMap[keyName];
    lws::ForkID forkidArg;

    if (!paramAny.Is<lws::ForkID>())
    {
        return std::string();
    }

    if (keyName == "forkid")
    {
        paramAny.UnpackTo(&forkidArg);
        if (forkidArg.ids_size() != 0)
            return forkidArg.ids(0);
        else
            return std::string();
       
    }

    return std::string();
}

std::string CDbpServer::GenerateSessionId()
{
    std::string session = CDbpUtils::RandomString();
    while (IsSessionExist(session))
    {
        session = CDbpUtils::RandomString();
    }

    return session;
}

void CDbpServer::CreateSession(const std::string& session, const std::string& client, const std::string& forkID, CDbpServerSocket* pDbpClient)
{
    CSessionProfile profile;
    profile.strSessionId = session;
    profile.strClient = client;
    profile.strForkId = forkID;
    profile.pDbpClient = pDbpClient;
    profile.nTimeStamp = CDbpUtils::CurrentUTC();

    pDbpClient->SetSession(session);

    mapSessionProfile.insert(std::make_pair(session, profile));
    bimapSessionClient.insert(position_pair(session, pDbpClient));
}

void CDbpServer::UpdateSession(const std::string& session, CDbpServerSocket* pDbpClient)
{
    if (bimapSessionClient.left.find(session) != bimapSessionClient.left.end())
    {
        auto pDbplient = bimapSessionClient.left.at(session);
        bimapSessionClient.left.erase(session);
        bimapSessionClient.right.erase(pDbpClient);
    }

    mapSessionProfile[session].pDbpClient = pDbpClient;
    mapSessionProfile[session].nTimeStamp = CDbpUtils::CurrentUTC();
    bimapSessionClient.insert(position_pair(session, pDbpClient));
}
