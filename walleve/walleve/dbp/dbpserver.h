#ifndef WALLEVE_DBP_SERVER_H
#define WALLEVE_DBP_SERVER_H

#include "walleve/netio/ioproc.h"
#include "walleve/dbp/dbpevent.h"

#include <boost/bimap.hpp>

namespace dbp{
    class Base;
}

namespace walleve{

class CDbpServer;


class CDbpHostConfig
{
public:
    CDbpHostConfig() {}
    CDbpHostConfig(const boost::asio::ip::tcp::endpoint& epHostIn,unsigned int nMaxConnectionsIn,unsigned int nSessionTimeoutIn, 
                    const CIOSSLOption& optSSLIn,const std::map<std::string,std::string>& mapUserPassIn,
                    const std::vector<std::string> vAllowMaskIn,const std::string& strIOModuleIn)
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
    std::map<std::string,std::string> mapUserPass;
    std::vector<std::string> vAllowMask;
    std::string strIOModule;
};

class CDbpProfile
{
public:
    CDbpProfile() : pIOModule(NULL),pSSLContext(NULL) {}
public:
    IIOModule *pIOModule;
    boost::asio::ssl::context* pSSLContext;
    std::map<std::string,std::string> mapAuthrizeUser;
    std::vector<std::string> vAllowMask;
    unsigned int nMaxConnections;
    unsigned int nSessionTimeout;
};

class CDbpClient
{
public:
    CDbpClient(CDbpServer *pServerIn,CDbpProfile *pProfileIn,
                CIOClient* pClientIn,uint64 nonce);
    ~CDbpClient();
    CDbpProfile *GetProfile();
    uint64 GetNonce();
    bool IsEventStream();
    void SetEventStream();
    void Activate();
    void SendResponse(CWalleveDbpConnected& body);
    void SendResponse(CWalleveDbpFailed& body);
    void SendResponse(CWalleveDbpNoSub& body);
    void SendResponse(CWalleveDbpReady& body);
    void SendResponse(CWalleveDbpAdded& body);
    void SendResponse(CWalleveDbpMethodResult& body);
    void SendPing(const std::string& id);
    void SendPong(const std::string& id);
    void SendResponse(int statusCode,const std::string& description);

protected:
    void StartReadHeader();
    void StartReadPayload(std::size_t nLength);

    void HandleReadHeader(std::size_t nTransferred);
    void HandleReadPayload(std::size_t nTransferred);
    void HandleReadCompleted();
    void HandleWritenResponse(std::size_t nTransferred);
private:
    void SendMessage(dbp::Base* pBaseMsg);
protected:
    CDbpServer* pServer;
    CDbpProfile *pProfile;
    CIOClient* pClient;
    uint64 nNonce;
    bool fEventStream;
    CWalleveBufStream ssRecv;
    CWalleveBufStream ssSend;
};

class CSessionProfile
{
public:
    CDbpClient* pDbpClient;
    std::string sessionId;
    uint64 timestamp;
};

class CDbpServer : public CIOProc, virtual public CWalleveDBPEventListener
{
public:
    CDbpServer();
    virtual ~CDbpServer();
    virtual CIOClient* CreateIOClient(CIOContainer *pContainer) override;
    void HandleClientRecv(CDbpClient *pDbpClient,void* anyObj);
    void HandleClientSent(CDbpClient *pDbpClient);
    void HandleClientError(CDbpClient *pDbpClient);
    void RespondError(CDbpClient *pDbpClient,int nStatusCode,const std::string& strError = "");
    void AddNewHost(const CDbpHostConfig& confHost);
protected:
    bool WalleveHandleInitialize() override;
    void WalleveHandleDeinitialize() override;
    void EnterLoop() override;
    void LeaveLoop() override;

    bool ClientAccepted(const boost::asio::ip::tcp::endpoint& epService,CIOClient *pClient) override;

    bool CreateProfile(const CDbpHostConfig& confHost);
    CDbpClient* AddNewClient(CIOClient *pClient,CDbpProfile *pDbpProfile);
    void RemoveClient(CDbpClient *pDbpClient);
    bool HandleEvent(CWalleveEventDbpConnected& event) override;
    bool HandleEvent(CWalleveEventDbpFailed& event) override;
    bool HandleEvent(CWalleveEventDbpNoSub& event) override;
    bool HandleEvent(CWalleveEventDbpReady& event) override;
    bool HandleEvent(CWalleveEventDbpAdded& event) override;
    bool HandleEvent(CWalleveEventDbpMethodResult& event) override;

    bool IsSessionTimeOut(CDbpClient* pDbpClient);

protected:
    std::vector<CDbpHostConfig> vecHostConfig;
    std::map<boost::asio::ip::tcp::endpoint,CDbpProfile> mapProfile;
    std::map<uint64,CDbpClient*> mapClient; // nonce => CDbpClient
   
    typedef boost::bimap<std::string,CDbpClient*> SessionClientBimapType; 
    typedef SessionClientBimapType::value_type  position_pair;
    SessionClientBimapType sessionClientBimap; //session id <=> CDbpClient
    std::map<std::string,CSessionProfile> sessionProfileMap; // session id => session profile
};
} //namespace walleve
#endif //WALLEVE_DBP_SERVER_H