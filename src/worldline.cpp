// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "worldline.h"
#include "mvdelegatecomm.h"
#include "mvdelegateverify.h"

using namespace std;
using namespace walleve;
using namespace multiverse;

#define ENROLLED_CACHE_COUNT		(120)
#define AGREEMENT_CACHE_COUNT		(16)

//////////////////////////////
// CWorldLine 

CWorldLine::CWorldLine()
: cacheEnrolled(ENROLLED_CACHE_COUNT), cacheAgreement(AGREEMENT_CACHE_COUNT)
{
    pCoreProtocol = NULL;
    pTxPool = NULL;
}

CWorldLine::~CWorldLine()
{
}

bool CWorldLine::WalleveHandleInitialize()
{
    if (!WalleveGetObject("coreprotocol",pCoreProtocol))
    {
        WalleveError("Failed to request coreprotocol\n");
        return false;
    }

    if (!WalleveGetObject("txpool",pTxPool))
    {
        WalleveError("Failed to request txpool\n");
        return false;
    }

    return true;
}

void CWorldLine::WalleveHandleDeinitialize()
{
    pCoreProtocol = NULL;
    pTxPool = NULL;
}

bool CWorldLine::WalleveHandleInvoke()
{
    if (!cntrBlock.Initialize(WalleveConfig()->pathData,WalleveConfig()->fDebug))
    {
        WalleveError("Failed to initialize container\n");
        return false;
    }

    if (!CheckContainer())
    {
        cntrBlock.Clear();
        WalleveLog("Block container is invalid,try rebuild from block storage\n");
        // Rebuild ... 
        if (!RebuildContainer())
        {
            cntrBlock.Clear(); 
            WalleveError("Failed to rebuild Block container,reconstruct all\n");
        } 
    }

    if (cntrBlock.IsEmpty())
    {
        CBlock block;
        pCoreProtocol->GetGenesisBlock(block);
        if (!InsertGenesisBlock(block))
        {
            WalleveError("Failed to create genesis block\n");
            return false;
        }
    }    
        
    return true;
}

void CWorldLine::WalleveHandleHalt()
{
    cntrBlock.Deinitialize();
    cacheEnrolled.Clear();
    cacheAgreement.Clear();
}

void CWorldLine::GetForkStatus(map<uint256,CForkStatus>& mapForkStatus)
{
    mapForkStatus.clear();

    multimap<int32,CBlockIndex*> mapForkIndex;
    cntrBlock.ListForkIndex(mapForkIndex);
    for (multimap<int32,CBlockIndex*>::iterator it = mapForkIndex.begin();it != mapForkIndex.end();++it)
    {
        CBlockIndex* pIndex = (*it).second;
        int32 nForkHeight = (*it).first;
        uint256 hashFork = pIndex->GetOriginHash();
        uint256 hashParent = pIndex->GetParentHash();

        if (hashParent != 0)
        {
            mapForkStatus[hashParent].mapSubline.insert(make_pair(nForkHeight,hashFork));
        } 

        map<uint256,CForkStatus>::iterator mi = mapForkStatus.insert(make_pair(hashFork,CForkStatus(hashFork,hashParent,nForkHeight))).first;
        CForkStatus& status = (*mi).second;
        status.hashLastBlock = pIndex->GetBlockHash();
        status.nLastBlockTime = pIndex->GetBlockTime();
        status.nLastBlockHeight = pIndex->GetBlockHeight();
        status.nMoneySupply = pIndex->GetMoneySupply();
    }
}

bool CWorldLine::GetForkProfile(const uint256& hashFork,CProfile& profile)
{
    return cntrBlock.RetrieveProfile(hashFork,profile);
}

bool CWorldLine::GetForkContext(const uint256& hashFork,CForkContext& ctxt)
{
    return cntrBlock.RetrieveForkContext(hashFork,ctxt);
}

bool CWorldLine::GetForkAncestry(const uint256& hashFork,vector<pair<uint256,uint256> > vAncestry)
{
    return cntrBlock.RetrieveAncestry(hashFork,vAncestry);
}

