// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "blockbase.h"
#include "template.h"
#include <boost/timer/timer.hpp>
#include <cstdio>

using namespace std;
using namespace boost::filesystem;
using namespace walleve;
using namespace multiverse::storage;

#define BLOCKFILE_PREFIX	"block"
#define LOGFILE_NAME            "storage.log"

//////////////////////////////
// CBlockBaseDBWalker

class CBlockWalker : public CBlockDBWalker
{
public:
    CBlockWalker(CBlockBase* pBaseIn) : pBase(pBaseIn) {}
    bool Walk(CBlockOutline& outline) { return pBase->LoadIndex(outline); } 
public:
    CBlockBase* pBase;
};

//////////////////////////////
// CBlockTxFilter

class CBlockTxFilter : public CBlockDBTxFilter
{
public:
    CBlockTxFilter(CBlockBase* pBlockBaseIn,CTxFilter& filterIn) 
    : CBlockDBTxFilter(filterIn),pBlockBase(pBlockBaseIn) 
    {
    }
    bool FoundTxIndex(const CDestination& destTxIn,int64 nValueTxIn,int nBlockHeight,uint32 nFile,uint32 nOffset)
    {
        uint256 hashFork;
        CTransaction tx;
        if (!pBlockBase->LoadTx(tx,nFile,nOffset,hashFork))
        {
            return false;
        }
        return filter.FoundTx(hashFork,CAssembledTx(tx,nBlockHeight,destTxIn,nValueTxIn));
    }
public:
    CBlockBase* pBlockBase;
};

//////////////////////////////
// CBlockView
 
CBlockView::CBlockView()
: pBlockBase(NULL),hashFork(uint64(0)),fCommittable(false) 
{
}

CBlockView::~CBlockView()
{
    Deinitialize();
}

void CBlockView::Initialize(CBlockBase* pBlockBaseIn,boost::shared_ptr<CBlockFork> spForkIn,
                            const uint256& hashForkIn,bool fCommittableIn)
{
    Deinitialize();

    pBlockBase = pBlockBaseIn;
    spFork     = spForkIn;
    hashFork   = hashForkIn;
    fCommittable = fCommittableIn;
    if (pBlockBase && spFork)
    {
        if (fCommittable)
        {
            spFork->UpgradeLock();
        }
        else
        {
            spFork->ReadLock();
        }
    }
    vTxRemove.clear();
    vTxAddNew.clear();
}

void CBlockView::Deinitialize()
{
    if (pBlockBase)
    {
        if (spFork)
        {
            if (fCommittable)
            {
                spFork->UpgradeUnlock();
            }
            else
            {
                spFork->ReadUnlock();
            }
            spFork = NULL;
        }
        pBlockBase = NULL;
        hashFork = 0;
        mapTx.clear();
        mapUnspent.clear();
    }
}

bool CBlockView::ExistsTx(const uint256& txid) const
{
    map<uint256,CTransaction>::const_iterator it = mapTx.find(txid);
    if (it != mapTx.end())
    {
        return (!(*it).second.IsNull());
    }
    return (!!(pBlockBase->ExistsTx(txid)));
}

bool CBlockView::RetrieveTx(const uint256& txid,CTransaction& tx)
{
    map<uint256,CTransaction>::const_iterator it = mapTx.find(txid);
    if (it != mapTx.end())
    {
        tx = (*it).second;
        return (!tx.IsNull());
    }
    return pBlockBase->RetrieveTx(txid,tx);
}

bool CBlockView::RetrieveUnspent(const CTxOutPoint& out,CTxOutput& unspent)
{
    map<CTxOutPoint,CUnspent>::const_iterator it = mapUnspent.find(out);
    if (it != mapUnspent.end())
    {
        unspent = (*it).second;
        return (!unspent.IsNull());
    }

    return pBlockBase->GetTxUnspent(hashFork,out,unspent);
}

void CBlockView::AddTx(const uint256& txid,const CTransaction& tx,const CDestination& destIn,int64 nValueIn)
{
    mapTx[txid] = tx;
    vTxAddNew.push_back(txid);

    for (int i = 0;i < tx.vInput.size();i++)
    {
        mapUnspent[tx.vInput[i].prevout].Disable();
    }
    CTxOutput output0(tx);
    if (!output0.IsNull())
    {
        mapUnspent[CTxOutPoint(txid,0)].Enable(output0);
    }
    CTxOutput output1(tx,destIn,nValueIn);
    if (!output1.IsNull())
    {
        mapUnspent[CTxOutPoint(txid,1)].Enable(output1);
    }
}

void CBlockView::RemoveTx(const uint256& txid,const CTransaction& tx,const CTxContxt& txContxt)
{
    mapTx[txid].SetNull();
    vTxRemove.push_back(txid);
    for (int i = 0;i < tx.vInput.size();i++)
    {
        const pair<int64,uint32>& input = txContxt.vInputValue[i];
        mapUnspent[tx.vInput[i].prevout].Enable(CTxOutput(txContxt.destIn,input.first,input.second));
    }    
    mapUnspent[CTxOutPoint(txid,0)].Disable();
    mapUnspent[CTxOutPoint(txid,1)].Disable();
}

void CBlockView::GetUnspentChanges(vector<CTxUnspent>& vAddNew,vector<CTxOutPoint>& vRemove)
{
    vAddNew.reserve(mapUnspent.size());
    vRemove.reserve(mapUnspent.size());

    for (map<CTxOutPoint,CUnspent>::iterator it = mapUnspent.begin();it != mapUnspent.end();++it)
    {
        const CTxOutPoint& out = (*it).first;
        const CUnspent& unspent = (*it).second;
        if (unspent.IsModified())
        {
            if (!unspent.IsNull())
            {
                vAddNew.push_back(CTxUnspent(out,unspent));
            }
            else
            {
                vRemove.push_back(out);
            }
        }
    }
}

void CBlockView::GetTxUpdated(set<uint256>& setUpdate)
{
    for (int i = 0;i < vTxRemove.size();i++)
    {
        const uint256& txid = vTxRemove[i];
        if (!mapTx[txid].IsNull())
        {
            setUpdate.insert(txid);
        }
    }
}

void CBlockView::GetTxRemoved(vector<uint256>& vRemove)
{
    vRemove.reserve(vTxRemove.size());
    for (int i = 0;i < vTxRemove.size();i++)
    {
        const uint256& txid = vTxRemove[i];
        if (mapTx[txid].IsNull())
        {
            vRemove.push_back(txid);
        }
    }
}

//////////////////////////////
// CBlockBase 

CBlockBase::CBlockBase()
: fDebugLog(false)
{
}

CBlockBase::~CBlockBase()
{
    dbBlock.Deinitialize();
    tsBlock.Deinitialize();
}

