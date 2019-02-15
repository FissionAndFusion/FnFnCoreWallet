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
#include <boost/foreach.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>

static std::size_t MSG_HEADER_LEN = 4;

#define DBPCLIENT_CONNECT_TIMEOUT 10 
namespace multiverse
{

CMvDbpClientSocket::CMvDbpClientSocket(IIOModule* pIOModuleIn,const uint64 nNonceIn,
                   CDbpClient* pDbpClientIn,CIOClient* pClientIn)
: pIOModule(pIOModuleIn),
  nNonce(nNonceIn),
  pDbpClient(pDbpClientIn),
  pClient(pClientIn),
  IsReading(false)
{
    ssRecv.Clear();
}
    
CMvDbpClientSocket::~CMvDbpClientSocket()
{
    if(pClient)
    {
        pClient->Close();
    }
}

IIOModule* CMvDbpClientSocket::GetIOModule()
{
    return pIOModule;
}
    
uint64 CMvDbpClientSocket::GetNonce()
{
    return nNonce;
}
    
CNetHost CMvDbpClientSocket::GetHost()
{
    return CNetHost(pClient->GetRemote());
}

std::string CMvDbpClientSocket::GetSession() const
{
    return strSessionId;
}

void CMvDbpClientSocket::SetSession(const std::string& session)
{
    strSessionId = session;
}

void CMvDbpClientSocket::ReadMessage()
{
    StartReadHeader();
}

void CMvDbpClientSocket::SendPong(const std::string& id)
{
    dbp::Pong pong;
    pong.set_id(id);

    google::protobuf::Any *any = new google::protobuf::Any();
    any->PackFrom(pong);

    SendMessage(dbp::Msg::PONG,any);
}

void CMvDbpClientSocket::SendPing(const std::string& id)
{
    dbp::Ping ping;
    ping.set_id(id);

    google::protobuf::Any *any = new google::protobuf::Any();
    any->PackFrom(ping);

    SendMessage(dbp::Msg::PING,any);
}

void CMvDbpClientSocket::SendConnectSession(const std::string& session, const std::vector<std::string>& forks)
{
    dbp::Connect connect;
    connect.set_session(session);
    connect.set_client("supernode");
    connect.set_version(1);

    google::protobuf::Any *any = new google::protobuf::Any();
    any->PackFrom(connect);

    SendMessage(dbp::Msg::CONNECT,any);
}

void CMvDbpClientSocket::SendEvent(CMvDbpVirtualPeerNetEvent& dbpEvent)
{
    dbp::Method method;
    method.set_id(CDbpUtils::RandomString());
    method.set_method("sendevent");

    google::protobuf::Any *params = new google::protobuf::Any();
    sn::VPeerNetEvent event;
    event.set_type(dbpEvent.type);
    event.set_data(std::string(dbpEvent.data.begin(), dbpEvent.data.end()));
    params->PackFrom(event);
    method.set_allocated_params(params);

    google::protobuf::Any *any = new google::protobuf::Any();
    any->PackFrom(method);

    SendMessage(dbp::Msg::METHOD, any);
}

bool CMvDbpClientSocket::IsSentComplete()
{
    return (ssSend.GetSize() == 0 && queueMessage.empty());
}

void CMvDbpClientSocket::SendMessage(dbp::Msg type, google::protobuf::Any* any)
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
    pClient->Write(ssSend,boost::bind(&CMvDbpClientSocket::HandleWritenRequest,this,_1, type));
    
}

void CMvDbpClientSocket::StartReadHeader()
{
    IsReading = true;
    pClient->Read(ssRecv, MSG_HEADER_LEN,
                  boost::bind(&CMvDbpClientSocket::HandleReadHeader, this, _1));
}

void CMvDbpClientSocket::StartReadPayload(std::size_t nLength)
{
    pClient->Read(ssRecv, nLength,
                  boost::bind(&CMvDbpClientSocket::HandleReadPayload, this, _1, nLength));
}

