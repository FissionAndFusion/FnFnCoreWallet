// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_FORKPSEUDOPEERNET_H
#define MULTIVERSE_FORKPSEUDOPEERNET_H

#include "mvproto.h"
#include "virtualpeernetevent.h"
#include "mvpeernet.h"

namespace multiverse
{

class CVirtualPeerNet: public network::CMvPeerNet, virtual public CVirtualPeerNetEventListener
{
public:
    enum class SUPER_NODE_TYPE : int
    {
        SUPER_NODE_TYPE_UNKN,
        SUPER_NODE_TYPE_FNFN = 0,
        SUPER_NODE_TYPE_ROOT,
        SUPER_NODE_TYPE_FORK
    };

public:
    CVirtualPeerNet();
    virtual ~CVirtualPeerNet();

    SUPER_NODE_TYPE GetSuperNodeType() {return typeNode;};
    void SetNodeTypeAsFnfn(bool fIsFnfnNode);
    void SetNodeTypeAsSuperNode(bool fIsRootNode);

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
    bool HandleRootPeerSub(const uint64& nNonce, const uint256& hashFork) override;
    bool HandleRootPeerUnSub(const uint64& nNonce, const uint256& hashFork) override;
    bool HandleRootPeerGetBlks(const uint64& nNonce, const uint256& hashFork) override;
    bool HandleRootPeerInv(const uint64& nNonce, const uint256& hashFork) override;
    bool HandleRootPeerGetData(const uint64& nNonce, const uint256& hashFork) override;
    bool HandleRootPeerBlock(const uint64& nNonce, const uint256& hashFork) override;
    bool HandleRootPeerTx(const uint64& nNonce, const uint256& hashFork) override;
protected:
    walleve::IIOModule* pDbpService;
    SUPER_NODE_TYPE typeNode;
private:
    static const std::string SENDER_NETCHN;
    static const std::string SENDER_DBPSVC;
};

}

#endif //MULTIVERSE_FORKPSEUDOPEERNET_H
