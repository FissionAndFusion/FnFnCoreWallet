// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_DELEGATEDB_H
#define  MULTIVERSE_DELEGATEDB_H

#include "uint256.h"
#include "destination.h"
#include "timeseries.h"

#include "walleve/walleve.h"

#include <map>

namespace multiverse
{
namespace storage
{

class CDelegateContext
{
    friend class walleve::CWalleveStream;
public:
    std::map<CDestination,int64> mapVote;
    std::map<uint256,std::map<CDestination,CDiskPos> > mapEnrollTx;
protected:
    template <typename O>
    void WalleveSerialize(walleve::CWalleveStream& s,O& opt)
    { 
        s.Serialize(mapVote,opt);
        s.Serialize(mapEnrollTx,opt);
    }
};

class CDelegateDB : public walleve::CKVDB
{
public:
    CDelegateDB() : cacheDelegate(MAX_CACHE_COUNT) {}
    bool Initialize(const boost::filesystem::path& pathData);
    void Deinitialize();
    bool AddNew(const uint256& hashBlock,const CDelegateContext& ctxtDelegate);
    bool Remove(const uint256& hashBlock);
    bool RetrieveDelegatedVote(const uint256& hashBlock,std::map<CDestination,int64>& mapVote);
    bool RetrieveEnrollTx(const uint256& hashAnchor,const std::vector<uint256>& vBlockRange,
                          std::map<CDestination,CDiskPos>& mapEnrollTxPos);
    void Clear();
protected:
    bool Retrieve(const uint256& hashBlock,CDelegateContext& ctxtDelegate);
protected:
    enum { MAX_CACHE_COUNT = 64, };
    walleve::CWalleveCache<uint256,CDelegateContext> cacheDelegate;
};

} // namespace storage
} // namespace multiverse

#endif //MULTIVERSE_DELEGATEDB_H

