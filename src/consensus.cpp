// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "consensus.h"
#include "address.h"

using namespace std;
using namespace walleve;
using namespace multiverse;

//////////////////////////////
// CDelegateTxFilter
class CDelegateTxFilter : public CTxFilter
{
public:
    CDelegateTxFilter(const uint256& hashForkIn,CDelegateContext& ctxtIn)
    : CTxFilter(ctxtIn.GetDestination(),ctxtIn.GetDestination()),hashFork(hashForkIn),ctxt(ctxtIn)
    {
    }
    bool FoundTx(const uint256& hashForkIn,const CAssembledTx& tx)
    {
        if (hashFork == hashForkIn)
        {
            ctxt.AddNewTx(tx);
        }
        return true;
    }
protected:
    uint256 hashFork;
    CDelegateContext& ctxt;
}; 

//////////////////////////////
// CDelegateContext 

CDelegateContext::CDelegateContext()
{
}

CDelegateContext::CDelegateContext(const crypto::CKey& keyDelegateIn,const CDestination& destOwnerIn)
{
    keyDelegate   = keyDelegateIn;
    destOwner     = destOwnerIn;
    templDelegate = CTemplatePtr(new CTemplateDelegate(keyDelegateIn.GetPubKey(),destOwner));
    destDelegate  = CDestination(templDelegate->GetTemplateId());
}

void CDelegateContext::Clear()
{
    mapTx.clear();
    mapUnspent.clear();
}

void CDelegateContext::ChangeTxSet(const CTxSetChange& change)
{
    for (std::size_t i = 0;i < change.vTxRemove.size();i++)
    {
        const uint256& txid = change.vTxRemove[i].first;
        map<uint256,CDelegateTx>::iterator it = mapTx.find(txid);
        if (it != mapTx.end())
        {
            BOOST_FOREACH(const CTxIn& txin,change.vTxRemove[i].second)
            {
                map<uint256,CDelegateTx>::iterator mi = mapTx.find(txin.prevout.hash);
                if (mi != mapTx.end())
                {
                    mapUnspent.insert(make_pair(txin.prevout,&(*mi).second));
                }
            }
            mapTx.erase(it);
        }
    }
    for (map<uint256,int>::const_iterator it = change.mapTxUpdate.begin();it != change.mapTxUpdate.end();++it)
    {
        const uint256& txid = (*it).first;
        map<uint256,CDelegateTx>::iterator mi = mapTx.find(txid);
        if (mi != mapTx.end())
        {
            (*mi).second.nBlockHeight = (*it).second;
        }
    }
    
    BOOST_FOREACH(const CAssembledTx& tx,change.vTxAddNew)
    {
        AddNewTx(tx);
    }
}

void CDelegateContext::AddNewTx(const CAssembledTx& tx)
{
    if (tx.sendTo == destDelegate)
    {
        if (tx.nType == CTransaction::TX_TOKEN && tx.destIn == destOwner)
        {
            uint256 txid = tx.GetHash();
            CDelegateTx &delegateTx = mapTx[txid];

            delegateTx = CDelegateTx(tx);
            mapUnspent.insert(make_pair(CTxOutPoint(txid,0),&delegateTx));
        }
        else if (tx.nType == CTransaction::TX_CERT && tx.destIn == destDelegate)
        {
            uint256 txid = tx.GetHash();
            CDelegateTx &delegateTx = mapTx[txid];

            delegateTx = CDelegateTx(tx);
            mapUnspent.insert(make_pair(CTxOutPoint(txid,0),&delegateTx));
            if (delegateTx.nChange != 0)
            {
                mapUnspent.insert(make_pair(CTxOutPoint(txid,1),&delegateTx));
            }
        }
    }
    if (tx.destIn == destDelegate)
    {
        BOOST_FOREACH(const CTxIn& txin,tx.vInput)
        {
            mapUnspent.erase(txin.prevout);
        }
    } 
}

