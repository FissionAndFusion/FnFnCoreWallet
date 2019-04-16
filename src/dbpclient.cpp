// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "dbpclient.h"
#include "dbputils.h"
#include "walleve/netio/netio.h"
#include "uint256.h"

#include <thread>
#include <chrono>
#include <memory>
#include <algorithm>
#include <openssl/rand.h>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>

static std::size_t MSG_HEADER_LEN = 4;

#define DBPCLIENT_CONNECT_TIMEOUT 10 
namespace multiverse
{

CDbpClientSocket::CDbpClientSocket(IIOModule* pIOModuleIn,const uint64 nNonceIn,
                   CDbpClient* pDbpClientIn,CIOClient* pClientIn, CDbpClientProfile* pProfileIn)
: pIOModule(pIOModuleIn),
  nNonce(nNonceIn),
  pDbpClient(pDbpClientIn),
  pClient(pClientIn),
  pProfile(pProfileIn),
  IsReading(false)
{
    ssRecv.Clear();
}
    
CDbpClientSocket::~CDbpClientSocket()
{
    if(pClient)
    {
        pClient->Close();
    }
}

IIOModule* CDbpClientSocket::GetIOModule()
{
    return pIOModule;
}

CDbpClientProfile* CDbpClientSocket::GetProfile() const
{
    return pProfile;
}
    
uint64 CDbpClientSocket::GetNonce()
{
    return nNonce;
}
    
CNetHost CDbpClientSocket::GetHost()
{
    return CNetHost(pClient->GetRemote());
}

std::string CDbpClientSocket::GetSession() const
{
    return strSessionId;
}

void CDbpClientSocket::SetSession(const std::string& session)
{
    strSessionId = session;
}

void CDbpClientSocket::ReadMessage()
{
    StartReadHeader();
}

void CDbpClientSocket::SendPong(const std::string& id)
{
    dbp::Pong pong;
    pong.set_id(id);

    google::protobuf::Any *any = new google::protobuf::Any();
    any->PackFrom(pong);

    SendMessage(dbp::Msg::PONG,any);
}

void CDbpClientSocket::SendPing(const std::string& id)
{
    dbp::Ping ping;
    ping.set_id(id);

    google::protobuf::Any *any = new google::protobuf::Any();
    any->PackFrom(ping);

    SendMessage(dbp::Msg::PING,any);
}

void CDbpClientSocket::SendSubScribeTopics(const std::vector<std::string>& topics)
{
    for(const auto& topic : topics)
    {
        SendSubscribeTopic(topic);
    }
}

void CDbpClientSocket::SendSubscribeTopic(const std::string& topic)
{
    dbp::Sub sub;
    sub.set_name(topic);
    std::string id(CDbpUtils::RandomString());
    sub.set_id(id);
    google::protobuf::Any *any = new google::protobuf::Any();
    any->PackFrom(sub);

    SendMessage(dbp::Msg::SUB,any);
}

void CDbpClientSocket::SendConnectSession(const std::string& session, const std::vector<std::string>& forks)
{
    dbp::Connect connect;
    connect.set_session(session);
    connect.set_client("supernode");
    connect.set_version(1);

    google::protobuf::Any *any = new google::protobuf::Any();
    any->PackFrom(connect);

    SendMessage(dbp::Msg::CONNECT,any);
}

void CDbpClientSocket::SendEvent(CMvDbpVirtualPeerNetEvent& dbpEvent)
{
    dbp::Method method;
    method.set_id(CDbpUtils::RandomString());
    method.set_method("sendevent");

    google::protobuf::Any *params = new google::protobuf::Any();
    sn::VPeerNetEvent event;
    event.set_type(dbpEvent.type);
    event.set_data(std::string(dbpEvent.data.begin(), dbpEvent.data.end()));

    std::cout << "sent event type " << dbpEvent.type << " [dbpclient]\n";

    params->PackFrom(event);
    method.set_allocated_params(params);

    google::protobuf::Any *any = new google::protobuf::Any();
    any->PackFrom(method);

    SendMessage(dbp::Msg::METHOD, any);
}

bool CDbpClientSocket::IsSentComplete()
{
    return (ssSend.GetSize() == 0 && queueMessage.empty());
}

void CDbpClientSocket::SendMessage(dbp::Msg type, google::protobuf::Any* any)
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
    pClient->Write(ssSend,boost::bind(&CDbpClientSocket::HandleWritenRequest,this,_1, type));
    
}

