// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef MULTIVERSE_DNSEED_H
#define MULTIVERSE_DNSEED_H

#include "config.h"
#include "mvbase.h"
#include "dnseedservice.h"
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
    void ClientFailToConnect(const boost::asio::ip::tcp::endpoint& epRemote)override;
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
    int beginFilterList();
    void filterAddressList();
    void requestGetTrustedHeight();
    void IOThreadFunc_test();
    void IOProcFilter(const boost::system::error_code& err);
    void IOThreadFunc_th();
    void IOProc_th(const boost::system::error_code& err);
    //trusted height
    void beginVoteHeight();
    void voteHeight(uint32 height);
protected:
    network::DNSeedService _dnseedService;
    std::string _confidentAddress;
    uint32  _confidentHeight;
    std::vector<std::pair<uint32,int>> _voteBox;
    bool _beginFilter=false;
    bool _isConfidentNodeCanConnect=false;
    std::vector<boost::asio::ip::tcp::endpoint> _testListBuf;
    //timer
    walleve::CWalleveThread thrIOProc_test;
    boost::asio::io_service ioService_test;
    boost::asio::deadline_timer timerFilter;
    //TrustedHeight timer
    walleve::CWalleveThread thrIOProc_th;
    boost::asio::io_service ioService_th;
    boost::asio::deadline_timer timer_th;
    
};
}
#endif