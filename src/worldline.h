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
    void GetForkStatus(std::map<uint256,CForkStatus>& mapForkStatus) override; 
    bool GetForkProfile(const uint256& hashFork,CProfile& profile) override;
    bool GetForkContext(const uint256& hashFork,CForkContext& ctxt) override;
    bool GetForkAncestry(const uint256& hashFork,std::vector<std::pair<uint256,uint256> > vAncestry) override;
    int  GetBlockCount(const uint256& hashFork) override;
    bool GetBlockLocation(const uint256& hashBlock,uint256& hashFork,int& nHeight) override;
    bool GetBlockHash(const uint256& hashFork,int nHeight,uint256& hashBlock) override;
    bool GetBlockHash(const uint256& hashFork,int nHeight,std::vector<uint256>& vBlockHash) override;
    bool GetLastBlock(const uint256& hashFork,uint256& hashBlock,int& nHeight,int64& nTime) override;
    bool GetLastBlockTime(const uint256& hashFork,int nDepth,std::vector<int64>& vTime) override;
    bool GetBlock(const uint256& hashBlock,CBlock& block) override;
    bool GetBlockEx(const uint256& hashBlock,CBlockEx& block) override;
    bool GetOrigin(const uint256& hashFork,CBlock& block) override;
    bool Exists(const uint256& hashBlock) override;
    bool GetTransaction(const uint256& txid,CTransaction& tx) override;
    bool ExistsTx(const uint256& txid) override;
    bool GetTxLocation(const uint256& txid,uint256& hashFork,int& nHeight) override;
    bool GetTxUnspent(const uint256& hashFork,const std::vector<CTxIn>& vInput,
                                                    std::vector<CTxOutput>& vOutput) override;
    bool FilterTx(CTxFilter& filter) override;
    MvErr AddNewForkContext(const CTransaction& txFork,CForkContext& ctxt) override;
    MvErr AddNewBlock(const CBlock& block,CWorldLineUpdate& update) override;
    MvErr AddNewOrigin(const CBlock& block,CWorldLineUpdate& update) override;
    bool GetProofOfWorkTarget(const uint256& hashPrev,int nAlgo,int& nBits,int64& nReward) override;
    bool GetDelegatedProofOfStakeReward(const uint256& hashPrev,std::size_t nWeight,int64& nReward) override;
    bool GetBlockLocator(const uint256& hashFork,CBlockLocator& locator) override;
    bool GetBlockInv(const uint256& hashFork,const CBlockLocator& locator,std::vector<uint256>& vBlockHash,std::size_t nMaxCount) override;
    bool GetBlockDelegateEnrolled(const uint256& hashBlock,std::map<CDestination,std::size_t>& mapWeight,
                                                           std::map<CDestination,std::vector<unsigned char> >& mapEnrollData) override;
protected:
    bool WalleveHandleInitialize() override;
    void WalleveHandleDeinitialize() override;
    bool WalleveHandleInvoke() override;
    void WalleveHandleHalt() override;
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