bool CDelegateContext::BuildEnrollTx(CTransaction& tx,int nBlockHeight,const uint256& hashAnchor,int64 nTxFee,
                                                      const vector<unsigned char>& vchData)
{
    tx.SetNull();
    tx.nType = CTransaction::TX_CERT;
    tx.hashAnchor = hashAnchor;
    tx.sendTo = destDelegate;
    tx.nAmount = 0;
    tx.nTxFee = nTxFee;
    tx.vchData = vchData;

    int64 nValueIn = 0;
    for (map<CTxOutPoint,CDelegateTx*>::iterator it = mapUnspent.begin();it != mapUnspent.end();++it)
    {
        const CTxOutPoint& txout = (*it).first;
        CDelegateTx* pTx = (*it).second;
        if (pTx->nLockUntil != 0 && nBlockHeight < pTx->nLockUntil)
        {
            continue;
        }
        tx.vInput.push_back(CTxIn(txout)); 
        nValueIn += (txout.n == 0 ? pTx->nAmount : pTx->nChange);
        if (nValueIn > nTxFee)
        {
            break;
        }
    }
    if (nValueIn <= nTxFee)
    {
        return false;
    }

    tx.nAmount = nValueIn - nTxFee;

    uint256 hash = tx.GetSignatureHash();
    vector<unsigned char> vchDelegateSig;
    if (!keyDelegate.Sign(hash,vchDelegateSig))
    {
        return false;
    }
    CTemplateDelegate* p = (CTemplateDelegate*)templDelegate.get();
    return p->BuildVssSignature(hash,vchDelegateSig,tx.vchSig);
} 

//////////////////////////////
// CConsensus 

CConsensus::CConsensus()
{
    pCoreProtocol = NULL;
    pWorldLine = NULL;
    pTxPool = NULL;
}

CConsensus::~CConsensus()
{
}

bool CConsensus::WalleveHandleInitialize()
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

    if (!MintConfig()->destMPVss.IsNull() && MintConfig()->keyMPVss != 0)
    {
        crypto::CKey key;
        key.SetSecret(crypto::CCryptoKeyData(MintConfig()->keyMPVss.begin(),MintConfig()->keyMPVss.end()));

        CDelegateContext ctxt(key,MintConfig()->destMPVss);
        mapContext.insert(make_pair(ctxt.GetDestination(),ctxt));

        mvDelegate.AddNewDelegate(ctxt.GetDestination());
        
        WalleveLog("AddNew delegate : %s\n",CMvAddress(ctxt.GetDestination()).ToString().c_str());
    }

    return true;
}

void CConsensus::WalleveHandleDeinitialize()
{
    mapContext.clear();

    pCoreProtocol = NULL;
    pWorldLine = NULL;
    pTxPool = NULL;
}

bool CConsensus::WalleveHandleInvoke()
{
    boost::unique_lock<boost::mutex> lock(mutex);

    if (!mvDelegate.Initialize())
    {
        WalleveError("Failed to initialize delegate\n");
        return false;
    }

    if (!LoadDelegateTx())
    {
        WalleveError("Failed to load delegate tx\n");
        return false;
    }
    
    return true;
}

void CConsensus::WalleveHandleHalt()
{
    boost::unique_lock<boost::mutex> lock(mutex);

    mvDelegate.Deinitialize();
    for (map<CDestination,CDelegateContext>::iterator it = mapContext.begin();it != mapContext.end();++it)
    {
        (*it).second.Clear();
    }
}