bool CBlockBase::Initialize(const CMvDBConfig& dbConfig,int nMaxDBConn,const path& pathDataLocation,bool fDebug,bool fRenewDB)
{
    if (!SetupLog(pathDataLocation,fDebug))
    {
        return false;
    }

    Log("B","Initializing... (Path : %s)\n",pathDataLocation.string().c_str());

    if (!dbBlock.DBPoolInitialize(dbConfig,nMaxDBConn))
    {
        Error("B","Failed MySQL not connect\n");
        return false;
    }

    if (!dbBlock.InnoDB()){
        Error("B","Failed MySQL not Support InnoDB\n");
        return false;
    }

    if (!dbBlock.Initialize(pathDataLocation))
    {
        Error("B","Failed to initialize block db\n");
        return false;
    }

    if (!tsBlock.Initialize(pathDataLocation / "block",BLOCKFILE_PREFIX))
    {
        dbBlock.Deinitialize();
        Error("B","Failed to initialize block tsfile\n");
        return false;
    }
    
    if (fRenewDB)
    {
        Clear();
    }
    else if (!LoadDB())
    {
        dbBlock.Deinitialize();
        tsBlock.Deinitialize();
        {
            CWalleveWriteLock wlock(rwAccess);

            ClearCache();
        }
        Error("B","Failed to load block db\n");
        return false;
    }
    Log("B","Initialized\n");
    return true;
}

void CBlockBase::Deinitialize()
{
    dbBlock.Deinitialize();
    tsBlock.Deinitialize();
    {
        CWalleveWriteLock wlock(rwAccess);

        ClearCache();
    }
    Log("B","Deinitialized\n");
}

bool CBlockBase::Exists(const uint256& hash) const
{
    CWalleveReadLock rlock(rwAccess);
 
    return (!!mapIndex.count(hash));
}

bool CBlockBase::ExistsTx(const uint256& txid)
{
    return dbBlock.ExistsTx(txid);
}

bool CBlockBase::IsEmpty() const
{
    CWalleveReadLock rlock(rwAccess);

    return mapIndex.empty();
}

void CBlockBase::Clear()
{
    CWalleveWriteLock wlock(rwAccess);

    dbBlock.RemoveAll();
    ClearCache();    
}

bool CBlockBase::Initiate(const uint256& hashGenesis,const CBlock& blockGenesis)
{
    if (!IsEmpty())
    {
        return false;
    }
    uint32 nFile,nOffset;
    if (!tsBlock.Write(blockGenesis,nFile,nOffset))
    {
        return false;
    }

    uint32 nTxOffset = nOffset + blockGenesis.GetTxSerializedOffset();
    uint256 txidMintTx = blockGenesis.txMint.GetHash();

    vector<pair<uint256,CTxIndex> > vTxNew;
    vTxNew.push_back(make_pair(txidMintTx,CTxIndex(blockGenesis.txMint,CDestination(),0,0,nFile,nTxOffset)));

    vector<CTxUnspent> vAddNew;
    vAddNew.push_back(CTxUnspent(CTxOutPoint(txidMintTx,0),CTxOutput(blockGenesis.txMint)));

    { 
        CWalleveWriteLock wlock(rwAccess);
        CBlockIndex* pIndexNew = AddNewIndex(hashGenesis,blockGenesis,nFile,nOffset);
        if (pIndexNew == NULL)
        {
            return false;
        }

        if (!dbBlock.AddNewBlock(CBlockOutline(pIndexNew)))
        {
            return false;
        }

        CProfile profile;
        if (!profile.Load(blockGenesis.vchProof))
        {
            return false;
        }

        CForkContext ctxt(hashGenesis,uint64(0),uint64(0),profile);
        if (!dbBlock.AddNewForkContext(ctxt))
        {
            return false;
        }
        
        if (!dbBlock.AddNewFork(hashGenesis))
        {
            return false;
        }

        boost::shared_ptr<CBlockFork> spFork = AddNewFork(profile,pIndexNew);
        if (spFork != NULL)
        {
            CWalleveWriteLock wForkLock(spFork->GetRWAccess());
     
            if (!dbBlock.UpdateFork(hashGenesis,hashGenesis,uint64(0),vTxNew,vector<uint256>(),vAddNew,vector<CTxOutPoint>()))
            {
                return false;
            }
            spFork->UpdateLast(pIndexNew);
        }
        else 
        {
            return false;
        }

        Log("B","Initiate genesis %s\n",hashGenesis.ToString().c_str());
    }
    return true;
}

bool CBlockBase::AddNew(const uint256& hash,CBlockEx& block,CBlockIndex** ppIndexNew)
{
    if (Exists(hash))
    {
        return false;
    }

    uint32 nFile,nOffset;
    if (!tsBlock.Write(block,nFile,nOffset))
    {
        return false;
    }
    { 
        CWalleveWriteLock wlock(rwAccess);

        CBlockIndex* pIndexNew = AddNewIndex(hash,block,nFile,nOffset);
        if (pIndexNew == NULL)
        {
            return false;
        }

        if (!dbBlock.AddNewBlock(CBlockOutline(pIndexNew)))
        {
            mapIndex.erase(hash);
            delete pIndexNew;
            return false;
        }
        
        if (pIndexNew->IsPrimary())
        {
            if (!UpdateDelegate(hash,block))
            {
                dbBlock.RemoveBlock(hash);
                mapIndex.erase(hash);
                delete pIndexNew;
                return false;
            }
        }

        *ppIndexNew = pIndexNew;
    }
    
    Log("B","AddNew block,hash=%s\n",hash.ToString().c_str());
    return true;
}

bool CBlockBase::AddNewForkContext(const CForkContext& ctxt)
{
    if (!dbBlock.AddNewForkContext(ctxt))
    {
        Error("F","Failed to addnew forkcontext in %s",ctxt.hashFork.GetHex().c_str());
        return false;
    }
    Log("F","AddNew forkcontext,hash=%s\n",ctxt.hashFork.GetHex().c_str());
    return true;
}

bool CBlockBase::Retrieve(const uint256& hash,CBlock& block)
{
    block.SetNull();

    CBlockIndex* pIndex;
    {
        CWalleveReadLock rlock(rwAccess);

        if (!(pIndex = GetIndex(hash)))
        {
            return false;
        }
    }
    if (!tsBlock.Read(block,pIndex->nFile,pIndex->nOffset))
    {
        return false;
    }
    return true;    
}

bool CBlockBase::Retrieve(const CBlockIndex* pIndex,CBlock& block)
{
    block.SetNull();

    if (!tsBlock.Read(block,pIndex->nFile,pIndex->nOffset))
    {
        return false;
    }
    return true;    
}

bool CBlockBase::Retrieve(const uint256& hash,CBlockEx& block)
{
    block.SetNull();

    CBlockIndex* pIndex;
    {
        CWalleveReadLock rlock(rwAccess);

        if (!(pIndex = GetIndex(hash)))
        {
            return false;
        }
    }
    if (!tsBlock.Read(block,pIndex->nFile,pIndex->nOffset))
    {
        return false;
    }
    return true;    
}

bool CBlockBase::Retrieve(const CBlockIndex* pIndex,CBlockEx& block)
{
    block.SetNull();

    if (!tsBlock.Read(block,pIndex->nFile,pIndex->nOffset))
    {
        return false;
    }
    return true;    
}

