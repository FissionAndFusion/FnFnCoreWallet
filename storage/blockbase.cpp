// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "blockbase.h"

using namespace std;
using namespace boost::filesystem;
using namespace walleve;
using namespace multiverse::storage;

#define BLOCKFILE_PREFIX	"block"
#define TXFILE_PREFIX		"tx"
#define LOGFILE_NAME            "storage.log"

//////////////////////////////
// CBlockBaseDBWalker

class CBlockWalker : public CBlockDBWalker
{
public:
    CBlockWalker(CBlockBase* pBaseIn) : pBase(pBaseIn) {}
    bool Walk(CDiskBlockIndex& diskIndex) { return pBase->LoadIndex(diskIndex); } 
public:
    CBlockBase* pBase;
};

//////////////////////////////
// CBlockView
 
CBlockView::CBlockView()
: pBlockBase(NULL),pBlockFork(NULL),hashFork(0),fCommittable(false) 
{
}

CBlockView::~CBlockView()
{
    Deinitialize();
}

void CBlockView::Initialize(CBlockBase* pBlockBaseIn,CBlockFork* pBlockForkIn,
                            const uint256& hashForkIn,bool fCommittableIn)
{
    Deinitialize();

    pBlockBase = pBlockBaseIn;
    pBlockFork = pBlockForkIn;
    hashFork = hashForkIn;
    fCommittable = fCommittableIn;
    if (pBlockBase && pBlockFork)
    {
        if (fCommittable)
        {
            pBlockFork->WriteLock();
        }
        else
        {
            pBlockFork->ReadLock();
        }
    }
}