int  CWorldLine::GetBlockCount(const uint256& hashFork)
{   
    int nCount = 0;
    CBlockIndex* pIndex = NULL;
    if (cntrBlock.RetrieveFork(hashFork,&pIndex))
    {
        while (pIndex != NULL)
        {
            pIndex = pIndex->pPrev;
            ++nCount;                               
        }
    }
    return nCount;
}   

bool CWorldLine::GetBlockLocation(const uint256& hashBlock,uint256& hashFork,int32& nHeight)
{
    CBlockIndex* pIndex = NULL;
    if (!cntrBlock.RetrieveIndex(hashBlock,&pIndex))
    {
        return false;
    }
    hashFork = pIndex->GetOriginHash();
    nHeight = pIndex->GetBlockHeight();
    return true;
}

bool CWorldLine::GetBlockHash(const uint256& hashFork,const int32 nHeight,uint256& hashBlock)
{
    CBlockIndex* pIndex = NULL;
    if (!cntrBlock.RetrieveFork(hashFork,&pIndex) || pIndex->GetBlockHeight() < nHeight)
    {
        return false;
    }
    while (pIndex != NULL && pIndex->GetBlockHeight() > nHeight)
    {
        pIndex = pIndex->pPrev;
    }
    while (pIndex != NULL && pIndex->GetBlockHeight() == nHeight && pIndex->IsExtended())
    {
        pIndex = pIndex->pPrev;
    }
    hashBlock = !pIndex ? uint64(0) : pIndex->GetBlockHash();
    return (pIndex != NULL);
}

bool CWorldLine::GetBlockHash(const uint256& hashFork,const int32 nHeight,vector<uint256>& vBlockHash)
{   
    CBlockIndex* pIndex = NULL;
    if (!cntrBlock.RetrieveFork(hashFork,&pIndex) || pIndex->GetBlockHeight() < nHeight)
    {       
        return false;                               
    }   
    while (pIndex != NULL && pIndex->GetBlockHeight() > nHeight)
    {
        pIndex = pIndex->pPrev;
    }
    while (pIndex != NULL && pIndex->GetBlockHeight() == nHeight)
    {
        vBlockHash.push_back(pIndex->GetBlockHash());
        pIndex = pIndex->pPrev;
    }
    std::reverse(vBlockHash.begin(),vBlockHash.end());
    return (!vBlockHash.empty());
}

bool CWorldLine::GetLastBlock(const uint256& hashFork,uint256& hashBlock,int32& nHeight,int64& nTime)
{
    CBlockIndex* pIndex = NULL;
    if (!cntrBlock.RetrieveFork(hashFork,&pIndex))
    {
        return false;
    }
    hashBlock = pIndex->GetBlockHash();
    nHeight = pIndex->GetBlockHeight();
    nTime = pIndex->GetBlockTime();
    return true;
}

bool CWorldLine::GetLastBlockTime(const uint256& hashFork,int32 nDepth,vector<int64>& vTime)
{
    CBlockIndex* pIndex = NULL;
    if (!cntrBlock.RetrieveFork(hashFork,&pIndex))
    {
        return false;
    }

    vTime.clear();
    while (nDepth > 0 && pIndex != NULL)
    {
        vTime.push_back(pIndex->GetBlockTime());
        if(!pIndex->IsExtended())
        {
            nDepth--;
        }
        pIndex = pIndex->pPrev;
    }
    return true;
}

bool CWorldLine::GetBlock(const uint256& hashBlock,CBlock& block)
{
    return cntrBlock.Retrieve(hashBlock,block);
}

bool CWorldLine::GetBlockEx(const uint256& hashBlock,CBlockEx& block)
{
    return cntrBlock.Retrieve(hashBlock,block);
}

bool CWorldLine::GetOrigin(const uint256& hashFork,CBlock& block)
{
    return cntrBlock.RetrieveOrigin(hashFork,block);
}

bool CWorldLine::Exists(const uint256& hashBlock)
{
    return cntrBlock.Exists(hashBlock);
}

