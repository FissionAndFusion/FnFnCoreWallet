// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_BLOCKDB_H
#define  MULTIVERSE_BLOCKDB_H

#include "dbpool.h"
#include "forkcontext.h"
#include "block.h"
#include "transaction.h"

namespace multiverse
{
namespace storage
{

class CBlockDBFork
{
public:
    CBlockDBFork(const uint256& hashForkIn=uint64(0),const uint256 hashRefIn=uint64(0),int nIndexIn=0)
    : hashFork(hashForkIn),hashRef(hashRefIn),nIndex(nIndexIn) {}
    uint256 hashFork;
    uint256 hashRef;
    int nIndex;
};

class CBlockDBWalker
{
public:
    virtual bool Walk(CBlockOutline& outline) = 0;
};

class CBlockDBTxFilter
{
public:
    CBlockDBTxFilter(CTxFilter& filterIn) 
    : filter(filterIn),destIn(filterIn.destIn),sendTo(filterIn.sendTo)
    {
    }
    virtual bool FoundTxIndex(const CDestination& destTxIn,int64 nValueTxIn,int nBlockHeight,uint32 nFile,uint32 nOffset) = 0;
public:
    CTxFilter& filter;
    CDestination& destIn;
    CDestination& sendTo;
};

class CBlockDB
{
public:
    CBlockDB();
    ~CBlockDB();
    bool Initialize();
    bool DBPoolInitialize(const CMvDBConfig& config,int nMaxDBConn);
    void Deinitialize();
    bool RemoveAll();
    bool AddNewForkContext(const CForkContext& ctxt);
    bool RetrieveForkContext(const uint256& hash,CForkContext& ctxt);
    bool FilterForkContext(CForkContextFilter& filer);
    bool AddNewFork(const uint256& hash);
    bool RemoveFork(const uint256& hash);
    bool RetrieveFork(std::vector<uint256>& vFork);
    bool UpdateFork(const uint256& hash,const uint256& hashRefBlock,const uint256& hashForkBased,
                    const std::vector<std::pair<uint256,CTxIndex> >& vTxNew,const std::vector<uint256>& vTxDel,
                    const std::vector<CTxUnspent>& vAddNew,const std::vector<CTxOutPoint>& vRemove);
    bool FetchFork(std::vector<CBlockDBFork>& vFork);
    bool AddNewBlock(const CBlockOutline& outline);
    bool RemoveBlock(const uint256& hash);
    bool ExistBlock(const uint256& hash);
    bool GetBlock(const uint256& hash, CBlockOutline& outline);
    bool UpdateDelegate(const uint256& hash,const std::map<CDestination,int64>& mapDelegate);
    bool UpdateEnroll(std::vector<std::pair<CTxIndex,uint256> >& vEnroll);
    bool WalkThroughBlock(CBlockDBWalker& walker);
    bool ExistsTx(const uint256& txid);
    bool RetrieveTxIndex(const uint256& txid,CTxIndex& txIndex);
    bool RetrieveTxPos(const uint256& txid,uint32& nFile,uint32& nOffset);
    bool RetrieveTxLocation(const uint256& txid,uint256& hashAnchor,int& nBlockHeight);
    bool RetrieveTxUnspent(const uint256& fork,const CTxOutPoint& out,CTxOutput& unspent);
    bool FilterTx(CBlockDBTxFilter& filter);
    bool RetrieveDelegate(const uint256& hash,int64 nMinAmount,std::map<CDestination,int64>& mapDelegate);
    bool RetrieveEnroll(const uint256& hashAnchor,const std::set<uint256>& setBlockRange, 
                        std::map<CDestination,std::pair<uint32,uint32> >& mapEnrollTxPos);
    bool InnoDB();
protected:
    int GetForkIndex(const uint256& hash)
    {
        std::map<uint256,int>::iterator it = mapForkIndex.find(hash);
        return  (it != mapForkIndex.end() ? (*it).second : -1);
    }
    bool CreateTable();
    bool LoadFork();
protected:
    CMvDBPool dbPool;
    std::map<uint256,int> mapForkIndex;
};

} // namespace storage
} // namespace multiverse

#endif //MULTIVERSE_BLOCKDB_H
