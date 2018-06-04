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
    bool GetBlockLocation(const uint256& hashBlock,uint256& hashFork,int& nHeight);
    bool GetBlockHash(const uint256& hashFork,int nHeight,uint256& hashBlock);
    bool GetLastBlock(const uint256& hashFork,uint256& hashBlock,int& nHeight,int64& nTime);
    bool GetBlock(const uint256& hashBlock,CBlock& block);
    bool Exists(const uint256& hashBlock);
    bool GetTransaction(const uint256& txid,CTransaction& tx);
    MvErr AddNewBlock(CBlock& block,std::vector<CTransaction>& vtx,CWorldLineUpdate& update);
    bool GetProofOfWorkTarget(const uint256& hashPrev,int nAlgo,int& nBits,int64& nReward);
protected:
    bool WalleveHandleInitialize();
    void WalleveHandleDeinitialize();
    bool WalleveHandleInvoke();
    void WalleveHandleHalt();
    bool CheckContainer();
    bool RebuildContainer();
    bool InsertGenesisBlock(CBlock& block);
    MvErr GetBlockTx(CBlockTx& tx,storage::CBlockView& view);
protected:
    boost::shared_mutex rwAccess;
    ICoreProtocol *pCoreProtocol;
    storage::CBlockBase cntrBlock;
};

} // namespace multiverse

#endif //MULTIVERSE_WORLDLINE_H

