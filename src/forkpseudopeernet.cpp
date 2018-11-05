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

    return true;
}

void CForkPseudoPeerNet::WalleveHandleDeinitialize()
{
    pDbpClient = NULL;
}

//This handler is responsible for receiving from dbp client
// and delivering to net channel
bool CForkPseudoPeerNet::HandleEvent(CFkEventNodeMessage& eventMessage)
{
    //TODO
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
