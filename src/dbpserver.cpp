// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "dbpserver.h"

#include <memory>
#include <algorithm>
#include <openssl/rand.h>
#include <boost/foreach.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>

#include "dbputils.h"

using namespace multiverse;
using namespace walleve;

static const std::size_t MSG_HEADER_LEN = 4;

CDbpClient::CDbpClient(CDbpServer* pServerIn, CDbpProfile* pProfileIn,
                       CIOClient* pClientIn, uint64 nonce)
    : pServer(pServerIn), pProfile(pProfileIn), pClient(pClientIn), nNonce(nonce)

{
}

CDbpClient::~CDbpClient()
{
    if (pClient)
    {
        pClient->Close();
    }
}

CDbpProfile* CDbpClient::GetProfile()
{
    return pProfile;
}

uint64 CDbpClient::GetNonce()
{
    return nNonce;
}

std::string CDbpClient::GetSession() const
{
    return strSessionId;
}

void CDbpClient::SetSession(const std::string& session)
{
    strSessionId = session;
}

void CDbpClient::Activate()
{
    ssRecv.Clear();

    StartReadHeader();
}

void CDbpClient::SendMessage(dbp::Base* pBaseMsg)
{
    if (ssSend.GetSize() != 0)
    {
        pClient->Write(ssSend, boost::bind(&CDbpClient::HandleWritenResponse, this, _1, OTHER));
        return;
    }

    WriteMessageToSendStream(pBaseMsg);

    pClient->Write(ssSend, boost::bind(&CDbpClient::HandleWritenResponse, this, _1, OTHER));
}

void CDbpClient::SendPingMessage(dbp::Base* pBaseMsg)
{
    if (!IsSentComplete())
    {
        pClient->Write(ssSend, boost::bind(&CDbpClient::HandleWritenResponse, this, _1, OTHER));
        return;
    }

    WriteMessageToSendStream(pBaseMsg);

    pClient->Write(ssSend, boost::bind(&CDbpClient::HandleWritenResponse, this, _1, PING));
}

void CDbpClient::SendAddedMessage(dbp::Base* pBaseMsg)
{
    if (!IsSentComplete())
    {
        queueAddedSend.push(*pBaseMsg);
        pClient->Write(ssSend, boost::bind(&CDbpClient::HandleWritenResponse, this, _1, OTHER));
        return;
    }

    WriteMessageToSendStream(pBaseMsg);

    pClient->Write(ssSend, boost::bind(&CDbpClient::HandleWritenResponse, this, _1, ADDED));
}

void CDbpClient::SendResponse(CMvDbpConnected& body)
{

    dbp::Base connectedMsgBase;
    connectedMsgBase.set_msg(dbp::Msg::CONNECTED);

    dbp::Connected connectedMsg;
    connectedMsg.set_session(body.session);

    google::protobuf::Any* any = new google::protobuf::Any();
    any->PackFrom(connectedMsg);

    connectedMsgBase.set_allocated_object(any);

    SendMessage(&connectedMsgBase);
}

void CDbpClient::SendResponse(CMvDbpFailed& body)
{
    dbp::Base failedMsgBase;
    failedMsgBase.set_msg(dbp::Msg::FAILED);

    dbp::Failed failedMsg;

    for (const int32& version : body.versions)
    {
        failedMsg.add_version(version);
    }

    failedMsg.set_reason(body.reason);
    failedMsg.set_session(body.session);

    google::protobuf::Any* any = new google::protobuf::Any();
    any->PackFrom(failedMsg);

    failedMsgBase.set_allocated_object(any);
    SendMessage(&failedMsgBase);
}

void CDbpClient::SendResponse(CMvDbpNoSub& body)
{
    dbp::Base noSubMsgBase;
    noSubMsgBase.set_msg(dbp::Msg::NOSUB);

    dbp::Nosub noSubMsg;
    noSubMsg.set_id(body.id);
    noSubMsg.set_error(body.error);

    google::protobuf::Any* any = new google::protobuf::Any();
    any->PackFrom(noSubMsg);

    noSubMsgBase.set_allocated_object(any);

    SendMessage(&noSubMsgBase);
}

