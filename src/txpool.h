// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_TXPOOL_H
#define  MULTIVERSE_TXPOOL_H

#include "mvbase.h"

namespace multiverse
{

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
    CAssembledTx* Get(uint256 txid) const
    {
        std::map<uint256,CAssembledTx*>::const_iterator mi = mapTx.find(txid);
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
        CAssembledTx* pTx = Get(out.hash);
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
    void AddNew(const uint256& txid,CAssembledTx& tx)
    {
        mapTx[txid] = &tx;
        for (int i = 0;i < tx.vInput.size();i++)
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
        CAssembledTx *pTx = Get(txid);
        if (pTx != NULL)
        {
            for (int i = 0;i < pTx->vInput.size();i++)
            {
                mapSpent.erase(pTx->vInput[i].prevout);
            }
            mapTx.erase(txid);
        } 
    }
    void InvalidateSpent(const CTxOutPoint& out,std::vector<uint256>& vInvolvedTx)
    {
        std::set<uint256> setInvolvedTx;
        std::vector<CTxOutPoint> vOutPoint;
        vOutPoint.push_back(out);
        for (int i = 0;i < vOutPoint.size();i++)
        {
            uint256 txidNextTx;
            if (GetSpent(vOutPoint[i],txidNextTx))
            {
                CAssembledTx* pNextTx = NULL;
                if ((pNextTx = Get(txidNextTx)) != NULL)
                {
                    BOOST_FOREACH(const CTxIn& txin,pNextTx->vInput)
                    {
                        SetUnspent(txin.prevout);
                    }
                    CTxOutPoint out0(txidNextTx,0);
                    if (IsSpent(out0))
                    {
                        vOutPoint.push_back(out0);
                    }
                    else
                    {
                        mapSpent.erase(out0);
                    }
                    CTxOutPoint out1(txidNextTx,1);
                    if (IsSpent(out1))
                    {
                        vOutPoint.push_back(out1);
                    }
                    else
                    {
                        mapSpent.erase(out1);
                    }
                    mapTx.erase(txidNextTx);
                    vInvolvedTx.push_back(txidNextTx);
                }
            }
        }
    }
    void Clear() 
    {
        mapTx.clear();
        mapSpent.clear();
    }
public:
    std::map<uint256,CAssembledTx*> mapTx;
    std::map<CTxOutPoint,CSpent> mapSpent;
};

class CTxPool : public ITxPool
{
public:
    CTxPool();
    ~CTxPool();
    bool Exists(const uint256& txid);
    void Clear();
    std::size_t Count(const uint256& fork) const;

    MvErr Push(CTransaction& tx);
    void Pop(const uint256& txid);
    bool Get(const uint256& txid,CTransaction& tx) const;
    bool Arrange(uint256& fork,std::vector<std::pair<uint256,CTransaction> >& vtx,std::size_t nMaxSize);
    bool FetchInputs(const uint256& hashFork,const CTransaction& tx,std::vector<CTxOutput>& vUnspent);
    void SynchronizeWorldLine(CWorldLineUpdate& update,CTxPoolUpdate& change);
/*
    void ForkUpdate(uint256& fork,int nForkHeight,const std::vector<CBlockTx>& vAddNew,const std::vector<uint256>& vRemove,
                    std::vector<uint256>& vInvalidTx);
*/
protected:
    bool WalleveHandleInitialize();
    void WalleveHandleDeinitialize();
    bool WalleveHandleInvoke();
    void WalleveHandleHalt();
    MvErr AddNew(CTxPoolView& txView,const uint256& txid,CTransaction& tx,const uint256& hashFork,int nForkHeight);
protected:
    mutable boost::shared_mutex rwAccess;
    ICoreProtocol* pCoreProtocol;
    IWorldLine* pWorldLine;
    std::map<uint256,CTxPoolView> mapPoolView;
    std::map<uint256,CAssembledTx> mapTx;
};

} // namespace multiverse

#endif //MULTIVERSE_TXPOOL_H

