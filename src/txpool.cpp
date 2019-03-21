// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "txpool.h"

using namespace std;
using namespace walleve;
using namespace multiverse;

//////////////////////////////
// CTxPoolCandidate
class CTxPoolCandidate
{
public:
    CTxPoolCandidate() : nTxSeq(-1),nTotalTxFee(0),nTotalSize(0) {}
    CTxPoolCandidate(const pair<uint256,CPooledTx*>& ptx) 
    : nTxSeq(ptx.second->nSequenceNumber),nTotalTxFee(0),nTotalSize(0)
    {
        AddNewTx(ptx);
    }
    void AddNewTx(const pair<uint256,CPooledTx*>& ptx)
    {
        if (mapPoolTx.insert(ptx).second)
        {
            nTotalTxFee += ptx.second->nTxFee;
            nTotalSize += ptx.second->nSerializeSize;
        }
    }
    bool Have(const uint256& txid) const
    {
        return (!!mapPoolTx.count(txid));
    } 
    int64 GetTxFeePerKB() const
    {
        return (nTotalSize != 0 ? (nTotalTxFee << 10) / nTotalSize : 0);
    }
public:
    size_t nTxSeq;
    int64 nTotalTxFee;
    size_t nTotalSize;
    map<uint256,CPooledTx*> mapPoolTx;
};

//////////////////////////////
// CTxPoolView
void CTxPoolView::InvalidateSpent(const CTxOutPoint& out,vector<uint256>& vInvolvedTx)
{
    vector<CTxOutPoint> vOutPoint;
    vOutPoint.push_back(out);
    for (std::size_t i = 0;i < vOutPoint.size();i++)
    {
        uint256 txidNextTx;
        if (GetSpent(vOutPoint[i],txidNextTx))
        {
            CPooledTx* pNextTx = NULL;
            if ((pNextTx = Get(txidNextTx)) != NULL)
            {
                for(const CTxIn& txin : pNextTx->vInput)
                {
                    SetUnspent(txin.prevout);
                }
                CTxOutPoint out0(txidNextTx,0);
                if (IsSpent(out0))
                {
                    vOutPoint.push_back(out0);
                }
                else
                {
                    mapSpent.erase(out0);
                }
                CTxOutPoint out1(txidNextTx,1);
                if (IsSpent(out1))
                {
                    vOutPoint.push_back(out1);
                }
                else
                {
                    mapSpent.erase(out1);
                }
                mapTx.erase(txidNextTx);
                vInvolvedTx.push_back(txidNextTx);
            }
        }
    }
}
 
void CTxPoolView::GetSortedTx(map<size_t,pair<uint256,CPooledTx*> >& mapSortedTx)
{
    for (map<uint256,CPooledTx*>::iterator mi = mapTx.begin();mi != mapTx.end(); ++mi)
    {
        CPooledTx* pPooledTx = (*mi).second;
        {
            mapSortedTx.insert(make_pair(pPooledTx->nSequenceNumber,(*mi)));
        }
    }
}

void CTxPoolView::GetInvolvedTx(map<size_t,pair<uint256,CPooledTx*> >& mapInvolvedTx,
                                set<CDestination>& setDest)
{
    for (map<uint256,CPooledTx*>::iterator mi = mapTx.begin();mi != mapTx.end(); ++mi)
    {
        CPooledTx* pPooledTx = (*mi).second;
        if (setDest.count(pPooledTx->sendTo) || setDest.count(pPooledTx->destIn))
        {
            mapInvolvedTx.insert(make_pair(pPooledTx->nSequenceNumber,(*mi)));
        }
    }
}

void CTxPoolView::ArrangeBlockTx(vector<CTransaction>& vtx,int64& nTotalTxFee,int64 nBlockTime,size_t nMaxSize)
{
    nTotalTxFee = 0;

    map<size_t,pair<uint256,CPooledTx*> > mapCandidate;
    for (map<uint256,CPooledTx*>::iterator it = mapTx.begin();it != mapTx.end();++it)
    {
        if ((*it).second->GetTxTime() <= nBlockTime)
        {
            mapCandidate.insert(make_pair((*it).second->nSequenceNumber,(*it)));
        }
    }
    
    size_t nTotalSize = 0;
    for (map<size_t,pair<uint256,CPooledTx*> >::iterator it = mapCandidate.begin();
         it != mapCandidate.end(); ++it)
    {
        if (nTotalSize + (*it).second.second->nSerializeSize > nMaxSize)
        {
            break;
        }
        vtx.push_back(*static_cast<CTransaction*>((*it).second.second));
        nTotalSize += (*it).second.second->nSerializeSize;
        nTotalTxFee += (*it).second.second->nTxFee; 
    }
}

