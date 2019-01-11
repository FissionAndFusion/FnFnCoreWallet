// Copyright (c) 2017-2019 The Multiverse developers
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
    pForkManager = NULL;
    pConsensus = NULL;
    pWallet = NULL;
    pService = NULL;
    pBlockMaker = NULL;
    pNetChannel = NULL;
    pDelegatedChannel = NULL;
}

CDispatcher::~CDispatcher()
{
}

bool CDispatcher::WalleveHandleInitialize()
{
    if (!WalleveGetObject("coreprotocol",pCoreProtocol))
    {
        WalleveError("Failed to request coreprotocol\n");
        return false;
    }

    if (!WalleveGetObject("worldline",pWorldLine))
    {
        WalleveError("Failed to request worldline\n");
        return false;
    }

    if (!WalleveGetObject("txpool",pTxPool))
    {
        WalleveError("Failed to request txpool\n");
        return false;
    }

    if (!WalleveGetObject("forkmanager",pForkManager))
    {
        WalleveError("Failed to request forkmanager\n");
        return false;
    }

    if (!WalleveGetObject("consensus",pConsensus))
    {
        WalleveError("Failed to request consensus\n");
        return false;
    }

    if (!WalleveGetObject("wallet",pWallet))
    {
        WalleveError("Failed to request wallet\n");
        return false;
    }

    if (!WalleveGetObject("service",pService))
    {
        WalleveError("Failed to request service\n");
        return false;
    }

    if (!WalleveGetObject("blockmaker",pBlockMaker))
    {
        WalleveError("Failed to request blockmaker\n");
        return false;
    }

    if (!WalleveGetObject("netchannel",pNetChannel))
    {
        WalleveError("Failed to request netchannel\n");
        return false;
    }

    if (!WalleveGetObject("delegatedchannel",pDelegatedChannel))
    {
        WalleveError("Failed to request delegatedchannel\n");
        return false;
    }

    return true;
}

void CDispatcher::WalleveHandleDeinitialize()
{
    pCoreProtocol = NULL;
    pWorldLine = NULL;
    pTxPool = NULL;
    pForkManager = NULL;
    pConsensus = NULL;
    pWallet = NULL;
    pService = NULL;
    pBlockMaker = NULL;
    pNetChannel = NULL;
    pDelegatedChannel = NULL;
}

bool CDispatcher::WalleveHandleInvoke()
{
    vector<uint256> vActive;
    if (!pForkManager->LoadForkContext(vActive))
    {
        WalleveError("Failed to load for context\n");
        return false;
    }

    BOOST_FOREACH(const uint256& hashFork,vActive)
    {
        ActivateFork(hashFork); 
    }

    return true;
}

void CDispatcher::WalleveHandleHalt()
{
}

MvErr CDispatcher::AddNewBlock(const CBlock& block,uint64 nNonce)
{
    boost::recursive_mutex::scoped_lock scoped_lock(mtxDispatcher);

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

    if (!block.IsVacant())
    {
        vector<uint256> vActive,vDeactive;
        pForkManager->ForkUpdate(updateWorldLine,vActive,vDeactive);

        BOOST_FOREACH(const uint256 hashFork,vActive)
        {
            ActivateFork(hashFork);
        }

        BOOST_FOREACH(const uint256 hashFork,vDeactive)
        {
            pNetChannel->UnsubscribeFork(hashFork);
        }
    }

    if (block.IsPrimary())
    {
        UpdatePrimaryBlock(block,updateWorldLine,changeTxSet);
    }

    return MV_OK;
}

