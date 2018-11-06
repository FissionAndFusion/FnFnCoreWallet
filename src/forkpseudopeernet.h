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

class IDbpClient;

class CForkPseudoPeerNet: public network::CMvPeerNet, virtual public CFkNodeEventListener
{
public:
    CForkPseudoPeerNet();
    ~CForkPseudoPeerNet();
protected:
    bool WalleveHandleInitialize() override;
    void WalleveHandleDeinitialize() override;
    bool HandleEvent(CFkEventNodeMessage& eventMessage) override;
    bool HandleEvent(CFkEventNodeActive& eventActive) override;
    bool HandleEvent(CFkEventNodeDeactive& eventDeactive) override;
    bool HandleEvent(CFkEventNodeSubscribe& eventSubscribe) override;
    bool HandleEvent(CFkEventNodeUnsubscribe& eventUnsubscribe) override;
    bool HandleEvent(CFkEventNodeGetBlocks& eventGetBlocks) override;
    bool HandleEvent(CFkEventNodeInv& eventInv) override;
    bool HandleEvent(CFkEventNodeGetData& eventGetData) override;
    bool HandleEvent(CFkEventNodeBlock& eventBlock) override;
    bool HandleEvent(CFkEventNodeTx& eventTx) override;
protected:
    IDbpClient* pDbpClient;
};

}

#endif //MULTIVERSE_FORKPSEUDOPEERNET_H
