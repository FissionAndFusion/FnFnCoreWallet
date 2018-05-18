// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_TRANSACTION_H
#define  MULTIVERSE_TRANSACTION_H

#include "uint256.h"
#include "destination.h"
#include "crypto.h"

#include <walleve/stream/stream.h>

class CTransaction
{
    friend class walleve::CWalleveStream;
public:
    uint16 nVersion;
    uint16 nType;
    uint256 hashAnchor;
    std::vector<uint256> vInput;
    CDestination sendTo;
    int64 nAmount;
    int64 nTxFee;
    std::vector<uint8> vchData;
    std::vector<uint8> vchSig;

    enum 
    {
        TYPE_TOKEN     = 0x0000,
        TYPE_CANDIDATE = 0x00ff,
        TYPE_STAKE     = 0x0100,
        TYPE_COSTAKE   = 0x0101,
        TYPE_INTERIM   = 0xffef,
        TYPE_GENESIS   = 0xffff,
    };
    CTransaction()
    {
        SetNull();
    }
    virtual void SetNull()
    {
        nVersion = 1;
        nType = 0;
        hashAnchor = 0;
        vInput.clear();
        sendTo.SetNull();
        nAmount = 0;
        nTxFee = 0;
        vchData.clear();
        vchSig.clear();
    }
    bool IsNull() const
    {
        return (vInput.empty() && sendTo.IsNull());
    }
    uint256 GetHash() const
    {
        walleve::CWalleveBufStream ss;
        ss << (*this);
        return multiverse::crypto::CryptoHash(ss.GetData(),ss.GetSize());
    }
    int64 GetChange(int64 nValueIn) const
    {
        return (nValueIn - nAmount - nTxFee);
    }
    friend bool operator==(const CTransaction& a, const CTransaction& b)
    {
        return (a.nVersion   == b.nVersion    &&
                a.nType      == b.nType       &&
                a.hashAnchor == b.hashAnchor  &&
                a.vInput     == b.vInput      &&
                a.sendTo     == b.sendTo      &&
                a.nAmount    == b.nAmount     &&
                a.nTxFee     == b.nTxFee      &&
                a.vchData    == b.vchData     &&
                a.vchSig     == b.vchSig        );
    }
    friend bool operator!=(const CTransaction& a, const CTransaction& b)
    {
        return !(a == b);
    }
protected:
    template <typename O>
    void WalleveSerialize(walleve::CWalleveStream& s,O& opt)
    {
        s.Serialize(nVersion,opt);
        s.Serialize(nType,opt);
        s.Serialize(hashAnchor,opt);
        s.Serialize(vInput,opt);
        s.Serialize(sendTo,opt);
        s.Serialize(nAmount,opt);
        s.Serialize(nTxFee,opt);
        s.Serialize(vchData,opt);
        s.Serialize(vchSig,opt);
    }
};

class CTxOutPoint
{
    friend class walleve::CWalleveStream;
public:
    uint256 hash;
    uint8   n;
public:
    CTxOutPoint() { SetNull(); }
    CTxOutPoint(uint256 hashIn, uint8 nIn) { hash = hashIn; n = nIn; }
    virtual void SetNull() { hash = 0; n = -1; }
    virtual bool IsNull() const { return (hash == 0 && n == -1); }
    friend bool operator<(const CTxOutPoint& a, const CTxOutPoint& b)
    {
        return (a.hash < b.hash || (a.hash == b.hash && a.n < b.n));
    }

    friend bool operator==(const CTxOutPoint& a, const CTxOutPoint& b)
    {
        return (a.hash == b.hash && a.n == b.n);
    }

    friend bool operator!=(const CTxOutPoint& a, const CTxOutPoint& b)
    {
        return !(a == b);
    }
protected:
    template <typename O>
    void WalleveSerialize(walleve::CWalleveStream& s,O& opt)
    {
        s.Serialize(hash,opt);
        s.Serialize(n,opt);
    }
};