void CDbpClientSocket::StartReadHeader()
{
    IsReading = true;
    pClient->Read(ssRecv, MSG_HEADER_LEN,
                  boost::bind(&CDbpClientSocket::HandleReadHeader, this, _1));
}

void CDbpClientSocket::StartReadPayload(std::size_t nLength)
{
    pClient->Read(ssRecv, nLength,
                  boost::bind(&CDbpClientSocket::HandleReadPayload, this, _1, nLength));
}

void CDbpClientSocket::HandleWritenRequest(std::size_t nTransferred, dbp::Msg type)
{ 
    if(nTransferred != 0)
    {
        if(ssSend.GetSize() != 0)
        {
            pClient->Write(ssSend,boost::bind(&CDbpClientSocket::HandleWritenRequest,this,_1, type));
            return;
        }

        if(ssSend.GetSize() == 0 && !queueMessage.empty())
        {
            auto messagePair = queueMessage.front();
            dbp::Msg msgType = messagePair.first;
            std::string bytes = messagePair.second;
            queueMessage.pop();
            ssSend.Write((char*)bytes.data(),bytes.size());
            pClient->Write(ssSend,boost::bind(&CDbpClientSocket::HandleWritenRequest,this,_1, msgType));
            return;
        }

        /*if(type != dbp::Msg::PING || type != dbp::Msg::PONG)
        {
            std::cout << "[>]  Sent Message type " << dbp::Msg_Name(type) << " Msg Size: " 
                << nTransferred << " \n";
        }*/

        if(!IsReading)
        {
            pDbpClient->HandleClientSocketSent(this);
        }
    
    }
    else
    {
        pDbpClient->HandleClientSocketError(this);
    }
}
    
void CDbpClientSocket::HandleReadHeader(std::size_t nTransferred)
{   
    if(nTransferred == 0)
    {
        pDbpClient->HandleClientSocketError(this);
        return;
    }
    
    if (nTransferred == MSG_HEADER_LEN)
    {
        std::string lenBuffer(ssRecv.GetData(), ssRecv.GetData() + MSG_HEADER_LEN);
        uint32_t nMsgHeaderLen = CDbpUtils::ParseLenFromMsgHeader(&lenBuffer[0], MSG_HEADER_LEN);
        if (nMsgHeaderLen == 0)
        {
            std::cerr << "Msg Base header length is 0 [dbpclient]" << std::endl;
            pDbpClient->HandleClientSocketError(this);
            return;
        }

        StartReadPayload(nMsgHeaderLen);
    }
    else
    {
        std::cerr << "Msg Base header length is not 4 [dbpclient]" << std::endl;
        pDbpClient->HandleClientSocketError(this);
    }
}
    
void CDbpClientSocket::HandleReadPayload(std::size_t nTransferred, uint32_t len)
{
    if(nTransferred == 0)
    {
        pDbpClient->HandleClientSocketError(this);
        return;
    }
    
    if (nTransferred == len)
    {
        HandleReadCompleted(len);
    }
    else
    {
        std::cerr << "pay load is not len. [dbpclient]" << std::endl;
        pDbpClient->HandleClientSocketError(this);
    }
}

