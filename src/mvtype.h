// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_TYPE_H
#define  MULTIVERSE_TYPE_H

#include "uint256.h"
#include "block.h"
#include "transaction.h"

#include <vector>
#include <map>
#include <set>

namespace multiverse
{

// Status
class CForkStatus
{
public:
    CForkStatus() {}
    CForkStatus(const uint256& hashOriginIn,const uint256& hashParentIn,int nForkHeightIn) 
    : hashOrigin(hashOrigin),hashParent(hashParentIn),nForkHeight(nForkHeightIn) {}
public:
    uint256 hashOrigin;
    uint256 hashParent;
    int nForkHeight;
    
    uint256 hashLastBlock;
    int64 nLastBlockTime;
    int nLastBlockHeight;
    int64 nMoneySupply;
    std::multimap<int,uint256> mapSubline;
};

// Notify
class CWorldLineUpdate
{
public:
    CWorldLineUpdate() { SetNull(); }
    CWorldLineUpdate(CBlockIndex* pIndex)
    {
        hashFork = pIndex->GetOriginHash();
        hashParent = pIndex->GetParentHash();
        nForkHeight = pIndex->pOrigin->GetBlockHeight() - 1;
        hashLastBlock = pIndex->GetBlockHash();
        nLastBlockTime = pIndex->GetBlockTime();
        nLastBlockHeight = pIndex->GetBlockHeight();
        nMoneySupply = pIndex->GetMoneySupply();
    }
    void SetNull() { hashFork = 0; }
    bool IsNull() const { return (hashFork == 0); }
public:
    uint256 hashFork;
    uint256 hashParent;
    int nForkHeight;
    uint256 hashLastBlock;
    int64 nLastBlockTime;
    int nLastBlockHeight;
    int64 nMoneySupply;
    std::vector<uint256> vTxAddNew;
    std::vector<uint256> vTxRemove;
    std::set<uint256> setTxUpdate;
};

/* Proof */
/*
class CMPSSProof
{
public:
    uint256 
}
*/

/* Protocol & Event */
class CNil
{
    friend class walleve::CWalleveStream;
protected:
    template <typename O>
    void WalleveSerialize(walleve::CWalleveStream& s,O& opt) {}
};

} // namespace multiverse

#endif //MULTIVERSE_TYPE_H