void CBlockView::Deinitialize()
{
    if (pBlockBase)
    {
        if (pBlockFork)
        {
            if (fCommittable)
            {
                pBlockFork->WriteUnlock();
            }
            else
            {
                pBlockFork->ReadUnlock();
            }
            pBlockFork = NULL;
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
    return (!!(pBlockBase->GetTxLocation(pBlockFork,txid)));
}

bool CBlockView::RetrieveTx(const uint256& txid,CTransaction& tx)
{
    map<uint256,CTransaction>::const_iterator it = mapTx.find(txid);
    if (it != mapTx.end())
    {
        tx = (*it).second;
        return (!tx.IsNull());
    }
    if (!pBlockBase->GetTxLocation(pBlockFork,txid))
    {
        return false;
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

void CBlockView::RemoveTx(const uint256& txid,vector<CTxUnspent>& vPrevout)
{
    mapTx[txid].SetNull();
    for (int i = 0;i < vPrevout.size();i++)
    {   
        mapUnspent[vPrevout[i]].Enable(vPrevout[i].output);
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
    tsTx.Deinitialize();
}

bool CBlockBase::Initialize(const CMvDBConfig& dbConfig,int nMaxDBConn,const path& pathDataLocation,bool fDebug,bool fRenewDB)
{
    if (!SetupLog(pathDataLocation,fDebug))
    {
        return false;
    }

    Log("B","Initializing... (Path : %s)\n",pathDataLocation.string().c_str());

    if (!dbBlock.Initialize(dbConfig,nMaxDBConn))
    {
        Error("B","Failed to initialize block db\n");
        return false;
    }
     
    if (!tsBlock.Initialize(pathDataLocation / "block",BLOCKFILE_PREFIX)
        || !tsTx.Initialize(pathDataLocation / "tx",TXFILE_PREFIX))
    {
        dbBlock.Deinitialize();
        Error("B","Failed to initialize block/tx tsfile\n");
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
        tsTx.Deinitialize();
        ClearCache();

        Error("B","Failed to load block db\n");
        return false;
    }
    Log("B","Initialized\n");
    return true;
}

void CBlockBase::Deinitialize()
{
    CWalleveWriteLock wlock(rwAccess);

    dbBlock.Deinitialize();
    tsBlock.Deinitialize();
    tsTx.Deinitialize();
    ClearCache();
    Log("B","Deinitialized\n");
}

bool CBlockBase::Exists(const uint256& hash) const
{
    CWalleveReadLock rlock(rwAccess);
 
    return (!!mapIndex.count(hash));
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

bool CBlockBase::AddNew(const uint256& hash,CBlock& block,vector<CBlockTx>& vtx,CBlockIndex** ppIndexNew)
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

    vector<uint256> vTxid;
    block.GetTxHash(vTxid);
    vector<pair<uint256,CTxIndex> > vTxIndex;
    vTxIndex.resize(vTxid.size());
    for (int i = 0;i < vTxid.size();i++)
    {
        const uint256& txid = vTxid[i];
        vTxIndex[i].first = txid;
        CTxIndex& txIndex = vTxIndex[i].second;
        if (!AddNewTx(txid,vtx[i],txIndex))
        {
            return false;
        }
    }
    int64 nMint = (vtx[0].nAmount - vtx[0].nValueIn);
    { 
        CWalleveWriteLock wlock(rwAccess);
        CBlockIndex* pIndexNew = AddNewIndex(hash,block,nMint,nFile,nOffset);
        if (pIndexNew == NULL)
        {
            return false;
        }

        if (!dbBlock.AddNewBlock(CDiskBlockIndex(pIndexNew,block),vTxIndex))
        {
            mapIndex.erase(hash);
            delete pIndexNew;
            return false;
        }
        *ppIndexNew = pIndexNew;
    }
    Log("B","AddNew block,hash=%s\n",hash.ToString().c_str());
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

bool CBlockBase::RetrieveIndex(const uint256& hash,CBlockIndex** ppIndex)
{
    CWalleveReadLock rlock(rwAccess);
    *ppIndex = GetIndex(hash);
    return (*ppIndex != NULL);
}

bool CBlockBase::RetrieveFork(const uint256& hash,CBlockIndex** ppIndex)
{
    CWalleveReadLock rlock(rwAccess);
    CBlockFork* pFork = GetFork(hash);
    if (pFork == NULL)
    {
        return false;
    }
    *ppIndex = pFork->GetLast();
    return true;
}

bool CBlockBase::RetrieveTx(const uint256& txid,CTransaction& tx)
{
    tx.SetNull();

    uint32 nTxFile,nTxOffset;
    {
        CWalleveReadLock rlock(rwAccess);
        if (!dbBlock.RetrieveTxPos(txid,nTxFile,nTxOffset))
        {
            return false;
        }
    }

    if (!tsTx.Read(tx,nTxFile,nTxOffset))
    {
        return false;
    }
    return true;
}

bool CBlockBase::GetBlockView(CBlockView& view)
{
    view.Initialize(this,NULL,0,false);
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

    CBlockFork* pFork = GetFork(hashOrigin);
    if (pFork == NULL)
    {
        return false;
    }
    
    view.Initialize(this,pFork,hashOrigin,fCommitable);

    CBlockIndex* pForkLast = pFork->GetLast(); 

    vector<CBlockIndex*> vPath;
    CBlockIndex* pBranch = GetBranch(pForkLast,pIndex,vPath);

    for (CBlockIndex* p = pForkLast;p != pBranch;p = p->pPrev)
    {
        // remove block tx;
        CBlock block;
        if (!tsBlock.Read(block,p->nFile,p->nOffset))
        {
            return false;
        }
        vector<uint256> vTxid;
        block.GetTxHash(vTxid);
        for (int j = vTxid.size() - 1;j >= 0;j--)
        {
            const uint256& txid = vTxid[j];
            CTxIndex txIndex;
            if (!dbBlock.RetrieveTxIndex(txid,txIndex))
            {
                return false;
            }
            vector<CTxUnspent> vPrevout;
            txIndex.GetPrevout(vPrevout);
            view.RemoveTx(txid,vPrevout);
        }
    }

    for (int i = 0;i < vPath.size();i++)
    {
        // add block tx;
        CBlock block;
        if (!tsBlock.Read(block,vPath[i]->nFile,vPath[i]->nOffset))
        {
            return false;
        }
        vector<uint256> vTxid;
        block.GetTxHash(vTxid);
        for (int j = 0;j < vTxid.size();j++)
        {
            const uint256& txid = vTxid[j];
            CTxIndex txIndex;
            if (!dbBlock.RetrieveTxIndex(txid,txIndex))
            {
                return false;
            }
            CTransaction tx;
            if (!tsTx.Read(tx,txIndex.nFile,txIndex.nOffset))
            {
                return false;
            }
            view.AddTx(txid,tx,txIndex.destIn,txIndex.nValueIn);
        }
    }
    return true;
}

bool CBlockBase::CommitBlockView(CBlockView& view,CBlockIndex* pIndexNew)
{
    const uint256 hashFork = pIndexNew->GetOriginHash();
    CBlockFork* pFork = NULL;
    if (hashFork == view.GetForkHash())
    {
        if (!view.IsCommittable())
        {
            return false;
        }
        pFork = view.GetFork();
    } 
    else
    {
        if (!dbBlock.AddNewFork(hashFork))
        {
            return false;
        }
        pFork = AddNewFork(pIndexNew);
    }
    vector<CTxUnspent> vAddNew;
    vector<CTxOutPoint> vRemove;
    view.GetUnspentChanges(vAddNew,vRemove);
    if (!dbBlock.UpdateFork(hashFork,pIndexNew->GetBlockHash(),view.GetForkHash(),vAddNew,vRemove))
    {
        return false;
    }
    pFork->UpdateLast(pIndexNew);
    Log("B","Update fork %s, last block hash=%s\n",hashFork.ToString().c_str(),
                                                   pIndexNew->GetBlockHash().ToString().c_str());
    return true;              
}

bool CBlockBase::LoadIndex(CDiskBlockIndex& diskIndex)
{
    uint256 hash = diskIndex.GetBlockHash();
    CBlockIndex* pIndexNew = new CBlockIndex(static_cast<CBlockIndex&>(diskIndex));
    if (pIndexNew == NULL)
    {
        return false;
    }
    map<uint256, CBlockIndex*>::iterator mi;
    mi = mapIndex.insert(make_pair(hash, pIndexNew)).first;
    pIndexNew->phashBlock = &((*mi).first);
    pIndexNew->pPrev = NULL;
    pIndexNew->pOrigin = pIndexNew;
    if (diskIndex.hashPrev != 0)
    {
        pIndexNew->pPrev = GetIndex(diskIndex.hashPrev);
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

CBlockIndex* CBlockBase::GetIndex(const uint256& hash) const
{
    map<uint256,CBlockIndex*>::const_iterator mi = mapIndex.find(hash);
    return (mi != mapIndex.end() ? (*mi).second : NULL);
}

CBlockFork* CBlockBase::GetFork(const uint256& hash) 
{
    map<uint256,CBlockFork>::iterator mi = mapFork.find(hash);
    return (mi != mapFork.end() ? &(*mi).second : NULL);
}

CBlockIndex* CBlockBase::GetBranch(CBlockIndex* pIndexRef,CBlockIndex* pIndex,vector<CBlockIndex*>& vPath)
{
    vPath.clear();
    while (pIndexRef->nHeight > pIndex->nHeight)
    {
        pIndexRef = pIndexRef->pPrev;
    }
    while (pIndex->nHeight > pIndexRef->nHeight)
    {
        vPath.push_back(pIndex);
        pIndex = pIndex->pPrev;
    }
    while (pIndex != pIndexRef)
    {
        vPath.push_back(pIndex);
        pIndex = pIndex->pPrev;
        pIndexRef = pIndexRef->pPrev;
    }
    return pIndex;
}

CBlockIndex* CBlockBase::AddNewIndex(const uint256& hash,CBlock& block,int64 nMint,uint32 nFile,uint32 nOffset)
{
    CBlockIndex* pIndexNew = new CBlockIndex(block,nFile,nOffset);
    if (pIndexNew != NULL)
    {
        map<uint256, CBlockIndex*>::iterator mi = mapIndex.insert(make_pair(hash, pIndexNew)).first;
        pIndexNew->phashBlock = &((*mi).first);
    
        int64 nMoneySupply = nMint;
        uint64 nChainTrust = block.GetBlockTrust();
        uint64 nRandBeacon = block.GetBlockBeacon();
        CBlockIndex* pIndexPrev = NULL;
        map<uint256, CBlockIndex*>::iterator miPrev = mapIndex.find(block.hashPrev);
        if (miPrev != mapIndex.end())
        {
            pIndexPrev = (*miPrev).second;
            pIndexNew->pPrev = pIndexPrev;
            pIndexNew->nHeight = pIndexPrev->nHeight + 1;
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

CBlockFork* CBlockBase::AddNewFork(CBlockIndex* pIndexLast)
{
    uint256 hash = pIndexLast->GetOriginHash();
    CBlockFork* pFork = &mapFork[hash];
    *pFork = CBlockFork(pIndexLast);
    CBlockIndex* pIndexPrev = pIndexLast->pOrigin->pPrev;
    if (pIndexPrev)
    {
        mapFork[pIndexPrev->GetOriginHash()].InsertSubline(pIndexPrev->GetBlockHeight(),hash);
        do 
        {
            pFork->InsertAncestry(pIndexPrev);
            pIndexPrev = pIndexPrev->pOrigin->pPrev;
        } while(pIndexPrev);
    }
    return pFork;
}

CBlockIndex* CBlockBase::GetTxLocation(CBlockFork* pFork,const uint256& txid)
{
    if (pFork == NULL)
    {
        return NULL;
    }
    vector<uint256> vBlockHash;
    if (!dbBlock.RetrieveTxLocation(txid,vBlockHash))
    {
        return NULL;
    }
    
    for (size_t i = 0;i < vBlockHash.size();i++)
    {
        CBlockIndex* pIndex = GetIndex(vBlockHash[i]);
        if (pFork->Have(pIndex))
        {
            return pIndex;
        }
    }
    return NULL; 
}

bool CBlockBase::GetTxUnspent(const uint256 fork,const CTxOutPoint& out,CTxOutput& unspent)
{
    return dbBlock.RetrieveTxUnspent(fork,out,unspent);
}

bool CBlockBase::AddNewTx(const uint256& txid,const CBlockTx& tx,CTxIndex& txIndex)
{
    if (!dbBlock.ExistsTx(txid))
    {
        uint32 nTxFile,nTxOffset;
        if (!tsTx.Write(static_cast<const CTransaction&>(tx),nTxFile,nTxOffset))
        {
            return false;
        }
        txIndex = CTxIndex(tx,nTxFile,nTxOffset);
    }
    else
    {
        txIndex.SetNull();
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
        AddNewFork(pIndex);
    }

    return true;
}

bool CBlockBase::SetupLog(const path& pathLocation,bool fDebug)
{
    if (!exists(pathLocation))
    {
        create_directories(pathLocation);
    }
    if (!is_directory(pathLocation))
    {
        return false;
    }
    if (!walleveLog.SetLogFilePath((pathLocation / LOGFILE_NAME).string()))
    {
        return false;
    }
    fDebugLog = fDebug;
    return true;
}
