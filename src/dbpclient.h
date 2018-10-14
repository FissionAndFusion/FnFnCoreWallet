// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_DBP_CLIENT_H
#define MULTIVERSE_DBP_CLIENT_H

#include "walleve/walleve.h"

#include <boost/bimap.hpp>
#include <boost/any.hpp>

#include "dbp.pb.h"
#include "lws.pb.h"

using namespace walleve;

namespace multiverse
{

class CMvDbpClient;

class CDbpClientConfig
{
public:
    CDbpClientConfig(){}
    CDbpClientConfig(const boost::asio::ip::tcp::endpoint& epParentHostIn,
                    const std::string&  SupportForksIn,
                    const CIOSSLOption& optSSLIn, 
                    const std::string& strIOModuleIn)
    : epParentHost(epParentHostIn),
      optSSL(optSSLIn),
      strIOModule(strIOModuleIn)
    {
        std::istringstream ss(SupportForksIn);
        std::string s;    
        while (std::getline(ss, s, ';')) 
        {
            vSupportForks.push_back(s);
        }
    }
public:
    boost::asio::ip::tcp::endpoint epParentHost;
    std::vector<std::string> vSupportForks;
    CIOSSLOption optSSL;
    std::string strIOModule;
};

class CDbpClientProfile
{
public:
    CDbpClientProfile() : pIOModule(NULL) {}
public:
    IIOModule* pIOModule;
    CIOSSLOption optSSL;
    std::vector<std::string> vSupportForks;
};

class CMvDbpClientSocket
{
public:
    CMvDbpClientSocket(IIOModule* pIOModuleIn,const uint64 nNonceIn,
                   CMvDbpClient* pDbpClientIn,CIOClient* pClientIn);
    ~CMvDbpClientSocket();

    IIOModule* GetIOModule();
    uint64 GetNonce();
    CNetHost GetHost();
    void WriteMessage();

    void SendConnectSession(const std::string& session, const std::vector<std::string>& forks);
protected:
    void StartReadHeader();
    void StartReadPayload(std::size_t nLength);
    
    void HandleWritenRequest(std::size_t nTransferred);
    void HandleReadHeader(std::size_t nTransferred);
    void HandleReadPayload(std::size_t nTransferred,uint32_t len);
    void HandleReadCompleted(uint32_t len);

    void SendMessage(dbp::Msg type, google::protobuf::Any* any);
protected:
    IIOModule* pIOModule;
    const uint64 nNonce;
    CMvDbpClient* pDbpClient;
    CIOClient* pClient;

    CWalleveBufStream ssRecv;
    CWalleveBufStream ssSend;
};

class CMvDbpClient : public walleve::CIOProc
{
public:
    CMvDbpClient();
    virtual ~CMvDbpClient() noexcept;

    void HandleClientSocketError(CMvDbpClientSocket* pClientSocket);
    void HandleClientSocketRecv(CMvDbpClientSocket* pClientSocket, const boost::any& anyObj);
    void AddNewClient(const CDbpClientConfig& confClient);
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

    bool ActivateConnect(CIOClient* pClient);
    void CloseConnect(CMvDbpClientSocket* pClientSocket);

protected:
    std::vector<CDbpClientConfig> vecClientConfig;
    std::map<boost::asio::ip::tcp::endpoint, CDbpClientProfile> mapProfile;
    std::map<uint64, CMvDbpClientSocket*> mapClientSocket;
};

} // namespace multiverse

#endif // MULTIVERSE_DBP_CLIENT_H