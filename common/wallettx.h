// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_WALLETTX_H
#define  MULTIVERSE_WALLETTX_H

#include "transaction.h"

class CWalletTx
{
public:
    uint16 nVersion;
    uint16 nType;
    uint32 nLockUntil;
    std::vector<CTxIn> vInput;
    CDestination sendTo;
    int64 nAmount;
    int64 nTxFee;
    CDestination destIn;
    int64 nValueIn;
    int nBlockHeight;
    int nFlags;
    uint256 txid;
    uint256 hashFork;
    // memory only
    mutable int nRefCount;
public:
    enum { WTX_ISMINE  = (1 << 0), WTX_FROMME = (1 << 1)};
    CWalletTx() { SetNull(); }
    CWalletTx(const uint256& txidIn,const CAssembledTx &tx,const uint256& hashForkIn,bool fIsMine,bool fFromMe)
    {
        nVersion     = tx.nVersion;
        nType        = tx.nType;
        nLockUntil   = tx.nLockUntil;
        vInput       = tx.vInput;
        sendTo       = tx.sendTo;
        nAmount      = tx.nAmount;
        nTxFee       = tx.nTxFee;
        destIn       = tx.destIn;
        nValueIn     = tx.nValueIn;
        nBlockHeight = tx.nBlockHeight;
        nFlags       = (fIsMine ? WTX_ISMINE : 0) | (fFromMe ? WTX_FROMME : 0);
        txid         = txidIn;
        hashFork     = hashForkIn;
        nRefCount = 0;
    }
    void SetNull()
    {
        nVersion     = 0;
        nType        = 0;
        nLockUntil   = 0;
        sendTo.SetNull();
        nAmount      = 0;
        nTxFee       = 0;
        destIn.SetNull();
        nValueIn     = 0;
        nBlockHeight = 0;
        nFlags       = 0;
        txid         = 0;
        hashFork     = 0;
    }
    bool IsNull() const { return (txid == 0); }
    bool IsMintTx() const 
    { 
        return (nType == CTransaction::TX_GENESIS || nType == CTransaction::TX_STAKE 
                || nType == CTransaction::TX_WORK);
    }
    bool IsMine() const { return (nFlags & WTX_ISMINE); }
    bool IsFromMe() const { return (nFlags & WTX_FROMME); }
    std::string GetTypeString() const
    {
        if (nType == CTransaction::TX_TOKEN) return std::string("token");
        if (nType == CTransaction::TX_CERT) return std::string("certification");
        if (nType == CTransaction::TX_GENESIS) return std::string("genesis");
        if (nType == CTransaction::TX_STAKE) return std::string("stake");
        if (nType == CTransaction::TX_WORK) return std::string("work");
        return std::string("undefined");
    }
    int64 GetChange() const
    {
        return (nValueIn - nAmount - nTxFee);
    }
    const CTxOutput GetOutput(int n=0) const
    {
        if (n == 0)
        {
            return CTxOutput(sendTo,nAmount,nLockUntil);
        }
        else if (n == 1)
        {
            return CTxOutput(destIn,GetChange(),0);
        }
        return CTxOutput();
    }
    int GetRefCount() const
    {
        return nRefCount;
    }
};

class CWalletTxOut
{
public:
    CWalletTxOut(CWalletTx* pWalletTxIn=NULL,int nIn=-1) : pWalletTx(pWalletTxIn),n(nIn) {}
    bool IsNull() const { return (pWalletTx == NULL || pWalletTx->GetOutput(n).IsNull()); }
    bool IsLocked(int nHeight) const { return (n == 0 && pWalletTx->nLockUntil > 0 && pWalletTx->nLockUntil < nHeight); }
    int GetDepth(int nHeight) const { return (pWalletTx->nBlockHeight >= 0 ? nHeight - pWalletTx->nBlockHeight + 1 : 0); }
    int64 GetAmount() const { return (n == 0 ? pWalletTx->nAmount : pWalletTx->GetChange()); }
    CTxOutPoint GetTxOutPoint() const { return CTxOutPoint(pWalletTx->txid,n); }
    void AddRef() const { ++pWalletTx->nRefCount; }
    void Release() const { --pWalletTx->nRefCount; }
    friend inline bool operator==(const CWalletTxOut& a,const CWalletTxOut& b)
    {
        return (a.pWalletTx == b.pWalletTx && a.n == b.n);
    }
    friend inline bool operator<(const CWalletTxOut& a,const CWalletTxOut& b)
    {
        return (a.pWalletTx < b.pWalletTx || (a.pWalletTx == b.pWalletTx && a.n < b.n));
    }
public:
    CWalletTx* pWalletTx;
    int n;
};

#endif //MULTIVERSE_WALLETTX_H

