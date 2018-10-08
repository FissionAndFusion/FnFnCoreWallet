// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "dispatcher.h"
#include "event.h"
#include <boost/foreach.hpp>

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
    pConsensus = NULL;
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

    if (!WalleveGetObject("consensus",pConsensus))
    {
        WalleveLog("Failed to request consensus\n");
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
    pConsensus = NULL;
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

MvErr CDispatcher::AddNewBlock(const CBlock& block,uint64 nNonce)
{
    MvErr err = MV_OK;
    if (!pWorldLine->Exists(block.hashPrev))
    {
        return MV_ERR_MISSING_PREV;
    }

    CWorldLineUpdate updateWorldLine;
    if (!block.IsOrigin())
    {
        err = pWorldLine->AddNewBlock(block,updateWorldLine);
    }
    else
    {
        err = pWorldLine->AddNewOrigin(block,updateWorldLine);
    }
    if (err != MV_OK || updateWorldLine.IsNull())
    {
        return err;
    }

    CTxSetChange changeTxSet;
    if (!pTxPool->SynchronizeWorldLine(updateWorldLine,changeTxSet))
    {
        return MV_ERR_SYS_DATABASE_ERROR;
    }

    if (block.IsOrigin())
    {
        if (!pWallet->AddNewFork(updateWorldLine.hashFork,updateWorldLine.hashParent,
                                                          updateWorldLine.nOriginHeight))
        {
            return MV_ERR_SYS_DATABASE_ERROR;
        }
    }

    if (!pWallet->SynchronizeTxSet(changeTxSet))
    {
        return MV_ERR_SYS_DATABASE_ERROR;
    }

    pService->NotifyWorldLineUpdate(updateWorldLine);

    if (!nNonce && !block.IsOrigin() && !block.IsVacant())
    {
        pNetChannel->BroadcastBlockInv(updateWorldLine.hashFork,block.GetHash());
    }

    if (block.IsPrimary())
    {
        UpdatePrimaryBlock(updateWorldLine,changeTxSet);
    }

    return MV_OK;
}

MvErr CDispatcher::AddNewTx(const CTransaction& tx,uint64 nNonce)
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

    if (hashFork == pCoreProtocol->GetGenesisBlockHash())
    {
        pConsensus->AddNewTx(CAssembledTx(tx,-1,destIn,nValueIn));
    }
    return MV_OK;
}

void CDispatcher::UpdatePrimaryBlock(const CWorldLineUpdate& updateWorldLine,const CTxSetChange& changeTxSet)
{
    CDelegateRoutine routineDelegate;
    pConsensus->PrimaryUpdate(updateWorldLine,changeTxSet,routineDelegate);
    BOOST_FOREACH(const CTransaction& tx,routineDelegate.vEnrollTx)
    {
        MvErr err = AddNewTx(tx);
        WalleveLog("Send DelegateTx %s (%s)\n",MvErrString(err),tx.GetHash().GetHex().c_str());
    }

    CMvEventBlockMakerUpdate *pBlockMakerUpdate = new CMvEventBlockMakerUpdate(0);
    if (pBlockMakerUpdate != NULL)
    {
        pBlockMakerUpdate->data.hashBlock = updateWorldLine.hashLastBlock;
        pBlockMakerUpdate->data.nBlockTime = updateWorldLine.nLastBlockTime;
        pBlockMakerUpdate->data.nBlockHeight = updateWorldLine.nLastBlockHeight;
        pBlockMaker->PostEvent(pBlockMakerUpdate);
    }

    for (int i = updateWorldLine.vBlockAddNew.size() - 1;i >= 0;i--)
    {
        BOOST_FOREACH(const CTransaction& tx,updateWorldLine.vBlockAddNew[i].vtx)
        {
            CTemplateId tid;
            if (tx.sendTo.GetTemplateId(tid) && tid.GetType() == TEMPLATE_FORK && !tx.vchData.empty())
            {
                ProcessForkTx(tx,updateWorldLine.nLastBlockHeight);
            }
        }
    }

    SyncForkHeight(updateWorldLine.nLastBlockHeight);
}

void CDispatcher::ProcessForkTx(const CTransaction& tx,int nPrimaryHeight)
{
    uint256 txid = tx.GetHash();

    CBlock block;
    try
    {
        CWalleveBufStream ss;
        ss.Write((const char*)&tx.vchData[0],tx.vchData.size());
        ss >> block;
        if (!block.IsOrigin() || block.IsPrimary())
        {
            throw std::runtime_error("invalid block");
        }
    }
    catch (...) 
    { 
        WalleveLog("Invalid orign block found in tx (%s)\n",txid.GetHex().c_str());
        return;
    }
    
    MvErr err = AddNewBlock(block);
    if (err == MV_OK)
    {
        WalleveLog("Add origin block in tx (%s), hash=%s\n",txid.GetHex().c_str(),
                                                            block.GetHash().GetHex().c_str());
    }
    else
    {
        WalleveLog("Add origin block in tx (%s) failed : %s\n",txid.GetHex().c_str(),
                                                               MvErrString(err));
    }
}

void CDispatcher::SyncForkHeight(int nPrimaryHeight)
{
    map<uint256,CForkStatus> mapForkStatus;
    pWorldLine->GetForkStatus(mapForkStatus);
    for (map<uint256,CForkStatus>::iterator it = mapForkStatus.begin();it != mapForkStatus.end();++it)
    {
        const uint256& hashFork = (*it).first;
        CForkStatus& status = (*it).second;
        
        vector<int64> vTimeStamp;
        int nDepth = nPrimaryHeight - status.nLastBlockHeight;

        if (nDepth > 1 && hashFork != pCoreProtocol->GetGenesisBlockHash()
            && pWorldLine->GetLastBlockTime(pCoreProtocol->GetGenesisBlockHash(),nDepth,vTimeStamp))
        {
            uint256 hashPrev = status.hashLastBlock;
            for (int nHeight = status.nLastBlockHeight + 1;nHeight < nPrimaryHeight;nHeight++)
            {
                CBlock block;
                block.nType = CBlock::BLOCK_VACANT;
                block.hashPrev = hashPrev;
                block.nTimeStamp = vTimeStamp[nPrimaryHeight - nHeight];
                if (AddNewBlock(block) != MV_OK)
                {
                    break;
                }
                hashPrev = block.GetHash();
            }
        }
    }
}