bool CBlockBase::RetrieveIndex(const uint256& hash,CBlockIndex** ppIndex)
{
    CWalleveReadLock rlock(rwAccess);

    *ppIndex = GetIndex(hash);
    return (*ppIndex != NULL);
}

bool CBlockBase::RetrieveFork(const uint256& hash,CBlockIndex** ppIndex)
{
    CWalleveReadLock rlock(rwAccess);

    boost::shared_ptr<CBlockFork> spFork = GetFork(hash);
    if (spFork != NULL)
    {
        CWalleveReadLock rForkLock(spFork->GetRWAccess());

        *ppIndex = spFork->GetLast();
        
        return true;
    }

    return false;
}

bool CBlockBase::RetrieveFork(const string& strName,CBlockIndex** ppIndex)
{
    CWalleveReadLock rlock(rwAccess);

    boost::shared_ptr<CBlockFork> spFork = GetFork(strName);
    if (spFork != NULL)
    {
        CWalleveReadLock rForkLock(spFork->GetRWAccess());

        *ppIndex = spFork->GetLast();
        
        return true;
    }

    return false;
}

bool CBlockBase::RetrieveProfile(const uint256& hash,CProfile& profile)
{
    CWalleveReadLock rlock(rwAccess);

    boost::shared_ptr<CBlockFork> spFork = GetFork(hash);
    if (spFork == NULL)
    {
        return false;
    }

    profile = spFork->GetProfile();

    return true;
}

bool CBlockBase::RetrieveForkContext(const uint256& hash,CForkContext& ctxt)
{
    return dbBlock.RetrieveForkContext(hash,ctxt);
}

bool CBlockBase::RetrieveAncestry(const uint256& hash,vector<pair<uint256,uint256> > vAncestry)
{
    CForkContext ctxt;
    if (!dbBlock.RetrieveForkContext(hash,ctxt))
    {
        return false;
    }
    
    while (ctxt.hashParent != 0)
    {
        vAncestry.push_back(make_pair(ctxt.hashParent,ctxt.hashJoint));
        if (!dbBlock.RetrieveForkContext(ctxt.hashParent,ctxt))
        {
            return false;
        }
    }

    std::reverse(vAncestry.begin(),vAncestry.end());
    return true;
}

bool CBlockBase::RetrieveOrigin(const uint256& hash,CBlock& block)
{
    block.SetNull();

    CForkContext ctxt;
    if (!dbBlock.RetrieveForkContext(hash,ctxt))
    {
        return false;
    }

    CTransaction tx;
    if (!RetrieveTx(ctxt.txidEmbedded,tx))
    {
        return false;
    }

    try
    {
        CWalleveBufStream ss;
        ss.Write((const char*)&tx.vchData[0],tx.vchData.size());
        ss >> block;
    }
    catch (exception& e)
    {
        StdError(__PRETTY_FUNCTION__, e.what());
        return false;
    }
    return true;
}

bool CBlockBase::RetrieveTx(const uint256& txid,CTransaction& tx)
{
    tx.SetNull();

    uint32 nTxFile,nTxOffset;
    if (!dbBlock.RetrieveTxPos(txid,nTxFile,nTxOffset))
    {
        return false;
    }

    if (!tsBlock.Read(tx,nTxFile,nTxOffset))
    {
        return false;
    }
    return true;
}

bool CBlockBase::RetrieveTxLocation(const uint256& txid,uint256& hashFork,int& nHeight)
{
    uint256 hashAnchor;
    if (!dbBlock.RetrieveTxLocation(txid,hashAnchor,nHeight))
    {
        return false;
    }

    {
        CWalleveReadLock rlock(rwAccess);
        CBlockIndex* pIndex = (hashAnchor != 0 ? GetIndex(hashAnchor) : GetOriginIndex(txid));
        if (pIndex == NULL)
        {
            return false;
        }
        hashFork = pIndex->GetOriginHash();
    }

    return true;
}

bool CBlockBase::RetrieveDelegate(const uint256& hash,int64 nMinAmount,map<CDestination,int64>& mapDelegate)
{
    return dbBlock.RetrieveDelegate(hash,nMinAmount,mapDelegate);
}

bool CBlockBase::RetrieveEnroll(const uint256& hashAnchor,const uint256& hashEnrollEnd,
                                map<CDestination,vector<unsigned char> >& mapEnrollData)
{
    CWalleveReadLock rlock(rwAccess);

    CBlockIndex* pIndex = GetIndex(hashEnrollEnd);
    set<uint256> setBlockRange;
    while (pIndex != NULL && pIndex->GetBlockHash() != hashAnchor)
    {
        setBlockRange.insert(pIndex->GetBlockHash());
        pIndex = pIndex->pPrev;
    }
    
    if (pIndex == NULL)
    {
        return false;
    }

    map<CDestination,pair<uint32,uint32> > mapEnrollTxPos;
    if (!dbBlock.RetrieveEnroll(hashAnchor,setBlockRange,mapEnrollTxPos))
    {
        return false;
    }
    
    for (map<CDestination,pair<uint32,uint32> >::iterator it = mapEnrollTxPos.begin();
         it != mapEnrollTxPos.end();++it)
    {
        CTransaction tx;
        if (!tsBlock.Read(tx,(*it).second.first,(*it).second.second))
        {
            return false;
        }
        mapEnrollData.insert(make_pair((*it).first,tx.vchData)); 
    }
    return true;
}

void CBlockBase::ListForkIndex(multimap<int,CBlockIndex*>& mapForkIndex)
{
    CWalleveReadLock rlock(rwAccess);

    mapForkIndex.clear();
    for (map<uint256,boost::shared_ptr<CBlockFork> >::iterator it = mapFork.begin();it != mapFork.end();++it)
    {
        CWalleveReadLock rForkLock((*it).second->GetRWAccess());

        CBlockIndex* pIndex = (*it).second->GetLast();
        mapForkIndex.insert(make_pair(pIndex->pOrigin->GetBlockHeight() - 1,pIndex));     
    }
}

bool CBlockBase::GetBlockView(CBlockView& view)
{
    boost::shared_ptr<CBlockFork> spFork;
    view.Initialize(this,spFork,uint64(0),false);
    return true;
}

