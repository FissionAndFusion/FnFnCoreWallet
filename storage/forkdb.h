// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_FORKDB_H
#define  MULTIVERSE_FORKDB_H

#include "uint256.h"
#include "forkcontext.h"
#include "walleve/walleve.h"
#include <map>

namespace multiverse
{
namespace storage
{

class CForkDB : public walleve::CKVDB
{
public:
    CForkDB() {}
    bool Initialize(const boost::filesystem::path& pathData);
    void Deinitialize();
    bool AddNewForkContext(const CForkContext& ctxt);
    bool RemoveForkContext(const uint256& hashFork);
    bool RetrieveForkContext(const uint256& hashFork,CForkContext& ctxt);
    bool ListForkContext(std::vector<CForkContext>& vForkCtxt);
    bool UpdateFork(const uint256& hashFork,const uint256& hashLastBlock = uint256());
    bool RemoveFork(const uint256& hashFork);
    bool RetrieveFork(const uint256& hashFork,uint256& hashLastBlock);
    bool ListFork(std::vector<std::pair<uint256,uint256> >& vFork);
    void Clear();
protected:
    bool LoadCtxtWalker(walleve::CWalleveBufStream& ssKey,walleve::CWalleveBufStream& ssValue,
                        std::multimap<int,CForkContext>& mapCtxt);
    bool LoadForkWalker(walleve::CWalleveBufStream& ssKey,walleve::CWalleveBufStream& ssValue,
                        std::multimap<int,uint256>& mapJoint,std::map<uint256,uint256>& mapFork);
};

} // namespace storage
} // namespace multiverse

#endif //MULTIVERSE_FORKDB_H

