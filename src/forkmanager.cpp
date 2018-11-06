// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "forkmanager.h"
#include <boost/foreach.hpp>

using namespace std;
using namespace walleve;
using namespace multiverse;

//////////////////////////////
// CForkManagerFilter

class CForkManagerFilter : public CForkContextFilter
{
public:
    CForkManagerFilter(CForkManager* pForkManagerIn,vector<uint256>& vActiveIn) 
    : pForkManager(pForkManagerIn), vActive(vActiveIn) 
    {
    }
    bool FoundForkContext(const CForkContext& ctxt)
    {
        return pForkManager->AddNewForkContext(ctxt,vActive);
    }
protected:
    CForkManager* pForkManager;
    vector<uint256>& vActive;
}; 

//////////////////////////////
// CForkManager 

CForkManager::CForkManager()
{
    pCoreProtocol = NULL;
    pWorldLine    = NULL;
    fAllowAnyFork = false;
}

CForkManager::~CForkManager()
{
}

bool CForkManager::WalleveHandleInitialize()
{
    if (!WalleveGetObject("coreprotocol",pCoreProtocol))
    {
        WalleveLog("Failed to request coreprotocol\n");
        return false;
    }

    if (!WalleveGetObject("worldline",pWorldLine))
    {
        WalleveLog("Failed to request worldline\n");
        return false;
    }

    return true;
}

void CForkManager::WalleveHandleDeinitialize()
{
    pCoreProtocol = NULL;
    pWorldLine    = NULL;
}

bool CForkManager::WalleveHandleInvoke()
{
    fAllowAnyFork = ForkConfig()->fAllowAnyFork;
    if (!fAllowAnyFork)
    {
        setForkAllowed.insert(pCoreProtocol->GetGenesisBlockHash());
        BOOST_FOREACH(const string& strFork,ForkConfig()->vFork)
        {
            uint256 hashFork(strFork);
            if (hashFork != 0)
            {
                setForkAllowed.insert(hashFork);
            }
        }
        BOOST_FOREACH(const string& strFork,ForkConfig()->vGroup)
        {
            uint256 hashFork(strFork);
            if (hashFork != 0)
            {
                setGroupAllowed.insert(hashFork);
            }
        }
    }
    
    vector<uint256> vActive;
    CForkManagerFilter filter(this,vActive);
    if (!pWorldLine->FilterForkContext(filter))
    {
        return false;
    }
    mapForkSched.insert(make_pair(pCoreProtocol->GetGenesisBlockHash(),CForkSchedule(true)));
    return true;
}

void CForkManager::WalleveHandleHalt()
{
    mapForkSched.clear();
    setForkAllowed.clear(); 
    setGroupAllowed.clear(); 
    fAllowAnyFork = false; 
}

bool CForkManager::AddNewForkContext(const CForkContext& ctxt,vector<uint256>& vActive)
{
    if (IsAllowedFork(ctxt.hashFork,ctxt.hashParent))
    {
        mapForkSched.insert(make_pair(ctxt.hashFork,CForkSchedule(true)));

        uint256 hashParent = ctxt.hashParent;
        uint256 hashJoint  = ctxt.hashJoint;
        uint256 hashFork   = ctxt.hashFork;
        for (;;)
        {
            if (pWorldLine->Exists(hashJoint))
            {
                vActive.push_back(hashFork);
                break;
            }

            CForkSchedule& schedParent = mapForkSched[hashParent];
            if (!schedParent.IsHalted())
            {
                schedParent.AddNewJoint(hashJoint,hashFork);
                break;
            }
            
            CForkContext ctxtParent;
            if (!pWorldLine->GetForkContext(hashParent,ctxtParent))
            {
                return false;
            }
            schedParent.AddNewJoint(hashJoint,hashFork);

            hashParent = ctxtParent.hashParent;
            hashJoint  = ctxtParent.hashJoint;
            hashFork   = ctxtParent.hashFork;
        }        
    }
    return true;
}

void CForkManager::ForkUpdate(const uint256& hashFork,const uint256& hashLastBlock,
                              std::vector<uint256>& vActive,std::vector<uint256>& vDeactive)
{
    CForkSchedule& sched = mapForkSched[hashFork];
    bool fHalted = sched.IsHalted();
    vector<uint256> vFork;
    sched.RemoveJoint(hashFork,vActive);
    if (sched.IsHalted() && !fHalted)
    {
        vDeactive.push_back(hashFork);
    }
}

bool CForkManager::IsAllowedFork(const uint256& hashFork,const uint256& hashParent) const
{
    if (fAllowAnyFork || setForkAllowed.count(hashFork) || setGroupAllowed.count(hashFork))
    {
        return true;
    }
    if (!setGroupAllowed.empty())
    {
        if (setGroupAllowed.count(hashParent))
        {
            return true;
        }
        uint256 hash = hashParent;
        CForkContext ctxtParent;
        while (hash != 0 && pWorldLine->GetForkContext(hash,ctxtParent))
        {
            if (setGroupAllowed.count(ctxtParent.hashParent))
            {
                return true;
            }
            hash = ctxtParent.hashParent;
        } 
    }
    return false;
}

