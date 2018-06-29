// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "dispatcher.h"
#include "event.h"

using namespace std;
using namespace walleve;
using namespace multiverse;

//////////////////////////////
// CDispatcher 

CDispatcher::CDispatcher()
{
    pCoreProtocol = NULL;
    pWorldLine = NULL;
    pTxPool = NULL;
    pWallet = NULL;
    pService = NULL;
    pBlockMaker = NULL;
}

CDispatcher::~CDispatcher()
{
}

bool CDispatcher::WalleveHandleInitialize()
{
    if (!WalleveGetObject("coreprotocol",pCoreProtocol))
    {
        WalleveLog("Failed to request coreprotocol\n");
        return false;
    }

    if (!WalleveGetObject("worldline",pWorldLine))
    {
        WalleveLog("Failed to request worldline\n");
        return false;
    }

    if (!WalleveGetObject("txpool",pTxPool))
    {
        WalleveLog("Failed to request txpool\n");
        return false;
    }

    if (!WalleveGetObject("wallet",pWallet))
    {
        WalleveLog("Failed to request wallet\n");
        return false;
    }

    if (!WalleveGetObject("service",pService))
    {
        WalleveLog("Failed to request service\n");
        return false;
    }

    if (!WalleveGetObject("blockmaker",pBlockMaker))
    {
        WalleveLog("Failed to request blockmaker\n");
        return false;
    }

    return true;
}

void CDispatcher::WalleveHandleDeinitialize()
{
    pCoreProtocol = NULL;
    pWorldLine = NULL;
    pTxPool = NULL;
    pWallet = NULL;
    pService = NULL;
    pBlockMaker = NULL;
}

bool CDispatcher::WalleveHandleInvoke()
{
    return true;
}

void CDispatcher::WalleveHandleHalt()
{
}

MvErr CDispatcher::AddNewBlock(CBlock& block)
{
    MvErr err = MV_OK;
    if (!pWorldLine->Exists(block.hashPrev))
    {
        return MV_ERR_MISSING_PREV;
    }

    CWorldLineUpdate updateWorldLine;
    err = pWorldLine->AddNewBlock(block,updateWorldLine);
    if (err != MV_OK || updateWorldLine.IsNull())
    {
        return err;
    }
/*
    vector<uint256> vTxAddNew;
    set<uint256> setTxUpdate(updateWorldLine.setTxUpdate);
    BOOST_FOREACH(const uint256& txid,updateWorldLine.vTxAddNew)
    {
        if (pTxPool->Pop(updateWorldLine.hashFork,txid))
        {
            setTxUpdate.insert(txid);
        }
        else
        {
            vTxAddNew.push_back(txid);
        }
    }

    BOOST_REVERSE_FOREACH(const uint256& txid,updateWorldLine.vTxRemove)
    {
        CTransaction tx;
        pWorldLine->GetTransaction(txid,tx);
    } 
*/
    pService->NotifyWorldLineUpdate(updateWorldLine);

    if (block.IsPrimary())
    {
        CMvEventBlockMakerUpdate *pBlockMakerUpdate = new CMvEventBlockMakerUpdate(0);
        if (pBlockMakerUpdate != NULL)
        {
            pBlockMakerUpdate->data.first = block.GetHash();
            pBlockMakerUpdate->data.second = block.GetBlockTime();
            pBlockMaker->PostEvent(pBlockMakerUpdate);
        }
    }
    return MV_OK;
}

MvErr CDispatcher::AddNewTx(CTransaction& tx)
{
    return MV_OK;
}