bool CWorldLine::GetTransaction(const uint256& txid,CTransaction& tx)
{
    return cntrBlock.RetrieveTx(txid,tx);
}

bool CWorldLine::ExistsTx(const uint256& txid)
{
    return cntrBlock.ExistsTx(txid);
}

bool CWorldLine::GetTxLocation(const uint256& txid,uint256& hashFork,int32& nHeight)
{
    return cntrBlock.RetrieveTxLocation(txid,hashFork,nHeight);
}

bool CWorldLine::GetTxUnspent(const uint256& hashFork,const vector<CTxIn>& vInput,vector<CTxOutput>& vOutput)
{
    vOutput.resize(vInput.size());
    storage::CBlockView view;
    if (!cntrBlock.GetForkBlockView(hashFork,view))
    {
        return false;
    }
    
    for (std::size_t i = 0;i < vInput.size();i++)
    {
        if (vOutput[i].IsNull())
        {
            view.RetrieveUnspent(vInput[i].prevout,vOutput[i]);
        }
    }
    return true;
}

bool CWorldLine::FilterTx(const uint256& hashFork,CTxFilter& filter)
{
    return cntrBlock.FilterTx(hashFork,filter);
}

bool CWorldLine::FilterTx(const uint256& hashFork, const int32 nDepth, CTxFilter& filter)
{
    return cntrBlock.FilterTx(hashFork, nDepth, filter);
}

bool CWorldLine::ListForkContext(vector<CForkContext>& vForkCtxt)
{
    return cntrBlock.ListForkContext(vForkCtxt);
}

MvErr CWorldLine::AddNewForkContext(const CTransaction& txFork,CForkContext& ctxt)
{
    uint256 txid = txFork.GetHash();

    CBlock block;
    CProfile profile;
    try
    {
        CWalleveBufStream ss;
        ss.Write((const char*)&txFork.vchData[0],txFork.vchData.size());
        ss >> block;
        if (!block.IsOrigin() || block.IsPrimary())
        {
            throw std::runtime_error("invalid block");
        }
        if (!profile.Load(block.vchProof))
        {
            throw std::runtime_error("invalid profile");
        }
    }
    catch (...)
    {
        WalleveError("Invalid orign block found in tx (%s)\n",txid.GetHex().c_str());
        return MV_ERR_BLOCK_INVALID_FORK;
    }
    uint256 hashFork = block.GetHash();

    CForkContext ctxtParent;
    if (!cntrBlock.RetrieveForkContext(profile.hashParent,ctxtParent))
    {
        WalleveLog("AddNewForkContext Retrieve parent context Error: %s \n",profile.hashParent.ToString().c_str());
        return MV_ERR_MISSING_PREV;
    }
    
    CProfile forkProfile;
    MvErr err = pCoreProtocol->ValidateOrigin(block,ctxtParent.GetProfile(),forkProfile);
    if (err != MV_OK)
    {
        WalleveLog("AddNewForkContext Validate Block Error(%s) : %s \n",MvErrString(err),hashFork.ToString().c_str());
        return err;
    }

    ctxt = CForkContext(block.GetHash(),block.hashPrev,txid,profile);
    if (!cntrBlock.AddNewForkContext(ctxt))
    {
        WalleveLog("AddNewForkContext Already Exists : %s \n",hashFork.ToString().c_str());
        return MV_ERR_ALREADY_HAVE;
    }

    return MV_OK;
}

