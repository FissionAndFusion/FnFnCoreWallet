// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "core.h"

using namespace std;                      
using namespace walleve; 
using namespace multiverse;

#define DEBUG(err, ...)         Debug((err),__FUNCTION__,__VA_ARGS__)

static const size_t MIN_TOKEN_TX_SIZE = 192;
static const int64  MAX_CLOCK_DRIFT   = 10 * 60;
///////////////////////////////
// CMvCoreProtocol

CMvCoreProtocol::CMvCoreProtocol()
{
}

CMvCoreProtocol::~CMvCoreProtocol()
{
}

bool CMvCoreProtocol::WalleveHandleInitialize()
{
    CBlock block;
    GetGenesisBlock(block);
    hashGenesisBlock = block.GetHash();
    return true;
}

const MvErr CMvCoreProtocol::Debug(const MvErr& err,const char* pszFunc,const char *pszFormat,...)
{
    string strFormat(pszFunc);
    strFormat += string(", ") + string(MvErrString(err)) + string(" : ") + string(pszFormat);
    va_list ap;
    va_start(ap,pszFormat);
    WalleveVDebug(strFormat.c_str(),ap);
    va_end(ap);
    return err;
}

const uint256& CMvCoreProtocol::GetGenesisBlockHash()
{
    return hashGenesisBlock;
}

/*
PubKey : 575f2041770496489120bb102d9dd55f5e75b0c4aa528d5762b92b59acd6d939
Secret : bf0ebca9be985104e88b6a24e54cedbcdd1696a8984c8fdd7bc96917efb5a1ed

PubKey : 749b1fe6ad43c24bd8bff20a222ef74fdf0763a7efa0761619b99ec73985016c
Secret : 05f07d1e09b60b74538a1e021c98956a7c508fa35f6c3eba01c9ab1f6a871611

PubKey : 6236d780f9f743707d57b3feb19d21c8a867577f5e83c163774222bb7ef8d8cb
Secret : 7c6a6aba05cec77a998c19649ee1fa0e29c7b5246d0e3a6501ee1d4d81dd73ea
*/

void CMvCoreProtocol::GetGenesisBlock(CBlock& block)
{
    block.SetNull();

    block.nVersion   = 1;
    block.nType      = CBlock::BLOCK_GENESIS;
    block.nTimeStamp = 1515745156;
    block.hashPrev   = 0;
    
    CTransaction& tx = block.txMint;
    tx.nType   = CTransaction::TX_GENESIS;
    tx.sendTo  = CDestination(multiverse::crypto::CPubKey(uint256("575f2041770496489120bb102d9dd55f5e75b0c4aa528d5762b92b59acd6d939")));
    tx.nAmount = 745000000 * COIN;

    const char* pszGenesis = "The third machine age : Revolution";
    block.vchProof = vector<uint8>((const uint8*)pszGenesis, (const uint8*)pszGenesis + strlen(pszGenesis));
    
}

MvErr CMvCoreProtocol::ValidateTransaction(const CTransaction& tx)
{
    // Basic checks that don't depend on any context
    if (tx.vInput.empty() && tx.nType != CTransaction::TX_GENESIS && tx.nType != CTransaction::TX_WORK)
    {
        return DEBUG(MV_ERR_TRANSACTION_INVALID,"tx vin is empty\n");
    }
    if (!tx.vInput.empty() && (tx.nType == CTransaction::TX_GENESIS || tx.nType == CTransaction::TX_WORK))
    {
        return DEBUG(MV_ERR_TRANSACTION_INVALID,"tx vin is not empty for genesis or work tx\n");
    }
    if (!tx.vchSig.empty() && tx.IsMintTx())
    {
        return DEBUG(MV_ERR_TRANSACTION_INVALID,"invalid signature\n");
    }
    if (tx.sendTo.IsNull())
    {
        return DEBUG(MV_ERR_TRANSACTION_OUTPUT_INVALID,"send to null address\n");
    }
    if (!MoneyRange(tx.nAmount))
    {
        return DEBUG(MV_ERR_TRANSACTION_OUTPUT_INVALID,"amount overflow %ld\n",tx.nAmount);
    }
    
    if (!MoneyRange(tx.nTxFee)
        || (tx.nType != CTransaction::TX_TOKEN && tx.nTxFee != 0)
        || (tx.nType == CTransaction::TX_TOKEN && tx.nTxFee < MIN_TX_FEE))
    {
        return DEBUG(MV_ERR_TRANSACTION_OUTPUT_INVALID,"txfee invalid %ld",tx.nTxFee);
    }

    set<CTxOutPoint> setInOutPoints;
    BOOST_FOREACH(const CTxIn& txin, tx.vInput)
    {
        if (txin.prevout.IsNull() || txin.prevout.n > 1)
        {
            return DEBUG(MV_ERR_TRANSACTION_INPUT_INVALID,"prevout invalid\n");
        }
        if (!setInOutPoints.insert(txin.prevout).second)
        {
            return DEBUG(MV_ERR_TRANSACTION_INPUT_INVALID,"duplicate inputs\n");
        }
    }

    if (GetSerializeSize(tx) > MAX_TX_SIZE)
    {
        return DEBUG(MV_ERR_TRANSACTION_OVERSIZE,"%u\n",GetSerializeSize(tx));
    }

    return MV_OK;
}

