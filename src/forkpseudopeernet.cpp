// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "forkpseudopeernet.h"

using namespace multiverse;

CForkPseudoPeerNet::CForkPseudoPeerNet()
: CMvPeerNet("forkpseudopeernet")
{
    pDbpService = nullptr;
    typeNode = SUPER_NODE_TYPE::SUPER_NODE_TYPE_FNFN;
}

CForkPseudoPeerNet::~CForkPseudoPeerNet()
{
}

bool CForkPseudoPeerNet::WalleveHandleInitialize()
{
    CMvPeerNet::WalleveHandleInitialize();

    if (!WalleveGetObject("dbpservice", pDbpService))
    {
        WalleveLog("Failed to request DBP service\n");
        return false;
    }

    return true;
}

void CForkPseudoPeerNet::WalleveHandleDeinitialize()
{
    CMvPeerNet::WalleveHandleDeinitialize();

    pDbpService = nullptr;
}

//messages come from p2p network - stem from real peer net and relayed by netchannel
bool CForkPseudoPeerNet::HandleEvent(CFkEventNodeBlockArrive& event)
{
    //fnfn node
    if(typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_FNFN)
    {
        return true;
    }

    //fork node
    if(typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_FORK)
    {
        CFkEventNodeBlockArrive *pEvent = new CFkEventNodeBlockArrive(event);
        if(nullptr != pEvent)
        {
            pNetChannel->PostEvent(pEvent);
        }
        return true;
    }

    //root node
    auto it = mapForkNodeHeight.find(event.hashFork);
    if(it == mapForkNodeHeight.end())
    {
        return true;
    }
    if((*it).second.second != event.data.GetHash())
    {//discard this block directly if it does not match the last block
        return true;
    }

    CFkEventNodeBlockArrive *pEvent = new CFkEventNodeBlockArrive(event);
    if(nullptr != pEvent)
    {
        pEvent->height += 1;
        pDbpService->PostEvent(pEvent);
    }
    return true;
}

bool CForkPseudoPeerNet::HandleEvent(CFkEventNodeTxArrive& event)
{
    //fnfn node
    if(typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_FNFN)
    {
        return true;
    }

    //fork node
    if(typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_FORK)
    {
        CFkEventNodeTxArrive *pEvent = new CFkEventNodeTxArrive(event);
        if(nullptr != pEvent)
        {
            pNetChannel->PostEvent(pEvent);
        }
        return true;
    }

    //root node
    if(!ExistForkID(event.hashFork))
    {
        return true;
    }
    CFkEventNodeTxArrive *pEvent = new CFkEventNodeTxArrive(event);
    if(nullptr != pEvent)
    {
        pDbpService->PostEvent(pEvent);
    }
    return true;
}

bool CForkPseudoPeerNet::HandleEvent(CFkEventNodeBlockRequest& event)
{
    //fnfn node
    if(typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_FNFN)
    {
        return true;
    }

    //fork node
    if(typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_FORK)
    {
        CFkEventNodeBlockRequest *pEvent = new CFkEventNodeBlockRequest(event);
        if(nullptr != pEvent)
        {
            pNetChannel->PostEvent(pEvent);
        }
        return true;
    }

    //root node
    if(!ExistForkID(event.hashFork))
    {
        return true;
    }
    CFkEventNodeBlockRequest *pEvent = new CFkEventNodeBlockRequest(event);
    if(nullptr != pEvent)
    {
        pDbpService->PostEvent(pEvent);
    }
    return true;
}

bool CForkPseudoPeerNet::HandleEvent(CFkEventNodeTxRequest& event)
{
    //fnfn node
    if(typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_FNFN)
    {
        return true;
    }

    //fork node
    if(typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_FORK)
    {
        CFkEventNodeTxRequest *pEvent = new CFkEventNodeTxRequest(event);
        if(nullptr != pEvent)
        {
            pNetChannel->PostEvent(pEvent);
        }
        return true;
    }

    //root node
    if(!ExistForkID(event.hashFork))
    {
        return true;
    }
    CFkEventNodeTxRequest *pEvent = new CFkEventNodeTxRequest(event);
    if(nullptr != pEvent)
    {
        pDbpService->PostEvent(pEvent);
    }
    return true;
}