MvErr CWorldLine::AddNewBlock(const CBlock& block,CWorldLineUpdate& update)
{
    uint256 hash = block.GetHash();
    MvErr err = MV_OK;

    if (cntrBlock.Exists(hash))
    {
        WalleveLog("AddNewBlock Already Exists : %s \n",hash.ToString().c_str());
        return MV_ERR_ALREADY_HAVE;
    }
    
    err = pCoreProtocol->ValidateBlock(block);
    if (err != MV_OK)
    {
        WalleveLog("AddNewBlock Validate Block Error(%s) : %s \n",MvErrString(err),hash.ToString().c_str());
        return err;
    }
   
    CBlockIndex* pIndexPrev;
    if (!cntrBlock.RetrieveIndex(block.hashPrev,&pIndexPrev))
    {
        WalleveLog("AddNewBlock Retrieve Prev Index Error: %s \n",block.hashPrev.ToString().c_str());
        return MV_ERR_SYS_STORAGE_ERROR;
    }

    int64 nReward;

    err = VerifyBlock(hash,block,pIndexPrev,nReward);
    if (err != MV_OK)
    {
        WalleveLog("AddNewBlock Verify Block Error(%s) : %s \n",MvErrString(err),hash.ToString().c_str());
        return err;
    } 

    storage::CBlockView view;
    if (!cntrBlock.GetBlockView(block.hashPrev,view,!block.IsOrigin()))
    {
        WalleveLog("AddNewBlock Get Block View Error: %s \n",block.hashPrev.ToString().c_str());
        return MV_ERR_SYS_STORAGE_ERROR;
    }
    
    if (!block.IsVacant())
    {
        view.AddTx(block.txMint.GetHash(),block.txMint);
    }

    CBlockEx blockex(block);
    vector<CTxContxt>& vTxContxt = blockex.vTxContxt;

    int64 nTotalFee = 0;

    vTxContxt.reserve(block.vtx.size());

    for(const CTransaction& tx : block.vtx)
    {
        uint256 txid = tx.GetHash();
        CTxContxt txContxt;
        err = GetTxContxt(view,tx,txContxt);
        if (err != MV_OK)
        {
            WalleveLog("AddNewBlock Get txContxt Error(%s) : %s \n",MvErrString(err),txid.ToString().c_str());
            return err;
        }
        if (!pTxPool->Exists(txid))
        {
            err = pCoreProtocol->VerifyBlockTx(tx,txContxt,pIndexPrev);
            if (err != MV_OK)
            {
                WalleveLog("AddNewBlock Verify BlockTx Error(%s) : %s \n",MvErrString(err),txid.ToString().c_str());
                return err;
            }
        }
        vTxContxt.push_back(txContxt);
        view.AddTx(txid,tx,txContxt.destIn,txContxt.GetValueIn());

        nTotalFee += tx.nTxFee;
    }

    if (block.txMint.nAmount > nTotalFee + nReward)
    {
        WalleveLog("AddNewBlock Mint tx amount invalid : (%ld > %ld + %ld \n",block.txMint.nAmount,nTotalFee,nReward);
        return MV_ERR_BLOCK_TRANSACTIONS_INVALID;
    }

    CBlockIndex* pIndexNew;
    if (!cntrBlock.AddNew(hash,blockex,&pIndexNew))
    {
        WalleveLog("AddNewBlock Storage AddNew Error : %s \n",hash.ToString().c_str());
        return MV_ERR_SYS_STORAGE_ERROR;
    }

    WalleveLog("AddNew Block : %s\n", pIndexNew->ToString().c_str());
    if (WalleveConfig()->fDebug)
    {
        WalleveDebug("New Block %s tx : %s\n",hash.ToString().c_str(), view.ToString().c_str());
    }

    CBlockIndex* pIndexFork = NULL;
    if (cntrBlock.RetrieveFork(pIndexNew->GetOriginHash(),&pIndexFork)
        && (pIndexFork->nChainTrust > pIndexNew->nChainTrust
            || (pIndexFork->nChainTrust == pIndexNew->nChainTrust && !pIndexNew->IsEquivalent(pIndexFork))))
    {
        return MV_OK;
    }

    if (!cntrBlock.CommitBlockView(view,pIndexNew))
    {
        WalleveLog("AddNewBlock Storage Commit BlockView Error : %s \n",hash.ToString().c_str());
        return MV_ERR_SYS_STORAGE_ERROR;
    }
   
    update = CWorldLineUpdate(pIndexNew);
    view.GetTxUpdated(update.setTxUpdate);
    if (!GetBlockChanges(pIndexNew,pIndexFork,update.vBlockAddNew,update.vBlockRemove))
    {
        WalleveLog("AddNewBlock Storage GetBlockChanges Error : %s \n",hash.ToString().c_str());
        return MV_ERR_SYS_STORAGE_ERROR;
    } 

    return MV_OK;
}

