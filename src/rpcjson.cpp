// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "rpcjson.h"

#include <boost/foreach.hpp>

#include "param.h"
#include "address.h"

using namespace std;
using namespace walleve;
using namespace json_spirit;
using namespace multiverse::rpc;

namespace multiverse
{
///////////////////////////////

int64 AmountFromValue(const double dAmount)
{
    if (dAmount <= 0.0 || dAmount > MAX_MONEY)
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Invalid amount");
    }
    int64 nAmount = (int64)(dAmount * COIN + 0.5);
    if (!MoneyRange(nAmount))
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Invalid amount");
    }
    return nAmount;
}

double ValueFromAmount(int64 amount)
{
    return ((double)amount / (double)COIN);
}

static string GetBlockTypeStr(uint16 nType,uint16 nMintType)
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

CBlockData BlockToJSON(const uint256& hashBlock,const CBlock& block,const uint256& hashFork,int nHeight)
{
    CBlockData data;
    data.strHash = hashBlock.GetHex();
    data.nVersion = block.nVersion;
    data.strType = GetBlockTypeStr(block.nType,block.txMint.nType);
    data.nTime = block.GetBlockTime();
    if (block.hashPrev != 0)
    {
        data.strPrev = block.hashPrev.GetHex();
    }
    data.strFork = hashFork.GetHex();
    data.nHeight = nHeight;
    
    data.strTxmint = block.txMint.GetHash().GetHex();
    BOOST_FOREACH(const CTransaction& tx,block.vtx)
    {
        data.vecTx.push_back(tx.GetHash().GetHex());
    }
    return data;
}

CTransactionData TxToJSON(const uint256& txid,const CTransaction& tx,const uint256& hashFork,int nDepth)
{
    CTransactionData ret;
    ret.strTxid = txid.GetHex();
    ret.nVersion = tx.nVersion;
    ret.strType = tx.GetTypeString();
    ret.nLockuntil = tx.nLockUntil;
    ret.strAnchor = tx.hashAnchor.GetHex();
    BOOST_FOREACH(const CTxIn& txin, tx.vInput)
    {
        CTransactionData::CVin vin;
        vin.nVout = txin.prevout.n;
        vin.strTxid = txin.prevout.hash.GetHex();
        ret.vecVin.push_back(move(vin));
    }
    ret.strSendto = CMvAddress(tx.sendTo).ToString();
    ret.fAmount = ValueFromAmount(tx.nAmount);
    ret.fTxfee = ValueFromAmount(tx.nTxFee);

    ret.strData = walleve::ToHexString(tx.vchData);
    ret.strSig = walleve::ToHexString(tx.vchSig);
    ret.strFork = hashFork.GetHex();
    if (nDepth >= 0)
    {
        ret.nConfirmations = nDepth;
    }

    return ret;
}

CWalletTxData WalletTxToJSON(const CWalletTx& wtx)
{
    CWalletTxData data;
    data.strTxid = wtx.txid.GetHex();
    data.strFork = wtx.hashFork.GetHex();
    if (wtx.nBlockHeight >= 0)
    {
        data.nBlockheight = wtx.nBlockHeight;
    }
    data.strType = wtx.GetTypeString();
    data.fSend = wtx.IsFromMe();
    if (!wtx.IsMintTx())
    {
        data.strFrom = CMvAddress(wtx.destIn).ToString();
    }
    data.strTo = CMvAddress(wtx.sendTo).ToString();
    data.fAmount = ValueFromAmount(wtx.nAmount);
    data.fFee = ValueFromAmount(wtx.nTxFee);
    data.nLockuntil = (boost::int64_t)wtx.nLockUntil;
    return data;
}

} // namespace multiverse
