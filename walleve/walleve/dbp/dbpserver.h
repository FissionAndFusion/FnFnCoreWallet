#ifndef WALLEVE_DBP_SERVER_H
#define WALLEVE_DBP_SERVER_H

#include "walleve/netio/ioproc.h"
#include "walleve/dbp/dbpevent.h"


namespace walleve{

class CDbpServer;


class CDbpHostConfig
{
public:
    CDbpHostConfig() {}
    CDbpHostConfig(const boost::asio::ip::tcp::endpoint& epHostIn,unsigned int nMaxConnectionsIn,
                    const CIOSSLOption& optSSLIn,const std::map<std::string,std::string>& mapUserPassIn,
                    const std::vector<std::string> vAllowMaskIn,const std::string& strIOModuleIn)
    : epHost(epHostIn),nMaxConnections(nMaxConnectionsIn),optSSL(optSSLIn),
      mapUserPass(mapUserPassIn),vAllowMask(vAllowMaskIn),strIOModule(strIOModuleIn)
    {
    }
public:
    boost::asio::ip::tcp::endpoint epHost;
    unsigned int nMaxConnections;
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
    void SendResponse(std::string& strResponse);
protected:
    void StartReadHeader();
    void StartReadPayload(std::size_t nLength);

    void HandleReadHeader(std::size_t nTransferred);
    void HandleReadPayload(std::size_t nTransferred);
    void HandleReadCompleted();
    void HandleWritenResponse(std::size_t nTransferred);
protected:
    CDbpServer* pServer;
    CDbpProfile *pProfile;
    CIOClient* pClient;
    uint64 nNonce;
    bool fEventStream;
    CWalleveBufStream ssRecv;
    CWalleveBufStream ssSend;
};

class CDbpServer : public CIOProc, virtual public CWalleveDBPEventListener
{
public:
    CDbpServer();
    virtual ~CDbpServer();
    CIOClient* CreateIOClient(CIOContainer *pContainer);
    void HandleClientRecv(CDbpClient *pDbpClient,void* anyObj);
    void HandleClientSent(CDbpClient *pDbpClient);
    void HandleClientError(CDbpClient *pDbpClient);
    void AddNewHost(const CDbpHostConfig& confHost);
protected:
    bool WalleveHandleInitialize();
    void WalleveHandleDeinitialize();
    void EnterLoop();
    void LeaveLoop();

    bool ClientAccepted(const boost::asio::ip::tcp::endpoint& epService,CIOClient *pClient);

    bool CreateProfile(const CDbpHostConfig& confHost);
    CDbpClient* AddNewClient(CIOClient *pClient,CDbpProfile *pDbpProfile);
    void RemoveClient(CDbpClient *pDbpClient);
    void RespondError(CDbpClient *pDbpClient,int nStatusCode,const std::string& strError = "");
    bool HandleEvent(CWalleveEventDbpRespond& eventRsp);
protected:
    std::vector<CDbpHostConfig> vecHostConfig;
    std::map<boost::asio::ip::tcp::endpoint,CDbpProfile> mapProfile;
    std::map<uint64,CDbpClient*> mapClient;  
};
} //namespace walleve
#endif //WALLEVE_DBP_SERVER_H