bool CBlockBase::GetBlockView(const uint256& hash,CBlockView& view,bool fCommitable)
{
    CWalleveReadLock rlock(rwAccess);

    CBlockIndex* pIndex = GetIndex(hash);
    if (pIndex == NULL)
    {
        return false;
    }
    uint256 hashOrigin = pIndex->GetOriginHash();

    boost::shared_ptr<CBlockFork> spFork = GetFork(hashOrigin);
    if (spFork == NULL)
    {
        return false;
    }
    
    view.Initialize(this,spFork,hashOrigin,fCommitable);

    CBlockIndex* pForkLast = spFork->GetLast(); 

    vector<CBlockIndex*> vPath;
    CBlockIndex* pBranch = GetBranch(pForkLast,pIndex,vPath);

    for (CBlockIndex* p = pForkLast;p != pBranch;p = p->pPrev)
    {
        // remove block tx;
        CBlockEx block;
        if (!tsBlock.Read(block,p->nFile,p->nOffset))
        {
            return false;
        }
        for (int j = block.vtx.size() - 1;j >= 0;j--)
        {
            view.RemoveTx(block.vtx[j].GetHash(),block.vtx[j],block.vTxContxt[j]);
        }
        if (!block.txMint.IsNull())
        {
            view.RemoveTx(block.txMint.GetHash(),block.txMint);
        }
    }

    for (int i = vPath.size() - 1;i >= 0;i--)
    {
        // add block tx;
        CBlockEx block;
        if (!tsBlock.Read(block,vPath[i]->nFile,vPath[i]->nOffset))
        {
            return false;
        }
        view.AddTx(block.txMint.GetHash(),block.txMint);
        for (int j = 0;j < block.vtx.size();j++)
        {
            const CTxContxt& txContxt = block.vTxContxt[j];
            view.AddTx(block.vtx[j].GetHash(),block.vtx[j],txContxt.destIn,txContxt.GetValueIn());
        }
    }
    return true;
}

bool CBlockBase::GetForkBlockView(const uint256& hashFork,CBlockView& view)
{
    boost::shared_ptr<CBlockFork> spFork = GetFork(hashFork);
    if (spFork == NULL)
    {
        return false;
    }
    view.Initialize(this,spFork,hashFork,false);
    return true;
}

bool CBlockBase::CommitBlockView(CBlockView& view,CBlockIndex* pIndexNew)
{
    const uint256 hashFork = pIndexNew->GetOriginHash();

    boost::shared_ptr<CBlockFork> spFork;

    if (hashFork == view.GetForkHash())
    {
        if (!view.IsCommittable())
        {
            return false;
        }
        spFork = view.GetFork();
    } 
    else
    {
        CProfile profile;
        if (!LoadForkProfile(pIndexNew->pOrigin,profile))
        {
            return false;
        }
        if (!dbBlock.AddNewFork(hashFork))
        {
            return false;
        }
        spFork = AddNewFork(profile,pIndexNew);
    }

    vector<pair<uint256,CTxIndex> > vTxNew;
    if (!GetTxNewIndex(view,pIndexNew,vTxNew))
    {
        return false;
    }

    vector<uint256> vTxDel;
    view.GetTxRemoved(vTxDel);

    vector<CTxUnspent> vAddNew;
    vector<CTxOutPoint> vRemove;
    view.GetUnspentChanges(vAddNew,vRemove);

    if (pIndexNew->IsPrimary())
    {
        if (!UpdateEnroll(pIndexNew,vTxNew))
        {
            return false;
        }
    }

    if (hashFork == view.GetForkHash())
    {
        spFork->UpgradeToWrite();
    }

    if (!dbBlock.UpdateFork(hashFork,pIndexNew->GetBlockHash(),view.GetForkHash(),vTxNew,vTxDel,vAddNew,vRemove))
    {
        return false;
    }
    spFork->UpdateLast(pIndexNew);

    Log("B","Update fork %s, last block hash=%s\n",hashFork.ToString().c_str(),
                                                   pIndexNew->GetBlockHash().ToString().c_str());
    return true;              
}

bool CBlockBase::LoadIndex(CBlockOutline& outline)
{
    uint256 hash = outline.GetBlockHash();
    CBlockIndex* pIndexNew = new CBlockIndex(static_cast<CBlockIndex&>(outline));
    if (pIndexNew == NULL)
    {
        return false;
    }
    map<uint256, CBlockIndex*>::iterator mi;
    mi = mapIndex.insert(make_pair(hash, pIndexNew)).first;
    pIndexNew->phashBlock = &((*mi).first);
    pIndexNew->pPrev = NULL;
    pIndexNew->pOrigin = pIndexNew;
    if (outline.hashPrev != 0)
    {
        pIndexNew->pPrev = GetIndex(outline.hashPrev);
        if (pIndexNew->pPrev == NULL)
        {
            mapIndex.erase(hash);
            delete pIndexNew;
            return false;
        }
        if (!pIndexNew->IsOrigin())
        {
            pIndexNew->pOrigin = pIndexNew->pPrev->pOrigin;
        }
    }
    return true;
}

bool CBlockBase::LoadTx(CTransaction& tx,uint32 nTxFile,uint32 nTxOffset,uint256& hashFork)
{
    tx.SetNull();
    if (!tsBlock.Read(tx,nTxFile,nTxOffset))
    {
        return false;
    }
    CBlockIndex* pIndex = (tx.hashAnchor != 0 ? GetIndex(tx.hashAnchor) : GetOriginIndex(tx.GetHash()));
    if (pIndex == NULL)
    {
        return false;
    }
    hashFork = pIndex->GetOriginHash();
    return true;
}

bool CBlockBase::FilterTx(CTxFilter& filter)
{
    CBlockTxFilter txFilter(this,filter); 
    return dbBlock.FilterTx(txFilter);
}

bool CBlockBase::FilterForkContext(CForkContextFilter& filter)
{
    return dbBlock.FilterForkContext(filter);
}

bool CBlockBase::GetForkBlockLocator(const uint256& hashFork,CBlockLocator& locator)
{
    CWalleveReadLock rlock(rwAccess);
    
    boost::shared_ptr<CBlockFork> spFork = GetFork(hashFork);
    if (spFork == NULL)
    {
        return false;
    }
  
    CBlockIndex* pIndex = NULL;
 
    {
        CWalleveReadLock rForkLock(spFork->GetRWAccess());

        pIndex = spFork->GetLast();
    }

    locator.vBlockHash.clear();
    int nStep = 1;
    while(pIndex && pIndex->GetOriginHash() == hashFork && !pIndex->IsOrigin())
    {
        locator.vBlockHash.push_back(pIndex->GetBlockHash());
        for (int i = 0; pIndex && i < nStep; i++)
        {
            pIndex = pIndex->pPrev;
        }
        if (locator.vBlockHash.size() > 10)
        {
            nStep *= 2;
        }
    }

    return true;
}

bool CBlockBase::GetForkBlockInv(const uint256& hashFork,const CBlockLocator& locator,vector<uint256>& vBlockHash,size_t nMaxCount)
{
    CWalleveReadLock rlock(rwAccess);

    boost::shared_ptr<CBlockFork> spFork = GetFork(hashFork);
    if (spFork == NULL)
    {
        return false;
    }
    
    CBlockIndex* pIndex = NULL;
    BOOST_FOREACH(const uint256& hash,locator.vBlockHash)
    {
        pIndex = GetIndex(hash);
        if (pIndex != NULL)
        {
            if (pIndex->GetOriginHash() != hashFork)
            {
                return false;
            }
            break;
        }
    }

    {
        CWalleveReadLock rForkLock(spFork->GetRWAccess());

        pIndex = (pIndex != NULL ? pIndex->pNext : spFork->GetOrigin()->pNext);
        while(pIndex != NULL && vBlockHash.size() < nMaxCount - 1)
        {
            vBlockHash.push_back(pIndex->GetBlockHash());
            pIndex = pIndex->pNext;
        }
        CBlockIndex* pIndexLast = spFork->GetLast();
        if (pIndex != NULL && pIndex != pIndexLast)
        {
            vBlockHash.push_back(pIndexLast->GetBlockHash());
        }
    }
    return true;
}

