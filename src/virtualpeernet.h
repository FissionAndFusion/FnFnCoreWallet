// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_VIRTUAL_PEERNET_H
#define MULTIVERSE_VIRTUAL_PEERNET_H

#include "mvproto.h"
#include "mvbase.h"
#include "virtualpeernetevent.h"
#include "mvpeernet.h"

namespace multiverse
{

class CVirtualPeerNet: public network::CMvPeerNet, virtual public CVirtualPeerNetEventListener
{
public:
    CVirtualPeerNet();
    virtual ~CVirtualPeerNet();

    SUPER_NODE_TYPE GetSuperNodeType() {return typeNode;};
    void SetNodeTypeAsFnfn(bool fIsFnfnNode);
    void SetNodeTypeAsSuperNode(bool fIsRootNode);
    void EnableSuperNode(bool fIsFork = false);

protected:
    bool WalleveHandleInitialize() override;
    void WalleveHandleDeinitialize() override;

    bool HandleEvent(network::CMvEventPeerActive& eventActive) override;
    bool HandleEvent(network::CMvEventPeerDeactive& eventDeactive) override;

    bool HandleEvent(walleve::CWalleveEventPeerNetReward& eventReward) override;
    bool HandleEvent(walleve::CWalleveEventPeerNetClose& eventClose) override;

    bool HandleEvent(network::CMvEventPeerSubscribe& eventSubscribe) override;
    bool HandleEvent(network::CMvEventPeerUnsubscribe& eventUnsubscribe) override;
    bool HandleEvent(network::CMvEventPeerInv& eventInv) override;
    bool HandleEvent(network::CMvEventPeerGetData& eventGetData) override;
    bool HandleEvent(network::CMvEventPeerGetBlocks& eventGetBlocks) override;
    bool HandleEvent(network::CMvEventPeerTx& eventTx) override;
    bool HandleEvent(network::CMvEventPeerBlock& eventBlock) override;

    bool HandlePeerHandshakedForForkNode(const network::CMvEventPeerActive& peerActive) override;
    bool DestroyPeerForForkNode(const network::CMvEventPeerDeactive& peerDeactive) override;

    bool HandleRootPeerSub(const uint64& nNonce, const uint256& hashFork, vector<uint256>& data) override;
    bool HandleRootPeerUnSub(const uint64& nNonce, const uint256& hashFork, vector<uint256>& data) override;
    bool HandleRootPeerGetBlocks(const uint64& nNonce, const uint256& hashFork, CBlockLocator& data) override;
    bool HandleRootPeerInv(const uint64& nNonce, const uint256& hashFork, vector<CInv>& data) override;
    bool HandleRootPeerGetData(const uint64& nNonce, const uint256& hashFork, vector<CInv>& data) override;
    bool HandleRootPeerBlock(const uint64& nNonce, const uint256& hashFork, CBlock& data) override;
    bool HandleRootPeerTx(const uint64& nNonce, const uint256& hashFork, CTransaction& data) override;

    bool IsMainFork(const uint256& hashFork) override;
    bool IsMyFork(const uint256& hashFork) override;
protected:
    walleve::IIOModule* pDbpService;
    ICoreProtocol* pCoreProtocol;
    IForkManager* pForkManager;
private:
    static const std::string SENDER_NETCHN;
    static const std::string SENDER_DBPSVC;
};

}

#endif // MULTIVERSE_VIRTUAL_PEERNET_H
