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
    bool GetBlockLocation(const uint256& hashBlock,uint256& hashFork,int& nHeight) override;
    bool GetBlockHash(const uint256& hashFork,int nHeight,uint256& hashBlock) override;
    bool GetLastBlock(const uint256& hashFork,uint256& hashBlock,int& nHeight,int64& nTime) override;
    bool GetBlock(const uint256& hashBlock,CBlock& block) override;
    bool Exists(const uint256& hashBlock) override;
    bool GetTransaction(const uint256& txid,CTransaction& tx) override;
    bool ExistsTx(const uint256& txid) override;
    bool GetTxLocation(const uint256& txid,uint256& hashFork,int& nHeight) override;
    bool GetTxUnspent(const uint256& hashFork,const std::vector<CTxIn>& vInput,
                                                    std::vector<CTxOutput>& vOutput) override;
    bool FilterTx(CTxFilter& filter) override;
    MvErr AddNewBlock(CBlock& block,CWorldLineUpdate& update) override;
    bool GetProofOfWorkTarget(const uint256& hashPrev,int nAlgo,int& nBits,int64& nReward) override;
    bool GetBlockLocator(const uint256& hashFork,CBlockLocator& locator) override;
    bool GetBlockInv(const uint256& hashFork,const CBlockLocator& locator,std::vector<uint256>& vBlockHash,std::size_t nMaxCount) override;
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

