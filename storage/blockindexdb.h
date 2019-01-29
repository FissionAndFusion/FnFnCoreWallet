// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_BLOCKINDEXDB_H
#define  MULTIVERSE_BLOCKINDEXDB_H

#include "block.h"
#include "walleve/walleve.h"

namespace multiverse
{
namespace storage
{

class CBlockDBWalker
{
public:
    virtual bool Walk(CBlockOutline& outline) = 0;
};

class CBlockIndexDB : public walleve::CKVDB
{
public:
    CBlockIndexDB() {}
    bool Initialize(const boost::filesystem::path& pathData);
    void Deinitialize();
    bool AddNewBlock(const CBlockOutline& outline);
    bool RemoveBlock(const uint256& hashBlock);
    bool WalkThroughBlock(CBlockDBWalker& walker);
    void Clear();
protected:
    bool LoadBlockWalker(walleve::CWalleveBufStream& ssKey,walleve::CWalleveBufStream& ssValue,
                         CBlockDBWalker& walker);
};

} // namespace storage
} // namespace multiverse

#endif //MULTIVERSE_BLOCKINDEXDB_H