void CDbpClientSocket::HandleReadCompleted(uint32_t len)
{ 
    char head[4];
    ssRecv.Read(head,4);
    std::string payloadBuffer(len, 0);
    ssRecv.Read(&payloadBuffer[0], len);
    IsReading = false;

    dbp::Base msgBase;
    if (!msgBase.ParseFromString(payloadBuffer))
    {
        std::cerr << "parse payload failed. [dbpclient]" << std::endl;
        pDbpClient->HandleClientSocketError(this);
        return;
    }

    dbp::Msg currentMsgType = msgBase.msg();
    google::protobuf::Any anyObj = msgBase.object();
    switch(currentMsgType)
    {
    case dbp::CONNECTED:
        pDbpClient->HandleClientSocketRecv(this,anyObj);
        break;
    case dbp::FAILED:
        pDbpClient->HandleClientSocketRecv(this,anyObj);
        break;
    case dbp::PING:
        pDbpClient->HandleClientSocketRecv(this,anyObj);
        break;
    case dbp::PONG:
        pDbpClient->HandleClientSocketRecv(this,anyObj);
        pDbpClient->HandleClientSocketSent(this);
        break;
    case dbp::RESULT:
        pDbpClient->HandleClientSocketRecv(this,anyObj);
        pDbpClient->HandleClientSocketSent(this);
        break;
    case dbp::NOSUB:
        pDbpClient->HandleClientSocketRecv(this,anyObj);
        break;
    case dbp::READY:
        pDbpClient->HandleClientSocketRecv(this,anyObj);
        pDbpClient->HandleClientSocketSent(this);
        break;
    case dbp::ADDED:
        pDbpClient->HandleClientSocketRecv(this,anyObj);
        pDbpClient->HandleClientSocketSent(this);
        break;
    default:
        std::cerr << "is not Message Base Type is unknown. [dbpclient]" << std::endl;
        pDbpClient->HandleClientSocketError(this);
        break;
    }
}

CDbpClient::CDbpClient()
  : walleve::CIOProc("dbpclient")
{
    pDbpService = NULL;
    fIsResolved = false;
    fIsRootNode = false;
    fIsSuperNode = false;
}

CDbpClient::~CDbpClient() noexcept
{
}


void CDbpClient::HandleClientSocketError(CDbpClientSocket* pClientSocket)
{
    std::cerr << "Dbp Client Socket Error." << std::endl; 
    
    CMvEventDbpBroken *pEventDbpBroken = new CMvEventDbpBroken(pClientSocket->GetSession());
    if(pEventDbpBroken)
    {
        pEventDbpBroken->data.session = pClientSocket->GetSession();
        pEventDbpBroken->data.from = "dbpclient";
        pClientSocket->GetIOModule()->PostEvent(pEventDbpBroken);
    }

    auto epRemote = pClientSocket->GetHost().ToEndPoint();
    RemoveClientSocket(pClientSocket);

    auto it = mapProfile.find(epRemote);
    if(it != mapProfile.end())
    {
        StartConnection(epRemote,DBPCLIENT_CONNECT_TIMEOUT, 
            it->second.optSSL.fEnable,it->second.optSSL);
    }
    else
    {
        std::cerr << "cannot find reconnect parent node " << 
            epRemote.address().to_string() << " failed, " 
            << "port " << epRemote.port() << std::endl;
    }
}

void CDbpClient::HandleClientSocketSent(CDbpClientSocket* pClientSocket)
{
    pClientSocket->ReadMessage();
}

void CDbpClient::HandleClientSocketRecv(CDbpClientSocket* pClientSocket, const boost::any& anyObj)
{
    if(anyObj.type() != typeid(google::protobuf::Any))
    {
        return;
    }

    google::protobuf::Any any = boost::any_cast<google::protobuf::Any>(anyObj);

    if(any.Is<dbp::Connected>())
    {
        HandleConnected(pClientSocket,&any);
    }
    else if(any.Is<dbp::Failed>())
    {
        HandleFailed(pClientSocket,&any);
    }
    else if(any.Is<dbp::Ping>())
    {
        if(!HaveAssociatedSessionOf(pClientSocket))
        {
            return;
        }

        if(IsSessionTimeout(pClientSocket))
        {
            HandleClientSocketError(pClientSocket);
            return;
        }
        
        HandlePing(pClientSocket,&any);
    }
    else if(any.Is<dbp::Pong>())
    {
        if(!HaveAssociatedSessionOf(pClientSocket))
        {
            return;
        }

        if(IsSessionTimeout(pClientSocket))
        {
            HandleClientSocketError(pClientSocket);
            return;
        }

        HandlePong(pClientSocket,&any);
    }
    else if(any.Is<dbp::Result>())
    {
        if(!HaveAssociatedSessionOf(pClientSocket))
        {
            return;
        }

        HandleResult(pClientSocket,&any);
    }
    else if(any.Is<dbp::Added>())
    {
        if(!HaveAssociatedSessionOf(pClientSocket))
        {
            return;
        }

        HandleAdded(pClientSocket,&any);
    }
    else if(any.Is<dbp::Ready>())
    {
        if(!HaveAssociatedSessionOf(pClientSocket))
        {
            return;
        }

        HandleReady(pClientSocket,&any);
    }
    else if(any.Is<dbp::Nosub>())
    {
        if(!HaveAssociatedSessionOf(pClientSocket))
        {
            return;
        }

        HandleNoSub(pClientSocket,&any);
    }
    else
    {

    }

}