MvErr CMvCoreProtocol::ValidateBlock(CBlock& block)
{
    // These are checks that are independent of context
    // Check timestamp
    if (block.GetBlockTime() > WalleveGetNetTime() + MAX_CLOCK_DRIFT)
    {
        return DEBUG(MV_ERR_BLOCK_TIMESTAMP_OUT_OF_RANGE,"%ld\n",block.GetBlockTime());
    }

    // Extended block should be not empty
    if (block.nType == CBlock::BLOCK_EXTENDED && block.vTxHash.empty())
    {
        return DEBUG(MV_ERR_BLOCK_TRANSACTIONS_INVALID,"empty extended block\n");
    }
    
    // Validate mint tx
    if (!block.txMint.IsMintTx() || ValidateTransaction(block.txMint) != MV_OK)
    {
        return DEBUG(MV_ERR_BLOCK_TRANSACTIONS_INVALID,"invalid mint tx\n");
    }

    size_t nMinSize = GetSerializeSize(block) + block.vTxHash.size() * MIN_TOKEN_TX_SIZE;
    if (nMinSize > MAX_BLOCK_SIZE)
    {
        return DEBUG(MV_ERR_BLOCK_OVERSIZE,"size overflow minsize=%u vtx=%u\n",nMinSize,block.vTxHash.size());
    }

    if (!CheckBlockSignature(block))
    {
        return DEBUG(MV_ERR_BLOCK_SIGNATURE_INVALID,"\n");
    }
    return MV_OK;
}

MvErr CMvCoreProtocol::ValidateBlock(CBlock& block,vector<CTransaction>& vtx)
{
    // These are checks that are independent of context
    // Check timestamp
    if (block.GetBlockTime() > WalleveGetNetTime() + MAX_CLOCK_DRIFT)
    {
        return DEBUG(MV_ERR_BLOCK_TIMESTAMP_OUT_OF_RANGE,"%ld\n",block.GetBlockTime());
    }

    // Extended block should be not empty
    if (block.nType == CBlock::BLOCK_EXTENDED && vtx.empty())
    {
        return DEBUG(MV_ERR_BLOCK_TRANSACTIONS_INVALID,"empty extended block\n");
    }
    
    // Validate mint tx
    if (!block.txMint.IsMintTx() || ValidateTransaction(block.txMint) != MV_OK)
    {
        return DEBUG(MV_ERR_BLOCK_TRANSACTIONS_INVALID,"invalid mint tx\n");
    }

    // Check block size and transactions 
    if (block.vTxHash.size() != vtx.size())
    {
        return DEBUG(MV_ERR_BLOCK_TXHASH_MISMATCH,"hash count mismatch\n");
    }
    size_t nSize = GetSerializeSize(block);
    set<uint256> setTxHash;
    for (size_t i = 0;i < vtx.size();i++)
    {
        const CTransaction& tx = vtx[i];
        if (tx.IsMintTx())
        {
            return DEBUG(MV_ERR_BLOCK_TRANSACTIONS_INVALID,"invalid tx type\n");
        }
        if (block.vTxHash[i] != tx.GetHash())
        {
            return DEBUG(MV_ERR_BLOCK_TXHASH_MISMATCH,"tx hash mismatch\n");
        }
        nSize += GetSerializeSize(tx);
        if (nSize > MAX_BLOCK_SIZE)
        {
            return DEBUG(MV_ERR_BLOCK_OVERSIZE,"size overflow %u tx(%u/%u)\n",nSize,i,vtx.size());
        }
        if (!setTxHash.insert(block.vTxHash[i]).second)
        {
            return DEBUG(MV_ERR_BLOCK_DUPLICATED_TRANSACTION,"duplicate tx\n");
        }
    }
    return MV_OK;
}

MvErr CMvCoreProtocol::VerifyBlock(CBlock& block,const CDestination& destIn,int64 nValueIn,
                                   int64 nTxFee,CBlockIndex* pIndexPrev)
{
    return MV_OK;
}

MvErr CMvCoreProtocol::VerifyBlockTx(CBlockTx& tx,storage::CBlockView& view)
{
    return MV_OK;
}

bool CMvCoreProtocol::CheckBlockSignature(const CBlock& block)
{
    return true;
}
///////////////////////////////
// CMvTestNetCoreProtocol

CMvTestNetCoreProtocol::CMvTestNetCoreProtocol()
{
}

void CMvTestNetCoreProtocol::GetGenesisBlock(CBlock& block)
{
}