void CTxPoolView::ArrangeBlockTx(map<size_t,pair<uint256,CPooledTx*> >& mapArrangedTx,size_t nMaxSize)
{
    size_t nTotalSize = 0;
    mapArrangedTx.clear();

    multimap<int64,CTxPoolCandidate> mapCandidate;
    for (map<uint256,CPooledTx*>::iterator it = mapTx.begin();it != mapTx.end();++it)
    {
        nTotalSize += (*it).second->nSerializeSize;

        CTxPoolCandidate candidateTx(*it);
        mapCandidate.insert(make_pair(-candidateTx.GetTxFeePerKB(),candidateTx));
    }

    if (nTotalSize > nMaxSize)
    {
        nTotalSize = 0;

        multimap<int64,CTxPoolCandidate>::iterator it;
        while ((it = mapCandidate.begin()) != mapCandidate.end())
        {
            CTxPoolCandidate candidate = (*it).second;
            mapCandidate.erase(it);
            if (mapArrangedTx.count(candidate.nTxSeq))
            {
                continue;
            }
            
            if (candidate.mapPoolTx.size() == 1)
            {
                vector<pair<uint256,CPooledTx*> > vTxTree;
                vTxTree.push_back(*candidate.mapPoolTx.begin());
                for (std::size_t i = 0;i < vTxTree.size();i++)
                {
                    CPooledTx* pPooledTx = vTxTree[i].second;
                    for(const CTxIn& txin : pPooledTx->vInput)
                    {
                        const uint256& txidPrev = txin.prevout.hash;
                        if (!candidate.Have(txidPrev))
                        {
                            map<uint256,CPooledTx*>::iterator mi = mapTx.find(txidPrev);
                            if (mi != mapTx.end() && !mapArrangedTx.count((*mi).second->nSequenceNumber))
                            {
                                candidate.AddNewTx(*mi);
                                vTxTree.push_back(*mi);
                            }
                        }
                    }
                }
                int64 nTxFeePerKB = candidate.GetTxFeePerKB();
                it = mapCandidate.begin();
                if (it != mapCandidate.end() && -(*it).first > nTxFeePerKB)
                {
                    mapCandidate.insert(make_pair(-nTxFeePerKB,candidate));
                    continue;
                }
            }
            if (nTotalSize + candidate.nTotalSize <= nMaxSize)
            {
                for (map<uint256,CPooledTx*>::iterator mi = candidate.mapPoolTx.begin();
                     mi != candidate.mapPoolTx.end();++mi)
                {
                    if (mapArrangedTx.insert(make_pair((*mi).second->nSequenceNumber,*mi)).second)
                    {
                        nTotalSize += (*mi).second->nSerializeSize;
                    }
                }    
                if (nTotalSize + MIN_TOKEN_TX_SIZE > nMaxSize)
                {
                    break;
                }
            }
        }
    }
    else
    {
        for (map<uint256,CPooledTx*>::iterator it = mapTx.begin();it != mapTx.end();++it)
        {
            mapArrangedTx.insert(make_pair((*it).second->nSequenceNumber,*it));
        }
    }
}
 
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
        WalleveError("Failed to request coreprotocol\n");
        return false;
    }

    if (!WalleveGetObject("worldline",pWorldLine))
    {
        WalleveError("Failed to request worldline\n");
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
    if (!datTxPool.Initialize(WalleveConfig()->pathData))
    {
        WalleveError("Failed to initialize txpool data\n");
        return false;
    }

    if (!LoadData())
    {
        WalleveError("Failed to load txpool data\n");
        return false;
    }

    return true;
}

