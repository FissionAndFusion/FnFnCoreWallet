// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_CORE_H
#define  MULTIVERSE_CORE_H

#include "mvbase.h"

namespace multiverse
{

class CMvCoreProtocol : public ICoreProtocol
{
public:
    CMvCoreProtocol();
    virtual ~CMvCoreProtocol();
    virtual const uint256& GetGenesisBlockHash();
    virtual void GetGenesisBlock(CBlock& block); 
    virtual MvErr ValidateTransaction(const CTransaction& tx);
    virtual MvErr ValidateBlock(CBlock& block);
    virtual MvErr VerifyBlock(CBlock& block,CBlockIndex* pIndexPrev);
    virtual MvErr VerifyBlockTx(CTransaction& tx,CTxContxt& txContxt,CBlockIndex* pIndexPrev);
    virtual MvErr VerifyTransaction(CTransaction& tx,const std::vector<CTxOutput>& vPrevOutput,int nForkHeight);
    virtual bool GetProofOfWorkTarget(CBlockIndex* pIndexPrev,int nAlgo,int& nBits,int64& nReward);
    virtual int GetProofOfWorkRunTimeBits(int nBits,int64 nTime,int64 nPrevTime);
    //virtual MvErr 
protected:
    bool WalleveHandleInitialize();
    const MvErr Debug(const MvErr& err,const char* pszFunc,const char *pszFormat,...); 
    bool CheckBlockSignature(const CBlock& block);
   int64 GetProofOfWorkReward(CBlockIndex* pIndexPrev);
protected:
    uint256 hashGenesisBlock;
    int nProofOfWorkLimit;
    int nProofOfWorkInit;
    int64 nProofOfWorkUpperTarget;
    int64 nProofOfWorkLowerTarget;
};

class CMvTestNetCoreProtocol : public CMvCoreProtocol
{
public:
    CMvTestNetCoreProtocol();
    void GetGenesisBlock(CBlock& block);
};

} // namespace multiverse

#endif //MULTIVERSE_BASE_H