void CDbpClient::AddNewClient(const CDbpClientConfig& confClient)
{
    vecClientConfig.push_back(confClient);
}

void CDbpClient::HandleConnected(CDbpClientSocket* pClientSocket, google::protobuf::Any* any)
{
    dbp::Connected connected;
    any->UnpackTo(&connected);

    std::cout << "[<] Recved Connected " << connected.session() << " [dbpclient]\n";

    CreateSession(connected.session(),pClientSocket);
    
    if(IsSessionExist(connected.session()))
    {
        StartPingTimer(connected.session());
        SubscribeDefaultTopics(pClientSocket);
    }
}

void CDbpClient::HandleFailed(CDbpClientSocket* pClientSocket, google::protobuf::Any* any)
{   
    dbp::Failed failed;
    any->UnpackTo(&failed);
    
    if(failed.reason() == "002")
    {
        std::cerr << "[<] session timeout: " << failed.reason() << std::endl;    
    }
    
   
    auto epRemote = pClientSocket->GetHost().ToEndPoint();
    HandleClientSocketError(pClientSocket);
    
    auto it = mapProfile.find(epRemote);
    if(it != mapProfile.end())
    {
        StartConnection(epRemote,DBPCLIENT_CONNECT_TIMEOUT, 
            it->second.optSSL.fEnable,it->second.optSSL);
    }
    else
    {
        std::cerr << "cannot find reconnect parent node " << 
            epRemote.address().to_string() << " failed, " 
            << "port " << epRemote.port() << std::endl;
    }
}
    
void CDbpClient::HandlePing(CDbpClientSocket* pClientSocket, google::protobuf::Any* any)
{
    dbp::Ping ping;
    any->UnpackTo(&ping);
    pClientSocket->SendPong(ping.id());
}
    
void CDbpClient::HandlePong(CDbpClientSocket* pClientSocket, google::protobuf::Any* any)
{
    dbp::Pong pong;
    any->UnpackTo(&pong);

    std::string session = bimapSessionClientSocket.right.at(pClientSocket);
    if(IsSessionExist(session))
    {
        mapSessionProfile[session].nTimeStamp = CDbpUtils::CurrentUTC();
    }
}

void CDbpClient::HandleResult(CDbpClientSocket* pClientSocket, google::protobuf::Any* any)
{
    dbp::Result result;
    any->UnpackTo(&result);

    if (!result.error().empty())
    {
        std::cerr << "[<]method error:" << result.error()  <<  " [dbpclient]" << std::endl;    
        return;
    }
}

void CDbpClient::HandleAdded(CDbpClientSocket* pClientSocket, google::protobuf::Any* any)
{
    dbp::Added added;
    any->UnpackTo(&added);

    if(added.name() == "event")
    {
        sn::VPeerNetEvent event;
        added.object().UnpackTo(&event);
        CMvEventDbpVirtualPeerNet* dbpEvent = new CMvEventDbpVirtualPeerNet("");
        dbpEvent->data.type = event.type();
        dbpEvent->data.data = std::vector<uint8>(event.data().begin(), event.data().end());
        pDbpService->PostEvent(dbpEvent);
    }

    // rpc route

    if (added.name() == RPC_CMD_TOPIC)
    {
        sn::RPCRouteEvent routeEvent;
        added.object().UnpackTo(&routeEvent);
        CMvEventRPCRouteAdded* pEvent = new CMvEventRPCRouteAdded("");
        pEvent->data.id = added.id();
        pEvent->data.name = added.name();
        pEvent->data.type = routeEvent.type();
        pEvent->data.vData = std::vector<uint8>(routeEvent.data().begin(), routeEvent.data().end());
        pDbpService->PostEvent(pEvent);
    }
}

void CDbpClient::HandleReady(CDbpClientSocket* pClientSocket, google::protobuf::Any* any)
{
    dbp::Ready ready;
    any->UnpackTo(&ready);
}

