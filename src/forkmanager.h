// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_FORKMANAGER_H
#define MULTIVERSE_FORKMANAGER_H

#include "mvbase.h"
#include "mvpeernet.h"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>

#include <stack>

namespace multiverse
{

class CForkIndex
{
public:
    CForkIndex() : nEmbeddedHeight(-1) {}
    CForkIndex(const CForkContext& ctxt) 
    : hashFork(ctxt.hashFork),hashParent(ctxt.hashParent),hashJoint(ctxt.hashJoint),nEmbeddedHeight(-1)
    {
    }
public:
    uint256 hashFork;
    uint256 hashParent;
    uint256 hashJoint;
    int nEmbeddedHeight;
};

typedef boost::multi_index_container<
  CForkIndex,
  boost::multi_index::indexed_by<
    boost::multi_index::ordered_unique<boost::multi_index::member<CForkIndex,uint256,&CForkIndex::hashFork> >,
    boost::multi_index::ordered_non_unique<boost::multi_index::member<CForkIndex,int,&CForkIndex::nEmbeddedHeight> >
  >
> CForkIndexSet;
typedef CForkIndexSet::nth_index<0>::type CForkIndexSetByFork;
typedef CForkIndexSet::nth_index<1>::type CForkIndexSetByHeight;


class CForkSchedule
{
public:
    CForkSchedule(bool fAllowedIn = false)
    : fAllowed(fAllowedIn), fSynchronized(false)
    {
    }
    bool IsHalted() const { return (!fAllowed && mapJoint.empty()); }
    void AddNewJoint(const uint256& hashJoint,const uint256& hashFork) 
    { 
        mapJoint.insert(std::make_pair(hashJoint,hashFork));
    }
    void RemoveJoint(const uint256& hashJoint,std::vector<uint256>& vFork)
    {
        std::multimap<uint256,uint256>::iterator it = mapJoint.lower_bound(hashJoint);
        while (it != mapJoint.upper_bound(hashJoint))
        {
            vFork.push_back((*it).second);
            mapJoint.erase(it++);
        }
    }
    void SetSyncStatus(bool fSync) { fSynchronized = fSync; }
    bool IsSynchronized() const { return fSynchronized; }
public:
    bool fAllowed;
    bool fSynchronized;
    std::multimap<uint256,uint256> mapJoint;
};

class CForkManager : public IForkManager
{
public:
    CForkManager();
    ~CForkManager();
    bool LoadForkContext(std::vector<uint256>& vActive) override;
    bool AddNewForkContext(const CForkContext& ctxt,std::vector<uint256>& vActive) override;
    void ForkUpdate(const uint256& hashFork,const uint256& hashLastBlock,
                    std::vector<uint256>& vActive,std::vector<uint256>& vDeactive) override;
    bool LoadForkContext(const CForkContext& ctxt);

protected:
    bool WalleveHandleInitialize() override;
    void WalleveHandleDeinitialize() override;
    bool WalleveHandleInvoke() override;
    void WalleveHandleHalt() override;
    bool IsAllowedFork(const uint256& hashFork,const uint256& hashParent) const;
    bool LoadForkIndex();
    bool AddNewForkContext(const CForkContext& ctxt);
    void RemoveForkContext(const uint256& hashFork);
protected:
    boost::shared_mutex rwAccess;
    ICoreProtocol* pCoreProtocol;
    IWorldLine* pWorldLine;
    bool fAllowAnyFork;
    std::set<uint256> setForkAllowed;
    std::set<uint256> setGroupAllowed;
    std::map<uint256,CForkSchedule> mapForkSched; 
    CForkIndexSet setForkIndex;
};

} // namespace multiverse

#endif //MULTIVERSE_FORKMANAGER_H
