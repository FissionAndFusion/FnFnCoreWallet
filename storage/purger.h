// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_PURGER_H
#define MULTIVERSE_PURGER_H

#include "dbconn.h"
#include <boost/filesystem.hpp>

namespace multiverse
{
namespace storage
{

class CPurger
{
public:
    bool ResetDB(const CMvDBConfig& dbConfig) const;
    bool ResetCache(const boost::filesystem::path& pathDataLocation) const;
    bool RemoveBlockFile(const boost::filesystem::path& pathDataLocation) const;
    bool operator() (const CMvDBConfig& dbConfig,const boost::filesystem::path& pathDataLocation) const
    {
        return (ResetDB(dbConfig) && ResetCache(pathDataLocation) && RemoveBlockFile(pathDataLocation)); 
    }
};

} // namespace storage
} // namespace multiverse

#endif //MULTIVERSE_PURGER_H
