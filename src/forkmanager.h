// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_FORKMANAGER_H
#define MULTIVERSE_FORKMANAGER_H

#include "mvbase.h"
#include "mvpeernet.h"

#include <stack>

namespace multiverse
{

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
    bool AddNewForkContext(const CForkContext& ctxt,std::vector<uint256>& vActive) override;
    void ForkUpdate(const uint256& hashFork,const uint256& hashLastBlock,
                    std::vector<uint256>& vActive,std::vector<uint256>& vDeactive) override;
protected:
    bool WalleveHandleInitialize() override;
    void WalleveHandleDeinitialize() override;
    bool WalleveHandleInvoke() override;
    void WalleveHandleHalt() override;
    bool IsAllowedFork(const uint256& hashFork,const uint256& hashParent) const;
protected:
    boost::shared_mutex rwAccess;
    ICoreProtocol* pCoreProtocol;
    IWorldLine* pWorldLine;
    bool fAllowAnyFork;
    std::set<uint256> setForkAllowed;
    std::set<uint256> setGroupAllowed;
    std::map<uint256,CForkSchedule> mapForkSched;
};

} // namespace multiverse

#endif //MULTIVERSE_FORKMANAGER_H