void CTxPool::WalleveHandleHalt()
{
    if (!SaveData())
    {
        WalleveError("Failed to save txpool data\n");
    }
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

MvErr CTxPool::Push(const CTransaction& tx,uint256& hashFork,CDestination& destIn,int64& nValueIn)
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

    int32 nHeight;
    if (!pWorldLine->GetBlockLocation(tx.hashAnchor,hashFork,nHeight))
    {
        return MV_ERR_TRANSACTION_INVALID;
    }
    
    CTxPoolView& txView = mapPoolView[hashFork];    
    MvErr err = AddNew(txView,txid,tx,hashFork,nHeight);
    if (err == MV_OK)
    {
        CPooledTx* pPooledTx = txView.Get(txid);
        if (pPooledTx == NULL)
        {
            return MV_ERR_NOT_FOUND;
        }
        destIn = pPooledTx->destIn;
        nValueIn = pPooledTx->nValueIn;
    }

    return err;
}   

void CTxPool::Pop(const uint256& txid)
{
    boost::unique_lock<boost::shared_mutex> wlock(rwAccess);
    map<uint256,CPooledTx>::iterator it = mapTx.find(txid);
    if (it == mapTx.end())
    {
        return;
    }   
    CPooledTx& tx = (*it).second;
    uint256 hashFork;
    int32 nHeight;
    if (!pWorldLine->GetBlockLocation(tx.hashAnchor,hashFork,nHeight))
    {
        return;
    }
    vector<uint256> vInvalidTx;
    CTxPoolView& txView = mapPoolView[hashFork];    
    txView.Remove(txid);
    txView.InvalidateSpent(CTxOutPoint(txid,0),vInvalidTx);
    txView.InvalidateSpent(CTxOutPoint(txid,1),vInvalidTx);
    for(const uint256& txidInvalid : vInvalidTx)
    {
        mapTx.erase(txidInvalid);
    }
}

bool CTxPool::Get(const uint256& txid,CTransaction& tx) const
{
    boost::shared_lock<boost::shared_mutex> rlock(rwAccess);
    map<uint256,CPooledTx>::const_iterator it = mapTx.find(txid);
    if (it != mapTx.end())
    {
        tx = (*it).second;
        return true;
    }    
    return false;
}

void CTxPool::ListTx(const uint256& hashFork,vector<pair<uint256,size_t> >& vTxPool)
{
    boost::shared_lock<boost::shared_mutex> rlock(rwAccess);
    map<uint256,CTxPoolView>::iterator it = mapPoolView.find(hashFork);
    if (it != mapPoolView.end())
    {
        CTxPoolView& txView = (*it).second;
        map<size_t,pair<uint256,CPooledTx*> > mapFilteredTx;
        txView.GetSortedTx(mapFilteredTx);
        for (map<size_t,pair<uint256,CPooledTx*> >::iterator mi = mapFilteredTx.begin();
             mi != mapFilteredTx.end();++mi)
        {
            vTxPool.push_back(make_pair((*mi).second.first,(*mi).second.second->nSerializeSize));
        }
    }
}

void CTxPool::ListTx(const uint256& hashFork,vector<uint256>& vTxPool)
{
    boost::shared_lock<boost::shared_mutex> rlock(rwAccess);
    map<uint256,CTxPoolView>::iterator it = mapPoolView.find(hashFork);
    if (it != mapPoolView.end())
    {
        CTxPoolView& txView = (*it).second;
        map<size_t,pair<uint256,CPooledTx*> > mapFilteredTx;
        txView.GetSortedTx(mapFilteredTx);
        for (map<size_t,pair<uint256,CPooledTx*> >::iterator mi = mapFilteredTx.begin();
             mi != mapFilteredTx.end();++mi)
        {
            vTxPool.push_back((*mi).second.first);
        }
    }
}

bool CTxPool::FilterTx(const uint256& hashFork,CTxFilter& filter)
{
    boost::shared_lock<boost::shared_mutex> rlock(rwAccess);

    map<uint256,CTxPoolView>::iterator it = mapPoolView.find(hashFork);
    if (it == mapPoolView.end())
    {
        return true;
    }
    
    CTxPoolView& txView = (*it).second;
    map<size_t,pair<uint256,CPooledTx*> > mapFilteredTx;
    txView.GetInvolvedTx(mapFilteredTx,filter.setDest);

    for (map<size_t,pair<uint256,CPooledTx*> >::iterator mi = mapFilteredTx.begin();
         mi != mapFilteredTx.end();++mi)
    {
        if (!filter.FoundTx(hashFork,*static_cast<CAssembledTx*>((*mi).second.second)))
        {
            return false;
        }
    }
    return true;
}

