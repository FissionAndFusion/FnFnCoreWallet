// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_DBP_SERVER_H
#define MULTIVERSE_DBP_SERVER_H

#include "walleve/netio/ioproc.h"
#include "event.h"

#include <boost/bimap.hpp>
#include <boost/any.hpp>

#include "dbputils.h"

using namespace walleve;

namespace multiverse
{

class CDbpServer;

class CDbpHostConfig
{
public:
    CDbpHostConfig() {}
    CDbpHostConfig(const boost::asio::ip::tcp::endpoint& epHostIn, unsigned int nMaxConnectionsIn, unsigned int nSessionTimeoutIn,
                 const CIOSSLOption& optSSLIn, const std::map<std::string, std::string>& mapUserPassIn,
                 const std::vector<std::string>& vAllowMaskIn, const std::string& strIOModuleIn)
      : epHost(epHostIn),
        nMaxConnections(nMaxConnectionsIn),
        nSessionTimeout(nSessionTimeoutIn),
        optSSL(optSSLIn),
        mapUserPass(mapUserPassIn),
        vAllowMask(vAllowMaskIn),
        strIOModule(strIOModuleIn)
    {
    }

public:
    boost::asio::ip::tcp::endpoint epHost;
    unsigned int nMaxConnections;
    unsigned int nSessionTimeout;
    CIOSSLOption optSSL;
    std::map<std::string, std::string> mapUserPass;
    std::vector<std::string> vAllowMask;
    std::string strIOModule;
};

class CDbpProfile
{
public:
    CDbpProfile() : pIOModule(NULL), pSSLContext(NULL) {}

public:
    IIOModule* pIOModule;
    boost::asio::ssl::context* pSSLContext;
    std::map<std::string, std::string> mapAuthrizeUser;
    std::vector<std::string> vAllowMask;
    unsigned int nMaxConnections;
    unsigned int nSessionTimeout;
};

class CDbpServerSocket
{
public:
    CDbpServerSocket(CDbpServer* pServerIn, CDbpProfile* pProfileIn,
             CIOClient* pClientIn, uint64 nonce);
    ~CDbpServerSocket();

    CDbpProfile* GetProfile();
    uint64 GetNonce();

    std::string GetSession() const;
    void SetSession(const std::string& session);

    void Activate();
    void SendResponse(CMvDbpConnected& body);
    void SendResponse(CMvDbpFailed& body);
    void SendResponse(CMvDbpNoSub& body);
    void SendResponse(CMvDbpReady& body);
    void SendResponse(const std::string& client, CMvDbpAdded& body);
    void SendResponse(const std::string& client, CMvDbpMethodResult& body);
    void SendResponse(const std::string& client, CMvRPCRouteAdded& body);
    void SendPong(const std::string& id);
    void SendPing(const std::string& id);
    void SendResponse(const std::string& reason, const std::string& description);
    void SendMessage(dbp::Msg type, google::protobuf::Any* any);

protected:
    void StartReadHeader();
    void StartReadPayload(std::size_t nLength);

    void HandleReadHeader(std::size_t nTransferred);
    void HandleReadPayload(std::size_t nTransferred, uint32_t len);
    void HandleReadCompleted(uint32_t len);
    void HandleWritenResponse(std::size_t nTransferred, dbp::Msg type);

    bool IsSentComplete();

private:
    std::string strSessionId;
    std::queue<std::pair<dbp::Msg,std::string>> queueMessage;
    bool IsReading;

protected:
    CDbpServer* pServer;
    CDbpProfile* pProfile;
    CIOClient* pClient;
    uint64 nNonce;

    CWalleveBufStream ssSend;
    CWalleveBufStream ssRecv;
};

class CSessionProfile
{
public:
    CDbpServerSocket* pDbpClient;
    std::string strSessionId;
    std::string strClient;
    uint64 nTimeStamp;
    std::shared_ptr<boost::asio::deadline_timer> ptrPingTimer;
    std::string strForkId; // for lws
    std::set<std::string> setChildForks; // for super node
};

class CDbpServer : public CIOProc, virtual public CDBPEventListener, virtual public CMvDBPEventListener
{
public:
    CDbpServer();
    virtual ~CDbpServer() noexcept;
    virtual CIOClient* CreateIOClient(CIOContainer* pContainer) override;

