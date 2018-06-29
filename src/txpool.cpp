// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "txpool.h"

using namespace std;
using namespace walleve;
using namespace multiverse;

//////////////////////////////
// CTxPool 

CTxPool::CTxPool()
{
    pCoreProtocol = NULL;
    pWorldLine = NULL;
}

CTxPool::~CTxPool()
{
}

bool CTxPool::WalleveHandleInitialize()
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

    return true;
}

void CTxPool::WalleveHandleDeinitialize()
{
    pCoreProtocol = NULL;
    pWorldLine = NULL;
}

bool CTxPool::WalleveHandleInvoke()
{
    return true;
}

void CTxPool::WalleveHandleHalt()
{
    Clear();
}

bool CTxPool::Exists(const uint256& txid)
{    
    boost::shared_lock<boost::shared_mutex> rlock(rwAccess);
    return (!!mapTx.count(txid));
}

void CTxPool::Clear()
{
    boost::unique_lock<boost::shared_mutex> wlock(rwAccess);
    mapPoolView.clear();
    mapTx.clear();
}

size_t CTxPool::Count(const uint256& fork) const
{
    boost::shared_lock<boost::shared_mutex> rlock(rwAccess);
    map<uint256,CTxPoolView>::const_iterator it = mapPoolView.find(fork);
    if (it != mapPoolView.end())
    {
        return ((*it).second.Count());
    }
    return 0;
}

MvErr CTxPool::Push(CTransaction& tx)
{
    boost::unique_lock<boost::shared_mutex> wlock(rwAccess);
    uint256 txid = tx.GetHash();
    
    if (mapTx.count(txid))
    {
        return MV_ERR_ALREADY_HAVE;
    }
    
    if (tx.IsMintTx())
    {
        return MV_ERR_TRANSACTION_INVALID;
    }

    uint256 hashFork;
    int nHeight;
    if (!pWorldLine->GetBlockLocation(tx.hashAnchor,hashFork,nHeight))
    {
        return MV_ERR_TRANSACTION_INVALID;
    }
    
    return AddNew(mapPoolView[hashFork],txid,tx,hashFork,nHeight);
}   

void CTxPool::Pop(const uint256& txid)
{
    boost::unique_lock<boost::shared_mutex> wlock(rwAccess);
}

bool CTxPool::Get(const uint256& txid,CTransaction& tx) const
{
    boost::shared_lock<boost::shared_mutex> rlock(rwAccess);
    map<uint256,CAssembledTx>::const_iterator it = mapTx.find(txid);
    if (it != mapTx.end())
    {
        tx = (*it).second;
        return true;
    }    
    return false;
}

bool CTxPool::Arrange(uint256& fork,vector<pair<uint256,CTransaction> >& vtx,size_t nMaxSize)
{
    return false;
}

bool CTxPool::FetchInputs(const uint256& hashFork,const CTransaction& tx,vector<CTxOutput>& vUnspent)
{
    if (!pWorldLine->GetTxUnspent(hashFork,tx.vInput,vUnspent))
    {
        return false;
    }
    {
        boost::shared_lock<boost::shared_mutex> rlock(rwAccess);
        CTxPoolView& txView = mapPoolView[hashFork];
        CDestination destIn;
        for (int i = 0;i < tx.vInput.size();i++)
        {
            if (txView.IsSpent(tx.vInput[i].prevout))
            {
                return false;
            }
            if (vUnspent[i].IsNull() && !txView.GetUnspent(tx.vInput[i].prevout,vUnspent[i]))
            {
                return false;
            }
            if (destIn.IsNull())
            {
                destIn = vUnspent[i].destTo;
            }
            else if (destIn != vUnspent[i].destTo)
            {
                return false;
            }
        }
    }
    return true;
}

