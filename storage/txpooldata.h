// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_TXPOOLDATA_H
#define  MULTIVERSE_TXPOOLDATA_H

#include "walleve/walleve.h"
#include "transaction.h"

#include <boost/filesystem.hpp>

namespace multiverse
{
namespace storage
{

class CTxPoolData
{
public:
    CTxPoolData();
    ~CTxPoolData();
    bool Initialize(const boost::filesystem::path& pathData);
    bool Remove();
    bool Save(const std::vector<std::pair<uint256,std::pair<uint256,CAssembledTx> > >& vTx);
    bool Load(std::vector<std::pair<uint256,std::pair<uint256,CAssembledTx> > >& vTx);
protected:
    boost::filesystem::path pathTxPoolFile;
};

} // namespace storage
} // namespace multiverse

#endif //MULTIVERSE_TXPOOLDATA_H

