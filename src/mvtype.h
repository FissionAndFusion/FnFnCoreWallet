// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_TYPE_H
#define  MULTIVERSE_TYPE_H

#include "uint256.h"
#include "block.h"
#include "transaction.h"
#include "mvproto.h"

#include <vector>
#include <map>
#include <set>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>

namespace multiverse
{

// Status
class CForkStatus
{
public:
    CForkStatus() {}
    CForkStatus(const uint256& hashOriginIn,const uint256& hashParentIn,int nOriginHeightIn) 
    : hashOrigin(hashOriginIn),hashParent(hashParentIn),nOriginHeight(nOriginHeightIn) {}
public:
    uint256 hashOrigin;
    uint256 hashParent;
    int nOriginHeight;
    
    uint256 hashLastBlock;
    int64 nLastBlockTime;
    int nLastBlockHeight;
    int64 nMoneySupply;
    std::multimap<int,uint256> mapSubline;
};

class CWalletBalance
{
public:
    int64 nAvailable;
    int64 nLocked;
    int64 nUnconfirmed;
public:
    void SetNull()
    {
        nAvailable = 0;
        nLocked = 0;
        nUnconfirmed = 0;
    }
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
        nOriginHeight = pIndex->pOrigin->GetBlockHeight() - 1;
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
    int nOriginHeight;
    uint256 hashLastBlock;
    int64 nLastBlockTime;
    int nLastBlockHeight;
    int64 nMoneySupply;
    std::set<uint256> setTxUpdate;
    std::vector<CBlockEx> vBlockAddNew;
    std::vector<CBlockEx> vBlockRemove;
};

class CTxSetChange
{
public:
    uint256 hashFork;
    std::map<uint256,int> mapTxUpdate;
    std::vector<CAssembledTx> vTxAddNew;
    std::vector<std::pair<uint256,std::vector<CTxIn> > > vTxRemove;
};

class CNetworkPeerUpdate
{
public:
    bool fActive;
    uint64 nPeerNonce;
    network::CAddress addrPeer;
};

class CTransactionUpdate
{
public:
    CTransactionUpdate() { SetNull(); }
    void SetNull() { hashFork = 0; }
    bool IsNull() const { return (hashFork == 0); }
public:
    uint256 hashFork;
    int64 nChange;
    CTransaction txUpdate;
};

class CDelegateRoutine
{
public:
    std::vector<std::pair<uint256,std::map<CDestination,size_t> > > vEnrolledWeight;

    std::vector<CTransaction> vEnrollTx;
    std::map<CDestination,std::vector<unsigned char> > mapDistributeData;
    std::map<CDestination,std::vector<unsigned char> > mapPublishData;
    bool fPublishCompleted;
public:
    CDelegateRoutine() : fPublishCompleted(false) {}
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

class CBlockMakerUpdate
{
public:
    uint256 hashBlock;
    int64 nBlockTime;
    int nBlockHeight;
    uint256 nAgreement;
    std::size_t nWeight;
};

class CBlockMakerAgreement
{
public:
    bool IsProofOfWork() const { return (vBallot.empty()); } 
public:
    uint256 nAgreement;
    std::size_t nWeight;
    std::vector<CDestination> vBallot;
};

/* Net Channel */
class CPeerKnownTx
{
public:
    CPeerKnownTx() {}
    CPeerKnownTx(const uint256& txidIn) : txid(txidIn),nTime(walleve::GetTime()) {}
public:
    uint256 txid;
    int64 nTime;
};

typedef boost::multi_index_container<
  CPeerKnownTx,
  boost::multi_index::indexed_by<
    boost::multi_index::ordered_unique<boost::multi_index::member<CPeerKnownTx,uint256,&CPeerKnownTx::txid> >,
    boost::multi_index::ordered_non_unique<boost::multi_index::member<CPeerKnownTx,int64,&CPeerKnownTx::nTime> >
  >
> CPeerKnownTxSet;

typedef CPeerKnownTxSet::nth_index<0>::type CPeerKnownTxSetById;
typedef CPeerKnownTxSet::nth_index<1>::type CPeerKnownTxSetByTime;

} // namespace multiverse

#endif //MULTIVERSE_TYPE_H
