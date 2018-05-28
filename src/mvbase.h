// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_BASE_H
#define  MULTIVERSE_BASE_H

#include "param.h"
#include "error.h"
#include "uint256.h"
#include "transaction.h"
#include "block.h"
#include "config.h"
#include "blockbase.h"

#include <walleve/walleve.h>
#include <boost/foreach.hpp>

namespace multiverse
{

class ICoreProtocol : public walleve::IWalleveBase
{
public:
    ICoreProtocol() : IWalleveBase("coreprotocol") {}
    virtual const uint256& GetGenesisBlockHash() = 0;
    virtual void GetGenesisBlock(CBlock& block) = 0;
    virtual MvErr ValidateBlock(CBlock& block,std::vector<CTransaction>& vTx) = 0;
    virtual MvErr VerifyBlock(CBlock& block,const CDestination& destIn,int64 nValueIn,
                              int64 nTxFee,CBlockIndex* pIndexPrev) = 0;
    virtual MvErr VerifyBlockTx(CBlockTx& tx,storage::CBlockView& view) = 0;
};

class IWorldLine : public walleve::IWalleveBase
{
public:
    IWorldLine() : IWalleveBase("worldline") {}
    virtual MvErr AddNewBlock(CBlock& block,std::vector<CTransaction>& vtx) = 0;    
    const CMvBasicConfig * WalleveConfig()
    {
        return dynamic_cast<const CMvBasicConfig *>(walleve::IWalleveBase::WalleveConfig());
    }
    const CMvStorageConfig * StorageConfig()
    {
        return dynamic_cast<const CMvStorageConfig *>(walleve::IWalleveBase::WalleveConfig());
    }
};

class ITxPool : public walleve::IWalleveBase
{
public:
    ITxPool() : IWalleveBase("txpool") {}
    virtual bool Exists(const uint256& txid) = 0;
    virtual void Clear() = 0;
    virtual std::size_t Count(const uint256& fork) const = 0;
    virtual MvErr AddNew(const CTransaction& tx) = 0;
    virtual void Remove(const uint256& txid) = 0;
    virtual bool Get(const uint256& txid,CTransaction& tx) const = 0;
    virtual bool Arrange(uint256& fork,std::vector<std::pair<uint256,CTransaction> >& vtx,std::size_t nMaxSize) = 0;
    virtual bool FetchInputs(uint256& fork,const CTransaction& tx,std::vector<CTxOutput>& vUnspent) = 0;
    virtual void ForkUpdate(uint256& fork,const std::vector<CBlockTx>& vAddNew,const std::vector<uint256>& vRemove) = 0;
};

class IBlockMaker : public walleve::IWalleveBase
{
public:
    IBlockMaker(const std::string& walleveOwnKeyIn) : IWalleveBase(walleveOwnKeyIn) {}
};

class IWallet : public walleve::IWalleveBase
{
public:
    IWallet() : IWalleveBase("wallet") {}
    const CMvBasicConfig * WalleveConfig()
    {
        return dynamic_cast<const CMvBasicConfig *>(walleve::IWalleveBase::WalleveConfig());
    }
};

class IDispatcher : public walleve::IWalleveBase
{
public:
    IDispatcher() : IWalleveBase("dispatcher") {}
    virtual MvErr AddNewBlock(const CBlock& block) = 0;
    virtual MvErr AddNewTx(const CTransaction& tx) = 0;
};

class IService : public walleve::IWalleveBase
{
public:
    IService() : IWalleveBase("service") {}
    /* System */
    virtual void Shutdown() = 0;
    /* Network */ 
    virtual int GetPeerCount() = 0;
    virtual bool AddNode(const walleve::CNetHost& node) = 0;
    virtual void RemoveNode(const walleve::CNetHost& node) = 0;
    /* Worldline & Tx Pool*/
    virtual int  GetForkCount() = 0;
    virtual bool GetForkGenealogy(const uint256& hashFork,std::vector<uint256>& vAncestry,std::vector<uint256>& vSubline) = 0;
    virtual bool GetForkFromBlock(const uint256 hashBlock,uint256& hashFork) = 0;
    virtual int  GetBlockCount(const uint256& hashFork) = 0;
    virtual bool GetBlockHash(const uint256& hashFork,int nHeight,uint256& hashBlock) = 0;
    virtual bool GetBlock(const uint256& hash,CBlock& block,CBlockIndex** ppBlockIndex) = 0;
    virtual void GetTxPool(const uint256& hashFork,std::vector<uint256>& vTxPool) = 0;
    virtual bool GetTransaction(const uint256& txid,CTransaction& tx,std::vector<uint256>& vInBlock) = 0;
    virtual MvErr SendTransaction(CTransaction& tx) = 0;
    /* Wallet */
    virtual bool ImportKey(const crypto::CKey& key) = 0;
    virtual bool ExportKey(const crypto::CPubKey& pubkey,int& nVersionRet,crypto::CCryptoCipher& cipherRet) = 0;
    virtual bool SignSignature(const crypto::CPubKey& pubkey,const uint256& hash,std::vector<unsigned char>& vchSig) = 0;
};

} // namespace multiverse

#endif //MULTIVERSE_BASE_H

