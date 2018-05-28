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
    virtual MvErr ValidateBlock(CBlock& block,std::vector<CTransaction>& vtx);
    virtual MvErr VerifyBlock(CBlock& block,const CDestination& destIn,int64 nValueIn,
                              int64 nTxFee,CBlockIndex* pIndexPrev);
    virtual MvErr VerifyBlockTx(CBlockTx& tx,storage::CBlockView& view);
    //virtual MvErr 
protected:
    bool WalleveHandleInitialize();
    const MvErr Debug(const MvErr& err,const char* pszFunc,const char *pszFormat,...); 
    bool CheckBlockSignature(const CBlock& block);
protected:
    uint256 hashGenesisBlock;
};

class CMvTestNetCoreProtocol : public CMvCoreProtocol
{
public:
    CMvTestNetCoreProtocol();
    void GetGenesisBlock(CBlock& block);
};

} // namespace multiverse

#endif //MULTIVERSE_BASE_H