bool CBlockBase::CheckConsistency(int nCheckLevel, int nCheckDepth)
{
    boost::timer::cpu_timer t_lock;
    t_lock.start();

    CWalleveReadLock rlock(rwAccess);

    Log("B", "Getting lock duration ===> %s.\n", t_lock.format().c_str());

    boost::timer::cpu_timer t_check;
    t_check.start();

    Log("B", "Check consistency with parameters check-level:%d and check-depth:%d.\n", nCheckLevel, nCheckDepth);

    int nLevel = nCheckLevel;
    if(nCheckLevel < 0)
    {
        nLevel = 0;
    }
    if(nCheckLevel > 3)
    {
        nLevel = 3;
    }
    int nDepth = nCheckDepth;

    Log("B", "Consistency checking level is %d\n", nLevel);

    vector<CBlockDBFork> vFork;
    if(!dbBlock.FetchFork(vFork))
    {
        Error("B", "Fetch fork failed.\n");
        return false;
    }

    for(const auto& fork : vFork)
    {
        boost::timer::cpu_timer t_fork;
        t_fork.start();

        //checking of level 0: fork/block

        //check field refblock of table fork must be in rows in table block
        CBlockIndex* pBlockRefIndex = GetIndex(fork.hashRef);
        if(!pBlockRefIndex)
        {
            Error("B", "Get referenced block index failed.\n");
            return false;
        }

        boost::shared_ptr<CBlockFork> spFork = GetFork(fork.hashFork);
        if(NULL == spFork)
        {
            Error("B", "Get fork failed.\n");
            return false;
        }
        CBlockIndex* pLastBlock = spFork->GetLast();
        if(NULL == pLastBlock)
        {
            Error("B", "Get last block index of current fork failed.\n");
            return false;
        }

        bool fIsMainFork = spFork->GetOrigin()->IsPrimary();

        if(0 == nDepth || pLastBlock->nHeight < nDepth)
        {
            nDepth = pLastBlock->nHeight;
            Log("B", "Consistency checking depth is {%d} for fork:{%s}\n", nDepth, fork.hashFork.ToString().c_str());
        }

        CBlockIndex* pIndex = pLastBlock;
        map<CDestination, int64> mapNextBlockDelegate;

        map<CTxOutPoint, CTxUnspent> mapUnspentUTXO;
        vector<CTxOutPoint> vSpentUTXO;

        while(pIndex && pLastBlock->nHeight - pIndex->nHeight < nDepth)
        {
            //be able to read from block files
            CBlockEx block;
            if(!tsBlock.ReadDirect(block, pIndex->nFile, pIndex->nOffset))
            {
                Error("B", "Retrieve block from file directly failed.\n");
                return false;
            }

            //consistent between database and block file
            if(!(pIndex->GetBlockHash() == block.GetHash()
                 && pIndex->pPrev->GetBlockHash() == block.hashPrev
                 && pIndex->nVersion == block.nVersion
                 && pIndex->nType == block.nType
                 && pIndex->nTimeStamp == block.nTimeStamp
                 && ((pIndex->nMintType == 0 ) ?
                     block.IsVacant()
                     : (!block.IsVacant() && pIndex->txidMint == block.txMint.GetHash() && pIndex->nMintType == block.txMint.nType))
                ))
            {
                Error("B", "Block info are not consistent in db and file.\n");
                return false;
            }

            //checking of level 1: transaction
            if(nLevel >= 1 && !pIndex->IsVacant())
            {
                auto lmdChkTx = [&] (const uint256& txid, const CTxIndex& pTxIndex) -> bool {
                    CTransaction tx;
                    if (!tsBlock.ReadDirect(tx, pTxIndex.nFile, pTxIndex.nOffset))
                    {
                        Error("B", "Retrieve tx from file directly failed.\n");
                        return false;
                    }

                    //consistent between database and block file
                    if(!(txid == tx.GetHash()
                          && pTxIndex.nVersion == tx.nVersion
                          && pTxIndex.nType == tx.nType
                          && pTxIndex.nLockUntil == tx.nLockUntil
                          && pTxIndex.hashAnchor == tx.hashAnchor
                          && pTxIndex.sendTo == tx.sendTo
                          && pTxIndex.nAmount == tx.nAmount)
                            )
                    {
                        return false;
                    }

                    return true;
                };

                CTxIndex pTxIdx;
                if(!dbBlock.RetrieveTxIndex(pIndex->txidMint, pTxIdx))
                {
                    Error("B", "Retrieve mint tx index from db failed.\n");
                    return false;
                }
                if(!lmdChkTx(pIndex->txidMint, pTxIdx))
                {
                    Error("B", "Mint tx info are not consistent in db and file.\n");
                    return false;
                }

                for(auto const& tx : block.vtx)
                {
                    pTxIdx.SetNull();
                    const uint256& txid = tx.GetHash();
                    if(!dbBlock.RetrieveTxIndex(txid, pTxIdx))
                    {
                        Error("B", "Retrieve token tx index from db failed.\n");
                        return false;
                    }
                    if(!lmdChkTx(txid, pTxIdx))
                    {
                        Error("B", "Token tx info are not consistent in db and file.\n");
                        return false;
                    }
                }

            }

            //checking of level 2: delegate/enroll
            if(nLevel >= 2 && fIsMainFork)
            {
                static bool fIsLastBlock = true;

                if(fIsLastBlock)
                {
                    if(!dbBlock.RetrieveDelegate(block.GetHash(), 0, mapNextBlockDelegate))
                    {
                        Error("B", "Retrieve the latest delegate record from db failed.\n");
                        return false;
                    }
                    fIsLastBlock = false;
                }
                else
                {   //compare delegate in this iteration with the previous one
                    map<CDestination, int64> mapPrevBlockDelegate;
                    if(!dbBlock.RetrieveDelegate(block.GetHash(), 0, mapPrevBlockDelegate))
                    {
                        Error("B", "Retrieve the following previous delegate record from db failed.\n");
                        return false;
                    }
                    if(mapNextBlockDelegate != mapPrevBlockDelegate)
                    {
                        Error("B", "Delegate records followed one by one do not match.\n");
                        return false;
                    }
                    mapNextBlockDelegate = mapPrevBlockDelegate;
                }

                if (block.txMint.nType == CTransaction::TX_STAKE)
                {
                    mapNextBlockDelegate[block.txMint.sendTo] -= block.txMint.nAmount;
                }

                map<pair<uint256, CDestination>, tuple<uint256, uint32, uint32>> mapEnrollRanged;
                for (int i = 0; i < block.vtx.size(); i++)
                {
                    const CTransaction& tx = block.vtx[i];
                    {
                        CTemplateId tid;
                        if(tx.sendTo.GetTemplateId(tid) && tid.GetType() == TEMPLATE_DELEGATE)
                        {
                            mapNextBlockDelegate[tx.sendTo] -= tx.nAmount;
                        }
                    }

                    const CTxContxt& txContxt = block.vTxContxt[i];
                    {
                        CTemplateId tid;
                        if(txContxt.destIn.GetTemplateId(tid) && tid.GetType() == TEMPLATE_DELEGATE)
                        {
                            mapNextBlockDelegate[txContxt.destIn] += tx.nAmount + tx.nTxFee;
                        }
                    }

                    if(tx.nType == CTransaction::TX_CERT)
                    {
                        const uint256& anchor = tx.hashAnchor;
                        const CDestination& dest = tx.sendTo;
                        const uint256& blk = block.GetHash();
                        CTxIndex txIdx;
                        if(!dbBlock.RetrieveTxIndex(tx.GetHash(), txIdx))
                        {
                            Error("B", "Retrieve enroll tx index from table transaction failed.\n");
                            return false;
                        }
                        const uint32& nFile = txIdx.nFile;
                        const uint32& nOffset = txIdx.nOffset;
                        mapEnrollRanged[make_pair(anchor, dest)] = make_tuple(blk, nFile, nOffset);
                    }
                }

                vector<CDestination> vDestNull;
                for(const auto& delegate : mapNextBlockDelegate)
                {
                    if(delegate.second < 0)
                    {
                        Error("B", "Amount on delegate template address must not be less than zero.\n");
                        return false;
                    }
                    if(delegate.second == 0)
                    {
                        vDestNull.push_back(delegate.first);
                    }
                }
                for(const auto& dest : vDestNull)
                {
                    mapNextBlockDelegate.erase(dest);
                }

                //compare enroll ranged in argument of nDepth with table enroll
                set<uint256> setBlockRange;
                setBlockRange.insert(block.GetHash());
                map<CDestination, pair<uint32, uint32>> mapRes;
                if(!dbBlock.RetrieveEnroll(block.hashPrev, setBlockRange, mapRes))
                {
                    Error("B", "Retrieve enroll tx records from table enroll failed.\n");
                    return false;
                }
                map<CDestination, pair<uint32, uint32>> mapResComp;
                for(const auto& enroll : mapEnrollRanged)
                {
                    const CDestination& dest = enroll.first.second;
                    const tuple<uint256, uint32, uint32>& pos = enroll.second;
                    const uint32& file = get<1>(pos);
                    const uint32& offset = get<2>(pos);
                    mapResComp.insert(make_pair(dest, make_pair(file, offset)));
                }
                if(mapRes != mapResComp)
                {
                    Error("B", "Enroll transactions in tables enroll and transaction do not match.\n");
                    return false;
                }
            }

            //checking of level 3: unspent
            if(nLevel >= 3)
            {
                static vector<pair<uint256, uint64> > preout;   //txid vs. height
                mapUnspentUTXO.insert(make_pair(CTxOutPoint(block.txMint.GetHash(), 0), CTxUnspent(CTxOutPoint(block.txMint.GetHash(), 0)
                                     , CTxOutput(block.txMint.sendTo, block.txMint.nAmount, block.txMint.nLockUntil))));

                for(const auto& tx : block.vtx)
                {
                    CTxIndex TxIdx;
                    if(!dbBlock.RetrieveTxIndex(tx.GetHash(), TxIdx))
                    {
                        assert(0);
                    }
                    int64 nChange = TxIdx.nValueIn - TxIdx.nAmount - tx.nTxFee;
                    if(nChange > 0 && tx.sendTo.IsTemplate())
                    {
                        cout << "there is a change with template address: " << tx.sendTo.GetHex() << endl;
                    }
                    if(nChange > 0)
                    {
                        cout << "there is a change with " << (tx.sendTo.IsPubKey() ? "pubkey " : "tempkey ") << tx.sendTo.GetHex() << endl;
                        int n = 1;
                    }
                    if(tx.nAmount == 11500000)
                    {
                        int n = 1;
                    }
                    if(tx.sendTo.IsPubKey())
                    {
                        cout << "send to a pubkey address of " << tx.sendTo.GetHex() << endl;
//                        preout.push_back(make_pair(tx.GetHash(), TxIdx.nBlockHeight));
                    }
                    mapUnspentUTXO.insert(make_pair(CTxOutPoint(tx.GetHash(), 0), CTxUnspent(CTxOutPoint(tx.GetHash(), 0), CTxOutput(tx.sendTo, tx.nAmount, tx.nLockUntil))));
                    if(nChange > 0)
                    {
                        if(TxIdx.nBlockHeight == 20170)
                        {
                            int n = 1;
                        }
                        Log("B", "Tx(%s) with a change(%s) on height(%d): to prepare to check.\n", tx.GetHash().ToString().c_str(), to_string(nChange).c_str(), TxIdx.nBlockHeight);
                        if(!CheckInputSingleAddressForTxWithChange(tx.GetHash()))
                        {
                            Error("B", "Tx(%s) with a change(%s) on height(%d): input must be a single address.\n", tx.GetHash().ToString().c_str(), to_string(nChange).c_str(), TxIdx.nBlockHeight);
                            return false;
                        }
                        else
                        {
                            mapUnspentUTXO.insert(make_pair(CTxOutPoint(tx.GetHash(), 1)
                                                 , CTxUnspent(CTxOutPoint(tx.GetHash(), 1)
                                                             , CTxOutput(TxIdx.destIn, nChange, tx.nLockUntil))));
                        }
                    }
                    for(const auto& txin : tx.vInput)
                    {
                        for(const auto& po : preout)
                        {
                            if(txin.prevout.hash == po.first)
                            {
                                int n = 1;
                            }
                        }
                        vSpentUTXO.push_back(txin.prevout);
                    }
                }

                vector<CTxOutPoint> vRemovedUTXO;
                for(const auto& spent : vSpentUTXO)
                {
                    for(const auto& po : preout)
                    {
                        if(spent.hash == po.first)
                        {
                            int n = 1;
                        }
                    }
                    if(mapUnspentUTXO.find(spent) != mapUnspentUTXO.end())
                    {
                        mapUnspentUTXO.erase(spent);
                        vRemovedUTXO.push_back(spent);
                    }
                }

                for(const auto& txDel : vRemovedUTXO)
                {
                    const auto& pos = find(vSpentUTXO.begin(), vSpentUTXO.end(), txDel);
                    vSpentUTXO.erase(pos);
                }

            }

            pIndex = pIndex->pPrev;
        }
        Log("B", "Checking duration before comparing unspent ===> %s\n", t_fork.format().c_str());
        if(nLevel >= 3)
        {
            //compare unspent with transaction
            if(!dbBlock.CompareRangedUnspentTx(fork.hashFork, mapUnspentUTXO))
            {


                Error("B", "{%d} ranged unspent records do not match with full collection of unspent.\n", mapUnspentUTXO.size());
                return false;
            }
        }

        Log("B", "Checking duration of fork{%s} ===> %s\n", fork.hashFork.ToString().c_str(), t_fork.format().c_str());
    }

    Log("B", "Checking duration ===> %s\n", t_check.format().c_str());

    Log("B", "Data consistency verified.\n");

    return true;
}

