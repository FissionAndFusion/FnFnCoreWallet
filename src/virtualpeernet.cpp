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

void CVirtualPeerNet::SetNodeTypeAsFnfn(bool fIsFnfnNode)
{
    if(fIsFnfnNode)
    {
        typeNode = SUPER_NODE_TYPE::SUPER_NODE_TYPE_FNFN;
    }
    else
    {
        typeNode = SUPER_NODE_TYPE::SUPER_NODE_TYPE_UNKN;
    }
}

void CVirtualPeerNet::SetNodeTypeAsSuperNode(bool fIsRootNode)
{
    if(fIsRootNode)
    {
        typeNode = SUPER_NODE_TYPE::SUPER_NODE_TYPE_ROOT;
    }
    else
    {
        typeNode = SUPER_NODE_TYPE::SUPER_NODE_TYPE_FORK;
    }
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

bool CVirtualPeerNet::HandleEvent(network::CMvEventPeerActive& eventActive)
{
    if(typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_FORK)
    {
        if(!HandleForkPeerActive(eventActive))
        {
            return false;
        }
    }
    return true;
}

bool CVirtualPeerNet::HandleEvent(network::CMvEventPeerDeactive& eventDeactive)
{
    if(typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_FORK)
    {
        if(!HandleForkPeerDeactive(eventDeactive))
        {
            return false;
        }
    }
    return true;
}

bool CVirtualPeerNet::HandleEvent(walleve::CWalleveEventPeerNetReward& eventReward)
{
    if(typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_ROOT
       || typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_FNFN)
    {
        return CPeerNet::HandleEvent(eventReward);
    }
    if(typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_FORK)
    {
        walleve::CWalleveEventPeerNetReward* pEvent = new walleve::CWalleveEventPeerNetReward(eventReward);
        if(nullptr == pEvent)
        {
            return false;
        }
        pDbpService->PostEvent(pEvent);
    }
    return true;
}

bool CVirtualPeerNet::HandleEvent(walleve::CWalleveEventPeerNetClose& eventClose)
{
    if(typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_ROOT
       || typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_FNFN)
    {
        return CPeerNet::HandleEvent(eventClose);
    }
    if(typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_FORK)
    {
        walleve::CWalleveEventPeerNetClose* pEvent = new walleve::CWalleveEventPeerNetClose(eventClose);
        if(nullptr == pEvent)
        {
            return false;
        }
        pDbpService->PostEvent(pEvent);
    }
    return true;
}

bool CVirtualPeerNet::HandleEvent(network::CMvEventPeerSubscribe& eventSubscribe)
{
    if(typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_ROOT
       || typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_FNFN)
    {
        return CMvPeerNet::HandleEvent(eventSubscribe);
    }
    if(typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_FORK)
    {
        network::CMvEventPeerSubscribe* pEvent = new network::CMvEventPeerSubscribe(eventSubscribe);
        if(nullptr == pEvent)
        {
            return false;
        }
        pDbpService->PostEvent(pEvent);
    }
    return true;
}

bool CVirtualPeerNet::HandleEvent(network::CMvEventPeerUnsubscribe& eventUnsubscribe)
{
    if(typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_ROOT
       || typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_FNFN)
    {
        return CMvPeerNet::HandleEvent(eventUnsubscribe);
    }
    if(typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_FORK)
    {
        network::CMvEventPeerUnsubscribe* pEvent = new network::CMvEventPeerUnsubscribe(eventUnsubscribe);
        if(nullptr == pEvent)
        {
            return false;
        }
        pDbpService->PostEvent(pEvent);
    }
    return true;
}

bool CVirtualPeerNet::HandleEvent(network::CMvEventPeerInv& eventInv)
{
    if(typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_ROOT
       || typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_FNFN)
    {
        return CMvPeerNet::HandleEvent(eventInv);
    }
    if(typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_FORK)
    {
        network::CMvEventPeerInv* pEvent = new network::CMvEventPeerInv(eventInv);
        if(nullptr == pEvent)
        {
            return false;
        }
        pDbpService->PostEvent(pEvent);
    }
    return true;
}

bool CVirtualPeerNet::HandleEvent(network::CMvEventPeerGetData& eventGetData)
{
    if(typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_ROOT
       || typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_FNFN)
    {
        return CMvPeerNet::HandleEvent(eventGetData);
    }
    if(typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_FORK)
    {
        network::CMvEventPeerGetData* pEvent = new network::CMvEventPeerGetData(eventGetData);
        if(nullptr == pEvent)
        {
            return false;
        }
        pDbpService->PostEvent(pEvent);
    }
    return true;
}

bool CVirtualPeerNet::HandleEvent(network::CMvEventPeerGetBlocks& eventGetBlocks)
{
    if(typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_ROOT
       || typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_FNFN)
    {
        return CMvPeerNet::HandleEvent(eventGetBlocks);
    }
    if(typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_FORK)
    {
        network::CMvEventPeerGetBlocks* pEvent = new network::CMvEventPeerGetBlocks(eventGetBlocks);
        if(nullptr == pEvent)
        {
            return false;
        }
        pDbpService->PostEvent(pEvent);
    }
    return true;
}

bool CVirtualPeerNet::HandleEvent(network::CMvEventPeerTx& eventTx)
{
    if(typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_ROOT
       || typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_FNFN)
    {
        return CMvPeerNet::HandleEvent(eventTx);
    }
    if(typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_FORK)
    {
        network::CMvEventPeerTx* pEvent = new network::CMvEventPeerTx(eventTx);
        if(nullptr == pEvent)
        {
            return false;
        }
        pDbpService->PostEvent(pEvent);
    }
    return true;
}

bool CVirtualPeerNet::HandleEvent(network::CMvEventPeerBlock& eventBlock)
{
    if(typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_ROOT
       || typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_FNFN)
    {
        return CMvPeerNet::HandleEvent(eventBlock);
    }
    if(typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_FORK)
    {
        network::CMvEventPeerBlock* pEvent = new network::CMvEventPeerBlock(eventBlock);
        if(nullptr == pEvent)
        {
            return false;
        }
        pDbpService->PostEvent(pEvent);
    }
    return true;
}

bool CVirtualPeerNet::HandlePeerHandshakedForForkNode(const network::CMvEventPeerActive& peerActive)
{
    if(typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_ROOT)
    {
        CMvEventPeerActive* pEvent = new CMvEventPeerActive(peerActive);
        if(nullptr == pEvent)
        {
            return false;
        }
        pDbpService->PostEvent(pEvent);
    }
    return true;
}

bool CVirtualPeerNet::DestroyPeerForForkNode(const network::CMvEventPeerDeactive& peerDeactive)
{
    if(typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_ROOT)
    {
        network::CMvEventPeerDeactive* pEvent = new network::CMvEventPeerDeactive(peerDeactive);
        if(nullptr == pEvent)
        {
            return false;
        }
        pDbpService->PostEvent(pEvent);
    }
    return true;
}