    void HandleClientRecv(CDbpServerSocket* pDbpClient, const boost::any& anyObj);
    void HandleClientSent(CDbpServerSocket* pDbpClient);
    void HandleClientError(CDbpServerSocket* pDbpClient);

    void HandleClientConnect(CDbpServerSocket* pDbpClient, google::protobuf::Any* any);
    void HandleClientSub(CDbpServerSocket* pDbpClient, google::protobuf::Any* any);
    void HandleClientUnSub(CDbpServerSocket* pDbpClient, google::protobuf::Any* any);
    void HandleClientMethod(CDbpServerSocket* pDbpClient, google::protobuf::Any* any);
    void HandleClientPing(CDbpServerSocket* pDbpClient, google::protobuf::Any* any);
    void HandleClientPong(CDbpServerSocket* pDbpClient, google::protobuf::Any* any);

    void RespondError(CDbpServerSocket* pDbpClient, const std::string& reason, const std::string& strError = "");
    void RespondFailed(CDbpServerSocket* pDbpClient, const std::string& reason);

    void AddNewHost(const CDbpHostConfig& confHost);

protected:
    bool WalleveHandleInitialize() override;
    void WalleveHandleDeinitialize() override;
    void EnterLoop() override;
    void LeaveLoop() override;

    bool ClientAccepted(const boost::asio::ip::tcp::endpoint& epService, CIOClient* pClient) override;

    bool CreateProfile(const CDbpHostConfig& confHost);
    CDbpServerSocket* AddNewClient(CIOClient* pClient, CDbpProfile* pDbpProfile);
    void RemoveClient(CDbpServerSocket* pDbpClient);
    void RemoveSession(CDbpServerSocket* pDbpClient);

    bool HandleEvent(CMvEventDbpConnected& event) override;
    bool HandleEvent(CMvEventDbpFailed& event) override;
    bool HandleEvent(CMvEventDbpNoSub& event) override;
    bool HandleEvent(CMvEventDbpReady& event) override;
    bool HandleEvent(CMvEventDbpAdded& event) override;
    bool HandleEvent(CMvEventDbpMethodResult& event) override;

    bool HandleEvent(CMvEventRPCRouteAdded & event) override;

    bool IsSessionTimeOut(CDbpServerSocket* pDbpClient);
    bool GetSessionForkId(CDbpServerSocket* pDbpClient, std::string& forkid);
    bool IsSessionReconnect(const std::string& session);
    bool IsSessionExist(const std::string& session);
    bool HaveAssociatedSessionOf(CDbpServerSocket* pDbpClient);

    std::string GetUdata(dbp::Connect* pConnect, const std::string& keyName);
    std::string GenerateSessionId();
    void CreateSession(const std::string& session, const std::string& client, const std::string& forkID, CDbpServerSocket* pDbpClient);
    void UpdateSession(const std::string& session, CDbpServerSocket* pDbpClient);

    void SendPingHandler(const boost::system::error_code& err, const CSessionProfile& sessionProfile);

protected:
    std::vector<CDbpHostConfig> vecHostConfig;
    std::map<boost::asio::ip::tcp::endpoint, CDbpProfile> mapProfile;
    std::map<uint64, CDbpServerSocket*> mapClient; // nonce => CDbpServerSocket

    typedef boost::bimap<std::string, CDbpServerSocket*> SessionClientBimapType;
    typedef SessionClientBimapType::value_type position_pair;
    SessionClientBimapType bimapSessionClient;                // session id <=> CDbpServerSocket
    std::map<std::string, CSessionProfile> mapSessionProfile; // session id => session profile
};
} //namespace multiverse
#endif //MULTIVERSE_DBP_SERVER_H
