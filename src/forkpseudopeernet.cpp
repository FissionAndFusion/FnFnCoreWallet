// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "forkpseudopeernet.h"

using namespace multiverse;

CForkPseudoPeerNet::CForkPseudoPeerNet()
{
    WalleveSetOwnKey("forknode");
}

CForkPseudoPeerNet::~CForkPseudoPeerNet()
{
}

bool CForkPseudoPeerNet::WalleveHandleInitialize()
{/*
    if (!WalleveGetObject("dbpclient", pDbpClient))
    {
        WalleveLog("Failed to request DBP client\n");
        return false;
    }*/
    if (!WalleveGetObject("netchannel", pNetChannel))
    {
        WalleveLog("Failed to request net channel\n");
        return false;
    }

    return true;
}

void CForkPseudoPeerNet::WalleveHandleDeinitialize()
{
    pDbpClient = NULL;
    pNetChannel = NULL;
}

//This handler is responsible for receiving from dbp client
// and delivering to net channel
bool CForkPseudoPeerNet::HandleEvent(CFkEventNodeMessage& eventMessage)
{
    uint64 nNonce;
    uint256 haskFork;
    ecForkEventType fkType = eventMessage.fkMsgType;
    switch (fkType)
    {
        case ecForkEventType::FK_EVENT_NODE_ACTIVE:
        {
            CFkEventNodeActive* pEventActive = new CFkEventNodeActive(nNonce, haskFork);
            pEventActive->data = boost::get<network::CAddress>(eventMessage.fkMsgData);
            pNetChannel->PostEvent(pEventActive);
            break;
        }
        case ecForkEventType::FK_EVENT_NODE_DEACTIVE:
        {
            CFkEventNodeDeactive* pEventDeactive = new CFkEventNodeDeactive(nNonce, haskFork);
            pEventDeactive->data = boost::get<network::CAddress>(eventMessage.fkMsgData);
            pNetChannel->PostEvent(pEventDeactive);
            break;
        }
        case ecForkEventType::FK_EVENT_NODE_SUBSCRIBE:
        {
            CFkEventNodeSubscribe* pEventSubscribe = new CFkEventNodeSubscribe(nNonce, haskFork);
            pEventSubscribe->data = boost::get<std::vector<uint256>>(eventMessage.fkMsgData);
            pNetChannel->PostEvent(pEventSubscribe);
            break;
        }
        case ecForkEventType::FK_EVENT_NODE_UNSUBSCRIBE:
        {
            CFkEventNodeUnsubscribe* pEventUnsubscribe = new CFkEventNodeUnsubscribe(nNonce, haskFork);
            pEventUnsubscribe->data = boost::get<std::vector<uint256>>(eventMessage.fkMsgData);
            pNetChannel->PostEvent(pEventUnsubscribe);
            break;
        }
        case ecForkEventType::FK_EVENT_NODE_GETBLOCKS:
        {
            CFkEventNodeGetBlocks* pEventGetBlocks = new CFkEventNodeGetBlocks(nNonce, haskFork);
            pEventGetBlocks->data = boost::get<CBlockLocator>(eventMessage.fkMsgData);
            pNetChannel->PostEvent(pEventGetBlocks);
            break;
        }
        case ecForkEventType::FK_EVENT_NODE_INV:
        {
            CFkEventNodeInv* pEventInv = new CFkEventNodeInv(nNonce, haskFork);
            pEventInv->data = boost::get<std::vector<network::CInv>>(eventMessage.fkMsgData);
            pNetChannel->PostEvent(pEventInv);
            break;
        }
        case ecForkEventType::FK_EVENT_NODE_GETDATA:
        {
            CFkEventNodeGetData* pEventGetData = new CFkEventNodeGetData(nNonce, haskFork);
            pEventGetData->data = boost::get<std::vector<network::CInv>>(eventMessage.fkMsgData);
            pNetChannel->PostEvent(pEventGetData);
            break;
        }
        case ecForkEventType::FK_EVENT_NODE_BLOCK:
        {
            CFkEventNodeBlock* pEventBlock = new CFkEventNodeBlock(nNonce, haskFork);
            pEventBlock->data = boost::get<CBlock>(eventMessage.fkMsgData);
            pNetChannel->PostEvent(pEventBlock);
            break;
        }
        case ecForkEventType::FK_EVENT_NODE_TX:
        {
            CFkEventNodeTx* pEventTx = new CFkEventNodeTx(nNonce, haskFork);
            pEventTx->data = boost::get<CTransaction>(eventMessage.fkMsgData);
            pNetChannel->PostEvent(pEventTx);
            break;
        }
    }
    return true;
}

//The following series of HandleEvent's is for responding to requests
// from CNetChannel object invoking Dispatch() of itself
bool CForkPseudoPeerNet::HandleEvent(CFkEventNodeActive& eventActive)
{
    CFkEventNodeActive* pEventActive = new CFkEventNodeActive(eventActive);
    //pDbpClient->PostEvent(pEventActive);
    return true;
}

bool CForkPseudoPeerNet::HandleEvent(CFkEventNodeDeactive& eventDeactive)
{
    CFkEventNodeDeactive* pEventDeactive = new CFkEventNodeDeactive(eventDeactive);
    //pDbpClient->PostEvent(pEventDeactive);
    return true;
}

bool CForkPseudoPeerNet::HandleEvent(CFkEventNodeSubscribe& eventSubscribe)
{
    CFkEventNodeSubscribe* pEventSubscribe = new CFkEventNodeSubscribe(eventSubscribe);
    //pDbpClient->PostEvent(pEventSubscribe);
    return true;
}

bool CForkPseudoPeerNet::HandleEvent(CFkEventNodeUnsubscribe& eventUnsubscribe)
{
    CFkEventNodeUnsubscribe* pEventUnsubscribe = new CFkEventNodeUnsubscribe(eventUnsubscribe);
    //pDbpClient->PostEvent(pEventUnsubscribe);
    return true;
}

bool CForkPseudoPeerNet::HandleEvent(CFkEventNodeGetBlocks& eventGetBlocks)
{
    CFkEventNodeGetBlocks* pEventGetBlocks = new CFkEventNodeGetBlocks(eventGetBlocks);
    //pDbpClient->PostEvent(pEventGetBlocks);
    return true;
}

bool CForkPseudoPeerNet::HandleEvent(CFkEventNodeInv& eventInv)
{
    CFkEventNodeInv* pEventInv = new CFkEventNodeInv(eventInv);
    //pDbpClient->PostEvent(pEventInv);
    return true;
}

bool CForkPseudoPeerNet::HandleEvent(CFkEventNodeGetData& eventGetData)
{
    CFkEventNodeGetData* pEventGetData = new CFkEventNodeGetData(eventGetData);
    //pDbpClient->PostEvent(pEventGetData);
    return true;
}

bool CForkPseudoPeerNet::HandleEvent(CFkEventNodeBlock& eventBlock)
{
    CFkEventNodeBlock *pEventBlock = new CFkEventNodeBlock(eventBlock);
    //pDbpClient->PostEvent(pEventBlock);
    return true;
}

bool CForkPseudoPeerNet::HandleEvent(CFkEventNodeTx& eventTx)
{
    CFkEventNodeTx *pEventTx = new CFkEventNodeTx(eventTx);
    //pDbpClient->PostEvent(pEventTx);
    return true;
}
