// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_CONSENSUS_H
#define  MULTIVERSE_CONSENSUS_H

#include "mvbase.h"
#include "mvdelegate.h"

namespace multiverse
{

class CDelegateTx
{
public:
    uint16 nVersion;
    uint16 nType;
    uint32 nTimeStamp;
    uint32 nLockUntil;
    int64  nAmount;
    int64  nChange;
    int32  nBlockHeight;
public:
    CDelegateTx() { SetNull(); }
    CDelegateTx(const CAssembledTx& tx)
    {
        nVersion     = tx.nVersion;
        nType        = tx.nType;
        nTimeStamp   = tx.nTimeStamp;
        nLockUntil   = tx.nLockUntil;
        nAmount      = tx.nAmount;
        nChange      = tx.GetChange();
        nBlockHeight = tx.nBlockHeight;
    }
    void SetNull()
    {
        nVersion     = 0;
        nType        = 0;
        nTimeStamp   = 0;
        nLockUntil   = 0;
        nAmount      = 0;
        nChange      = 0;
        nBlockHeight = 0;
    }
    bool IsNull() const
    {
        return (nVersion == 0);
    }
    bool IsLocked(const uint32 n, const int32 nBlockHeight) const
    {
        if (n == (nLockUntil >> 31))
        {
            return (nBlockHeight < (nLockUntil & 0x7FFFFFFF));
        }
        return false;
    }
    int64 GetTxTime() const
    {
        return ((int64)nTimeStamp);
    }
};

class CDelegateContext
{
public:
    CDelegateContext();
    CDelegateContext(const crypto::CKey& keyDelegateIn,const CDestination& destOwnerIn);
    void Clear(); 
    const CDestination GetDestination() const { return destDelegate; }
    void ChangeTxSet(const CTxSetChange& change);
    void AddNewTx(const CAssembledTx& tx);
    bool BuildEnrollTx(CTransaction& tx,const int32 nBlockHeight,int64 nTime,
                       const uint256& hashAnchor,int64 nTxFee,const std::vector<unsigned char>& vchData);
protected:
    CDestination destDelegate;
    crypto::CKey keyDelegate;
    CDestination destOwner;
    CTemplatePtr templDelegate; 
    std::map<uint256,CDelegateTx> mapTx;
    std::map<CTxOutPoint,CDelegateTx*> mapUnspent;
};

class CConsensus : public IConsensus
{
public:
    CConsensus();
    ~CConsensus();
    void PrimaryUpdate(const CWorldLineUpdate& update,const CTxSetChange& change,CDelegateRoutine& routine) override;
    void AddNewTx(const CAssembledTx& tx) override;
    bool AddNewDistribute(const int32 nAnchorHeight,const CDestination& destFrom,const std::vector<unsigned char>& vchDistribute) override;
    bool AddNewPublish(const int32 nAnchorHeight,const CDestination& destFrom,const std::vector<unsigned char>& vchPublish) override;
    void GetAgreement(const int32 nTargetHeight,uint256& nAgreement,std::size_t& nWeight,std::vector<CDestination>& vBallot) override;
    void GetProof(const int32 nTargetHeight,std::vector<unsigned char>& vchProof) override;
protected:
    bool WalleveHandleInitialize() override;
    void WalleveHandleDeinitialize() override;
    bool WalleveHandleInvoke() override;
    void WalleveHandleHalt() override;

    bool LoadDelegateTx();
    bool LoadChain();
protected:
    boost::mutex mutex;
    ICoreProtocol* pCoreProtocol;
    IWorldLine* pWorldLine;
    ITxPool* pTxPool;
    delegate::CMvDelegate mvDelegate;
    std::map<CDestination,CDelegateContext> mapContext;
};

} // namespace multiverse

#endif //MULTIVERSE_CONSENSUS_H

