// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_BLOCKDB_H
#define  MULTIVERSE_BLOCKDB_H

#include "dbpool.h"
#include "forkcontext.h"
#include "block.h"
#include "transaction.h"
#include "unspentdb.h"
#include "delegatedb.h"

namespace multiverse
{
namespace storage
{

class CBlockDBWalker
{
public:
    virtual bool Walk(CBlockOutline& outline) = 0;
};

class CBlockDB
{
public:
    CBlockDB();
    ~CBlockDB();
    bool Initialize(const boost::filesystem::path& pathData);
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
    bool AddNewBlock(const CBlockOutline& outline);
    bool RemoveBlock(const uint256& hash);
    bool UpdateDelegateContext(const uint256& hash,const CDelegateContext& ctxtDelegate);
    bool WalkThroughBlock(CBlockDBWalker& walker);
    bool ExistsTx(const uint256& txid);
    bool RetrieveTxIndex(const uint256& txid,CTxIndex& txIndex);
    bool RetrieveTxPos(const uint256& txid,uint32& nFile,uint32& nOffset);
    bool RetrieveTxLocation(const uint256& txid,uint256& hashAnchor,int& nBlockHeight);
    bool RetrieveTxUnspent(const uint256& fork,const CTxOutPoint& out,CTxOutput& unspent);
    bool RetrieveDelegate(const uint256& hash,std::map<CDestination,int64>& mapDelegate);
    bool RetrieveEnroll(const uint256& hashAnchor,const std::vector<uint256>& vBlockRange, 
                                                  std::map<CDestination,CDiskPos>& mapEnrollTxPos);
    bool InnoDB();
protected:
    bool CreateTable();
    bool LoadFork();
protected:
    CMvDBPool dbPool;
    CUnspentDB dbUnspent;
    CDelegateDB dbDelegate;
};

} // namespace storage
} // namespace multiverse

#endif //MULTIVERSE_BLOCKDB_H