class CTxOutput
{
public:
    CDestination destTo;
    int64 nAmount;
public:
    CTxOutput() { SetNull(); }
    CTxOutput(const CDestination destToIn,int64 nAmountIn) : destTo(destToIn),nAmount(nAmountIn) {}
    CTxOutput(const CTransaction& tx)
    {
        destTo = tx.sendTo; nAmount = tx.nAmount;
    }
    CTxOutput(const CTransaction& tx,const CDestination& destToIn,int64 nValueIn)
    {
        destTo = destToIn; nAmount = tx.GetChange(nValueIn);
    }
    void SetNull()
    {
        destTo.SetNull();
        nAmount = 0; 
    }
    bool IsNull() const { return (destTo.IsNull() == 0 || nAmount == 0); }
};

class CTxUnspent : public CTxOutPoint
{
public:
    CTxOutput output;
public:
    CTxUnspent() { SetNull(); }
    CTxUnspent(const CTxOutPoint& out,const CTxOutput& outputIn) : CTxOutPoint(out),output(outputIn) {}
    void SetNull()
    {
        CTxOutPoint::SetNull();
        output.SetNull();
    }
    bool IsNull() const { return (CTxOutPoint::IsNull() || output.IsNull()); }
};

class CTxInput
{
    friend class walleve::CWalleveStream;
public:
    CTxOutPoint prevout;
    int64 nAmount;
public:
    CTxInput() { prevout.SetNull(); nAmount = 0; } 
    CTxInput(const CTxOutPoint& prevoutIn,int64 nAmountIn) : prevout(prevoutIn), nAmount(nAmountIn) {}
protected:
    template <typename O>
    void WalleveSerialize(walleve::CWalleveStream& s,O& opt)
    {
        s.Serialize(prevout,opt);
        s.Serialize(nAmount,opt);
    }
};

class CBlockTx : public CTransaction
{
public:
    CDestination destIn;
    int64 nValueIn;
    std::vector<CTxInput> vTxInput;
public:
    CBlockTx() { SetNull(); }
    CBlockTx(const CTransaction& tx) : CTransaction(tx),nValueIn(0) {}
    void SetNull()
    {
        CTransaction::SetNull();
        destIn.SetNull();
        nValueIn = 0;
        vTxInput.clear();
    }
    int64 GetChange() const
    {
        return (nValueIn - nAmount - nTxFee);
    }
    
};

class CTxIndex
{
public:
    uint16 nVersion;
    uint16 nType;
    uint256 hashAnchor;
    CDestination destIn;
    int64 nValueIn;
    std::vector<CTxInput> vTxInput;
    uint32 nFile;
    uint32 nOffset;
public:
    CTxIndex()
    {
        SetNull();
    }
    CTxIndex(const CTransaction& tx,const CDestination& destInIn,const std::vector<CTxInput>& vTxInputIn,
                                    uint32 nFileIn,uint32 nOffsetIn)
    {
        nVersion   = tx.nVersion;
        nType      = tx.nType;
        hashAnchor = tx.hashAnchor;
        destIn     = destInIn;
        vTxInput   = vTxInputIn;
        nValueIn   = 0;
        nFile      = nFileIn;
        nOffset    = nOffsetIn;
        for (int i = 0;i < vTxInput.size();i++)
        {
            nValueIn += vTxInput[i].nAmount;
        }
    }
    CTxIndex(const CBlockTx& tx,uint32 nFileIn,uint32 nOffsetIn)
    {
        nVersion   = tx.nVersion;
        nType      = tx.nType;
        hashAnchor = tx.hashAnchor;
        destIn     = tx.destIn;
        vTxInput   = tx.vTxInput;
        nValueIn   = tx.nValueIn;
        nFile      = nFileIn;
        nOffset    = nOffsetIn;
    }
    void SetNull()
    {
        nVersion   = 0;
        nType      = 0;
        hashAnchor = 0;
        nValueIn   = 0;
        nFile      = 0;
        nOffset    = 0;
        destIn.SetNull();
        vTxInput.clear();
    }
    bool IsNull() const 
    { 
        return (nVersion == 0 || nFile == 0); 
    };
    void GetPrevout(std::vector<CTxUnspent>& vPrevout) const
    {
        vPrevout.resize(vTxInput.size());
        for (int i = 0;i < vTxInput.size();i++)
        {
            vPrevout[i] = CTxUnspent(vTxInput[i].prevout,CTxOutput(destIn,vTxInput[i].nAmount));
        }
    }
};

#endif //MULTIVERSE_TRANSACTION_H