void CDbpClient::SendResponse(CMvDbpReady& body)
{
    dbp::Base readyMsgBase;
    readyMsgBase.set_msg(dbp::Msg::READY);

    dbp::Ready readyMsg;
    readyMsg.set_id(body.id);

    google::protobuf::Any* any = new google::protobuf::Any();
    any->PackFrom(readyMsg);

    readyMsgBase.set_allocated_object(any);

    SendMessage(&readyMsgBase);
}

static void CreateLwsTransaction(const CMvDbpTransaction* dbptx, lws::Transaction* tx)
{
    tx->set_nversion(dbptx->nVersion);
    tx->set_ntype(dbptx->nType);
    tx->set_nlockuntil(dbptx->nLockUntil);

    std::string hashAnchor(dbptx->hashAnchor.begin(), dbptx->hashAnchor.end());
    tx->set_hashanchor(hashAnchor);

    for (const auto& input : dbptx->vInput)
    {
        std::string inputHash(input.hash.begin(), input.hash.end());
        auto add = tx->add_vinput();
        add->set_hash(inputHash);
        add->set_n(input.n);
    }

    lws::Transaction::CDestination* dest = new lws::Transaction::CDestination();
    dest->set_prefix(dbptx->cDestination.prefix);
    dest->set_size(dbptx->cDestination.size);

    std::string destData(dbptx->cDestination.data.begin(), dbptx->cDestination.data.end());
    dest->set_data(destData);
    tx->set_allocated_cdestination(dest);

    tx->set_namount(dbptx->nAmount);
    tx->set_ntxfee(dbptx->nTxFee);

    std::string mintVchData(dbptx->vchData.begin(), dbptx->vchData.end());
    tx->set_vchdata(mintVchData);

    std::string mintVchSig(dbptx->vchData.begin(), dbptx->vchData.end());
    tx->set_vchsig(mintVchSig);

    std::string hash(dbptx->hash.begin(), dbptx->hash.end());
    tx->set_hash(hash);
}

static void CreateLwsBlock(CMvDbpBlock* pBlock, lws::Block& block)
{
    block.set_nversion(pBlock->nVersion);
    block.set_ntype(pBlock->nType);
    block.set_ntimestamp(pBlock->nTimeStamp);

    std::string hashPrev(pBlock->hashPrev.begin(), pBlock->hashPrev.end());
    block.set_hashprev(hashPrev);

    std::string hashMerkle(pBlock->hashMerkle.begin(), pBlock->hashMerkle.end());
    block.set_hashmerkle(hashMerkle);

    std::string vchproof(pBlock->vchProof.begin(), pBlock->vchProof.end());
    block.set_vchproof(vchproof);

    std::string vchSig(pBlock->vchSig.begin(), pBlock->vchSig.end());
    block.set_vchsig(vchSig);

    //txMint
    lws::Transaction* txMint = new lws::Transaction();
    CreateLwsTransaction(&(pBlock->txMint), txMint);
    block.set_allocated_txmint(txMint);

    //repeated vtx
    for (const auto& tx : pBlock->vtx)
    {
        CreateLwsTransaction(&tx, block.add_vtx());
    }

    block.set_nheight(pBlock->nHeight);
    std::string hash(pBlock->hash.begin(), pBlock->hash.end());
    block.set_hash(hash);
}