bool CBlockBase::CheckInputSingleAddressForTxWithChange(const uint256& txid)
{
    CTransaction tx;
    if(!RetrieveTx(txid, tx))
    {
        Error("B", "[CBlockBase::CheckInputSingleAddressForTxWithChange](%s): Failed to call to RetrieveTx.\n", txid.ToString().c_str());
        return false;
    }

    //validate if this is a transaction which has a change
    CTxIndex TxIdx;
    if(!dbBlock.RetrieveTxIndex(tx.GetHash(), TxIdx))
    {
        assert(0);
    }
    int64 nChange = TxIdx.nValueIn - TxIdx.nAmount - tx.nTxFee;
    if(nChange <= 0)
    {
        Error("B", "[CBlockBase::CheckInputSingleAddressForTxWithChange]: Tx(%s) is not a transaction with change.\n", txid.ToString().c_str());
        assert(0);
    }
    Log("B", "[CheckInputSingleAddressForTxWithChange]txid:(%s)change:(%s)\n", txid.ToString().c_str(), to_string(nChange).c_str());

    //get all inputs whose index is 0 if any
    vector<CDestination> vDestNoChange;
    vector<uint256> vTxExistChange;
    for(const auto& i : tx.vInput)
    {
        if(i.prevout.n == 0)
        {
            CTxIndex txIdx;
            if(!dbBlock.RetrieveTxIndex(i.prevout.hash, txIdx))
            {
                Error("B", "[CBlockBase::CheckInputSingleAddressForTxWithChange](%s): Failed to call to RetrieveTxIndex.\n", txid.ToString().c_str());
                return false;
            }
            vDestNoChange.push_back(txIdx.sendTo);
        }
        else
        {
            vTxExistChange.push_back(i.prevout.hash);
        }
    }
    sort(vDestNoChange.begin(), vDestNoChange.end());
    unique(vDestNoChange.begin(), vDestNoChange.end());

    //if destinations are not equal, return false
    if(vDestNoChange.size() > 1)
    {
        Error("B", "[CBlockBase::CheckInputSingleAddressForTxWithChange](%s): {vDestNoChange.size() > 1}.\n", txid.ToString().c_str());
        return false;
    }

    //if destination from input is not equal to output, return false
    if(vDestNoChange.size() == 1)
    {
        CTxIndex txIdx;
        if(!dbBlock.RetrieveTxIndex(txid, txIdx))
        {
            Error("B", "[CBlockBase::CheckInputSingleAddressForTxWithChange](%s): Failed to call to RetrieveTxIndex{vDestNoChange.size() == 1}.\n", txid.ToString().c_str());
            return false;
        }
        CDestination dest = *(vDestNoChange.begin());
        if(dest != txIdx.sendTo)
        {
            Error("B", "[CBlockBase::CheckInputSingleAddressForTxWithChange](%s): {dest != txIdx.sendTo}.\n", txid.ToString().c_str());
            return false;
        }
    }

    //if exist output index is 1, recur the process
    vector<bool> vRes;
    for(const auto& i : vTxExistChange)
    {
        vRes.push_back(CheckInputSingleAddressForTxWithChange(i));
    }

    if(!vRes.empty())
    {
        int nFalse = count(vRes.begin(), vRes.end(), false);
        if(nFalse <= 0)
        {
            return true;
        }
        else
        {
            Error("B", "Tx(%s) one or more preout validate failed.\n", txid.ToString().c_str());
            return false;
        }
//        return count(vRes.begin(), vRes.end(), false) <= 0;
    }
    else
    {
        return true;
    }
}

