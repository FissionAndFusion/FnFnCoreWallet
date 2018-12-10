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

void DestroyPeerForForkNode(network::CMvEventPeerDeactive& peerDeactive)
{
    if(typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_ROOT)
    {
        CFkEventNodePeerDeactive* pEvent = new CFkEventNodePeerDeactive(peerDeactive.nNonce);
        if(nullptr != pEvent)
        {
            pEvent->data = peerDeactive.data;
            pDbpService->PostEvent(pEvent);
        }
    }
}