void CConsensus::PrimaryUpdate(const CWorldLineUpdate& update,const CTxSetChange& change,CDelegateRoutine& routine)
{
    boost::unique_lock<boost::mutex> lock(mutex);

    int nStartHeight = update.nLastBlockHeight - update.vBlockAddNew.size();
    if (!update.vBlockRemove.empty())
    {
        int nPrevBlockHeight = nStartHeight + update.vBlockRemove.size();
        mvDelegate.Rollback(nPrevBlockHeight,nStartHeight);
    }

    for (map<CDestination,CDelegateContext>::iterator it = mapContext.begin();it != mapContext.end();++it)
    {
        (*it).second.ChangeTxSet(change);
    }

    int nBlockHeight = nStartHeight + 1;

    for (int i = update.vBlockAddNew.size() - 1;i > 0;i--) 
    {
        uint256 hash = update.vBlockAddNew[i].GetHash();

        map<CDestination,size_t> mapWeight;
        map<CDestination,vector<unsigned char> > mapEnrollData;

        if (pWorldLine->GetBlockDelegateEnrolled(hash,mapWeight,mapEnrollData))
        {
            delegate::CMvDelegateEvolveResult result;
            mvDelegate.Evolve(nBlockHeight,mapWeight,mapEnrollData,result); 
        }

        routine.vEnrolledWeight.push_back(make_pair(hash,mapWeight));

        nBlockHeight++;
    }

    if (!update.vBlockAddNew.empty())
    {
        uint256 hash = update.vBlockAddNew[0].GetHash();

        map<CDestination,size_t> mapWeight;
        map<CDestination,vector<unsigned char> > mapEnrollData;

        if (pWorldLine->GetBlockDelegateEnrolled(hash,mapWeight,mapEnrollData))
        {
            delegate::CMvDelegateEvolveResult result;
            mvDelegate.Evolve(nBlockHeight,mapWeight,mapEnrollData,result); 
            
            int nDistributeTargetHeight = nBlockHeight + MV_CONSENSUS_DISTRIBUTE_INTERVAL + 1;
            int nPublishTargetHeight = nBlockHeight + 1;
            
            for (map<CDestination,vector<unsigned char> >::iterator it = result.mapEnrollData.begin();
                 it != result.mapEnrollData.end();++it)
            {
                map<CDestination,CDelegateContext>::iterator mi = mapContext.find((*it).first);
                if (mi != mapContext.end())
                { 
                    CTransaction tx;
                    if ((*mi).second.BuildEnrollTx(tx,nBlockHeight,hash,0,(*it).second))
                    {
                        routine.vEnrollTx.push_back(tx);
                    }
                }
            } 
            for (map<CDestination,vector<unsigned char> >::iterator it = result.mapDistributeData.begin();
                 it != result.mapDistributeData.end();++it)
            {
                mvDelegate.HandleDistribute(nDistributeTargetHeight,(*it).first,(*it).second);
            }
            routine.mapDistributeData = result.mapDistributeData;

            for (map<CDestination,vector<unsigned char> >::iterator it = result.mapPublishData.begin();
                 it != result.mapPublishData.end();++it)
            {
                bool fCompleted = false;
                mvDelegate.HandlePublish(nPublishTargetHeight,(*it).first,(*it).second,fCompleted);
                routine.fPublishCompleted = (routine.fPublishCompleted || fCompleted);
            }
            routine.mapPublishData = result.mapPublishData;
        }
        routine.vEnrolledWeight.push_back(make_pair(hash,mapWeight));
    }
}

void CConsensus::AddNewTx(const CAssembledTx& tx)
{
    boost::unique_lock<boost::mutex> lock(mutex);
    for (map<CDestination,CDelegateContext>::iterator it = mapContext.begin();it != mapContext.end();++it)
    {
        (*it).second.AddNewTx(tx);
    }
}

bool CConsensus::AddNewDistribute(int nAnchorHeight,const CDestination& destFrom,const vector<unsigned char>& vchDistribute)
{
    boost::unique_lock<boost::mutex> lock(mutex);
    int nDistributeTargetHeight = nAnchorHeight + MV_CONSENSUS_DISTRIBUTE_INTERVAL + 1;
    return mvDelegate.HandleDistribute(nDistributeTargetHeight,destFrom,vchDistribute);
}

bool CConsensus::AddNewPublish(int nAnchorHeight,const CDestination& destFrom,const vector<unsigned char>& vchPublish)
{
    boost::unique_lock<boost::mutex> lock(mutex);
    int nPublishTargetHeight = nAnchorHeight + 1;
    bool fCompleted = false;
    return mvDelegate.HandlePublish(nPublishTargetHeight,destFrom,vchPublish,fCompleted);
}

void CConsensus::GetAgreement(int nTargetHeight,uint256& nAgreement,size_t& nWeight,vector<CDestination>& vBallot)
{
    boost::unique_lock<boost::mutex> lock(mutex);
    map<CDestination,size_t> mapBallot;
    mvDelegate.GetAgreement(nTargetHeight,nAgreement,nWeight,mapBallot);
    pCoreProtocol->GetDelegatedBallot(nAgreement,nWeight,mapBallot,vBallot);
}

void CConsensus::GetProof(int nTargetHeight,vector<unsigned char>& vchProof)
{
    boost::unique_lock<boost::mutex> lock(mutex);
    mvDelegate.GetProof(nTargetHeight,vchProof);
}

bool CConsensus::LoadDelegateTx()
{
    for (map<CDestination,CDelegateContext>::iterator it = mapContext.begin();it != mapContext.end();++it)
    {
        CDelegateTxFilter txFilter(pCoreProtocol->GetGenesisBlockHash(),(*it).second);
        if (!pWorldLine->FilterTx(txFilter) || !pTxPool->FilterTx(txFilter))
        {
            return false;
        }
    }
    return true;
}