CBlockIndex* CBlockBase::GetIndex(const uint256& hash) const
{
    map<uint256,CBlockIndex*>::const_iterator mi = mapIndex.find(hash);
    return (mi != mapIndex.end() ? (*mi).second : NULL);
}

CBlockIndex* CBlockBase::GetBranch(CBlockIndex* pIndexRef,CBlockIndex* pIndex,vector<CBlockIndex*>& vPath)
{
    vPath.clear();
    while (pIndex != pIndexRef)
    {
        if (pIndexRef->GetBlockTime() > pIndex->GetBlockTime())
        {
            pIndexRef = pIndexRef->pPrev;
        }
        else if (pIndex->GetBlockTime() > pIndexRef->GetBlockTime())
        {
            vPath.push_back(pIndex);
            pIndex = pIndex->pPrev;
        }
        else
        {
            vPath.push_back(pIndex);
            pIndex = pIndex->pPrev;
            pIndexRef = pIndexRef->pPrev;
        }
    }
    return pIndex;
}

CBlockIndex* CBlockBase::GetOriginIndex(const uint256& txidMint) const
{
    CBlockIndex* pIndex = NULL;
    for (map<uint256,boost::shared_ptr<CBlockFork> >::const_iterator mi = mapFork.begin();mi != mapFork.end();++mi)
    {
        CBlockIndex* pIndex = (*mi).second->GetOrigin();
        if (pIndex->txidMint == txidMint)
        {
            return pIndex;
        }
    }
    return NULL;
}

CBlockIndex* CBlockBase::AddNewIndex(const uint256& hash,const CBlock& block,uint32 nFile,uint32 nOffset)
{
    CBlockIndex* pIndexNew = new CBlockIndex(block,nFile,nOffset);
    if (pIndexNew != NULL)
    {
        map<uint256, CBlockIndex*>::iterator mi = mapIndex.insert(make_pair(hash, pIndexNew)).first;
        pIndexNew->phashBlock = &((*mi).first);
    
        int64 nMoneySupply = block.GetBlockMint();
        uint64 nChainTrust = block.GetBlockTrust();
        uint64 nRandBeacon = block.GetBlockBeacon();
        CBlockIndex* pIndexPrev = NULL;
        map<uint256, CBlockIndex*>::iterator miPrev = mapIndex.find(block.hashPrev);
        if (miPrev != mapIndex.end())
        {
            pIndexPrev = (*miPrev).second;
            pIndexNew->pPrev = pIndexPrev;
            pIndexNew->nHeight = pIndexPrev->nHeight + (pIndexNew->IsExtended() ? 0 : 1);
            if (!pIndexNew->IsOrigin())
            {
                pIndexNew->pOrigin = pIndexPrev->pOrigin;
                nRandBeacon ^= pIndexNew->pOrigin->nRandBeacon;
            }
            nMoneySupply += pIndexPrev->nMoneySupply;
            nChainTrust += pIndexPrev->nChainTrust;
        }
        pIndexNew->nMoneySupply = nMoneySupply;
        pIndexNew->nChainTrust = nChainTrust;
        pIndexNew->nRandBeacon = nRandBeacon;
    }
    return pIndexNew;
}

