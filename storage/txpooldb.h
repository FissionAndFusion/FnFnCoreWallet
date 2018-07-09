// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_TXPOOLDB_H
#define  MULTIVERSE_TXPOOLDB_H

#include "dbconn.h"
#include "key.h"
#include "wallettx.h"

namespace multiverse
{
namespace storage
{

class CTxPoolDBTxWalker
{
public:
    virtual bool Walk(const uint256& txid,const uint256& hashFork,const CAssembledTx& tx) = 0;
};

class CTxPoolDB
{
public:
    CTxPoolDB();
    ~CTxPoolDB();
    bool Initialize(const CMvDBConfig& config);
    void Deinitialize();
    bool UpdateTx(const uint256& hashFork,const std::vector<std::pair<uint256,CAssembledTx> >& vAddNew,
                                          const std::vector<uint256>& vRemove=std::vector<uint256>());
    bool WalkThroughTx(CTxPoolDBTxWalker& walker); 
protected:
    bool CreateTable();
protected:
    CMvDBConn dbConn;
};

} // namespace storage
} // namespace multiverse

#endif //MULTIVERSE_TXPOOLDB_H