MvErr CWorldLine::AddNewOrigin(const CBlock& block,CWorldLineUpdate& update)
{
    uint256 hash = block.GetHash();
    MvErr err = MV_OK;

    if (cntrBlock.Exists(hash))
    {
        WalleveLog("AddNewOrigin Already Exists : %s \n",hash.ToString().c_str());
        return MV_ERR_ALREADY_HAVE;
    }
    
    err = pCoreProtocol->ValidateBlock(block);
    if (err != MV_OK)
    {
        WalleveLog("AddNewOrigin Validate Block Error(%s) : %s \n",MvErrString(err),hash.ToString().c_str());
        return err;
    }
   
    CBlockIndex* pIndexPrev;
    if (!cntrBlock.RetrieveIndex(block.hashPrev,&pIndexPrev))
    {
        WalleveLog("AddNewOrigin Retrieve Prev Index Error: %s \n",block.hashPrev.ToString().c_str());
        return MV_ERR_SYS_STORAGE_ERROR;
    }

    CProfile parent;
    if (!cntrBlock.RetrieveProfile(pIndexPrev->GetOriginHash(),parent))
    {
        WalleveLog("AddNewOrigin Retrieve parent profile Error: %s \n",block.hashPrev.ToString().c_str());
        return MV_ERR_SYS_STORAGE_ERROR;
    }
    CProfile profile;
    err = pCoreProtocol->ValidateOrigin(block,parent,profile);
    if (err != MV_OK)
    {
        WalleveLog("AddNewOrigin Validate Origin Error(%s): %s \n",MvErrString(err),hash.ToString().c_str());
        return err;
    }
   
    CBlockIndex* pIndexDuplicated;
    if (cntrBlock.RetrieveFork(profile.strName,&pIndexDuplicated))
    {
        WalleveLog("AddNewOrigin Validate Origin Error(duplated fork name): %s, \nexisted: %s\n",
                   hash.ToString().c_str(),pIndexDuplicated->GetOriginHash().GetHex().c_str());
        return MV_ERR_ALREADY_HAVE;
    }

    storage::CBlockView view;

    if (profile.IsIsolated())
    {
        if (!cntrBlock.GetBlockView(view))
        {
            WalleveLog("AddNewOrigin Get Block View Error: %s \n",block.hashPrev.ToString().c_str());
            return MV_ERR_SYS_STORAGE_ERROR;
        }
    }
    else
    {
        if (!cntrBlock.GetBlockView(block.hashPrev,view,false))
        {
            WalleveLog("AddNewOrigin Get Block View Error: %s \n",block.hashPrev.ToString().c_str());
            return MV_ERR_SYS_STORAGE_ERROR;
        }
    }
    
    if (block.txMint.nAmount != 0)
    {
        view.AddTx(block.txMint.GetHash(),block.txMint);
    }

    CBlockIndex* pIndexNew;
    CBlockEx blockex(block);

    if (!cntrBlock.AddNew(hash,blockex,&pIndexNew))
    {
        WalleveLog("AddNewOrigin Storage AddNew Error : %s \n",hash.ToString().c_str());
        return MV_ERR_SYS_STORAGE_ERROR;
    }

    WalleveLog("AddNew Origin Block : %s \n",hash.ToString().c_str());
    WalleveLog("    %s\n",pIndexNew->ToString().c_str());

    if (!cntrBlock.CommitBlockView(view,pIndexNew))
    {
        WalleveLog("AddNewOrigin Storage Commit BlockView Error : %s \n",hash.ToString().c_str());
        return MV_ERR_SYS_STORAGE_ERROR;
    }
   
    update = CWorldLineUpdate(pIndexNew);
    view.GetTxUpdated(update.setTxUpdate);
    update.vBlockAddNew.push_back(blockex);

    return MV_OK;
}

