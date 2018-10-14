// Copyright (c) 2017-2018 The Multiverse developers
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

static int MSG_HEADER_LEN = 4;

#define DBPCLIENT_CONNECT_TIMEOUT 10 
namespace multiverse
{

CMvDbpClientSocket::CMvDbpClientSocket(IIOModule* pIOModuleIn,const uint64 nNonceIn,
                   CMvDbpClient* pDbpClientIn,CIOClient* pClientIn)
: pIOModule(pIOModuleIn),
  nNonce(nNonceIn),
  pDbpClient(pDbpClientIn),
  pClient(pClientIn)
{

}
    
CMvDbpClientSocket::~CMvDbpClientSocket()
{

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
    
void CMvDbpClientSocket::WriteMessage()
{
    pClient->Write(ssSend,boost::bind(&CMvDbpClientSocket::HandleWritenRequest,this,_1));
}

void CMvDbpClientSocket::SendConnectSession(const std::string& session, const std::vector<std::string>& forks)
{
    dbp::Connect connect;
    connect.set_session(session);
    connect.set_version(1);

    google::protobuf::Any anyFork;
    lws::ForkID forkidMsg;

    for(const auto& fork : forks)
    {
        forkidMsg.add_ids(fork);
    }
    
    anyFork.PackFrom(forkidMsg);
    (*connect.mutable_udata())["supernode-forks"] = anyFork;

    google::protobuf::Any *any = new google::protobuf::Any();
    any->PackFrom(connect);

    SendMessage(dbp::Msg::CONNECT,any);
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
  
    ssSend.Write((char*)bytes.data(),bytes.size());
    
    WriteMessage();
}

void CMvDbpClientSocket::StartReadHeader()
{
    pClient->Read(ssRecv, MSG_HEADER_LEN,
                  boost::bind(&CMvDbpClientSocket::HandleReadHeader, this, _1));
}

void CMvDbpClientSocket::StartReadPayload(std::size_t nLength)
{
     pClient->Read(ssRecv, nLength,
                  boost::bind(&CMvDbpClientSocket::HandleReadPayload, this, _1, nLength));
}

void CMvDbpClientSocket::HandleWritenRequest(std::size_t nTransferred)
{ 
    
    if(ssSend.GetSize() != 0)
    {
        pClient->Write(ssSend,boost::bind(&CMvDbpClientSocket::HandleWritenRequest,this,_1));
        return;
    }

    if(nTransferred != 0)
    {
        StartReadHeader();
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
        std::string lenBuffer(MSG_HEADER_LEN, 0);
        ssRecv.Read(&lenBuffer[0], MSG_HEADER_LEN);

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
    std::string payloadBuffer(len, 0);
    ssRecv.Read(&payloadBuffer[0], len);

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
    default:
        std::cerr << "is not Message Base Type is unknown. [dbpclient]" << std::endl;
        pDbpClient->HandleClientSocketError(this);
        break;
    }

}

CMvDbpClient::CMvDbpClient()
  : walleve::CIOProc("dbpclient")
{
}

CMvDbpClient::~CMvDbpClient(){}


void CMvDbpClient::HandleClientSocketError(CMvDbpClientSocket* pClientSocket)
{
    std::cerr << "Client Socket Error." << std::endl;
    CloseConnect(pClientSocket);
}

void CMvDbpClient::HandleClientSocketRecv(CMvDbpClientSocket* pClientSocket, const boost::any& anyObj)
{
    if(anyObj.type() != typeid(google::protobuf::Any))
    {
        return;
    }

    google::protobuf::Any any = boost::any_cast<google::protobuf::Any>(anyObj);

    if(any.Is<dbp::Connected>())
    {
        dbp::Connected connected;
        any.UnpackTo(&connected);
        std::cout << "[<]connected session is: " << connected.session() << std::endl;
    }
    else if(any.Is<dbp::Failed>())
    {
        dbp::Failed failed;
        any.UnpackTo(&failed);
        std::cout << "[<]connect session failed: " << failed.reason() << std::endl;
    }
}

void CMvDbpClient::AddNewClient(const CDbpClientConfig& confClient)
{
    vecClientConfig.push_back(confClient);
}

bool CMvDbpClient::WalleveHandleInitialize()
{
    // init client config
    for(const auto & confClient : vecClientConfig)
    {
        if(!CreateProfile(confClient))
        {
            return false;
        }
    }

    return true;
}

void CMvDbpClient::WalleveHandleDeinitialize()
{
    // delete client config
}

void CMvDbpClient::EnterLoop()
{
    // start resource
    WalleveLog("Dbp Client starting:\n");
    
    for(std::map<boost::asio::ip::tcp::endpoint, CDbpClientProfile>::iterator it = mapProfile.begin();
         it != mapProfile.end(); ++it)
    {
        bool fEnableSSL = (*it).second.optSSL.fEnable;
        if(!StartConnection(it->first,DBPCLIENT_CONNECT_TIMEOUT,fEnableSSL,it->second.optSSL))
        {
            WalleveLog("Start to connect parent node %s failed,  port = %d\n",
                       (*it).first.address().to_string().c_str(),
                       (*it).first.port());
        }
        else
        {
            WalleveLog("Start to connect parent node %s success,  port = %d\n",
                       (*it).first.address().to_string().c_str(),
                       (*it).first.port());
        }  
    }
}

void CMvDbpClient::LeaveLoop()
{
    WalleveLog("Dbp Client stop\n");
    // destory resource
}

bool CMvDbpClient::ClientConnected(CIOClient* pClient)
{
    auto it = mapProfile.find(pClient->GetRemote());
    if(it == mapProfile.end())
    {
        return false;
    }

    WalleveLog("Connect parent node %s success,  port = %d\n",
                       (*it).first.address().to_string().c_str(),
                       (*it).first.port());
    
    std::cout << "Connect parent node" << 
        (*it).first.address().to_string() << "success, " 
        << "port " << (*it).first.port() << std::endl;

    return ActivateConnect(pClient);
}

void CMvDbpClient::ClientFailToConnect(const boost::asio::ip::tcp::endpoint& epRemote)
{
    WalleveLog("Connect parent node %s failed,  port = %d\n reconnectting",
                       epRemote.address().to_string().c_str(),
                       epRemote.port());

    std::cerr << "Connect parent node" << 
        epRemote.address().to_string() << "failed, " 
        << "port " << epRemote.port() << std::endl;
    
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
    
void CMvDbpClient::Timeout(uint64 nNonce,uint32 nTimerId)
{
    std::cerr << "time out" << std::endl;

    WalleveLog("Connect parent node %s timeout,  nonce = %d  timerid = %d\n",
                       nNonce,
                       nTimerId);
}

bool CMvDbpClient::CreateProfile(const CDbpClientConfig& confClient)
{
    CDbpClientProfile profile;
    if(!WalleveGetObject(confClient.strIOModule,profile.pIOModule))
    {
        WalleveLog("Failed to request %s\n", confClient.strIOModule.c_str());
        return false;
    }

    if(confClient.optSSL.fEnable)
        profile.optSSL = confClient.optSSL;
    
    profile.vSupportForks = confClient.vSupportForks;
    
    mapProfile[confClient.epParentHost] = profile;

    return true;
}

bool CMvDbpClient::StartConnection(const boost::asio::ip::tcp::endpoint& epRemote, int64 nTimeout,bool fEnableSSL,
    const CIOSSLOption& optSSL)
{
    if(fEnableSSL)
    {
        return Connect(epRemote,nTimeout) ? true : false;
    }
    else
    {
        return SSLConnect(epRemote,nTimeout,optSSL) ? true : false;
    }
}

bool CMvDbpClient::ActivateConnect(CIOClient* pClient)
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
        return false;
    }

    mapClientSocket.insert(std::make_pair(nNonce,pDbpClientSocket));
    
    std::vector<std::string> vSupportForks = mapProfile[pClient->GetRemote()].vSupportForks;
    
    pDbpClientSocket->SendConnectSession("",vSupportForks);
    
    return true;
}

void CMvDbpClient::CloseConnect(CMvDbpClientSocket* pClientSocket)
{
    delete pClientSocket;
}


} // namespace multiverse