boost::shared_ptr<CBlockFork> CBlockBase::GetFork(const uint256& hash) 
{
    map<uint256,boost::shared_ptr<CBlockFork> >::iterator mi = mapFork.find(hash);
    return (mi != mapFork.end() ? (*mi).second : NULL);
}

boost::shared_ptr<CBlockFork> CBlockBase::GetFork(const std::string& strName) 
{
    for (map<uint256,boost::shared_ptr<CBlockFork> >::iterator mi = mapFork.begin(); mi != mapFork.end(); ++mi)
    {
        const CProfile& profile = (*mi).second->GetProfile();
        if (profile.strName == strName)
        {
            return ((*mi).second);
        }
    }
    return NULL;
}

boost::shared_ptr<CBlockFork> CBlockBase::AddNewFork(const CProfile& profileIn,CBlockIndex* pIndexLast)
{
    boost::shared_ptr<CBlockFork> spFork = boost::shared_ptr<CBlockFork>(new CBlockFork(profileIn,pIndexLast));
    if (spFork != NULL)
    {
        spFork->UpdateNext();
        mapFork.insert(make_pair(pIndexLast->GetOriginHash(),spFork));
    }

    return spFork;
}

bool CBlockBase::LoadForkProfile(const CBlockIndex* pIndexOrigin,CProfile& profile)
{
    profile.SetNull();

    CBlock block;
    if (!Retrieve(pIndexOrigin,block))
    {
        return false;
    }

    if (!profile.Load(block.vchProof))
    {
        return false;
    }

    return true;
}

bool CBlockBase::UpdateDelegate(const uint256& hash,CBlockEx& block)
{
    map<CDestination,int64> mapDelegate;
    if (!dbBlock.RetrieveDelegate(block.hashPrev,0,mapDelegate))
    {
        return false;
    }
    
    if (block.txMint.nType == CTransaction::TX_STAKE)
    {
        mapDelegate[block.txMint.sendTo] += block.txMint.nAmount;
    }

    for (int i = 0;i < block.vtx.size();i++)
    {
        CTransaction& tx = block.vtx[i];
        {
            CTemplateId tid;
            if (tx.sendTo.GetTemplateId(tid) && tid.GetType() == TEMPLATE_DELEGATE)
            {
                mapDelegate[tx.sendTo] += tx.nAmount;
            }
        }

        CTxContxt& txContxt = block.vTxContxt[i]; 
        {
            CTemplateId tid;
            if (txContxt.destIn.GetTemplateId(tid) && tid.GetType() == TEMPLATE_DELEGATE)
            {
                mapDelegate[txContxt.destIn] -= tx.nAmount + tx.nTxFee;
            }
        }
    }

    return dbBlock.UpdateDelegate(hash,mapDelegate);
}

bool CBlockBase::UpdateEnroll(CBlockIndex* pIndexNew,const vector<pair<uint256,CTxIndex> >& vTxNew)
{
    vector<pair<CTxIndex,uint256> > vEnroll;
    for (int i = 0;i < vTxNew.size();i++)
    {
        const CTxIndex& txIndex = vTxNew[i].second;
        if (txIndex.nType == CTransaction::TX_CERT)
        {
            CBlockIndex* pIndex = pIndexNew;
            while (pIndex->GetBlockHeight() > txIndex.nBlockHeight)
            {
                pIndex = pIndex->pPrev;
            }
            vEnroll.push_back(make_pair(txIndex,pIndex->GetBlockHash()));
        }
    }
    return (vEnroll.empty() || dbBlock.UpdateEnroll(vEnroll));
}

bool CBlockBase::GetTxUnspent(const uint256 fork,const CTxOutPoint& out,CTxOutput& unspent)
{
    return dbBlock.RetrieveTxUnspent(fork,out,unspent);
}

bool CBlockBase::GetTxNewIndex(CBlockView& view,CBlockIndex* pIndexNew,vector<pair<uint256,CTxIndex> >& vTxNew)
{
    vector<CBlockIndex*> vPath;
    if (view.GetFork() != NULL && view.GetFork()->GetLast() != NULL)
    {
        GetBranch(view.GetFork()->GetLast(),pIndexNew,vPath);
    }
    else
    {
        vPath.push_back(pIndexNew);
    }

    CWalleveBufStream ss;
    for (int i = vPath.size() - 1;i >= 0;i--)
    {
        CBlockIndex* pIndex = vPath[i];
        CBlockEx block;
        if (!tsBlock.Read(block,pIndex->nFile,pIndex->nOffset))
        {
            return false;
        }
        int nHeight = pIndex->GetBlockHeight();
        uint32 nOffset = pIndex->nOffset + block.GetTxSerializedOffset();

        if (!block.txMint.IsNull())
        {
            CTxIndex txIndex(block.txMint,CDestination(),0,nHeight,pIndex->nFile,nOffset);
            vTxNew.push_back(make_pair(block.txMint.GetHash(),txIndex));
        }
        nOffset += ss.GetSerializeSize(block.txMint);

        CVarInt var(block.vtx.size());
        nOffset += ss.GetSerializeSize(var);
        for (int i = 0;i < block.vtx.size();i++)
        {
            CTransaction& tx = block.vtx[i];
            CTxContxt& txCtxt = block.vTxContxt[i];
            uint256 txid = tx.GetHash();
            CTxIndex txIndex(tx,txCtxt.destIn,txCtxt.GetValueIn(),nHeight,pIndex->nFile,nOffset);
            vTxNew.push_back(make_pair(txid,txIndex));
            nOffset += ss.GetSerializeSize(tx);
        }
    }
    return true;
}

void CBlockBase::ClearCache()
{
    map<uint256, CBlockIndex *>::iterator mi;
    for (mi = mapIndex.begin(); mi != mapIndex.end(); ++mi)
    {
        delete (*mi).second;
    }
    mapIndex.clear(); 
    mapFork.clear();
}

bool CBlockBase::LoadDB()
{
    CWalleveWriteLock wlock(rwAccess);

    ClearCache();
    CBlockWalker walker(this);
    if (!dbBlock.WalkThroughBlock(walker))
    {
        ClearCache();
        return false;
    }

    vector<uint256> vFork;
    if (!dbBlock.RetrieveFork(vFork))
    {
        ClearCache();
        return false;
    }
    for (int i = 0;i < vFork.size();i++)
    {   
        CBlockIndex* pIndex = GetIndex(vFork[i]);
        if (pIndex == NULL)
        {
            ClearCache();
            return false;
        }
        CProfile profile;
        if (!LoadForkProfile(pIndex->pOrigin,profile))
        {
            return false;
        }
        boost::shared_ptr<CBlockFork> spFork = AddNewFork(profile,pIndex);
        if (spFork == NULL)
        {
            return false;
        }
    }

    return true;
}

bool CBlockBase::SetupLog(const path& pathLocation,bool fDebug)
{

    if (!walleveLog.SetLogFilePath((pathLocation / LOGFILE_NAME).string()))
    {
        return false;
    }
    fDebugLog = fDebug;
    return true;
}