void CTxPool::ArrangeBlockTx(const uint256& hashFork,int64 nBlockTime,size_t nMaxSize,
                             vector<CTransaction>& vtx,int64& nTotalTxFee)
{
    boost::shared_lock<boost::shared_mutex> rlock(rwAccess);
    CTxPoolView& txView = mapPoolView[hashFork];
    txView.ArrangeBlockTx(vtx,nTotalTxFee,nBlockTime,nMaxSize);
}

bool CTxPool::FetchInputs(const uint256& hashFork,const CTransaction& tx,vector<CTxOutput>& vUnspent)
{
    boost::shared_lock<boost::shared_mutex> rlock(rwAccess);
    CTxPoolView& txView = mapPoolView[hashFork];

    vUnspent.resize(tx.vInput.size());

    for (std::size_t i = 0;i < tx.vInput.size();i++)
    {
        if (txView.IsSpent(tx.vInput[i].prevout))
        {
            return false;
        }
        txView.GetUnspent(tx.vInput[i].prevout,vUnspent[i]);
    }

    if (!pWorldLine->GetTxUnspent(hashFork,tx.vInput,vUnspent))
    {
        return false;
    }

    CDestination destIn;
    for (std::size_t i = 0;i < tx.vInput.size();i++)
    {
        if (vUnspent[i].IsNull())
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
    
    return true;
}

bool CTxPool::SynchronizeWorldLine(const CWorldLineUpdate& update,CTxSetChange& change)
{
    change.hashFork = update.hashFork;

    boost::unique_lock<boost::shared_mutex> wlock(rwAccess);
    
    vector<uint256> vInvalidTx;
    CTxPoolView& txView = mapPoolView[update.hashFork];

    int32 nHeight = update.nLastBlockHeight - update.vBlockAddNew.size() + 1;
    BOOST_REVERSE_FOREACH(const CBlockEx& block,update.vBlockAddNew)
    {
        if (block.txMint.nAmount != 0)
        {
            change.vTxAddNew.push_back(CAssembledTx(block.txMint,nHeight));
        }
        for (std::size_t i = 0;i < block.vtx.size();i++)
        {
            const CTransaction& tx = block.vtx[i];
            const CTxContxt& txContxt = block.vTxContxt[i];
            uint256 txid = tx.GetHash();
            if (!update.setTxUpdate.count(txid))
            {
                if (txView.Exists(txid))
                {
                    txView.Remove(txid);
                    mapTx.erase(txid);
                    change.mapTxUpdate.insert(make_pair(txid,nHeight));
                }
                else
                {
                    for(const CTxIn& txin : tx.vInput)
                    {
                        txView.InvalidateSpent(txin.prevout,vInvalidTx);
                    }
                    change.vTxAddNew.push_back(CAssembledTx(tx,nHeight,txContxt.destIn,txContxt.GetValueIn()));
                }
            }
            else
            {
               change.mapTxUpdate.insert(make_pair(txid,nHeight));
            }
        }
        nHeight++;
    }

    vector<pair<uint256,vector<CTxIn> > > vTxRemove;
    for(const CBlockEx& block : update.vBlockRemove)
    {
        for (int i = block.vtx.size() - 1; i >= 0; i--)
        {
            const CTransaction& tx = block.vtx[i];
            uint256 txid = tx.GetHash();
            if (!update.setTxUpdate.count(txid))
            {
                uint256 spent0,spent1;
                
                txView.GetSpent(CTxOutPoint(txid,0),spent0);
                txView.GetSpent(CTxOutPoint(txid,1),spent1);
                if (AddNew(txView,txid,tx,update.hashFork,update.nLastBlockHeight) == MV_OK)
                {
                    if (spent0 != 0) txView.SetSpent(CTxOutPoint(txid,0),spent0);
                    if (spent1 != 0) txView.SetSpent(CTxOutPoint(txid,1),spent0);

                    change.mapTxUpdate.insert(make_pair(txid,-1));
                }
                else
                {
                    txView.InvalidateSpent(CTxOutPoint(txid,0),vInvalidTx);
                    txView.InvalidateSpent(CTxOutPoint(txid,1),vInvalidTx);
                    vTxRemove.push_back(make_pair(txid,tx.vInput));
                }
            }
        }
        if (block.txMint.nAmount != 0)
        {
            uint256 txidMint = block.txMint.GetHash();
            CTxOutPoint outMint(txidMint,0);
            txView.InvalidateSpent(outMint,vInvalidTx);

            vTxRemove.push_back(make_pair(txidMint,block.txMint.vInput));
        }
    }

    change.vTxRemove.reserve(vInvalidTx.size() + vTxRemove.size());
    BOOST_REVERSE_FOREACH(const uint256& txid,vInvalidTx)
    {
        map<uint256,CPooledTx>::iterator it = mapTx.find(txid);
        if (it != mapTx.end())
        {
            change.vTxRemove.push_back(make_pair(txid,(*it).second.vInput));
            mapTx.erase(it);
        } 
    }
    change.vTxRemove.insert(change.vTxRemove.end(),vTxRemove.begin(),vTxRemove.end());

    return true;
} 

bool CTxPool::LoadData()
{
    boost::unique_lock<boost::shared_mutex> wlock(rwAccess);

    vector<pair<uint256,pair<uint256,CAssembledTx> > > vTx;
    if (!datTxPool.Load(vTx))
    {
        return false;
    }

    for (int i = 0;i < vTx.size();i++)
    {
        const uint256& hashFork = vTx[i].first;
        const uint256& txid     = vTx[i].second.first;
        const CAssembledTx& tx  = vTx[i].second.second;

        map<uint256,CPooledTx>::iterator mi = mapTx.insert(make_pair(txid,CPooledTx(tx,GetSequenceNumber()))).first;
        mapPoolView[hashFork].AddNew(txid,(*mi).second);
    }
    return true;
}

bool CTxPool::SaveData()
{
    boost::shared_lock<boost::shared_mutex> rlock(rwAccess);

    map<size_t,pair<uint256,pair<uint256,CAssembledTx> > > mapSortTx;
    for (map<uint256,CTxPoolView>::iterator it = mapPoolView.begin();it != mapPoolView.end();++it)
    {
        map<uint256,CPooledTx*>& mapForkTx = (*it).second.mapTx;
        for (map<uint256,CPooledTx*>::iterator mi = mapForkTx.begin();mi != mapForkTx.end();++mi)
        {
            size_t n = (*mi).second->nSequenceNumber;
            mapSortTx[n] = make_pair((*it).first,make_pair((*mi).first,static_cast<CAssembledTx&>(*(*mi).second)));
        }
    }
    
    vector<pair<uint256,pair<uint256,CAssembledTx> > > vTx;
    vTx.reserve(mapSortTx.size());
    for (map<size_t,pair<uint256,pair<uint256,CAssembledTx> > >::iterator it = mapSortTx.begin();
         it != mapSortTx.end(); ++it)
    {
        vTx.push_back((*it).second);
    }

    return datTxPool.Save(vTx);
}

MvErr CTxPool::AddNew(CTxPoolView& txView,const uint256& txid,const CTransaction& tx,const uint256& hashFork,const int32 nForkHeight)
{
    vector<CTxOutput> vPrevOutput;
    vPrevOutput.resize(tx.vInput.size());
    for (int i = 0;i < tx.vInput.size();i++)
    {
        if (txView.IsSpent(tx.vInput[i].prevout))
        {
            return MV_ERR_TRANSACTION_CONFLICTING_INPUT;
        }
        txView.GetUnspent(tx.vInput[i].prevout,vPrevOutput[i]);
    }
 
    if (!pWorldLine->GetTxUnspent(hashFork,tx.vInput,vPrevOutput))
    {
        return MV_ERR_SYS_STORAGE_ERROR;
    }

    int64 nValueIn = 0;
    for (int i = 0;i < tx.vInput.size();i++)
    {
        if (vPrevOutput[i].IsNull())
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
    map<uint256,CPooledTx>::iterator mi;
    mi = mapTx.insert(make_pair(txid,CPooledTx(tx,-1,GetSequenceNumber(),destIn,nValueIn))).first;
    txView.AddNew(txid,(*mi).second);

    return MV_OK;
}