void CMvDbpClientSocket::HandleWritenRequest(std::size_t nTransferred, dbp::Msg type)
{ 
    if(nTransferred != 0)
    {
        if(ssSend.GetSize() != 0)
        {
            pClient->Write(ssSend,boost::bind(&CMvDbpClientSocket::HandleWritenRequest,this,_1, type));
            return;
        }

        if(ssSend.GetSize() == 0 && !queueMessage.empty())
        {
            auto messagePair = queueMessage.front();
            dbp::Msg msgType = messagePair.first;
            std::string bytes = messagePair.second;
            queueMessage.pop();
            ssSend.Write((char*)bytes.data(),bytes.size());
            pClient->Write(ssSend,boost::bind(&CMvDbpClientSocket::HandleWritenRequest,this,_1, msgType));
            return;
        }

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
    
void CMvDbpClientSocket::HandleReadHeader(std::size_t nTransferred)
{   
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
    
void CMvDbpClientSocket::HandleReadPayload(std::size_t nTransferred,uint32_t len)
{
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

void CMvDbpClientSocket::HandleReadCompleted(uint32_t len)
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
}

CDbpClient::~CDbpClient() noexcept
{
}


void CDbpClient::HandleClientSocketError(CMvDbpClientSocket* pClientSocket)
{
    std::cerr << "Client Socket Error." << std::endl; 
    
    CMvEventDbpBroken *pEventDbpBroken = new CMvEventDbpBroken(pClientSocket->GetSession());
    if(pEventDbpBroken)
    {
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

void CDbpClient::HandleClientSocketSent(CMvDbpClientSocket* pClientSocket)
{
    pClientSocket->ReadMessage();
}

void CDbpClient::HandleClientSocketRecv(CMvDbpClientSocket* pClientSocket, const boost::any& anyObj)
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
        
        HandlePing(pClientSocket,&any);
    }
    else if(any.Is<dbp::Pong>())
    {
        if(!HaveAssociatedSessionOf(pClientSocket))
        {
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

void CDbpClient::HandleConnected(CMvDbpClientSocket* pClientSocket, google::protobuf::Any* any)
{
    dbp::Connected connected;
    any->UnpackTo(&connected);
    CreateSession(connected.session(),pClientSocket);
    
    if(IsSessionExist(connected.session()))
    {
        StartPingTimer(connected.session());
    }
}

void CDbpClient::HandleFailed(CMvDbpClientSocket* pClientSocket, google::protobuf::Any* any)
{   
    dbp::Failed failed;
    any->UnpackTo(&failed);
    
    if(failed.reason() == "002")
    {
        std::cerr << "[<] session timeout: " << failed.reason() << std::endl;    
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
    
void CDbpClient::HandlePing(CMvDbpClientSocket* pClientSocket, google::protobuf::Any* any)
{
    dbp::Ping ping;
    any->UnpackTo(&ping);
    pClientSocket->SendPong(ping.id());
}
    
void CDbpClient::HandlePong(CMvDbpClientSocket* pClientSocket, google::protobuf::Any* any)
{
    dbp::Pong pong;
    any->UnpackTo(&pong);

    std::string session = bimapSessionClientSocket.right.at(pClientSocket);
    if(IsSessionExist(session))
    {
        mapSessionProfile[session].nTimeStamp = CDbpUtils::CurrentUTC();
    }
}

void CDbpClient::HandleResult(CMvDbpClientSocket* pClientSocket, google::protobuf::Any* any)
{
    dbp::Result result;
    any->UnpackTo(&result);

    if (!result.error().empty())
    {
        std::cerr << "[<]method error:" << result.error()  <<  " [dbpclient]" << std::endl;    
        return;
    }
}

void CDbpClient::HandleAdded(CMvDbpClientSocket* pClientSocket, google::protobuf::Any* any)
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
}

void CDbpClient::HandleReady(CMvDbpClientSocket* pClientSocket, google::protobuf::Any* any)
{
    dbp::Ready ready;
    any->UnpackTo(&ready);
}

void CDbpClient::HandleNoSub(CMvDbpClientSocket* pClientSocket, google::protobuf::Any* any)
{
    dbp::Nosub nosub;
    any->UnpackTo(&nosub);
}

bool CDbpClient::WalleveHandleInitialize()
{
    // init client config
    for(const auto & confClient : vecClientConfig)
    {
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
    
    for(std::map<boost::asio::ip::tcp::endpoint, CDbpClientProfile>::iterator it = mapProfile.begin();
         it != mapProfile.end(); ++it)
    {
        bool fEnableSSL = (*it).second.optSSL.fEnable;

        if((*it).second.epParentHost.address().is_loopback())
        {
            continue;
        }
       
        if(!StartConnection(it->first,DBPCLIENT_CONNECT_TIMEOUT,fEnableSSL,it->second.optSSL))
        {
            WalleveWarn("Start to connect parent node %s failed,  port = %d\n",
                       (*it).first.address().to_string().c_str(),
                       (*it).first.port());
        }
        else
        {
            WalleveLog("Start to connect parent node %s,  port = %d\n",
                    (*it).first.address().to_string().c_str(),
                    (*it).first.port());
        } 
       
    }
}

void CDbpClient::LeaveLoop()
{
    // destory resource
    std::vector<CMvSessionProfile> vProfile;
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

    return ActivateConnect(pClient);
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

bool CDbpClient::CreateProfile(const CDbpClientConfig& confClient)
{
    CDbpClientProfile profile;
    if(!WalleveGetObject(confClient.strIOModule,profile.pIOModule))
    {
        WalleveError("Failed to request %s\n", confClient.strIOModule.c_str());
        return false;
    }

    if(confClient.optSSL.fEnable)
        profile.optSSL = confClient.optSSL;
    
    profile.strPrivateKey = confClient.strPrivateKey;
    profile.epParentHost = confClient.epParentHost;
    mapProfile[confClient.epParentHost] = profile;

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

void CDbpClient::SendPingHandler(const boost::system::error_code& err, const CMvSessionProfile& sessionProfile)
{
    if (err != boost::system::errc::success)
    {
        return;
    }

    if(!HaveAssociatedSessionOf(sessionProfile.pClientSocket))
    {
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

void CDbpClient::CreateSession(const std::string& session, CMvDbpClientSocket* pClientSocket)
{
    CMvSessionProfile profile;
    profile.strSessionId = session;
    profile.nTimeStamp = CDbpUtils::CurrentUTC();
    profile.pClientSocket = pClientSocket;

    pClientSocket->SetSession(session);

    mapSessionProfile.insert(std::make_pair(session,profile));
    bimapSessionClientSocket.insert(position_pair(session,pClientSocket));
}

bool CDbpClient::HaveAssociatedSessionOf(CMvDbpClientSocket* pClientSocket)
{
    return bimapSessionClientSocket.right.find(pClientSocket) != bimapSessionClientSocket.right.end();
}

bool CDbpClient::IsSessionExist(const std::string& session)
{
    return mapSessionProfile.find(session) != mapSessionProfile.end();
}

bool CDbpClient::ActivateConnect(CIOClient* pClient)
{
  
    uint64 nNonce = 0;
    RAND_bytes((unsigned char *)&nNonce, sizeof(nNonce));
    while (mapClientSocket.count(nNonce))
    {
        RAND_bytes((unsigned char *)&nNonce, sizeof(nNonce));
    }
    
    IIOModule* pIOModule = mapProfile[pClient->GetRemote()].pIOModule;
    CMvDbpClientSocket* pDbpClientSocket = new CMvDbpClientSocket(pIOModule,nNonce,this,pClient);
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

void CDbpClient::CloseConnect(CMvDbpClientSocket* pClientSocket)
{
    delete pClientSocket;
}

void CDbpClient::RemoveSession(CMvDbpClientSocket* pClientSocket)
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

void CDbpClient::RemoveClientSocket(CMvDbpClientSocket* pClientSocket)
{
    RemoveSession(pClientSocket);
    CloseConnect(pClientSocket);
}

CMvDbpClientSocket* CDbpClient::PickOneSessionSocket() const
{
    CMvDbpClientSocket* pClientSocket = NULL;
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
    CMvDbpClientSocket* pClientSocket = PickOneSessionSocket();
    if(!pClientSocket) return false;
    
    pClientSocket->SendEvent(event.data);
    return true;
}

} // namespace multiverse