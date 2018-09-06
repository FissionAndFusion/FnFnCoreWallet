#ifndef MULTIVERSE_DNSEED_H
#define MULTIVERSE_DNSEED_H

#include "config.h"
#include "mvbase.h"
#include "mvpeernet.h"
#include <boost/asio.hpp>
#include "walleve/netio/netio.h"

namespace multiverse
{

class CDNSeed: public network::CMvPeerNet
{
public:
    CDNSeed();
    ~CDNSeed();
    bool CheckPeerVersion(uint32 nVersionIn,uint64 nServiceIn,const std::string& subVersionIn)override;
    virtual int GetPrimaryChainHeight();
    virtual bool HandlePeerRecvMessage(walleve::CPeer *pPeer,int nChannel,int nCommand,
                               walleve::CWalleveBufStream& ssPayload)override;
    virtual bool HandlePeerHandshaked(walleve::CPeer *pPeer,uint32 nTimerId)override;
    virtual void BuildHello(walleve::CPeer *pPeer,walleve::CWalleveBufStream& ssPayload)override;
    virtual void dnseedTestConnSuccess(walleve::CPeer *pPeer)override;
protected:
    bool WalleveHandleInitialize();
    void WalleveHandleDeinitialize();
    virtual void ClientFailToConnect(const boost::asio::ip::tcp::endpoint& epRemote)override;
    virtual void DestroyPeer(walleve::CPeer* pPeer) override;
    virtual void ProcessAskFor(walleve::CPeer* pPeer) override;
    virtual walleve::CPeer* CreatePeer(walleve::CIOClient *pClient,uint64 nNonce,bool fInBound)override;

    const CMvNetworkConfig * NetworkConfig()
    {
        return dynamic_cast<const CMvNetworkConfig *>(walleve::IWalleveBase::WalleveConfig());
    }
    const CMvStorageConfig * StorageConfig()
    {
        return dynamic_cast<const CMvStorageConfig *>(walleve::IWalleveBase::WalleveConfig());
    }
    void IOThreadFunc();
    void IOProcFilter(const boost::system::error_code& err);
    void filterAddressList();
protected:
    //timer
    walleve::CWalleveThread thrIOProc;
    boost::asio::io_service ioService;
    boost::asio::deadline_timer timerFilter;
    uint32  _newestHeight;
};
}
#endif