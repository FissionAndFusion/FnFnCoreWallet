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
                    const std::string& strPrivateKeyIn,
                    const CIOSSLOption& optSSLIn, 
                    const std::string& strIOModuleIn,
                    bool enableForkNode,
                    bool enableSuperNode)
    : epParentHost(epParentHostIn),
      strPrivateKey(strPrivateKeyIn),
      optSSL(optSSLIn),
      strIOModule(strIOModuleIn),
      fEnableForkNode(enableForkNode),
      fEnableSuperNode(enableSuperNode)
    {
    }
public:
    boost::asio::ip::tcp::endpoint epParentHost;
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

class CMvDbpClientSocket
{
public:
    CMvDbpClientSocket(IIOModule* pIOModuleIn,const uint64 nNonceIn,
                   CDbpClient* pDbpClientIn,CIOClient* pClientIn);
    ~CMvDbpClientSocket();

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

    void SendForkId(const std::string& fork);
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
    CMvDbpClientSocket* pClientSocket;
    std::string strSessionId;
    uint64 nTimeStamp;
    std::shared_ptr<boost::asio::deadline_timer> ptrPingTimer;
};

class CDbpClient : public walleve::CIOProc, virtual public CDBPEventListener
{
public:
    CDbpClient();
    virtual ~CDbpClient() noexcept;

    void HandleClientSocketError(CMvDbpClientSocket* pClientSocket);
    void HandleClientSocketSent(CMvDbpClientSocket* pClientSocket);
    void HandleClientSocketRecv(CMvDbpClientSocket* pClientSocket, const boost::any& anyObj);   
    void AddNewClient(const CDbpClientConfig& confClient);

    void HandleConnected(CMvDbpClientSocket* pClientSocket, google::protobuf::Any* any);
    void HandleFailed(CMvDbpClientSocket* pClientSocket, google::protobuf::Any* any);
    void HandlePing(CMvDbpClientSocket* pClientSocket, google::protobuf::Any* any);
    void HandlePong(CMvDbpClientSocket* pClientSocket, google::protobuf::Any* any);
    void HandleResult(CMvDbpClientSocket* pClientSocket, google::protobuf::Any* any);
    void HandleAdded(CMvDbpClientSocket* pClientSocket, google::protobuf::Any* any);
    void HandleReady(CMvDbpClientSocket* pClientSocket, google::protobuf::Any* any);
    void HandleNoSub(CMvDbpClientSocket* pClientSocket, google::protobuf::Any* any);

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
    void CreateSession(const std::string& session, CMvDbpClientSocket* pClientSocket);
    bool HaveAssociatedSessionOf(CMvDbpClientSocket* pClientSocket);
    bool IsSessionExist(const std::string& session);  

    bool ActivateConnect(CIOClient* pClient);
    void CloseConnect(CMvDbpClientSocket* pClientSocket);
    void RemoveSession(CMvDbpClientSocket* pClientSocket);
    void RemoveClientSocket(CMvDbpClientSocket* pClientSocket);
    CMvDbpClientSocket* PickOneSessionSocket() const;

protected:
    std::vector<CDbpClientConfig> vecClientConfig;
    std::map<boost::asio::ip::tcp::endpoint, CDbpClientProfile> mapProfile;
    std::map<uint64, CMvDbpClientSocket*> mapClientSocket; // nonce => CDbpClientSocket

    typedef boost::bimap<std::string, CMvDbpClientSocket*> SessionClientSocketBimapType;
    typedef SessionClientSocketBimapType::value_type position_pair;
    SessionClientSocketBimapType bimapSessionClientSocket;      // session id <=> CMvDbpClientSocket
    std::map<std::string, CMvSessionProfile> mapSessionProfile; // session id => session profile


private:
    IIOModule* pDbpService;
};

} // namespace multiverse

#endif // MULTIVERSE_DBP_CLIENT_H