//messages come from fork node cluster
bool CForkPseudoPeerNet::HandleEvent(CFkEventNodeUpdateForkState& event)
{
    //fnfn node
    if(typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_FNFN)
    {
        return true;
    }

    //root node or fork node
    if(!ExistForkID(event.hashFork) && event.height > 0)
    {
        return false;
    }
    if(!ExistForkID(event.hashFork) && event.height == 0)
    {
        mapForkNodeHeight[event.hashFork] = std::make_pair(0, event.hashBlock);
    }
    else
    {
        mapForkNodeHeight[event.hashFork] = std::make_pair(event.height, event.hashBlock);
    }
    return true;
}

bool CForkPseudoPeerNet::HandleEvent(CFkEventNodeSendBlockNotice& event)
{
    //fnfn node
    if(typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_FNFN)
    {
        return true;
    }

    //fork node
    if(typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_FORK)
    {
        CFkEventNodeSendBlockNotice *pEvent = new CFkEventNodeSendBlockNotice(event);
        if(nullptr != pEvent)
        {
            pDbpService->PostEvent(pEvent);
        }
        return true;
    }

    //root node
    if(!ExistForkID(event.hashFork))
    {
        return false;
    }
    CFkEventNodeSendBlockNotice *pEvent = new CFkEventNodeSendBlockNotice(event);
    if(nullptr != pEvent)
    {
        pNetChannel->PostEvent(pEvent);
    }
    return true;
}

bool CForkPseudoPeerNet::HandleEvent(CFkEventNodeSendTxNotice& event)
{
    //fnfn node
    if(typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_FNFN)
    {
        return true;
    }

    //fork node
    if(typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_FORK)
    {
        CFkEventNodeSendTxNotice *pEvent = new CFkEventNodeSendTxNotice(event);
        if(nullptr != pEvent)
        {
            pDbpService->PostEvent(pEvent);
        }
        return true;
    }

    //root node
    if(!ExistForkID(event.hashFork))
    {
        return false;
    }
    CFkEventNodeSendTxNotice *pEvent = new CFkEventNodeSendTxNotice(event);
    if(nullptr != pEvent)
    {
        pNetChannel->PostEvent(pEvent);
    }
    return true;
}

bool CForkPseudoPeerNet::HandleEvent(CFkEventNodeSendBlock& event)
{
    //fnfn node
    if(typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_FNFN)
    {
        return true;
    }

    //fork node
    if(typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_FORK)
    {
        CFkEventNodeSendBlock *pEvent = new CFkEventNodeSendBlock(event);
        if(nullptr != pEvent)
        {
            pDbpService->PostEvent(pEvent);
        }
        return true;
    }

    //root node
    if(!ExistForkID(event.hashFork))
    {
        return false;
    }
    CFkEventNodeSendBlock *pEvent = new CFkEventNodeSendBlock(event);
    if(nullptr != pEvent)
    {
        pNetChannel->PostEvent(pEvent);
    }
    return true;
}

bool CForkPseudoPeerNet::HandleEvent(CFkEventNodeSendTx& event)
{
    //fnfn node
    if(typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_FNFN)
    {
        return true;
    }

    //fork node
    if(typeNode == SUPER_NODE_TYPE::SUPER_NODE_TYPE_FORK)
    {
        CFkEventNodeSendTx *pEvent = new CFkEventNodeSendTx(event);
        if(nullptr != pEvent)
        {
            pDbpService->PostEvent(pEvent);
        }
        return true;
    }

    //root node
    if(!ExistForkID(event.hashFork))
    {
        return false;
    }
    CFkEventNodeSendTx *pEvent = new CFkEventNodeSendTx(event);
    if(nullptr != pEvent)
    {
        pNetChannel->PostEvent(pEvent);
    }
    return true;
}

bool CForkPseudoPeerNet::HandleEvent(CFkEventNodeIsForkNode& event)
{
    typeNode = event.fIsForkNode ? SUPER_NODE_TYPE::SUPER_NODE_TYPE_FORK
                                 : SUPER_NODE_TYPE::SUPER_NODE_TYPE_ROOT;
}