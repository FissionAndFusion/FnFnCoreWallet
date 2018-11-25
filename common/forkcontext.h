// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_FORKCONTEXT_H
#define  MULTIVERSE_FORKCONTEXT_H

#include "uint256.h"
#include "destination.h"
#include "profile.h"
#include <string>

class CForkContext
{
public:
    std::string strName;
    std::string strSymbol;
    uint256 hashFork;
    uint256 hashParent;
    uint256 hashJoint;
    uint256 txidEmbedded;
    uint16  nVersion;
    uint16  nFlag;
    int64   nMintReward;
    int64   nMinTxFee;
    CDestination destOwner;
public:
    enum 
    {
        FORK_FLAG_ISOLATED = (1<<0),
        FORK_FLAG_PRIVATE  = (1<<1),
        FORK_FLAG_ENCLOSED = (1<<2),
    };
    CForkContext() { SetNull(); }
    CForkContext(const uint256& hashForkIn,const uint256& hashJointIn,const uint256& txidEmbeddedIn,
                 const CProfile& profile)
    {
        hashFork     = hashForkIn;
        hashJoint    = hashJointIn;
        txidEmbedded = txidEmbeddedIn;
        strName      = profile.strName;
        strSymbol    = profile.strSymbol;
        hashParent   = profile.hashParent;
        nVersion     = profile.nVersion;
        nFlag        = profile.nFlag;
        nMintReward  = profile.nMintReward;
        nMinTxFee    = profile.nMinTxFee;
        destOwner    = profile.destOwner; 
    }
    virtual void SetNull()
    {
        hashFork     = 0;
        hashParent   = 0;
        hashJoint    = 0;
        txidEmbedded = 0;
        nVersion     = 1;
        nFlag        = 0;
        nMintReward  = 0;
        nMinTxFee    = 0;
        strName.clear();
        strSymbol.clear();
        destOwner.SetNull();
    }
    bool IsNull() const { return strName.empty(); }
    bool IsIsolated() const { return (nFlag & FORK_FLAG_ISOLATED); }
    bool IsPrivate() const { return (nFlag & FORK_FLAG_PRIVATE); }
    bool IsEnclosed() const { return (nFlag & FORK_FLAG_ENCLOSED); }
    void SetFlag(uint16 flag) { nFlag |= flag; }
    void ResetFlag(uint16 flag) { nFlag &= ~flag; }
    void SetFlag(bool fIsolated,bool fPrivate,bool fEnclosed)
    {
        nFlag = 0;
        nFlag |= (fIsolated ? FORK_FLAG_ISOLATED : 0);
        nFlag |= (fPrivate  ? FORK_FLAG_PRIVATE  : 0);
        nFlag |= (fEnclosed ? FORK_FLAG_ENCLOSED : 0);
    }
    const CProfile GetProfile() const
    {
        CProfile profile;
        profile.strName      = strName;
        profile.strSymbol    = strSymbol;
        profile.hashParent   = hashParent;
        profile.nVersion     = nVersion;
        profile.nFlag        = nFlag;
        profile.nMintReward  = nMintReward;
        profile.nMinTxFee    = nMinTxFee;
        profile.destOwner    = destOwner; 
        return profile;
    }
};

/*
    if hashParent = 0 and strSymnol is empty, select all fork,
    if hashParent != 0, select subline of given parent,
    if strSymnol is not empty, select fork whose symbol = strSymbol,
    if hashParent != 0 and strSymnol is not empty, select subline whose symbol = strSymbol,
*/
class CForkContextFilter
{
public:
    CForkContextFilter(const uint256& hashParentIn = uint64(0),const std::string& strSymbolIn = "")
    : hashParent(hashParentIn),strSymbol(strSymbolIn)
    {
    }
    virtual bool FoundForkContext(const CForkContext& ctxt);
public:
    const uint256& hashParent;
    const std::string& strSymbol;
};

#endif //MULTIVERSE_FORKCONTEXT_H

