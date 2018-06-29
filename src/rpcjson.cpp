// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "rpcjson.h"
#include "param.h"
#include "address.h"
#include <boost/foreach.hpp>

using namespace std;
using namespace walleve;
using namespace json_spirit;

namespace multiverse
{
///////////////////////////////

int64 AmountFromValue(const Value& value)
{
    double dAmount = value.get_real();
    if (dAmount <= 0.0 || dAmount > MAX_MONEY)
    {
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid amount");
    }
    int64 nAmount = (int64)(dAmount * COIN + 0.5);
    if (!MoneyRange(nAmount))
    {
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid amount");
    }
    return nAmount;
}

Value ValueFromAmount(int64 amount)
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

Object BlockToJSON(const uint256& hashBlock,const CBlock& block,const uint256& hashFork,int nHeight)
{
    Object result;
    result.push_back(Pair("hash",hashBlock.GetHex()));
    result.push_back(Pair("version",block.nVersion));
    result.push_back(Pair("type",GetBlockTypeStr(block.nType,block.txMint.nType)));
    result.push_back(Pair("time", (boost::uint64_t)block.GetBlockTime()));
    if (block.hashPrev != 0)
    {
        result.push_back(Pair("prev",block.hashPrev.GetHex()));
    }
    result.push_back(Pair("fork",hashFork.GetHex()));
    result.push_back(Pair("height",nHeight));
    
    result.push_back(Pair("txmint",block.txMint.GetHash().GetHex()));
    Array txhash;
    BOOST_FOREACH(const CTransaction& tx,block.vtx)
    {
        txhash.push_back(tx.GetHash().GetHex());
    }
    result.push_back(Pair("tx", txhash));
    return result;
}

Object TxToJSON(const uint256& txid,const CTransaction& tx,const uint256& hashFork,int nDepth)
{
    Object entry;
    entry.push_back(Pair("txid", txid.GetHex()));
    entry.push_back(Pair("version", (boost::int64_t)tx.nVersion));
    entry.push_back(Pair("type", tx.GetTypeString()));
    entry.push_back(Pair("lockuntil", (boost::int64_t)tx.nLockUntil));
    entry.push_back(Pair("anchor", tx.hashAnchor.GetHex()));
    Array vin;
    BOOST_FOREACH(const CTxIn& txin, tx.vInput)
    {
        Object in;
        in.push_back(Pair("txid", txin.prevout.hash.GetHex()));
        in.push_back(Pair("vout", (boost::int64_t)txin.prevout.n));
        vin.push_back(in);
    }
    entry.push_back(Pair("vin", vin));
    entry.push_back(Pair("sendto",CMvAddress(tx.sendTo).ToString()));
    entry.push_back(Pair("amount",ValueFromAmount(tx.nAmount)));
    entry.push_back(Pair("txfee",ValueFromAmount(tx.nTxFee)));

    entry.push_back(Pair("data",walleve::ToHexString(tx.vchData)));
    entry.push_back(Pair("sig",walleve::ToHexString(tx.vchSig)));
    entry.push_back(Pair("fork", hashFork.GetHex()));
    if (nDepth >= 0)
    {
        entry.push_back(Pair("confirmations", (boost::int64_t)nDepth));
    }

    return entry;
}

Object WalletTxToJSON(const CWalletTx& wtx)
{
    Object entry;
    entry.push_back(Pair("txid", wtx.txid.GetHex()));
    entry.push_back(Pair("fork", wtx.hashFork.GetHex()));
    if (wtx.nBlockHeight >= 0)
    {
        entry.push_back(Pair("blockheight", wtx.nBlockHeight));
    }
    entry.push_back(Pair("type", wtx.GetTypeString()));
    entry.push_back(Pair("send", wtx.IsFromMe()));
    if (!wtx.IsMintTx())
    {
        entry.push_back(Pair("from", CMvAddress(wtx.destIn).ToString()));
    }
    entry.push_back(Pair("to", CMvAddress(wtx.sendTo).ToString()));
    entry.push_back(Pair("amount", ValueFromAmount(wtx.nAmount)));
    entry.push_back(Pair("fee", ValueFromAmount(wtx.nTxFee)));
    entry.push_back(Pair("lockuntil", (boost::int64_t)wtx.nLockUntil));
    return entry;
}

} // namespace multiverse
