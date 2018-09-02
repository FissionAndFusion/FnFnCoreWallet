// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_PEERNET_H
#define  MULTIVERSE_PEERNET_H

#include "mvproto.h"
#include "mvpeerevent.h"
#include "walleve/walleve.h"

namespace multiverse 
{
namespace network 
{

class IMvNetChannel : public walleve::IIOModule, virtual public CMvPeerEventListener
{
public:
    IMvNetChannel() : IIOModule("netchannel") {}
    virtual int GetPrimaryChainHeight() = 0;
    virtual void BroadcastBlockInv(const uint256& hashFork,const uint256& hashBlock,const std::set<uint64>& setKnownPeer=std::set<uint64>())=0;
    virtual void BroadcastTxInv(const uint256& hashFork)=0;

};

class CMvPeerNet : public walleve::CPeerNet, virtual public CMvPeerEventListener
{
public:
    CMvPeerNet();
    ~CMvPeerNet();
    virtual void BuildHello(walleve::CPeer *pPeer,walleve::CWalleveBufStream& ssPayload);
    void HandlePeerWriten(walleve::CPeer *pPeer);
    bool HandlePeerHandshaked(walleve::CPeer *pPeer,uint32 nTimerId);
    virtual bool HandlePeerRecvMessage(walleve::CPeer *pPeer,int nChannel,int nCommand,
                               walleve::CWalleveBufStream& ssPayload); 
protected:
    bool WalleveHandleInitialize();
    void WalleveHandleDeinitialize();
    bool HandleEvent(CMvEventPeerInv& eventInv);
    bool HandleEvent(CMvEventPeerGetData& eventGetData);
    bool HandleEvent(CMvEventPeerGetBlocks& eventGetBlocks);
    bool HandleEvent(CMvEventPeerTx& eventTx);
    bool HandleEvent(CMvEventPeerBlock& eventBlock);
    virtual walleve::CPeer* CreatePeer(walleve::CIOClient *pClient,uint64 nNonce,bool fInBound);
    void DestroyPeer(walleve::CPeer* pPeer);
    walleve::CPeerInfo* GetPeerInfo(walleve::CPeer* pPeer,walleve::CPeerInfo* pInfo);
    bool SendDataMessage(uint64 nNonce,int nCommand,walleve::CWalleveBufStream& ssPayload);
    void SetInvTimer(uint64 nNonce,std::vector<CInv>& vInv);
    void ProcessAskFor(walleve::CPeer* pPeer);
    void Configure(uint32 nMagicNumIn,uint32 nVersionIn,uint64 nServiceIn,const std::string& subVersionIn,bool fEnclosedIn)
    {
        nMagicNum = nMagicNumIn; nVersion = nVersionIn; nService = nServiceIn;
        subVersion = subVersionIn; fEnclosed = fEnclosedIn;
    }
    virtual bool CheckPeerVersion(uint32 nVersionIn,uint64 nServiceIn,const std::string& subVersionIn) = 0;
protected:
    IMvNetChannel* pNetChannel;
    uint32 nMagicNum;
    uint32 nVersion;
    uint64 nService;
    bool fEnclosed;
    std::string subVersion;    
    std::set<boost::asio::ip::tcp::endpoint> setDNSeed;
};

} // namespace network
} // namespace multiverse

#endif //MULTIVERSE_PEERNET_H

