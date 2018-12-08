// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_FORKPSEUDOPEERNET_H
#define MULTIVERSE_FORKPSEUDOPEERNET_H

#include "mvproto.h"
#include "forkpeerevent.h"
#include "mvpeernet.h"

namespace multiverse
{

class CVirtualPeerNet: public network::CMvPeerNet, virtual public CFkNodeEventListener
{
public:
    enum class SUPER_NODE_TYPE : int
    {
        SUPER_NODE_TYPE_FNFN = 0,
        SUPER_NODE_TYPE_ROOT,
        SUPER_NODE_TYPE_FORK
    };

public:
    CVirtualPeerNet();
    ~CVirtualPeerNet();

    SUPER_NODE_TYPE GetSuperNodeType() {return typeNode;};
protected:
    bool WalleveHandleInitialize() override;
    void WalleveHandleDeinitialize() override;

    bool HandleEvent(CFkEventNodePeerActive& event) override;

    void HandlePeerHandshakedForForkNode(network::CMvEventPeerActive& peerActive) override;

protected:
    walleve::IIOModule* pDbpService;
    SUPER_NODE_TYPE typeNode;
};

}

#endif //MULTIVERSE_FORKPSEUDOPEERNET_H
