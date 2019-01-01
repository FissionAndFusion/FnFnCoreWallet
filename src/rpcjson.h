// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#ifndef  MULTIVERSE_RPCJSON_H
#define  MULTIVERSE_RPCJSON_H

#include "block.h"
#include "transaction.h"
#include "wallettx.h"
#include "json/json_spirit_reader_template.h"
#include "json/json_spirit_writer_template.h"
#include "json/json_spirit_utils.h"
#include "rpc/rpc.h"

namespace multiverse
{

int64 AmountFromValue(const double dAmount);

double ValueFromAmount(int64 amount);

rpc::CBlockData BlockToJSON(const uint256& hashBlock,const CBlock& block,const uint256& hashFork,int nHeight);

rpc::CTransactionData TxToJSON(const uint256& txid,const CTransaction& tx,const uint256& hashFork,int nDepth);

rpc::CWalletTxData WalletTxToJSON(const CWalletTx& wtx);

} // namespace multiverse

#endif //MULTIVERSE_RPCJSON_H

