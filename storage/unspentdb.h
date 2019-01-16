// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_UNSPENTDB_H
#define  MULTIVERSE_UNSPENTDB_H

#include "walleve/walleve.h"
#include "transaction.h"

namespace multiverse
{
namespace storage
{

class CForkUnspentDBWalker
{
public:
    virtual bool Walk(const CTxOutPoint& txout,const CTxOutput& output) = 0;
};

class CForkUnspentCheckWalker : public CForkUnspentDBWalker
{
public:
    CForkUnspentCheckWalker(const std::map<uint256, CTxUnspent>& mapUnspent)
            : nMatch(0), nAll(0), nRanged(mapUnspent.size()), mapUnspentTx(mapUnspent) {};
    bool Walk(const CTxOutPoint& txout, const CTxOutput& output) override
    {
        ++nAll;
        for(const auto& item : mapUnspentTx)
        {
            const CTxUnspent& unspent = item.second;
            if(unspent.hash == txout.hash && unspent.n == txout.n
                    && unspent.output.destTo == output.destTo
                    && unspent.output.nAmount == output.nAmount
                    && unspent.output.nLockUntil == output.nLockUntil)
            {
                ++nMatch;
            }
        }
    };

public:
    uint64 nMatch;
    uint64 nAll;
    uint64 nRanged;
    const std::map<uint256, CTxUnspent>& mapUnspentTx;
};

class CForkUnspentDB : public walleve::CKVDB
{
public:
    CForkUnspentDB(const boost::filesystem::path& pathDB);
    ~CForkUnspentDB();
    bool RemoveAll();
    bool UpdateUnspent(const std::vector<CTxUnspent>& vAddNew,const std::vector<CTxOutPoint>& vRemove);
    bool WriteUnspent(const CTxOutPoint& txout,const CTxOutput& output);
    bool ReadUnspent(const CTxOutPoint& txout,CTxOutput& output);
    bool Copy(CForkUnspentDB& dbUnspent);
    bool WalkThroughUnspent(CForkUnspentDBWalker& walker); 
protected:
    bool DBWalker(walleve::CWalleveBufStream& ssKey, walleve::CWalleveBufStream& ssValue) { return false; }
    bool CopyWalker(walleve::CWalleveBufStream& ssKey, walleve::CWalleveBufStream& ssValue,
                    CForkUnspentDB& dbUnspent);
    bool LoadWalker(walleve::CWalleveBufStream& ssKey, walleve::CWalleveBufStream& ssValue,
                    CForkUnspentDBWalker& walker);
};

class CUnspentDB
{
public:
    bool Initialize(const boost::filesystem::path& pathData);
    void Deinitialize();
    bool Exists(const uint256& hashFork) { return (!!mapUnspenDB.count(hashFork)); }
    bool AddNew(const uint256& hashFork);
    bool Remove(const uint256& hashFork);
    void Clear();
    bool Update(const uint256& hashFork,
                const std::vector<CTxUnspent>& vAddNew,const std::vector<CTxOutPoint>& vRemove);
    bool Retrieve(const uint256& hashFork,const CTxOutPoint& txout,CTxOutput& output);
    bool Copy(const uint256& srcFork,const uint256& destFork);
    bool WalkThrough(const uint256& hashFork,CForkUnspentDBWalker& walker);
protected:
    boost::filesystem::path pathUnspent;
    std::map<uint256,std::shared_ptr<CForkUnspentDB> > mapUnspenDB;
};

} // namespace storage
} // namespace multiverse

#endif //MULTIVERSE_UNSPENTDB_H