void CTxPool::SynchronizeWorldLine(CWorldLineUpdate& update,CTxPoolUpdate& change)
{
    vector<uint256> vTxAddNew,vTxRemove;
    map<uint256,int>& mapTxUpdate = change.mapTxUpdate;
    
    boost::unique_lock<boost::shared_mutex> wlock(rwAccess);
    
    vector<uint256> vInvalidTx;
    CTxPoolView& txView = mapPoolView[update.hashFork];

    int nHeight = update.nForkHeight;

    BOOST_FOREACH(CBlockEx& block,update.vBlockAddNew)
    {
        BOOST_FOREACH(CTransaction& tx,block.vtx)
        {
            uint256 txid = tx.GetHash();
            if (!update.setTxUpdate.count(txid))
            {
                if (txView.Exists(txid))
                {
                    txView.Remove(txid);
                    mapTx.erase(txid);
                    mapTxUpdate.insert(make_pair(txid,nHeight));
                }
                else
                {
                    BOOST_FOREACH(const CTxIn& txin,tx.vInput)
                    {
                        txView.InvalidateSpent(txin.prevout,vInvalidTx);
                    }
                    vTxAddNew.push_back(txid);
                }
            }
            else
            {
                mapTxUpdate.insert(make_pair(txid,nHeight));
            }
        }
        nHeight--;
    }

    BOOST_REVERSE_FOREACH(CBlockEx& block,update.vBlockRemove)
    {
        CTxOutPoint outMint(block.txMint.GetHash(),0);
        txView.InvalidateSpent(outMint,vInvalidTx);

        for (int i = block.vtx.size() - 1; i >= 0; i--)
        {
            CTransaction& tx = block.vtx[i];
            uint256 txid = tx.GetHash();
            if (!update.setTxUpdate.count(txid))
            {
                uint256 spent0,spent1;
                
                txView.GetSpent(CTxOutPoint(txid,0),spent0);
                txView.GetSpent(CTxOutPoint(txid,1),spent1);
                if (AddNew(txView,txid,tx,update.hashFork,update.nForkHeight) == MV_OK)
                {
                    if (spent0 != 0) txView.SetSpent(CTxOutPoint(txid,0),spent0);
                    if (spent1 != 0) txView.SetSpent(CTxOutPoint(txid,1),spent0);

                    mapTxUpdate.insert(make_pair(txid,-1));
                }
                else
                {
                    txView.InvalidateSpent(CTxOutPoint(txid,0),vInvalidTx);
                    txView.InvalidateSpent(CTxOutPoint(txid,1),vInvalidTx);
                    vTxRemove.push_back(txid);
                }
            }
        }
    }

    vTxRemove.insert(vTxRemove.end(),vInvalidTx.begin(),vInvalidTx.end());
    BOOST_FOREACH(const uint256& txid,vInvalidTx)
    {
        mapTx.erase(txid);
    }
}

MvErr CTxPool::AddNew(CTxPoolView& txView,const uint256& txid,CTransaction& tx,const uint256& hashFork,int nForkHeight)
{
    vector<CTxOutput> vPrevOutput;
    if (!pWorldLine->GetTxUnspent(hashFork,tx.vInput,vPrevOutput))
    {
        return MV_ERR_SYS_STORAGE_ERROR;
    }
    int64 nValueIn = 0;
    for (int i = 0;i < tx.vInput.size();i++)
    {
        if (txView.IsSpent(tx.vInput[i].prevout))
        {
            return MV_ERR_TRANSACTION_CONFLICTING_INPUT;
        }
        if (vPrevOutput[i].IsNull() && !txView.GetUnspent(tx.vInput[i].prevout,vPrevOutput[i]))
        {
            return MV_ERR_TRANSACTION_CONFLICTING_INPUT;
        }
        nValueIn += vPrevOutput[i].nAmount;
    }
    
    MvErr err = pCoreProtocol->VerifyTransaction(tx,vPrevOutput,nForkHeight);
    if (err != MV_OK)
    {
        return err;
    }
    
    CDestination destIn = vPrevOutput[0].destTo;
    map<uint256,CAssembledTx>::iterator mi = mapTx.insert(make_pair(txid,CAssembledTx(tx,destIn,nValueIn))).first;
    txView.AddNew(txid,(*mi).second);

    return MV_OK;
}
