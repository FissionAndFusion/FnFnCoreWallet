// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_TXPOOL_H
#define  MULTIVERSE_TXPOOL_H

#include "mvbase.h"

namespace multiverse
{

class CTxCache
{
public:
    std::size_t Size() const { return mapTx.size(); }
    bool Exists(const uint256& txid) const 
    { 
        return (!!mapTx.count(txid)); 
    }
    CTransaction* Get(uint256 txid) const
    {
        std::map<uint256,CTransaction*>::const_iterator mi = mapTx.find(txid);
        return (mi != mapTx.end() ? (*mi).second : NULL);
    }
    bool IsSpent(const CTxOutPoint& out) const
    {
        return (!!mapNextTx.count(out));
    }
    void AddNew(const uint256& txid,CTransaction* pTx)
    {
        mapTx[txid] = pTx;
        BOOST_FOREACH(const CTxIn& txin,pTx->vInput)
        {
            mapNextTx[txin.prevout] = txid;
        }
    }
    void Remove(const uint256& txid)
    {
        std::map<uint256,CTransaction*>::iterator mi = mapTx.find(txid);
        if (mi != mapTx.end())
        {
            CTransaction* pTx = (*mi).second;
            BOOST_FOREACH(const CTxIn& txin,pTx->vInput)
            {
                mapNextTx.erase(txin.prevout);
            }
            mapTx.erase(mi);
        }
    }
    void Clear()
    {
        mapTx.clear();
        mapNextTx.clear();
    }
protected:
    std::map<uint256,CTransaction*> mapTx;
    std::map<CTxOutPoint,uint256> mapNextTx;
};

class CTxPool : public ITxPool
{
public:
    CTxPool();
    ~CTxPool();
    bool Exists(const uint256& txid);
    void Clear();
    std::size_t Count(const uint256& fork) const;
    bool Push(const uint256& hashFork,const uint256& txid,CTransaction& tx);
    bool Pop(const uint256& hashFork,const uint256& txid);

    MvErr AddNew(const CTransaction& tx);
    void Remove(const uint256& txid);
    bool Get(const uint256& txid,CTransaction& tx) const;
    bool Arrange(uint256& fork,std::vector<std::pair<uint256,CTransaction> >& vtx,std::size_t nMaxSize);
    bool FetchInputs(uint256& fork,const CTransaction& tx,std::vector<CTxOutput>& vUnspent);
    void ForkUpdate(uint256& fork,const std::vector<CBlockTx>& vAddNew,const std::vector<uint256>& vRemove);
protected:
    bool WalleveHandleInitialize();
    void WalleveHandleDeinitialize();
    bool WalleveHandleInvoke();
    void WalleveHandleHalt();
protected:
    mutable boost::shared_mutex rwAccess;
    ICoreProtocol* pCoreProtocol;
    IWorldLine* pWorldLine;
    std::map<uint256,CTxCache> mapTxCache;
    std::map<uint256,CTransaction> mapTx;
};

} // namespace multiverse

#endif //MULTIVERSE_TXPOOL_H

