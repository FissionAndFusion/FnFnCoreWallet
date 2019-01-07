// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_PROFILE_H
#define  MULTIVERSE_PROFILE_H

#include "destination.h"
#include <string>
#include <vector>

enum 
{
    PROFILE_VERSION     = 0,
    PROFILE_NAME        = 1,
    PROFILE_SYMBOL      = 2,
    PROFILE_FLAG        = 3,
    PROFILE_MINTXFEE    = 4,
    PROFILE_MINTREWARD  = 5,
    PROFILE_OWNER       = 6,
    PROFILE_PARENT      = 7,
    PROFILE_JOINTHEIGHT = 8,
    PROFILE_MAX,
};

class CProfile
{
public:
    int nVersion;
    std::string strName;
    std::string strSymbol;
    uint8 nFlag;
    int64 nMinTxFee;
    int64 nMintReward;
    CDestination destOwner;
    uint256 hashParent;
    int nJointHeight;
public:
    enum {PROFILE_FLAG_ISOLATED=1,PROFILE_FLAG_PRIVATE=2,PROFILE_FLAG_ENCLOSED=4};
    CProfile() { SetNull(); }
    virtual void SetNull()
    {
        nVersion     = 1;
        nFlag        = 0; 
        nMinTxFee    = 0;
        nMintReward  = 0;
        hashParent   = 0;
        nJointHeight = -1;
        destOwner.SetNull(); 
        strName.clear();
        strSymbol.clear();
    }
    bool IsNull() const { return strName.empty(); }
    bool IsIsolated() const { return (nFlag & PROFILE_FLAG_ISOLATED); }
    bool IsPrivate() const { return (nFlag & PROFILE_FLAG_PRIVATE); }
    bool IsEnclosed() const { return (nFlag & PROFILE_FLAG_ENCLOSED); }
    void SetFlag(bool fIsolated,bool fPrivate,bool fEnclosed)
    {
        nFlag = 0;
        nFlag |= (fIsolated ? PROFILE_FLAG_ISOLATED : 0);
        nFlag |= (fPrivate  ? PROFILE_FLAG_PRIVATE  : 0);
        nFlag |= (fEnclosed ? PROFILE_FLAG_ENCLOSED : 0);
    }
    bool Save(std::vector<unsigned char>& vchProfile); 
    bool Load(const std::vector<unsigned char>& vchProfile); 
};

#endif //MULTIVERSE_PROFILE_H

