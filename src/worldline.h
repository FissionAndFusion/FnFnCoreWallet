// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_WORLDLINE_H
#define  MULTIVERSE_WORLDLINE_H

#include "mvbase.h"
#include "blockbase.h"
#include <map>

namespace multiverse
{

class CWorldLine : public IWorldLine
{
public:
    CWorldLine();
    ~CWorldLine();
    void GetForkStatus(std::map<uint256,CForkStatus>& mapForkStatus); 
    bool GetForkProfile(const uint256& hashFork,CProfile& profile);
    bool GetBlockLocation(const uint256& hashBlock,uint256& hashFork,int& nHeight);
    bool GetBlockHash(const uint256& hashFork,int nHeight,uint256& hashBlock);
    bool GetLastBlock(const uint256& hashFork,uint256& hashBlock,int& nHeight,int64& nTime);
    bool GetLastBlockTime(const uint256& hashFork,int nDepth,std::vector<int64>& vTime);
    bool GetBlock(const uint256& hashBlock,CBlock& block);
    bool Exists(const uint256& hashBlock);
    bool GetTransaction(const uint256& txid,CTransaction& tx);
    bool ExistsTx(const uint256& txid);
    bool GetTxLocation(const uint256& txid,uint256& hashFork,int& nHeight);
    bool GetTxUnspent(const uint256& hashFork,const std::vector<CTxIn>& vInput,
                                                    std::vector<CTxOutput>& vOutput);
    bool FilterTx(CTxFilter& filter);
    MvErr AddNewBlock(const CBlock& block,CWorldLineUpdate& update);
    MvErr AddNewOrigin(const CBlock& block,CWorldLineUpdate& update);
    bool GetProofOfWorkTarget(const uint256& hashPrev,int nAlgo,int& nBits,int64& nReward);
    bool GetDelegatedProofOfStakeReward(const uint256& hashPrev,std::size_t nWeight,int64& nReward);
    bool GetBlockLocator(const uint256& hashFork,CBlockLocator& locator);
    bool GetBlockInv(const uint256& hashFork,const CBlockLocator& locator,std::vector<uint256>& vBlockHash,std::size_t nMaxCount);
    bool GetBlockDelegateEnrolled(const uint256& hashBlock,std::map<CDestination,std::size_t>& mapWeight,
                                                           std::map<CDestination,std::vector<unsigned char> >& mapEnrollData);
protected:
    bool WalleveHandleInitialize();
    void WalleveHandleDeinitialize();
    bool WalleveHandleInvoke();
    void WalleveHandleHalt();
    bool CheckContainer();
    bool RebuildContainer();
    bool InsertGenesisBlock(CBlock& block);
    MvErr GetTxContxt(storage::CBlockView& view,const CTransaction& tx,CTxContxt& txContxt);
    bool GetBlockChanges(const CBlockIndex* pIndexNew,const CBlockIndex* pIndexFork,
                         std::vector<CBlockEx>& vBlockAddNew,std::vector<CBlockEx>& vBlockRemove);
protected:
    boost::shared_mutex rwAccess;
    ICoreProtocol *pCoreProtocol;
    storage::CBlockBase cntrBlock;
};

} // namespace multiverse

#endif //MULTIVERSE_WORLDLINE_H