void CDbpClient::HandleNoSub(CDbpClientSocket* pClientSocket, google::protobuf::Any* any)
{
    dbp::Nosub nosub;
    any->UnpackTo(&nosub);
}

bool CDbpClient::WalleveHandleInitialize()
{
    // init client config
    for(const auto & confClient : vecClientConfig)
    {
        if(confClient.fEnableSuperNode && !confClient.fEnableForkNode)
        {
            fIsRootNode = true;
            fIsSuperNode = true;
            continue;
        }

        if(confClient.fEnableSuperNode && confClient.fEnableForkNode)
        {
            fIsRootNode = false;
            fIsSuperNode = true;
        }
        
        if(!CreateProfile(confClient))
        {
            return false;
        }
    }

    if(!WalleveGetObject("dbpservice",pDbpService))
    {
        WalleveError("request dbpservice failed in dbpclient.");
        return false;
    }

    return true;
}

void CDbpClient::WalleveHandleDeinitialize()
{
    pDbpService = NULL;
    // delete client config
    mapProfile.clear();
}

void CDbpClient::EnterLoop()
{
    // start resource
    WalleveLog("Dbp Client starting:\n");
}

void CDbpClient::LeaveLoop()
{
    // destory resource
    std::vector<CDbpClientSessionProfile> vProfile;
    for(auto it = mapSessionProfile.begin(); it != mapSessionProfile.end(); ++it)
    {
        vProfile.push_back((*it).second);
    }

    for(const auto& profile : vProfile)
    {
        RemoveClientSocket(profile.pClientSocket);
    }

    WalleveLog("Dbp Client stop\n");
}

void CDbpClient::HeartBeat()
{
    if(fIsSuperNode && !fIsRootNode && !fIsResolved)
    {
        try
        {
            if(!parentHost.ToEndPoint().address().to_string().empty())
                ResolveHost(parentHost);
        }
        catch(const std::exception& e)
        {
            WalleveWarn("DbpClient HeartBeat Warning:\n");
            WalleveWarn(e.what());
        }
    }
}

bool CDbpClient::ClientConnected(CIOClient* pClient)
{
    auto it = mapProfile.find(pClient->GetRemote());
    if(it == mapProfile.end())
    {
        return false;
    }

    WalleveLog("Connect parent node %s success,  port = %d\n",
                       (*it).first.address().to_string().c_str(),
                       (*it).first.port());

    return ActivateConnect(pClient, &(*it).second);
}

void CDbpClient::ClientFailToConnect(const boost::asio::ip::tcp::endpoint& epRemote)
{
    WalleveWarn("Connect parent node %s failed,  port = %d\n reconnectting\n",
                       epRemote.address().to_string().c_str(),
                       epRemote.port());

    std::cerr << "Connect parent node " << 
        epRemote.address().to_string() << " failed, " 
        << "port " << epRemote.port() << " and reconnectting." << std::endl;
    
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    auto it = mapProfile.find(epRemote);
    if(it != mapProfile.end())
    {
        StartConnection(epRemote,DBPCLIENT_CONNECT_TIMEOUT, 
            it->second.optSSL.fEnable,it->second.optSSL);
    }
    else
    {
        std::cerr << "cannot find reconnect parent node " << 
            epRemote.address().to_string() << " failed, " 
            << "port " << epRemote.port() << std::endl;
    }
}
    
void CDbpClient::Timeout(uint64 nNonce,uint32 nTimerId)
{
    std::cerr << "time out" << std::endl;

    WalleveWarn("Connect parent node %s timeout,  nonce = %d  timerid = %d\n",
                       nNonce,
                       nTimerId);
}