bool CWorldLine::GetProofOfWorkTarget(const uint256& hashPrev,int nAlgo,int& nBits,int64& nReward)
{
    CBlockIndex* pIndexPrev;
    if (!cntrBlock.RetrieveIndex(hashPrev,&pIndexPrev))
    {
        WalleveLog("GetProofOfWorkTarget : Retrieve Prev Index Error: %s \n",hashPrev.ToString().c_str());
        return false;
    }
    if (!pIndexPrev->IsPrimary())
    {
        WalleveLog("GetProofOfWorkTarget : Previous is not primary: %s \n",hashPrev.ToString().c_str());
        return false;
    }
    if (!pCoreProtocol->GetProofOfWorkTarget(pIndexPrev,nAlgo,nBits,nReward))
    {
        WalleveLog("GetProofOfWorkTarget : Unknown proof-of-work algo: %s \n",hashPrev.ToString().c_str());
        return false;
    }
    return true;
}

bool CWorldLine::GetDelegatedProofOfStakeReward(const uint256& hashPrev,size_t nWeight,int64& nReward)
{
    CBlockIndex* pIndexPrev;
    if (!cntrBlock.RetrieveIndex(hashPrev,&pIndexPrev))
    {
        WalleveLog("GetDelegatedProofOfStakeReward : Retrieve Prev Index Error: %s \n",hashPrev.ToString().c_str());
        return false;
    }
/*
    if (!pIndexPrev->IsPrimary())
    {
        WalleveLog("GetDelegatedProofOfStakeReward : Previous is not primary: %s \n",hashPrev.ToString().c_str());
        return false;
    }
*/    
    nReward = pCoreProtocol->GetDelegatedProofOfStakeReward(pIndexPrev,nWeight);
    return true;
}

bool CWorldLine::GetBlockLocator(const uint256& hashFork,CBlockLocator& locator)
{
    return cntrBlock.GetForkBlockLocator(hashFork,locator);
}

bool CWorldLine::GetBlockInv(const uint256& hashFork,const CBlockLocator& locator,vector<uint256>& vBlockHash,size_t nMaxCount)
{
    return cntrBlock.GetForkBlockInv(hashFork,locator,vBlockHash,nMaxCount);
}

bool CWorldLine::GetBlockDelegateEnrolled(const uint256& hashBlock,CDelegateEnrolled& enrolled)
{
    enrolled.Clear();

    if (cacheEnrolled.Retrieve(hashBlock,enrolled))
    {
        return true;
    }

    CBlockIndex* pIndex;
    if (!cntrBlock.RetrieveIndex(hashBlock,&pIndex))
    {
        WalleveLog("GetBlockDelegateEnrolled : Retrieve block Index Error: %s \n",hashBlock.ToString().c_str());
        return false;
    }

    int64 nDelegateWeightRatio = (pIndex->GetMoneySupply() + DELEGATE_THRESH - 1) / DELEGATE_THRESH;

    if (pIndex->GetBlockHeight() < MV_CONSENSUS_ENROLL_INTERVAL)
    {
        return true;
    }
    vector<uint256> vBlockRange;
    for (int i = 0;i < MV_CONSENSUS_ENROLL_INTERVAL;i++)
    {
        vBlockRange.push_back(pIndex->GetBlockHash());
        pIndex = pIndex->pPrev;
    }

    if (!cntrBlock.RetrieveAvailDelegate(hashBlock,pIndex->GetBlockHash(),vBlockRange,nDelegateWeightRatio,
                                                   enrolled.mapWeight,enrolled.mapEnrollData))
    {
        WalleveLog("GetBlockDelegateEnrolled : Retrieve Avail Delegate Error: %s \n",hashBlock.ToString().c_str());
        return false;
    }

    cacheEnrolled.AddNew(hashBlock,enrolled);

    return true;
}

