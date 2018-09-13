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
    pNetChannel = NULL;
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

    if (!WalleveGetObject("netchannel",pNetChannel))
    {
        WalleveLog("Failed to request netchannel\n");
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
    pNetChannel = NULL;
}

bool CDispatcher::WalleveHandleInvoke()
{
    return true;
}

void CDispatcher::WalleveHandleHalt()
{
}

MvErr CDispatcher::AddNewBlock(CBlock& block,uint64 nNonce)
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

    CTxSetChange changeTxSet;
    if (!pTxPool->SynchronizeWorldLine(updateWorldLine,changeTxSet))
    {
        return MV_ERR_SYS_DATABASE_ERROR;
    }
    if (!pWallet->SynchronizeTxSet(changeTxSet))
    {
        return MV_ERR_SYS_DATABASE_ERROR;
    }

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

    if (!nNonce)
    {
        pNetChannel->BroadcastBlockInv(updateWorldLine.hashFork,block.GetHash());
    }

    return MV_OK;
}

MvErr CDispatcher::AddNewTx(CTransaction& tx,uint64 nNonce)
{
    MvErr err = MV_OK;
    err = pCoreProtocol->ValidateTransaction(tx);
    if (err != MV_OK)
    {
        return err;
    }
    set<uint256> setMissingPrevTx;
    BOOST_FOREACH(const CTxIn& txin,tx.vInput)
    {
        if (!pTxPool->Exists(txin.prevout.hash) && !pWorldLine->ExistsTx(txin.prevout.hash))
        {
            setMissingPrevTx.insert(txin.prevout.hash);
        }
    }
    if (!setMissingPrevTx.empty())
    {
        return MV_ERR_MISSING_PREV;
    }
    uint256 hashFork;
    CDestination destIn;
    int64 nValueIn;
    err = pTxPool->Push(tx,hashFork,destIn,nValueIn);
    if (err != MV_OK)
    {
        return err;
    }
    if (!pWallet->UpdateTx(hashFork,CAssembledTx(tx,-1,destIn,nValueIn)))
    {
        return MV_ERR_SYS_DATABASE_ERROR;
    }

    CTransactionUpdate updateTransaction;
    updateTransaction.hashFork = hashFork; 
    updateTransaction.txUpdate = tx;
    pService->NotifyTransactionUpdate(updateTransaction);

    if (!nNonce)
    {
        pNetChannel->BroadcastTxInv(hashFork);
    }
    return MV_OK;
}