void CDbpClient::HostResolved(const CNetHost& host, const boost::asio::ip::tcp::endpoint& ep)
{
    CDbpClientConfig confClient = boost::any_cast<CDbpClientConfig>(host.data);
    CDbpClientProfile profile;
    if (!WalleveGetObject(confClient.strIOModule, profile.pIOModule))
    {
        WalleveError("Failed to request %s\n", confClient.strIOModule.c_str());
        return;
    }

    fIsResolved = true;

    if (confClient.optSSL.fEnable)
        profile.optSSL = confClient.optSSL;

    profile.strPrivateKey = confClient.strPrivateKey;
    profile.epParentHost = confClient.epParentHost;
    profile.nSessionTimeout = confClient.nSessionTimeout;
    mapProfile[ep] = profile;
    confClient.epParentHost = ep;

    bool fEnableSSL = profile.optSSL.fEnable;

    if (!StartConnection(ep, DBPCLIENT_CONNECT_TIMEOUT, fEnableSSL, profile.optSSL))
    {
        WalleveWarn("Start to connect parent node %s failed,  port = %d\n",
                    ep.address().to_string().c_str(),
                    ep.port());
    }
    else
    {
        WalleveLog("Start to connect parent node %s,  port = %d\n",
                   ep.address().to_string().c_str(),
                   ep.port());
    }
}

void CDbpClient::HostFailToResolve(const CNetHost& host)
{
}

bool CDbpClient::CreateProfile(const CDbpClientConfig& confClient)
{
    CDbpClientProfile profile;
    if(!WalleveGetObject(confClient.strIOModule,profile.pIOModule))
    {
        WalleveError("Failed to request %s\n", confClient.strIOModule.c_str());
        return false;
    }
    parentHost = CNetHost(confClient.strParentHost, confClient.nDbpPort, "", confClient);
    return true;
}

bool CDbpClient::StartConnection(const boost::asio::ip::tcp::endpoint& epRemote, int64 nTimeout,bool fEnableSSL,
    const CIOSSLOption& optSSL)
{
    if(!fEnableSSL)
    {
        return Connect(epRemote,nTimeout) ? true : false;
    }
    else
    {
        return SSLConnect(epRemote,nTimeout,optSSL) ? true : false;
    }
}

void CDbpClient::SendPingHandler(const boost::system::error_code& err, const CDbpClientSessionProfile& sessionProfile)
{
    if (err != boost::system::errc::success)
    {
        return;
    }

    if(IsSessionTimeout(sessionProfile.pClientSocket))
    {
        std::cerr << "######### dbp client session time out ############\n";
        HandleClientSocketError(sessionProfile.pClientSocket);
        return;
    }

    std::string utc = std::to_string(CDbpUtils::CurrentUTC());
    sessionProfile.pClientSocket->SendPing(utc);
    
    sessionProfile.ptrPingTimer->expires_at(sessionProfile.ptrPingTimer->expires_at() + boost::posix_time::seconds(3));
    sessionProfile.ptrPingTimer->async_wait(boost::bind(&CDbpClient::SendPingHandler,
                                                        this, boost::asio::placeholders::error,
                                                        boost::ref(sessionProfile)));
}
 
void CDbpClient::StartPingTimer(const std::string& session)
{
    auto& profile = mapSessionProfile[session];
    
    profile.ptrPingTimer = 
        std::make_shared<boost::asio::deadline_timer>(this->GetIoService(),
                                                      boost::posix_time::seconds(3));
    profile.ptrPingTimer->expires_at(profile.ptrPingTimer->expires_at() +
                                        boost::posix_time::seconds(3));
    profile.ptrPingTimer->async_wait(boost::bind(&CDbpClient::SendPingHandler,
                                                    this, boost::asio::placeholders::error,
                                                    boost::ref(profile)));
}

void CDbpClient::SubscribeDefaultTopics(CDbpClientSocket* pClientSocket)
{
    std::vector<std::string> vTopics{ RPC_CMD_TOPIC };
    pClientSocket->SendSubScribeTopics(vTopics);
}

void CDbpClient::CreateSession(const std::string& session, CDbpClientSocket* pClientSocket)
{
    CDbpClientSessionProfile profile;
    profile.strSessionId = session;
    profile.nTimeStamp = CDbpUtils::CurrentUTC();
    profile.pClientSocket = pClientSocket;

    pClientSocket->SetSession(session);

    mapSessionProfile.insert(std::make_pair(session,profile));
    bimapSessionClientSocket.insert(position_pair(session,pClientSocket));
}

bool CDbpClient::HaveAssociatedSessionOf(CDbpClientSocket* pClientSocket)
{
    return bimapSessionClientSocket.right.find(pClientSocket) != bimapSessionClientSocket.right.end();
}