void CDbpClient::SendResponse(CMvDbpAdded& body)
{
    dbp::Base addedMsgBase;
    addedMsgBase.set_msg(dbp::Msg::ADDED);

    dbp::Added addedMsg;
    addedMsg.set_id(body.id);
    addedMsg.set_name(body.name);

    if (body.anyAddedObj.type() == typeid(CMvDbpBlock))
    {
        CMvDbpBlock tempBlock = boost::any_cast<CMvDbpBlock>(body.anyAddedObj);

        lws::Block block;
        CreateLwsBlock(&tempBlock, block);

        google::protobuf::Any* anyBlock = new google::protobuf::Any();
        anyBlock->PackFrom(block);
        addedMsg.set_allocated_object(anyBlock);
    }
    else if (body.anyAddedObj.type() == typeid(CMvDbpTransaction))
    {
        CMvDbpTransaction tempTx = boost::any_cast<CMvDbpTransaction>(body.anyAddedObj);

        std::unique_ptr<lws::Transaction> tx(new lws::Transaction());
        CreateLwsTransaction(&tempTx, tx.get());

        google::protobuf::Any* anyTx = new google::protobuf::Any();
        anyTx->PackFrom(*tx);

        addedMsg.set_allocated_object(anyTx);
    }
    else
    {
    }

    google::protobuf::Any* anyAdded = new google::protobuf::Any();
    anyAdded->PackFrom(addedMsg);

    addedMsgBase.set_allocated_object(anyAdded);
    SendAddedMessage(&addedMsgBase);
}

