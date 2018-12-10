// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "virtualpeernet.h"
#include "walleve/peernet/peer.h"

using namespace multiverse;
using namespace walleve;

CVirtualPeerNet::CVirtualPeerNet()
: CMvPeerNet("virtualpeernet")
{
    pDbpService = nullptr;
    typeNode = SUPER_NODE_TYPE::SUPER_NODE_TYPE_FNFN;
}

CVirtualPeerNet::~CVirtualPeerNet()
{
}

bool CVirtualPeerNet::WalleveHandleInitialize()
{
    CMvPeerNet::WalleveHandleInitialize();

    if (!WalleveGetObject("dbpservice", pDbpService))
    {
        WalleveLog("Failed to request DBP service\n");
        return false;
    }

    return true;
}

void CVirtualPeerNet::WalleveHandleDeinitialize()
{
    CMvPeerNet::WalleveHandleDeinitialize();

    pDbpService = nullptr;
}

bool CVirtualPeerNet::HandleEvent(CFkEventNodePeerActive& event)
{
    if(typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_FORK)
    {
        uint64 nNonce = event.nNonce;
        network::CAddress addr = event.data;
        if(!HandleForkPeerActive(nNonce, addr))
        {
            return false;
        }
    }
    return true;
}

bool CVirtualPeerNet::HandleEvent(walleve::CWalleveEventPeerNetReward& eventReward)
{
    return CPeerNet::HandleEvent(eventReward);
}

bool CVirtualPeerNet::HandleEvent(walleve::CWalleveEventPeerNetClose& eventClose)
{
    return CPeerNet::HandleEvent(eventClose);
}

bool CVirtualPeerNet::HandleEvent(network::CMvEventPeerSubscribe& eventSubscribe)
{
    return CMvPeerNet::HandleEvent(eventSubscribe);
}

bool CVirtualPeerNet::HandleEvent(network::CMvEventPeerUnsubscribe& eventUnsubscribe)
{
    return CMvPeerNet::HandleEvent(eventUnsubscribe);
}

bool CVirtualPeerNet::HandleEvent(network::CMvEventPeerInv& eventInv)
{
    return CMvPeerNet::HandleEvent(eventInv);
}

bool CVirtualPeerNet::HandleEvent(network::CMvEventPeerGetData& eventGetData)
{
    return CMvPeerNet::HandleEvent(eventGetData);
}

bool CVirtualPeerNet::HandleEvent(network::CMvEventPeerGetBlocks& eventGetBlocks)
{
    return CMvPeerNet::HandleEvent(eventGetBlocks);
}

bool CVirtualPeerNet::HandleEvent(network::CMvEventPeerTx& eventTx)
{
    return CMvPeerNet::HandleEvent(eventTx);
}

bool CVirtualPeerNet::HandleEvent(network::CMvEventPeerBlock& eventBlock)
{
    return CMvPeerNet::HandleEvent(eventBlock);
}

void CVirtualPeerNet::HandlePeerHandshakedForForkNode(network::CMvEventPeerActive& peerActive)
{
    if(typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_ROOT)
    {
        CFkEventNodePeerActive* pEvent = new CFkEventNodePeerActive(peerActive.nNonce);
        if(nullptr != pEvent)
        {
            pEvent->data = peerActive.data;
            pDbpService->PostEvent(pEvent);
        }
    }
}

void CVirtualPeerNet::DestroyPeerForForkNode(network::CMvEventPeerDeactive& peerDeactive)
{
    if(typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_ROOT)
    {
        network::CMvEventPeerDeactive* pEvent = new network::CMvEventPeerDeactive(peerDeactive);
        if(nullptr != pEvent)
        {
            pEvent->data = peerDeactive.data;
            pDbpService->PostEvent(pEvent);
        }
    }
}