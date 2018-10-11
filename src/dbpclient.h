// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_DBP_CLIENT_H
#define MULTIVERSE_DBP_CLIENT_H

#include "walleve/walleve.h"

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
    CMvDbpClientSocket(const std::string& strIOModuleIn,const uint64 nNonceIn,
                   CMvDbpClient* pDbpClientIn,CIOClient* pClientIn);
    ~CMvDbpClientSocket();

    const std::string& GetIOModule();
    uint64 GetNonce();
    CNetHost GetHost();
    void Activate();
protected:
    void StartReadHeader();
    void StartReadPayload(std::size_t nLength);
    
    void HandleWritenRequest(std::size_t nTransferred);
    void HandleReadHeader(std::size_t nTransferred);
    void HandleReadPayload(std::size_t nTransferred,uint32_t len);
    void HandleReadCompleted(uint32_t len);
protected:
    const std::string strIOModule;
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

protected:
    std::vector<CDbpClientConfig> vecClientConfig;
    std::map<boost::asio::ip::tcp::endpoint, CDbpClientProfile> mapProfile;
};

} // namespace multiverse

#endif // MULTIVERSE_DBP_CLIENT_H