bool CWorldLine::GetBlockDelegateAgreement(const uint256& hashBlock,CDelegateAgreement& agreement)
{
    agreement.Clear();

    if (cacheAgreement.Retrieve(hashBlock,agreement))
    {
        return true;
    }

    CBlockIndex* pIndex;
    if (!cntrBlock.RetrieveIndex(hashBlock,&pIndex))
    {
        WalleveLog("GetBlockDelegateAgreement : Retrieve block Index Error: %s \n",hashBlock.ToString().c_str());
        return false;
    }

    if (pIndex->GetBlockHeight() < MV_CONSENSUS_INTERVAL)
    {
        return true;
    }

    CBlock block;
    if (!cntrBlock.Retrieve(pIndex,block))
    {
        WalleveLog("GetBlockDelegateAgreement : Retrieve block Error: %s \n",hashBlock.ToString().c_str());
        return false;
    }

    for (int i = 0;i < MV_CONSENSUS_DISTRIBUTE_INTERVAL + 1;i++)
    {
        pIndex = pIndex->pPrev;
    }

    CDelegateEnrolled enrolled;

    if (!GetBlockDelegateEnrolled(pIndex->GetBlockHash(),enrolled))
    {
        return false;
    }

    delegate::CMvDelegateVerify verifier(enrolled.mapWeight,enrolled.mapEnrollData);
    map<CDestination,size_t> mapBallot;
    if (!verifier.VerifyProof(block.vchProof,agreement.nAgreement,agreement.nWeight,mapBallot))
    {
        WalleveLog("GetBlockDelegateAgreement : Invalid block proof : %s \n",hashBlock.ToString().c_str());
        return false;
    }
    
    pCoreProtocol->GetDelegatedBallot(agreement.nAgreement,agreement.nWeight,mapBallot,agreement.vBallot); 

    cacheAgreement.AddNew(hashBlock,agreement);

    return true;
}

bool CWorldLine::CheckContainer()
{
    if (cntrBlock.IsEmpty())
    {
        return true;
    }
    if (!cntrBlock.Exists(pCoreProtocol->GetGenesisBlockHash()))
    {
        return false;
    }
    return cntrBlock.CheckConsistency(StorageConfig()->nCheckLevel, StorageConfig()->nCheckDepth);
}

bool CWorldLine::RebuildContainer()
{
    return false;
}

bool CWorldLine::InsertGenesisBlock(CBlock& block)
{
    return cntrBlock.Initiate(block.GetHash(),block);
}

MvErr CWorldLine::GetTxContxt(storage::CBlockView& view,const CTransaction& tx,CTxContxt& txContxt)
{
    txContxt.SetNull();
    for(const CTxIn& txin : tx.vInput)
    {
        CTxOutput output;
        if (!view.RetrieveUnspent(txin.prevout,output))
        {
            return MV_ERR_MISSING_PREV;
        }
        if (txContxt.destIn.IsNull())
        {
            txContxt.destIn = output.destTo;
        }
        else if (txContxt.destIn != output.destTo)
        {
            return MV_ERR_TRANSACTION_INVALID;
        }
        txContxt.vin.push_back(CTxInContxt(output));
    }
    return MV_OK; 
}

bool CWorldLine::GetBlockChanges(const CBlockIndex* pIndexNew,const CBlockIndex* pIndexFork,
                                 vector<CBlockEx>& vBlockAddNew,vector<CBlockEx>& vBlockRemove)
{
    while (pIndexNew != pIndexFork)
    {
        int64 nLastBlockTime = pIndexFork ? pIndexFork->GetBlockTime() : -1;
        if (pIndexNew->GetBlockTime() >= nLastBlockTime)
        {
            CBlockEx block;
            if (!cntrBlock.Retrieve(pIndexNew,block))
            {
                return false;
            }
            vBlockAddNew.push_back(block);
            pIndexNew = pIndexNew->pPrev;
        }
        else
        {
            CBlockEx block;
            if (!cntrBlock.Retrieve(pIndexFork,block))
            {
                return false;
            }
            vBlockRemove.push_back(block);
            pIndexFork = pIndexFork->pPrev;
        }
    }
    return true;
}

