// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_BLOCK_H
#define  MULTIVERSE_BLOCK_H

#include "uint256.h"
#include "proof.h"
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
        BLOCK_GENESIS    = 0xffff,
        BLOCK_ORIGIN     = 0xff00,
        BLOCK_PRIMARY    = 0x0001,
        BLOCK_SUBSIDIARY = 0x0002,
        BLOCK_EXTENDED   = 0x0004,
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
    bool IsPrimary() const
    {
        return (nType & 1);
    }
    bool IsProofOfWork() const
    {
        return (txMint.nType == CTransaction::TX_WORK);
    }
    uint256 GetHash() const
    {
        walleve::CWalleveBufStream ss;
        ss << (*this);
        return multiverse::crypto::CryptoHash(ss.GetData(),ss.GetSize());
    }
    uint256 GetSignatureHash() const
    {
        walleve::CWalleveBufStream ss;
        ss << nVersion << nType << nTimeStamp << hashPrev << txMint << vchProof << vTxHash;
        return multiverse::crypto::CryptoHash(ss.GetData(),ss.GetSize());
    }
    void GetSerializedProofOfWorkData(std::vector<unsigned char>& vchProofOfWork) const
    {
        walleve::CWalleveBufStream ss;
        ss << nVersion << nType << nTimeStamp << hashPrev << vchProof;
        vchProofOfWork.assign(ss.GetData(),ss.GetData() + ss.GetSize());
    }
    int64 GetBlockTime() const
    {
        return (int64)nTimeStamp;
    }
    uint64 GetBlockBeacon(int idx = 0) const
    {
        if (vchProof.empty())
        {
            return hashPrev.Get64(idx & 3);
        }
        return 0;
    }
    uint64 GetBlockTrust() const
    {
        if (vchProof.empty())
        {
            return 1;
        }
        else if (IsProofOfWork())
        {
            return 1; 
        }
        return 0;
    }
    int64 GetBlockMint(int64 nValueIn) const
    {
        return (txMint.nAmount - nValueIn); 
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
    uint256 txidMint;
    uint16  nMintType;
    uint16  nVersion;
    uint16  nType;
    uint32  nTimeStamp;
    uint32  nHeight;
    uint64  nRandBeacon;
    uint64  nChainTrust;
    int64   nMoneySupply;
    uint8   nProofAlgo;
    uint8   nProofBits;
    uint32  nFile;
    uint32  nOffset;
public:
    CBlockIndex()
    {
        phashBlock = NULL;
        pOrigin = this;
        pPrev = NULL;
        txidMint = 0;
        nMintType = 0;
        nVersion = 0;
        nType = 0;
        nTimeStamp = 0;
        nHeight = 0;
        nChainTrust = 0;
        nRandBeacon = 0;
        nMoneySupply = 0;
        nProofAlgo = 0;
        nProofBits = 0;
        nFile = 0;
        nOffset = 0;
    }
    CBlockIndex(CBlock& block,uint32 nFileIn,uint32 nOffsetIn)
    {
        phashBlock = NULL;
        pOrigin = this;
        pPrev = NULL;
        txidMint = block.txMint.GetHash();
        nMintType = block.txMint.nType;
        nVersion = block.nVersion;
        nType = block.nType;
        nTimeStamp = block.nTimeStamp;
        nHeight = 0;
        nChainTrust = 0;
        nMoneySupply = 0;
        nRandBeacon = 0;
        if (IsProofOfWork() && block.vchProof.size() >= CProofOfHashWorkCompact::PROOFHASHWORK_SIZE)
        {
            CProofOfHashWorkCompact proof;
            proof.Load(block.vchProof);
            nProofAlgo = proof.nAlgo;
            nProofBits = proof.nBits;
        }
        else
        {
            nProofAlgo = 0;
            nProofBits = 0;
        }
        nFile = nFileIn;
        nOffset = nOffsetIn;
    }
    uint256 GetBlockHash() const
    {
        return *phashBlock;
    }
    int GetBlockHeight() const
    {
        return nHeight;
    }
    int64 GetBlockTime() const
    {
        return (int64)nTimeStamp;
    }
    uint256 GetOriginHash() const
    {
        return pOrigin->GetBlockHash();
    }
    uint256 GetParentHash() const
    {
        return (!pOrigin->pPrev ? 0 : pOrigin->pPrev->GetOriginHash());
    }
    int64 GetMoneySupply() const
    {
        return nMoneySupply;
    }
    bool IsOrigin() const
    {
        return (nType >> 15);
    }
    bool IsPrimary() const
    {
        return (nType & 1);
    }
    bool IsProofOfWork() const
    {
        return (nMintType == CTransaction::TX_WORK);
    }
    const std::string GetBlockType() const
    {
        if (nType == CBlock::BLOCK_GENESIS) return std::string("genesis");
        if (nType == CBlock::BLOCK_ORIGIN) return std::string("origin");
        if (nType == CBlock::BLOCK_EXTENDED) return std::string("extended");
        std::string str("undefined-");
        if (nType == CBlock::BLOCK_PRIMARY) str = "primary-";
        if (nType == CBlock::BLOCK_SUBSIDIARY) str = "subsidiary-";
        if (nMintType == CTransaction::TX_WORK) return (str + "pow"); 
        if (nMintType == CTransaction::TX_STAKE) return (str + "dpos"); 
        return str;
    }
    std::string ToString() const
    {
        std::ostringstream oss;
        oss << "CBlockIndex : hash=" << GetBlockHash().ToString() 
                         << " prev=" << (pPrev ? pPrev->GetBlockHash().ToString() : "NULL")
                         << " height=" << nHeight
                         << " type=" << GetBlockType();
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
        std::ostringstream oss;
        oss << "CDiskBlockIndex : hash=" << GetBlockHash().ToString() 
                             << " prev=" << hashPrev.ToString()
                             << " height=" << nHeight << " file=" << nFile << " offset=" << nOffset 
                             << " type=" << GetBlockType();
        return oss.str();
    }
};

#endif //MULTIVERSE_BLOCK_H

