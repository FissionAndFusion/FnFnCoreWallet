// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_TXPOOLDB_H
#define  MULTIVERSE_TXPOOLDB_H

#include "walleve/walleve.h"
#include "transaction.h"

namespace multiverse
{
namespace storage
{

class CTxPoolDBTxWalker
{
public:
    virtual bool Walk(const uint256& txid,const uint256& hashFork,const CAssembledTx& tx) = 0;
};

class CTxPoolDB : public walleve::CKVDB
{
public:
    CTxPoolDB();
    ~CTxPoolDB();
    bool Initialize(const boost::filesystem::path& pathData);
    void Deinitialize();
    bool RemoveAll();
    bool UpdateTx(const uint256& hashFork,const std::vector<std::pair<uint256,CAssembledTx> >& vAddNew,
                                          const std::vector<uint256>& vRemove=std::vector<uint256>());
    bool WalkThroughTx(CTxPoolDBTxWalker& walker); 
protected:
    bool DBWalker(walleve::CWalleveBufStream& ssKey, walleve::CWalleveBufStream& ssValue) { return false; }
    bool LoadWalker(walleve::CWalleveBufStream& ssKey, walleve::CWalleveBufStream& ssValue,
                    std::map<uint64,std::pair<uint256,std::pair<uint256,CAssembledTx> > > & mapTx);
protected:
    uint64 nSequence;
};

} // namespace storage
} // namespace multiverse

#endif //MULTIVERSE_TXPOOLDB_H

