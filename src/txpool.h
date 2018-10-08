// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_TXPOOL_H
#define  MULTIVERSE_TXPOOL_H

#include "mvbase.h"
#include "txpooldb.h"

namespace multiverse
{

class CPooledTx : public CAssembledTx
{
public:
    std::size_t nSequenceNumber;
    std::size_t nSerializeSize;
public:
    CPooledTx() { SetNull(); }
    CPooledTx(const CAssembledTx& tx,std::size_t nSequenceNumberIn)
    : CAssembledTx(tx),nSequenceNumber(nSequenceNumberIn)
    {
        nSerializeSize = walleve::GetSerializeSize(static_cast<const CTransaction&>(tx));
    }
    CPooledTx(const CTransaction& tx,int nBlockHeightIn,std::size_t nSequenceNumberIn,const CDestination& destInIn=CDestination(),int64 nValueInIn=0)
    : CAssembledTx(tx,nBlockHeightIn,destInIn,nValueInIn),nSequenceNumber(nSequenceNumberIn)
    {
        nSerializeSize = walleve::GetSerializeSize(tx);
    }
    void SetNull()
    {
        CAssembledTx::SetNull();
        nSequenceNumber = 0;
        nSerializeSize = 0;
    }
};

class CTxPoolView
{
public:
    class CSpent : public CTxOutput
    {
    public:
        CSpent() : txidNextTx(0) {}
        CSpent(const CTxOutput& output) : CTxOutput(output),txidNextTx(0) {}
        CSpent(const uint256& txidNextTxIn) : txidNextTx(txidNextTxIn) {}
        void SetSpent(const uint256& txidNextTxIn) { *this = CSpent(txidNextTxIn); }
        void SetUnspent(const CTxOutput& output) { *this = CSpent(output); }
        bool IsSpent() const { return (txidNextTx != 0); };
    public:
        uint256 txidNextTx;
    };
public:
    std::size_t Count() const { return mapTx.size(); }
    bool Exists(const uint256& txid) const
    {
        return (!!mapTx.count(txid));
    }
    CPooledTx* Get(uint256 txid) const
    {
        std::map<uint256,CPooledTx*>::const_iterator mi = mapTx.find(txid);
        return (mi != mapTx.end() ? (*mi).second : NULL);
    }
    bool IsSpent(const CTxOutPoint& out) const
    {
        std::map<CTxOutPoint,CSpent>::const_iterator it = mapSpent.find(out);
        if (it != mapSpent.end())
        {
            return (*it).second.IsSpent();
        }
        return false;
    }
    bool GetUnspent(const CTxOutPoint& out,CTxOutput& unspent) const
    {
        std::map<CTxOutPoint,CSpent>::const_iterator it = mapSpent.find(out);
        if (it != mapSpent.end() && !(*it).second.IsSpent())
        {
            unspent = static_cast<CTxOutput>((*it).second);
            return (!unspent.IsNull());
        }
        return false;     
    }
    bool GetSpent(const CTxOutPoint& out,uint256& txidNextTxRet) const 
    { 
        std::map<CTxOutPoint,CSpent>::const_iterator it = mapSpent.find(out);
        if (it != mapSpent.end())
        {
            txidNextTxRet = (*it).second.txidNextTx;
            return (*it).second.IsSpent();
        }
        return false;
    } 
    void SetUnspent(const CTxOutPoint& out)
    {
        CPooledTx* pTx = Get(out.hash);
        if (pTx != NULL)
        {
            mapSpent[out].SetUnspent(pTx->GetOutput(out.n));
        }
        else
        {
            mapSpent.erase(out);
        }
    }
    void SetSpent(const CTxOutPoint& out,const uint256& txidNextTxIn)
    {
        mapSpent[out].SetSpent(txidNextTxIn);
    }
    void AddNew(const uint256& txid,CPooledTx& tx)
    {
        mapTx[txid] = &tx;
        for (std::size_t i = 0;i < tx.vInput.size();i++)
        {
            mapSpent[tx.vInput[i].prevout].SetSpent(txid);
        }
        CTxOutput output;
        output = tx.GetOutput(0);
        if (!output.IsNull())
        {
            mapSpent[CTxOutPoint(txid,0)].SetUnspent(output);
        }
        output = tx.GetOutput(1); 
        if (!output.IsNull())
        {
            mapSpent[CTxOutPoint(txid,1)].SetUnspent(output);
        }
    }
    void Remove(const uint256& txid)
    {
        CPooledTx *pTx = Get(txid);
        if (pTx != NULL)
        {
            for (std::size_t i = 0;i < pTx->vInput.size();i++)
            {
                SetUnspent(pTx->vInput[i].prevout);
            }
            mapTx.erase(txid);
        } 
    }
    void Clear() 
    {
        mapTx.clear();
        mapSpent.clear();
    }
    void InvalidateSpent(const CTxOutPoint& out,std::vector<uint256>& vInvolvedTx);
    void GetFilteredTx(std::map<std::size_t,std::pair<uint256,CPooledTx*> >& mapFilteredTx,
                       const CDestination& sendTo=CDestination(),const CDestination& destIn=CDestination());
    void ArrangeBlockTx(std::map<std::size_t,std::pair<uint256,CPooledTx*> >& mapArrangedTx,std::size_t nMaxSize);
public:
    std::map<uint256,CPooledTx*> mapTx;
    std::map<CTxOutPoint,CSpent> mapSpent;
};

class CTxPool : public ITxPool
{
public:
    CTxPool();
    ~CTxPool();
    bool Exists(const uint256& txid) override;
    void Clear() override;
    std::size_t Count(const uint256& fork) const override;
    MvErr Push(const CTransaction& tx,uint256& hashFork,CDestination& destIn,int64& nValueIn);
    void Pop(const uint256& txid) override;
    bool Get(const uint256& txid,CTransaction& tx) const override;
    void ListTx(const uint256& hashFork,std::vector<std::pair<uint256,std::size_t> >& vTxPool) override;
    void ListTx(const uint256& hashFork,std::vector<uint256>& vTxPool) override;
    bool FilterTx(CTxFilter& filter) override;
    void ArrangeBlockTx(const uint256& hashFork,std::size_t nMaxSize,std::vector<CTransaction>& vtx,int64& nTotalTxFee) override;
    bool FetchInputs(const uint256& hashFork,const CTransaction& tx,std::vector<CTxOutput>& vUnspent) override;
    bool SynchronizeWorldLine(CWorldLineUpdate& update,CTxSetChange& change) override;
    bool LoadTx(const uint256& txid,const uint256& hashFork,const CAssembledTx& tx);
protected:
    bool WalleveHandleInitialize() override;
    void WalleveHandleDeinitialize() override;
    bool WalleveHandleInvoke() override;
    void WalleveHandleHalt() override;
    bool LoadDB();
    MvErr AddNew(CTxPoolView& txView,const uint256& txid,const CTransaction& tx,const uint256& hashFork,int nForkHeight);
    std::size_t GetSequenceNumber()
    {
        if (mapTx.empty())
        {
            nLastSequenceNumber = 0;
        }
        return ++nLastSequenceNumber;
    }
protected:
    storage::CTxPoolDB dbTxPool;
    mutable boost::shared_mutex rwAccess;
    ICoreProtocol* pCoreProtocol;
    IWorldLine* pWorldLine;
    std::map<uint256,CTxPoolView> mapPoolView;
    std::map<uint256,CPooledTx> mapTx;
    std::size_t nLastSequenceNumber;
};

} // namespace multiverse

#endif //MULTIVERSE_TXPOOL_H

