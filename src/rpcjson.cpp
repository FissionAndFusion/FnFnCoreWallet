// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "rpcjson.h"
#include "param.h"
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
    BOOST_FOREACH(const uint256& txid,block.vTxHash)
    {
        txhash.push_back(txid.GetHex());
    }
    result.push_back(Pair("tx", txhash));
    return result;
}


} // namespace multiverse