MvErr CDispatcher::AddNewTx(const CTransaction& tx,uint64 nNonce)
{
    boost::recursive_mutex::scoped_lock scoped_lock(mtxDispatcher);

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

    CAssembledTx assembledTx(tx,-1,destIn,nValueIn);
    if (!pWallet->AddNewTx(hashFork,assembledTx))
    {
        return MV_ERR_SYS_DATABASE_ERROR;
    }

    CTransactionUpdate updateTransaction;
    updateTransaction.hashFork = hashFork; 
    updateTransaction.txUpdate = tx;
    updateTransaction.nChange = assembledTx.GetChange();
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

bool CDispatcher::AddNewDistribute(const uint256& hashAnchor,const CDestination& dest,const vector<unsigned char>& vchDistribute)
{
    uint256 hashFork;
    int nHeight;
    if (pWorldLine->GetBlockLocation(hashAnchor,hashFork,nHeight) && hashFork == pCoreProtocol->GetGenesisBlockHash())
    {
        return pConsensus->AddNewDistribute(nHeight,dest,vchDistribute);
    }
    return false;
}

bool CDispatcher::AddNewPublish(const uint256& hashAnchor,const CDestination& dest,const vector<unsigned char>& vchPublish)
{
    uint256 hashFork;
    int nHeight;
    if (pWorldLine->GetBlockLocation(hashAnchor,hashFork,nHeight) && hashFork == pCoreProtocol->GetGenesisBlockHash())
    {
        return pConsensus->AddNewPublish(nHeight,dest,vchPublish);
    }
    return false;
}

void CDispatcher::UpdatePrimaryBlock(const CBlock& block,const CWorldLineUpdate& updateWorldLine,const CTxSetChange& changeTxSet)
{
    CDelegateRoutine routineDelegate;
    pConsensus->PrimaryUpdate(updateWorldLine,changeTxSet,routineDelegate);

    pDelegatedChannel->PrimaryUpdate(updateWorldLine.nLastBlockHeight - updateWorldLine.vBlockAddNew.size(),
                                     routineDelegate.vEnrolledWeight,routineDelegate.mapDistributeData,routineDelegate.mapPublishData);

    BOOST_FOREACH(const CTransaction& tx,routineDelegate.vEnrollTx)
    {
        MvErr err = AddNewTx(tx);
        WalleveLog("Send DelegateTx %s (%s)\n",MvErrString(err),tx.GetHash().GetHex().c_str());
    }

    CMvEventBlockMakerUpdate *pBlockMakerUpdate = new CMvEventBlockMakerUpdate(0);
    if (pBlockMakerUpdate != NULL)
    {
        CProofOfSecretShare proof;
        proof.Load(block.vchProof);
        pBlockMakerUpdate->data.hashBlock = updateWorldLine.hashLastBlock;
        pBlockMakerUpdate->data.nBlockTime = updateWorldLine.nLastBlockTime;
        pBlockMakerUpdate->data.nBlockHeight = updateWorldLine.nLastBlockHeight;
        pBlockMakerUpdate->data.nAgreement = proof.nAgreement;
        pBlockMakerUpdate->data.nWeight = proof.nWeight;
        pBlockMaker->PostEvent(pBlockMakerUpdate);
    }

    SyncForkHeight(updateWorldLine.nLastBlockHeight);
}

void CDispatcher::ActivateFork(const uint256& hashFork)
{
    WalleveLog("Activating fork %s ...\n",hashFork.GetHex().c_str());
    if (!pWorldLine->Exists(hashFork))
    {
        CForkContext ctxt;
        if (!pWorldLine->GetForkContext(hashFork,ctxt))
        {
            WalleveWarn("Failed to find fork context %s\n",hashFork.GetHex().c_str());
            return;
        }

        CTransaction txFork;
        if (!pWorldLine->GetTransaction(ctxt.txidEmbedded,txFork))
        {
            WalleveWarn("Failed to find tx fork %s\n",hashFork.GetHex().c_str());
            return;
        }
 
        if (!ProcessForkTx(ctxt.txidEmbedded,txFork))
        {
            return;
        }
        WalleveLog("Add origin block in tx (%s), hash=%s\n",ctxt.txidEmbedded.GetHex().c_str(),
                                                            hashFork.GetHex().c_str());
    }
    pNetChannel->SubscribeFork(hashFork);
    WalleveLog("Activated fork %s ...\n",hashFork.GetHex().c_str());
}

bool CDispatcher::ProcessForkTx(const uint256& txid,const CTransaction& tx)
{
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
        WalleveWarn("Invalid orign block found in tx (%s)\n",txid.GetHex().c_str());
        return false;
    }
    
    MvErr err = AddNewBlock(block);
    if (err != MV_OK)
    {
        WalleveLog("Add origin block in tx (%s) failed : %s\n",txid.GetHex().c_str(),
                                                               MvErrString(err));
        return false;
    }
    return true;
}

void CDispatcher::SyncForkHeight(int nPrimaryHeight)
{
    map<uint256,CForkStatus> mapForkStatus;
    pWorldLine->GetForkStatus(mapForkStatus);
    for (map<uint256,CForkStatus>::iterator it = mapForkStatus.begin();it != mapForkStatus.end();++it)
    {
        const uint256& hashFork = (*it).first;
        CForkStatus& status = (*it).second;
        if (!pForkManager->IsAllowed(hashFork) || !pNetChannel->IsForkSynchronized(hashFork))
        {
            continue;
        }

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
