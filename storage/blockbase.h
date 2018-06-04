// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_BLOCKBASE_H
#define  MULTIVERSE_BLOCKBASE_H

#include "timeseries.h"
#include "blockdb.h"
#include "block.h"
#include "walleve/walleve.h"

#include <map>
#include <boost/thread/thread.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>

namespace multiverse
{
namespace storage
{

class CBlockBase;

class CBlockFork
{
public:
    CBlockFork(CBlockIndex* pIndexLastIn=NULL) 
    : pIndexLast(pIndexLastIn),spAccess(new walleve::CWalleveRWAccess())
    {
    }
    void ReadLock() { spAccess->ReadLock(); }
    void WriteLock() { spAccess->WriteLock(); }
    void ReadUnlock() { spAccess->ReadUnlock(); }
    void WriteUnlock() { spAccess->WriteUnlock(); }
    CBlockIndex* GetLast() const { return pIndexLast; }
    CBlockIndex* GetOrigin() const { return pIndexLast->pOrigin; }
    void UpdateLast(CBlockIndex* pIndexLastIn) { pIndexLast = pIndexLastIn; }
    void InsertAncestry(CBlockIndex* pAncestry) 
    {
        mapAncestry.insert(std::make_pair(pAncestry->GetOriginHash(),pAncestry)); 
    }
    void InsertSubline(int nHeight,const uint256& hash)
    {
        mapSubline.insert(std::make_pair(nHeight,hash));
    }
    void GetSubline(int nStart,std::vector<uint256>& vSubline) const
    {
        for (std::multimap<int,uint256>::const_iterator it = mapSubline.lower_bound(nStart);it != mapSubline.end();++it)
        {
            vSubline.push_back((*it).second);
        }
    }
    bool Have(CBlockIndex* pIndex) const
    {
        if (!pIndex)
        {
            return false;
        }
        uint256 hashFork = pIndex->GetOriginHash();
        CBlockIndex* p = NULL;
        if (hashFork == pIndexLast->GetOriginHash())
        {
            p = pIndexLast;
        }
        else
        {
            std::map<uint256,CBlockIndex*>::const_iterator it = mapAncestry.find(hashFork);
            if (it != mapAncestry.end())
            {
                p = (*it).second;
            }
        }
        while (p != pIndex && p != NULL && !p->IsOrigin())
        {
            p = p->pPrev;
        }
        return (p == pIndex);
    }
protected:
    CBlockIndex* pIndexLast;
    boost::shared_ptr<walleve::CWalleveRWAccess> spAccess;
    std::map<uint256,CBlockIndex*> mapAncestry;
    std::multimap<int,uint256> mapSubline;
};

class CBlockView
{
public:
    class CUnspent : public CTxOutput
    {
    public:
        int nOpt;
    public:
        CUnspent() : nOpt(0) {}
        void Enable(const CTxOutput& output) 
        { destTo = output.destTo; nAmount = output.nAmount; nLockUntil = output.nLockUntil; nOpt++; }
        void Disable() { SetNull(); nOpt--; }
        bool IsModified() const { return (nOpt != 0); }
    };
    CBlockView();
    ~CBlockView();
    void Initialize(CBlockBase* pBlockBaseIn,CBlockFork* pBlockForkIn,
                    const uint256& hashForkIn,bool fCommittableIn);
    void Deinitialize();
    bool IsCommittable() const { return fCommittable; }
    CBlockFork* GetFork() const { return pBlockFork; };
    const uint256& GetForkHash() const { return hashFork; };
    bool ExistsTx(const uint256& txid) const;
    bool RetrieveTx(const uint256& txid,CTransaction& tx);
    bool RetrieveUnspent(const CTxOutPoint& out,CTxOutput& unspent);
    void AddTx(const uint256& txid,const CTransaction& tx,const CDestination& destIn=CDestination(),int64 nValueIn=0);
    void AddTx(const uint256& txid,const CBlockTx& tx) { AddTx(txid,tx,tx.destIn,tx.nValueIn); }
    void RemoveTx(const uint256& txid,std::vector<CTxUnspent>& vPrevout);
    void GetUnspentChanges(std::vector<CTxUnspent>& vAddNew,std::vector<CTxOutPoint>& vRemove);
    void GetTxChanges(std::vector<uint256>& vAddNew,std::vector<uint256>& vRemove,std::set<uint256>& setUpdate);
protected:
    CBlockBase* pBlockBase;
    CBlockFork* pBlockFork;
    uint256 hashFork;
    bool fCommittable;
    std::map<uint256,CTransaction> mapTx;
    std::map<CTxOutPoint,CUnspent> mapUnspent;
    std::vector<uint256> vTxRemove;
    std::vector<uint256> vTxAddNew;
};

class CBlockBase
{
    friend class CBlockView;
public:
    CBlockBase();
    ~CBlockBase();
    bool Initialize(const CMvDBConfig& dbConfig,int nMaxDBConn,
                    const boost::filesystem::path& pathDataLocation,bool fDebug,bool fRenewDB=false);
    void Deinitialize();
    void Clear();
    bool IsEmpty() const;
    bool Exists(const uint256& hash) const;
    bool AddNew(const uint256& hash,CBlock& block,std::vector<CBlockTx>& vtx,CBlockIndex** ppIndexNew);
    bool Retrieve(const uint256& hash,CBlock& block);
    bool RetrieveIndex(const uint256& hash,CBlockIndex** ppIndex);
    bool RetrieveFork(const uint256& hash,CBlockIndex** ppIndex);
    bool RetrieveTx(const uint256& txid,CTransaction& tx);
    void ListForkIndex(std::multimap<int,CBlockIndex*>& mapForkIndex);
    bool GetBlockView(CBlockView& view);
    bool GetBlockView(const uint256& hash,CBlockView& view,bool fCommitable=false);
    bool CommitBlockView(CBlockView& view,CBlockIndex* pIndexNew);
    bool LoadIndex(CDiskBlockIndex& diskIndex);
protected:
    CBlockIndex* GetIndex(const uint256& hash) const;
    CBlockFork* GetFork(const uint256& hash);
    CBlockIndex* GetBranch(CBlockIndex* pIndexRef,CBlockIndex* pIndex,std::vector<CBlockIndex*>& vPath);
    CBlockIndex* AddNewIndex(const uint256& hash,CBlock& block,int64 nMint,uint32 nFile,uint32 nOffset);
    CBlockFork* AddNewFork(CBlockIndex* pIndexLast);
    CBlockIndex* GetTxLocation(CBlockFork* pFork,const uint256& txid);    
    bool GetTxUnspent(const uint256 fork,const CTxOutPoint& out,CTxOutput& unspent);
    bool AddNewTx(const uint256& txid,const CBlockTx& tx,CTxIndex& txIndex);
    void ClearCache();
    bool LoadDB();
    bool SetupLog(const boost::filesystem::path& pathDataLocation,bool fDebug);
    void Log(const char* pszIdent,const char *pszFormat,...)
    {
        va_list ap;
        va_start(ap, pszFormat);
        walleveLog(pszIdent,"",pszFormat, ap);
        va_end(ap);
    }
    void Error(const char* pszIdent,const char *pszFormat,...)
    {
        va_list ap;
        va_start(ap, pszFormat);
        walleveLog(pszIdent,"ERROR",pszFormat, ap);
        va_end(ap);
    }
    void Debug(const char* pszIdent,const char *pszFormat,...)
    {
        if (fDebugLog)
        {
            va_list ap;
            va_start(ap, pszFormat);
            walleveLog(pszIdent,"DEBUG",pszFormat, ap);
            va_end(ap);
        }
    }
protected:
    mutable walleve::CWalleveRWAccess rwAccess;
    walleve::CWalleveLog walleveLog;
    bool fDebugLog;
    CBlockDB dbBlock;
    CTimeSeries tsBlock;
    CTimeSeries tsTx;
    std::map<uint256,CBlockIndex*> mapIndex;
    std::map<uint256,CBlockFork> mapFork;
};

} // namespace storage
} // namespace multiverse

#endif //MULTIVERSE_BLOCKBASE_H

