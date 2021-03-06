// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_UNSPENTDB_H
#define  MULTIVERSE_UNSPENTDB_H

#include "transaction.h"
#include "walleve/walleve.h"

#include <boost/thread/thread.hpp>

namespace multiverse
{
namespace storage
{

class CForkUnspentDBWalker
{
public:
    virtual bool Walk(const CTxOutPoint& txout,const CTxOutput& output) = 0;
};

class CForkUnspentDB : public walleve::CKVDB
{
    typedef std::map<CTxOutPoint,CTxOutput> MapType;
    class CDblMap
    {
    public:
        CDblMap() : nIdxUpper(0) {}
        MapType& GetUpperMap()
        {
            return mapCache[nIdxUpper];
        }
        MapType& GetLowerMap()
        {
            return mapCache[nIdxUpper ^ 1];
        }
        void Flip()
        {
            MapType& mapLower = mapCache[nIdxUpper ^ 1];
            mapLower.clear();
            nIdxUpper = nIdxUpper ^ 1;
        }
        void Clear()
        {
            mapCache[0].clear();
            mapCache[1].clear();
            nIdxUpper = 0;
        }
    protected:
        MapType mapCache[2];
        int nIdxUpper;
    };
public:
    CForkUnspentDB(const boost::filesystem::path& pathDB);
    ~CForkUnspentDB();
    bool RemoveAll();
    bool UpdateUnspent(const std::vector<CTxUnspent>& vAddNew,const std::vector<CTxOutPoint>& vRemove);
    bool WriteUnspent(const CTxOutPoint& txout,const CTxOutput& output);
    bool ReadUnspent(const CTxOutPoint& txout,CTxOutput& output);
    bool Copy(CForkUnspentDB& dbUnspent);
    void SetCache(const CDblMap& dblCacheIn) { dblCache = dblCacheIn; }
    bool WalkThroughUnspent(CForkUnspentDBWalker& walker);
    bool Flush();
protected:
    bool CopyWalker(walleve::CWalleveBufStream& ssKey, walleve::CWalleveBufStream& ssValue,
                    CForkUnspentDB& dbUnspent);
    bool LoadWalker(walleve::CWalleveBufStream& ssKey, walleve::CWalleveBufStream& ssValue,
                    CForkUnspentDBWalker& walker,const MapType& mapUpper,const MapType& mapLower);
protected:
    walleve::CWalleveRWAccess rwUpper;
    walleve::CWalleveRWAccess rwLower;    
    CDblMap dblCache;
};

class CUnspentDB
{
public:
    CUnspentDB();
    bool Initialize(const boost::filesystem::path& pathData);
    void Deinitialize();
    bool Exists(const uint256& hashFork) { return (!!mapUnspentDB.count(hashFork)); }
    bool AddNewFork(const uint256& hashFork);
    bool RemoveFork(const uint256& hashFork);
    void Clear();
    bool Update(const uint256& hashFork,
                const std::vector<CTxUnspent>& vAddNew,const std::vector<CTxOutPoint>& vRemove);
    bool Retrieve(const uint256& hashFork,const CTxOutPoint& txout,CTxOutput& output);
    bool Copy(const uint256& srcFork,const uint256& destFork);
    bool WalkThrough(const uint256& hashFork,CForkUnspentDBWalker& walker);
protected:
    void FlushProc();
protected:
    boost::filesystem::path pathUnspent;
    walleve::CWalleveRWAccess rwAccess;
    std::map<uint256,std::shared_ptr<CForkUnspentDB> > mapUnspentDB;

    boost::mutex mtxFlush;
    boost::condition_variable condFlush;
    boost::thread* pThreadFlush;
    bool fStopFlush;
};

} // namespace storage
} // namespace multiverse

#endif //MULTIVERSE_UNSPENTDB_H

