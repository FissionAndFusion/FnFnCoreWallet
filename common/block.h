// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_BLOCK_H
#define  MULTIVERSE_BLOCK_H

#include "uint256.h"
#include "bignum.h"
#include "transaction.h"
#include <vector>
#include <walleve/stream/stream.h>
#include <walleve/stream/datastream.h>

class CBlock
{
    friend class walleve::CWalleveStream;
public:
    uint16  nVersion;
    uint16  nType;
    uint32  nTimeStamp;
    uint256 hashPrev;
    CTransaction txMint;
    std::vector<uint8> vchProof;
    std::vector<uint256> vTxHash;
    std::vector<uint8> vchSig;
    enum
    {
        BLOCK_GENESIS   = 0xffff,
        BLOCK_ORIGIN    = 0xff00,
        BLOCK_COINSTAKE = 0x0001,
        BLOCK_DELEGATED = 0x0002,
        BLOCK_EXTENDED  = 0x0003,
    };
public:
    CBlock()
    {
        SetNull();
    }
    void SetNull()
    {
        nVersion   = 1;
        nType      = 0;
        nTimeStamp = 0;
        hashPrev   = 0;
        txMint.SetNull();
        vchProof.clear();
        vTxHash.clear();
        vchSig.clear();
    }
    bool IsNull() const
    {
        return (nType == 0 || nTimeStamp == 0 || txMint.IsNull());
    }
    bool IsOrigin() const
    {
        return (nType >> 15);
    }
    uint256 GetHash() const
    {
        walleve::CWalleveBufStream ss;
        ss << (*this);
        return multiverse::crypto::CryptoHash(ss.GetData(),ss.GetSize());
    }
    uint64 GetBlockBeacon(int idx = 0) const
    {
        if (vchProof.empty())
        {
            return hashPrev.Get64(idx & 3);
        }
        return 0;
    }
    void GetTxHash(std::vector<uint256>& vTxHashRet) const
    {
        vTxHashRet.clear();
        vTxHashRet.push_back(txMint.GetHash());
        vTxHashRet.insert(vTxHashRet.end(),vTxHash.begin(),vTxHash.end());
    }
protected:
    template <typename O>
    void WalleveSerialize(walleve::CWalleveStream& s,O& opt)
    {
        s.Serialize(nVersion,opt);
        s.Serialize(nType,opt);
        s.Serialize(nTimeStamp,opt);
        s.Serialize(hashPrev,opt);
        s.Serialize(txMint,opt);
        s.Serialize(vchProof,opt);
        s.Serialize(vTxHash,opt);
        s.Serialize(vchSig,opt);
    }    
};

class CBlockIndex
{
public:
    const uint256* phashBlock;
    CBlockIndex* pOrigin;
    CBlockIndex* pPrev;
    uint256 txidStake;
    uint16  nVersion;
    uint16  nType;
    uint32  nTimeStamp;
    uint32  nHeight;
    uint64  nRandBeacon;
    uint64  nChainTrust;
    int64   nMoneySupply;
    uint32  nFile;
    uint32  nOffset;
public:
    CBlockIndex()
    {
        phashBlock = NULL;
        pOrigin = this;
        pPrev = NULL;
        txidStake = 0;
        nVersion = 0;
        nType = 0;
        nTimeStamp = 0;
        nHeight = 0;
        nChainTrust = 0;
        nMoneySupply = 0;
        nRandBeacon = 0;
        nFile = 0;
        nOffset = 0;
    }
    CBlockIndex(CBlock& block,uint32 nFileIn,uint32 nOffsetIn)
    {
        phashBlock = NULL;
        pOrigin = this;
        pPrev = NULL;
        txidStake = block.txMint.GetHash();
        nVersion = block.nVersion;
        nType = block.nType;
        nTimeStamp = block.nTimeStamp;
        nHeight = 0;
        nChainTrust = 0;
        nMoneySupply = 0;
        nRandBeacon = 0;
        nFile = nFileIn;
        nOffset = nOffsetIn;
    }
    uint256 GetBlockHash() const
    {
        return *phashBlock;
    }
    uint256 GetOriginHash() const
    {
        return pOrigin->GetBlockHash();
    }
    bool IsOrigin() const
    {
        return (nType >> 15);
    }
    std::string ToString() const
    {
        const char* sType[5] = {"genesis","undefined","coinstake","delegated","extended"};
        std::ostringstream oss;
        oss << "CBlockIndex : hash=" << GetBlockHash().ToString() 
                         << " prev=" << (pPrev ? pPrev ->GetBlockHash().ToString() : "Genesis")
                         << " nHeight=" << nHeight << " nFile=" << nFile << " nOffset=" << nOffset 
                         << " nType=" << sType[nType + 1];
        return oss.str();
    }
};

class CDiskBlockIndex : public CBlockIndex
{
public:
    uint256 hashBlock;
    uint256 hashPrev;
    uint32 nTxs;
public:
    CDiskBlockIndex()
    {
        hashBlock = 0;
        hashPrev = 0;
        nTxs = 0;
    }
    CDiskBlockIndex(const CBlockIndex* pIndex,const CBlock& block) : CBlockIndex(*pIndex)
    {
        hashBlock = pIndex->GetBlockHash();
        hashPrev = block.hashPrev;
        nTxs = block.vTxHash.size(); 
    }
    uint256 GetBlockHash() const
    {
        return hashBlock;
    }
    std::string ToString() const
    {
        const char* sType[5] = {"genesis","undefined","coinstake","delegated","extended"};
        std::ostringstream oss;
        oss << "CBlockIndex : hash=" << GetBlockHash().ToString() 
                         << " prev=" << hashPrev.ToString()
                         << " nHeight=" << nHeight << " nFile=" << nFile << " nOffset=" << nOffset 
                         << " nType=" << sType[nType + 1];
        return oss.str();
    }
};

/*
class CBlock : public CTransaction
{
public:
    uint32  nTime;
    uint32  nBits;
    uint256 hashPrev;
    uint256 hashTxStake;
    std::vector<uint256> vCandidate;
    std::vector<uint256> vTxHash;

    CBlock()
    {
    }
    CBlock(const CTransaction& txMint)
    : CTransaction(txMint)
    {
        if (txMint.nType == TYPE_TOKEN)
        {
            SetNull();
        }
        else
        {
            DecodeData();
        }
    }
    void EncodeData()
    {
        vchData.clear();
        vchData.reserve(80 + 32 * (vTxHash.size() + vCandidate.size()));
        walleve::CWalleveODataStream(vchData) << nTime << nBits << hashPrev << hashTxStake 
                                              << vCandidate << vTxHash;
    }
protected:
    void DecodeData()
    {
        walleve::CWalleveIDataStream(vchData) >> nTime >> nBits >> hashPrev >> hashTxStake 
                                              >> vCandidate >> vTxHash;
    }
};


class CBlockIndex
{
public:
    const uint256* phashBlock;
    CBlockIndex* pPrev;
    CBlockIndex* pCoRef;
    std::vector<CBlockIndex*> vNext;
    uint256 hashMerkleRoot;
    uint256 txidStake;
    CBigNum bnChainTrust;
    uint256 hashProofOfStake;
    uint32  nHeight;
    uint32  nVersion;
    uint32  nTime;
    uint32  nBits;
};
*/

#endif //MULTIVERSE_BLOCK_H

