// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_DBP_CLIENT_H
#define MULTIVERSE_DBP_CLIENT_H

#include "walleve/walleve.h"
#include "event.h"
#include "mvbase.h"

#include <boost/bimap.hpp>
#include <boost/any.hpp>
#include <boost/algorithm/string.hpp>
#include <queue>

#include "dbputils.h"

using namespace walleve;

namespace multiverse
{

class CDbpClient;

class CDbpClientConfig
{
public:
    CDbpClientConfig(){}
    CDbpClientConfig(const boost::asio::ip::tcp::endpoint& epParentHostIn,
                    unsigned int nSessionTimeoutIn,
                    const std::string& strPrivateKeyIn,
                    const CIOSSLOption& optSSLIn, 
                    const std::string& strIOModuleIn,
                    bool enableForkNode,
                    bool enableSuperNode)
    : epParentHost(epParentHostIn),
      nSessionTimeout(nSessionTimeoutIn),
      strPrivateKey(strPrivateKeyIn),
      optSSL(optSSLIn),
      strIOModule(strIOModuleIn),
      fEnableForkNode(enableForkNode),
      fEnableSuperNode(enableSuperNode)
    {
    }
public:
    boost::asio::ip::tcp::endpoint epParentHost;
    unsigned int nSessionTimeout;
    std::string strPrivateKey;
    CIOSSLOption optSSL;
    std::string strIOModule;
    bool fEnableForkNode;
    bool fEnableSuperNode;
};

class CDbpClientProfile
{
public:
    CDbpClientProfile() : pIOModule(NULL) {}
public:
    IIOModule* pIOModule;
    CIOSSLOption optSSL;
    boost::asio::ip::tcp::endpoint epParentHost;
    std::string strPrivateKey;
};

class CDbpClientSocket
{
public:
    CDbpClientSocket(IIOModule* pIOModuleIn,const uint64 nNonceIn,
                   CDbpClient* pDbpClientIn,CIOClient* pClientIn);
    ~CDbpClientSocket();

    IIOModule* GetIOModule();
    uint64 GetNonce();
    CNetHost GetHost();
    std::string GetSession() const;
    void SetSession(const std::string& session);
    
    void ReadMessage();
    void SendPong(const std::string& id);
    void SendPing(const std::string& id);

    void SendConnectSession(const std::string& session, const std::vector<std::string>& forks);
    void SendEvent(CMvDbpVirtualPeerNetEvent& dbpEvent);

protected:
    void StartReadHeader();
    void StartReadPayload(std::size_t nLength);
    
    void HandleWritenRequest(std::size_t nTransferred, dbp::Msg type);
    void HandleReadHeader(std::size_t nTransferred);
    void HandleReadPayload(std::size_t nTransferred,uint32_t len);
    void HandleReadCompleted(uint32_t len);

    void SendSubscribeTopic(const std::string& topic);
    void SendMessage(dbp::Msg type, google::protobuf::Any* any);

    bool IsSentComplete();

private:
    std::string strSessionId;

protected:
    IIOModule* pIOModule;
    const uint64 nNonce;
    CDbpClient* pDbpClient;
    CIOClient* pClient;

    CWalleveBufStream ssRecv;
    CWalleveBufStream ssSend;

    std::queue<std::pair<dbp::Msg, std::string>> queueMessage;
    bool IsReading;
};

class CMvSessionProfile
{
public:
    CDbpClientSocket* pClientSocket;
    std::string strSessionId;
    uint64 nTimeStamp;
    std::shared_ptr<boost::asio::deadline_timer> ptrPingTimer;
};

class CDbpClient : public walleve::CIOProc, virtual public CDBPEventListener
{
public:
    CDbpClient();
    virtual ~CDbpClient() noexcept;

    void HandleClientSocketError(CDbpClientSocket* pClientSocket);
    void HandleClientSocketSent(CDbpClientSocket* pClientSocket);
    void HandleClientSocketRecv(CDbpClientSocket* pClientSocket, const boost::any& anyObj);   
    void AddNewClient(const CDbpClientConfig& confClient);

    void HandleConnected(CDbpClientSocket* pClientSocket, google::protobuf::Any* any);
    void HandleFailed(CDbpClientSocket* pClientSocket, google::protobuf::Any* any);
    void HandlePing(CDbpClientSocket* pClientSocket, google::protobuf::Any* any);
    void HandlePong(CDbpClientSocket* pClientSocket, google::protobuf::Any* any);
    void HandleResult(CDbpClientSocket* pClientSocket, google::protobuf::Any* any);
    void HandleAdded(CDbpClientSocket* pClientSocket, google::protobuf::Any* any);
    void HandleReady(CDbpClientSocket* pClientSocket, google::protobuf::Any* any);
    void HandleNoSub(CDbpClientSocket* pClientSocket, google::protobuf::Any* any);

    bool HandleEvent(CMvEventDbpVirtualPeerNet& event) override;

protected:
    bool WalleveHandleInitialize() override;
    void WalleveHandleDeinitialize() override;
    void EnterLoop() override;
    void LeaveLoop() override;

    bool ClientConnected(CIOClient* pClient) override;
    void ClientFailToConnect(const boost::asio::ip::tcp::endpoint& epRemote) override;
    void Timeout(uint64 nNonce,uint32 nTimerId) override;

    bool CreateProfile(const CDbpClientConfig& confClient);
    bool StartConnection(const boost::asio::ip::tcp::endpoint& epRemote, int64 nTimeout, bool fEnableSSL,
            const CIOSSLOption& optSSL);
   
    void StartPingTimer(const std::string& session);
    void SendPingHandler(const boost::system::error_code& err, const CMvSessionProfile& sessionProfile);
    void CreateSession(const std::string& session, CDbpClientSocket* pClientSocket);
    bool HaveAssociatedSessionOf(CDbpClientSocket* pClientSocket);
    bool IsSessionExist(const std::string& session);  

    bool ActivateConnect(CIOClient* pClient);
    void CloseConnect(CDbpClientSocket* pClientSocket);
    void RemoveSession(CDbpClientSocket* pClientSocket);
    void RemoveClientSocket(CDbpClientSocket* pClientSocket);
    CDbpClientSocket* PickOneSessionSocket() const;

protected:
    std::vector<CDbpClientConfig> vecClientConfig;
    std::map<boost::asio::ip::tcp::endpoint, CDbpClientProfile> mapProfile;
    std::map<uint64, CDbpClientSocket*> mapClientSocket; // nonce => CDbpClientSocket

    typedef boost::bimap<std::string, CDbpClientSocket*> SessionClientSocketBimapType;
    typedef SessionClientSocketBimapType::value_type position_pair;
    SessionClientSocketBimapType bimapSessionClientSocket;      // session id <=> CDbpClientSocket
    std::map<std::string, CMvSessionProfile> mapSessionProfile; // session id => session profile


private:
    IIOModule* pDbpService;
};

} // namespace multiverse

#endif // MULTIVERSE_DBP_CLIENT_H