bool CWorldLine::GetBlockDelegateAgreement(const uint256& hashBlock,const CBlock& block,const CBlockIndex* pIndexPrev,
                                                                    CDelegateAgreement& agreement)
{
    agreement.Clear();

    if (pIndexPrev->GetBlockHeight() < MV_CONSENSUS_INTERVAL - 1)
    {
        return true;
    }

    const CBlockIndex* pIndex = pIndexPrev;

    for (int i = 0;i < MV_CONSENSUS_DISTRIBUTE_INTERVAL;i++)
    {
        pIndex = pIndex->pPrev;
    }

    CDelegateEnrolled enrolled;

    if (!GetBlockDelegateEnrolled(pIndex->GetBlockHash(),enrolled))
    {
        return false;
    }

    delegate::CMvDelegateVerify verifier(enrolled.mapWeight,enrolled.mapEnrollData);
    map<CDestination,size_t> mapBallot;
    if (!verifier.VerifyProof(block.vchProof,agreement.nAgreement,agreement.nWeight,mapBallot))
    {
        WalleveLog("GetBlockDelegateAgreement : Invalid block proof : %s \n",hashBlock.ToString().c_str());
        return false;
    }
    
    pCoreProtocol->GetDelegatedBallot(agreement.nAgreement,agreement.nWeight,mapBallot,agreement.vBallot); 

    cacheAgreement.AddNew(hashBlock,agreement);

    return true;
}

MvErr CWorldLine::VerifyBlock(const uint256& hashBlock,const CBlock& block,CBlockIndex* pIndexPrev,int64& nReward)
{
    nReward = 0;
    if (block.IsOrigin())
    {
        return MV_ERR_BLOCK_INVALID_FORK;
    }

    if (block.IsPrimary())
    {
        if (!pIndexPrev->IsPrimary())
        {
            return MV_ERR_BLOCK_INVALID_FORK;
        }

        CDelegateAgreement agreement;
        if (!GetBlockDelegateAgreement(hashBlock,block,pIndexPrev,agreement))
        {
            return MV_ERR_BLOCK_PROOF_OF_STAKE_INVALID;
        }

        if (agreement.IsProofOfWork())
        {
            return pCoreProtocol->VerifyProofOfWork(block,pIndexPrev,nReward);
        }
        else
        {
            return pCoreProtocol->VerifyDelegatedProofOfStake(block,pIndexPrev,agreement,nReward);
        }
    }
    else if (!block.IsVacant())
    {
        if (pIndexPrev->IsPrimary())
        {
            return MV_ERR_BLOCK_INVALID_FORK;
        }

        CProofOfPiggyback proof;
        proof.Load(block.vchProof);
        
        CDelegateAgreement agreement;
        if (!GetBlockDelegateAgreement(proof.hashRefBlock,agreement))
        {
            return MV_ERR_BLOCK_PROOF_OF_STAKE_INVALID;
        }

        if (agreement.nAgreement != proof.nAgreement || agreement.nWeight != proof.nWeight
            || agreement.IsProofOfWork())
        {
            return MV_ERR_BLOCK_PROOF_OF_STAKE_INVALID;
        }

        CBlockIndex* pIndexRef = NULL;
        if (!cntrBlock.RetrieveIndex(proof.hashRefBlock,&pIndexRef))
        {
            return MV_ERR_BLOCK_PROOF_OF_STAKE_INVALID;
        }

        if (block.IsExtended())
        {
            CBlock blockPrev;
            if (!cntrBlock.Retrieve(pIndexPrev,blockPrev) || blockPrev.IsVacant())
            {
                return MV_ERR_MISSING_PREV;
            }

            CProofOfPiggyback proofPrev;
            proofPrev.Load(blockPrev.vchProof);
            if (proof.nAgreement != proofPrev.nAgreement || proof.nWeight != proofPrev.nWeight)
            {
                return MV_ERR_BLOCK_PROOF_OF_STAKE_INVALID;
            }
        }

        return pCoreProtocol->VerifySubsidiary(block,pIndexPrev,pIndexRef,agreement,nReward);
    }
   
    return MV_OK;
}