void CDbpClient::SendResponse(CMvDbpMethodResult& body)
{
    dbp::Base resultMsgBase;
    resultMsgBase.set_msg(dbp::Msg::RESULT);

    dbp::Result resultMsg;
    resultMsg.set_id(body.id);
    resultMsg.set_error(body.error);

    auto dispatchHandler = [&](const boost::any& obj) -> void {
        if (obj.type() == typeid(CMvDbpBlock))
        {
            CMvDbpBlock tempBlock = boost::any_cast<CMvDbpBlock>(obj);

            lws::Block block;
            CreateLwsBlock(&tempBlock, block);
            resultMsg.add_result()->PackFrom(block);
        }
        else if (obj.type() == typeid(CMvDbpTransaction))
        {
            CMvDbpTransaction tempTx = boost::any_cast<CMvDbpTransaction>(obj);

            std::unique_ptr<lws::Transaction> tx(new lws::Transaction());
            CreateLwsTransaction(&tempTx, tx.get());
            resultMsg.add_result()->PackFrom(*tx);
        }
        else if (obj.type() == typeid(CMvDbpSendTxRet))
        {
            CMvDbpSendTxRet txret = boost::any_cast<CMvDbpSendTxRet>(obj);
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

    resultMsgBase.set_allocated_object(anyResult);

    SendMessage(&resultMsgBase);
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

void CDbpClient::SendPing(const std::string& id)
{
    dbp::Base pingMsgBase;
    pingMsgBase.set_msg(dbp::Msg::PING);

    dbp::Ping msg;
    msg.set_id(id);

    google::protobuf::Any* any = new google::protobuf::Any();
    any->PackFrom(msg);

    pingMsgBase.set_allocated_object(any);

    SendPingMessage(&pingMsgBase);
}

void CDbpClient::SendResponse(const std::string& reason, const std::string& description)
{
    dbp::Base errorMsgBase;
    errorMsgBase.set_msg(dbp::Msg::ERROR);

    dbp::Error errorMsg;
    errorMsg.set_reason(reason);
    errorMsg.set_explain(description);

    google::protobuf::Any* any = new google::protobuf::Any();
    any->PackFrom(errorMsg);

    errorMsgBase.set_allocated_object(any);

    SendMessage(&errorMsgBase);
}

void CDbpClient::StartReadHeader()
{
    pClient->Read(ssRecv, MSG_HEADER_LEN,
                  boost::bind(&CDbpClient::HandleReadHeader, this, _1));
}

void CDbpClient::StartReadPayload(std::size_t nLength)
{
    pClient->Read(ssRecv, nLength,
                  boost::bind(&CDbpClient::HandleReadPayload, this, _1, nLength));
}

void CDbpClient::WriteMessageToSendStream(dbp::Base* pBaseMsg)
{
    std::string bytes;
    pBaseMsg->SerializeToString(&bytes);

    unsigned char msgLenBuf[MSG_HEADER_LEN];
    CDbpUtils::WriteLenToMsgHeader(bytes.size(), (char *)msgLenBuf, MSG_HEADER_LEN);
    ssSend.Write((char *)msgLenBuf, MSG_HEADER_LEN);
    ssSend.Write((char *)bytes.data(), bytes.size());
}

bool CDbpClient::IsSentComplete()
{
    return (ssSend.GetSize() == 0 && queueAddedSend.empty());
}

void CDbpClient::HandleReadHeader(std::size_t nTransferred)
{
    if (nTransferred == MSG_HEADER_LEN)
    {
        std::string lenBuffer(MSG_HEADER_LEN, 0);
        ssRecv.Read(&lenBuffer[0], MSG_HEADER_LEN);

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

void CDbpClient::HandleReadPayload(std::size_t nTransferred, uint32_t len)
{
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

void CDbpClient::HandleReadCompleted(uint32_t len)
{
    std::string payloadBuffer(len, 0);
    ssRecv.Read(&payloadBuffer[0], len);

    dbp::Base msgBase;
    if (!msgBase.ParseFromString(payloadBuffer))
    {
        std::cerr << "parse payload failed. " << std::endl;
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

void CDbpClient::HandleWritenResponse(std::size_t nTransferred, CDbpClient::SendType type)
{
    if (nTransferred != 0)
    {
        if (ssSend.GetSize() != 0)
        {
            pClient->Write(ssSend, boost::bind(&CDbpClient::HandleWritenResponse,
                                               this, _1, type));
            return;
        }

        if (ssSend.GetSize() == 0 && !queueAddedSend.empty())
        {
            dbp::Base base;
            base = queueAddedSend.front();
            WriteMessageToSendStream(&base);
            queueAddedSend.pop();

            pClient->Write(ssSend, boost::bind(&CDbpClient::HandleWritenResponse, this, _1, ADDED));
            return;
        }

        if (type == OTHER)
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

CDbpServer::~CDbpServer()
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

void CDbpServer::HandleClientConnect(CDbpClient* pDbpClient, google::protobuf::Any* any)
{

    dbp::Connect connectMsg;
    any->UnpackTo(&connectMsg);

    std::string session = connectMsg.session();

    if (!IsSessionReconnect(session))
    {
        session = GenerateSessionId();
        std::string forkid = GetUdata(&connectMsg, "forkid");
        CreateSession(session, forkid, pDbpClient);

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

void CDbpServer::HandleClientSub(CDbpClient* pDbpClient, google::protobuf::Any* any)
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

void CDbpServer::HandleClientUnSub(CDbpClient* pDbpClient, google::protobuf::Any* any)
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

void CDbpServer::HandleClientMethod(CDbpClient* pDbpClient, google::protobuf::Any* any)
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

    if (methodMsg.method() == "getblocks")
        methodBody.method = CMvDbpMethod::Method::GET_BLOCKS;
    if (methodMsg.method() == "gettransaction")
        methodBody.method = CMvDbpMethod::Method::GET_TX;
    if (methodMsg.method() == "sendtransaction")
        methodBody.method = CMvDbpMethod::Method::SEND_TX;

    if (methodBody.method == CMvDbpMethod::Method::GET_BLOCKS && methodMsg.params().Is<lws::GetBlocksArg>())
    {
        lws::GetBlocksArg args;
        methodMsg.params().UnpackTo(&args);

        std::string forkid;
        GetSessionForkId(pDbpClient, forkid);

        methodBody.params.insert(std::make_pair("forkid", forkid));
        methodBody.params.insert(std::make_pair("hash", args.hash()));
        methodBody.params.insert(std::make_pair("number", boost::lexical_cast<std::string>(args.number())));
    }
    else if (methodBody.method == CMvDbpMethod::Method::GET_TX && methodMsg.params().Is<lws::GetTxArg>())
    {
        lws::GetTxArg args;
        methodMsg.params().UnpackTo(&args);
        methodBody.params.insert(std::make_pair("hash", args.hash()));
    }
    else if (methodBody.method == CMvDbpMethod::Method::SEND_TX && methodMsg.params().Is<lws::SendTxArg>())
    {
        lws::SendTxArg args;
        methodMsg.params().UnpackTo(&args);
        methodBody.params.insert(std::make_pair("data", args.data()));
    }
    else
    {
        delete pEventDbpMethod;
        return;
    }

    pDbpClient->GetProfile()->pIOModule->PostEvent(pEventDbpMethod);
}

void CDbpServer::HandleClientPing(CDbpClient* pDbpClient, google::protobuf::Any* any)
{
    dbp::Ping pingMsg;
    any->UnpackTo(&pingMsg);
    pDbpClient->SendPong(pingMsg.id());
}

void CDbpServer::HandleClientPong(CDbpClient* pDbpClient, google::protobuf::Any* any)
{
    dbp::Pong pongMsg;
    any->UnpackTo(&pongMsg);

    std::string session = bimapSessionClient.right.at(pDbpClient);
    if (IsSessionExist(session))
    {
        mapSessionProfile[session].nTimeStamp = CDbpUtils::CurrentUTC();
    }

    this->HandleClientSent(pDbpClient);
}

void CDbpServer::HandleClientRecv(CDbpClient* pDbpClient, const boost::any& anyObj)
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
            RemoveSession(pDbpClient);
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
            RemoveSession(pDbpClient);
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
            RemoveSession(pDbpClient);
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
            RemoveSession(pDbpClient);
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
            RemoveSession(pDbpClient);
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

void CDbpServer::HandleClientSent(CDbpClient* pDbpClient)
{
    // keep-alive connection,do not remove
    pDbpClient->Activate();
}

void CDbpServer::HandleClientError(CDbpClient* pDbpClient)
{
    std::cerr << "Client Error. " << std::endl;
    RemoveClient(pDbpClient);
}

void CDbpServer::AddNewHost(const CDbpHostConfig& confHost)
{
    vecHostConfig.push_back(confHost);
}

bool CDbpServer::WalleveHandleInitialize()
{
    BOOST_FOREACH (const CDbpHostConfig& confHost, vecHostConfig)
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
        if (!StartService((*it).first, (*it).second.nMaxConnections))
        {
            WalleveLog("Setup service %s failed, listen port = %d, connection limit %d\n",
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
    std::vector<CDbpClient*> vClient;
    for (std::map<uint64, CDbpClient*>::iterator it = mapClient.begin(); it != mapClient.end(); ++it)
    {
        vClient.push_back((*it).second);
    }

    BOOST_FOREACH (CDbpClient *pClient, vClient)
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
    return AddNewClient(pClient, &(*it).second) != NULL;
}

bool CDbpServer::CreateProfile(const CDbpHostConfig& confHost)
{
    CDbpProfile profile;
    if (!WalleveGetObject(confHost.strIOModule, profile.pIOModule))
    {
        WalleveLog("Failed to request %s\n", confHost.strIOModule.c_str());
        return false;
    }

    if (confHost.optSSL.fEnable)
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

CDbpClient *CDbpServer::AddNewClient(CIOClient* pClient, CDbpProfile* pDbpProfile)
{
    uint64 nNonce = 0;
    RAND_bytes((unsigned char *)&nNonce, sizeof(nNonce));
    while (mapClient.count(nNonce))
    {
        RAND_bytes((unsigned char *)&nNonce, sizeof(nNonce));
    }

    CDbpClient* pDbpClient = new CDbpClient(this, pDbpProfile, pClient, nNonce);
    if (pDbpClient)
    {
        mapClient.insert(std::make_pair(nNonce, pDbpClient));
        pDbpClient->Activate();
    }

    return pDbpClient;
}

void CDbpServer::RemoveSession(CDbpClient* pDbpClient)
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

void CDbpServer::RemoveClient(CDbpClient* pDbpClient)
{
    RemoveSession(pDbpClient);
    delete pDbpClient;
}

void CDbpServer::RespondError(CDbpClient* pDbpClient, const std::string& reason, const std::string& strError)
{
    pDbpClient->SendResponse(reason, strError);
}

void CDbpServer::RespondFailed(CDbpClient* pDbpClient, const std::string& reason)
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
    sessionProfile.pDbpClient->SendPing(utc);

    sessionProfile.ptrPingTimer->expires_at(sessionProfile.ptrPingTimer->expires_at() + boost::posix_time::seconds(1));
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

    CDbpClient* pDbpClient = (*it).second.pDbpClient;
    CMvDbpConnected& connectedBody = event.data;

    pDbpClient->SendResponse(connectedBody);

    it->second.ptrPingTimer =
        std::make_shared<boost::asio::deadline_timer>(this->GetIoService(),
                                                      boost::posix_time::seconds(1));

    it->second.ptrPingTimer->expires_at(it->second.ptrPingTimer->expires_at() +
                                        boost::posix_time::seconds(1));
    it->second.ptrPingTimer->async_wait(boost::bind(&CDbpServer::SendPingHandler,
                                                    this, boost::asio::placeholders::error,
                                                    boost::ref(it->second)));

    return true;
}

bool CDbpServer::HandleEvent(CMvEventDbpFailed& event)
{
    std::map<uint64, CDbpClient *>::iterator it = mapClient.find(event.nNonce);
    if (it == mapClient.end())
    {
        std::cerr << "cannot find nonce [failed]" << std::endl;
        return false;
    }

    CDbpClient* pDbpClient = (*it).second;
    CMvDbpFailed& failedBody = event.data;

    pDbpClient->SendResponse(failedBody);

    RemoveClient(pDbpClient);

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

    CDbpClient* pDbpClient = (*it).second.pDbpClient;
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

    CDbpClient* pDbpClient = (*it).second.pDbpClient;
    CMvDbpReady& readyBody = event.data;

    pDbpClient->SendResponse(readyBody);

    return true;
}

bool CDbpServer::HandleEvent(CMvEventDbpAdded& event)
{
    auto it = mapSessionProfile.find(event.strSessionId);
    if (it == mapSessionProfile.end())
    {
        std::cerr << "cannot find session [Added] " << event.strSessionId << std::endl;
        return false;
    }

    CDbpClient* pDbpClient = (*it).second.pDbpClient;
    CMvDbpAdded& addedBody = event.data;

    pDbpClient->SendResponse(addedBody);

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

    CDbpClient* pDbpClient = (*it).second.pDbpClient;
    CMvDbpMethodResult& resultBody = event.data;

    pDbpClient->SendResponse(resultBody);

    return true;
}

bool CDbpServer::IsSessionTimeOut(CDbpClient* pDbpClient)
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

bool CDbpServer::GetSessionForkId(CDbpClient* pDbpClient, std::string& forkid)
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

bool CDbpServer::HaveAssociatedSessionOf(CDbpClient* pDbpClient)
{
    return bimapSessionClient.right.find(pDbpClient) != bimapSessionClient.right.end();
}

std::string CDbpServer::GetUdata(dbp::Connect* pConnect, const std::string& keyName)
{
    auto customParamsMap = pConnect->udata();

    if (keyName == "forkid")
    {
        google::protobuf::Any paramAny = customParamsMap[keyName];
        lws::ForkID forkidArg;

        if (paramAny.Is<lws::ForkID>())
        {
            paramAny.UnpackTo(&forkidArg);
            if (forkidArg.ids_size() != 0)
                return forkidArg.ids(0);
            else
                return std::string();
        }
        else
        {
            return std::string();
        }
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

void CDbpServer::CreateSession(const std::string& session, const std::string& forkID, CDbpClient* pDbpClient)
{
    CSessionProfile profile;
    profile.strSessionId = session;
    profile.strForkId = forkID;
    profile.pDbpClient = pDbpClient;
    profile.nTimeStamp = CDbpUtils::CurrentUTC();

    pDbpClient->SetSession(session);

    mapSessionProfile.insert(std::make_pair(session, profile));
    bimapSessionClient.insert(position_pair(session, pDbpClient));
}

void CDbpServer::UpdateSession(const std::string& session, CDbpClient* pDbpClient)
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