bool CDbpClient::IsSessionExist(const std::string& session)
{
    return mapSessionProfile.find(session) != mapSessionProfile.end();
}

bool CDbpClient::IsSessionTimeout(CDbpClientSocket* pClientSocket)
{
     if (HaveAssociatedSessionOf(pClientSocket))
    {
        auto timeout = pClientSocket->GetProfile()->nSessionTimeout;
        std::string assciatedSession = bimapSessionClientSocket.right.at(pClientSocket);
        uint64 lastTimeStamp = mapSessionProfile[assciatedSession].nTimeStamp;
        return (CDbpUtils::CurrentUTC() - lastTimeStamp > timeout) ? true : false;
    }
    else
    {
        return false;
    }
}

bool CDbpClient::ActivateConnect(CIOClient* pClient, CDbpClientProfile* pProfile)
{
    uint64 nNonce = 0;
    RAND_bytes((unsigned char *)&nNonce, sizeof(nNonce));
    while (mapClientSocket.count(nNonce))
    {
        RAND_bytes((unsigned char *)&nNonce, sizeof(nNonce));
    }
    
    IIOModule* pIOModule = mapProfile[pClient->GetRemote()].pIOModule;
    CDbpClientSocket* pDbpClientSocket = new CDbpClientSocket(pIOModule,nNonce,this,pClient, pProfile);
    if(!pDbpClientSocket)
    {
        std::cerr << "Create Client Socket error\n";
        return false;
    }

    mapClientSocket.insert(std::make_pair(nNonce,pDbpClientSocket));
    
    std::vector<std::string> vSupportForks{};
    
    pDbpClientSocket->SendConnectSession("",vSupportForks);
    
    
    return true;
}

void CDbpClient::CloseConnect(CDbpClientSocket* pClientSocket)
{
    delete pClientSocket;
}

void CDbpClient::RemoveSession(CDbpClientSocket* pClientSocket)
{
    if (HaveAssociatedSessionOf(pClientSocket))
    {
        std::string assciatedSession = bimapSessionClientSocket.right.at(pClientSocket);
        bimapSessionClientSocket.left.erase(assciatedSession);
        bimapSessionClientSocket.right.erase(pClientSocket);
        
        mapSessionProfile[assciatedSession].ptrPingTimer->cancel();
        mapSessionProfile.erase(assciatedSession);
    }
}

void CDbpClient::RemoveClientSocket(CDbpClientSocket* pClientSocket)
{
    RemoveSession(pClientSocket);
    CloseConnect(pClientSocket);
}

CDbpClientSocket* CDbpClient::PickOneSessionSocket() const
{
    CDbpClientSocket* pClientSocket = NULL;
    if(mapSessionProfile.size() > 0)
    {
        pClientSocket = mapSessionProfile.begin()->second.pClientSocket;
    }
    else
    {
        std::cerr << "mapSessionProfile is empty\n";
    }

    return pClientSocket;
}

bool CDbpClient::HandleEvent(CMvEventDbpVirtualPeerNet& event)
{
    CDbpClientSocket* pClientSocket = PickOneSessionSocket();
    if(!pClientSocket) return false;
    
    pClientSocket->SendEvent(event.data);
    return true;
}

// rpc route

void CDbpClientSocket::SendRPCRouteResult(CMvRPCRouteResult& result)
{
    dbp::Method method;
    method.set_id(CDbpUtils::RandomString());
    method.set_method("rpcroute");

    google::protobuf::Any *params = new google::protobuf::Any();
    sn::RPCRouteArgs args;
    args.set_type(result.type);
    args.set_data(std::string(result.vData.begin(), result.vData.end()));
    args.set_rawdata(std::string(result.vRawData.begin(), result.vRawData.end()));
    params->PackFrom(args);
    method.set_allocated_params(params);

    google::protobuf::Any *any = new google::protobuf::Any();
    any->PackFrom(method);

    SendMessage(dbp::Msg::METHOD, any);
}

bool CDbpClient::HandleEvent(CMvEventRPCRouteResult& event)
{
    CDbpClientSocket* pClientSocket = PickOneSessionSocket();
    if(!pClientSocket) return false;
    
    pClientSocket->SendRPCRouteResult(event.data);
    return true;
}

//
} // namespace multiverse