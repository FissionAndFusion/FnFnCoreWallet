// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "error.h"

using namespace multiverse;

static const char * _MvErrFailed = "operation failed";
static const char * _MvErrUnknown = "unkown error";
static const char * _MvErrString[] =
{
    // MV_OK = 0,
    "",
    // MV_ERR_UNAVAILABLE,                           // 1
    "unavailable",
    /* container */
    //MV_ERR_NOT_FOUND,                              // 2
    "not found",
    //MV_ERR_ALREADY_HAVE,                           // 3
    "already have",
    //MV_ERR_MISSING_PREV,                           // 4
    "missing previous",
    /* system */
    //MV_ERR_SYS_DATABASE_ERROR,                     // 5
    "database error",
    //MV_ERR_SYS_OUT_OF_DISK_SPACE,                  // 6
    "out of disk space",
    //MV_ERR_SYS_STORAGE_ERROR,                      // 7
    "storage error",
    //MV_ERR_SYS_OUT_OF_MEMORY,                      // 8
    "out of memory",
    /* block */
    //MV_ERR_BLOCK_OVERSIZE,                         // 9
    "block oversize",
    //MV_ERR_BLOCK_PROOF_OF_WORK_INVALID,            // 10
    "block proof-of-work is invalid",
    //MV_ERR_BLOCK_PROOF_OF_STAKE_INVALID,           // 11
    "block proof-of-stake is invalid",
    //MV_ERR_BLOCK_TIMESTAMP_OUT_OF_RANGE,           // 12
    "block timestamp is out of range",
    //MV_ERR_BLOCK_COINBASE_INVALID,                 // 13
    "block coinbase is invalid",
    //MV_ERR_BLOCK_COINSTAKE_INVALID,                // 14
    "block coinstake is invalid",
    //MV_ERR_BLOCK_TRANSACTIONS_INVALID,             // 15
    "block transactions are invalid",
    //MV_ERR_BLOCK_DUPLICATED_TRANSACTION,           // 16
    "block contain duplicated transaction",
    //MV_ERR_BLOCK_SIGOPCOUNT_OUT_OF_BOUND,          // 17
    "block sigopt is out of bound",
    //MV_ERR_BLOCK_TXHASH_MISMATCH,                  // 18
    "block txid is mismatch",  
    //MV_ERR_BLOCK_SIGNATURE_INVALID,                // 19
    "block signature is invalid",
    //MV_ERR_BLOCK_INVALID_FORK,                     // 20
    "block found invalid fork",
    /* transaction */
    //MV_ERR_TRANSACTION_INVALID,                    // 21
    "transaction invalid",
    //MV_ERR_TRANSACTION_OVERSIZE,                   // 22
    "transaction oversize",
    //MV_ERR_TRANSACTION_OUTPUT_INVALID,             // 23
    "transaction outputs are invalid",
    //MV_ERR_TRANSACTION_INPUT_INVALID,              // 24
    "transaction inputs are invalid",
    //MV_ERR_TRANSACTION_TIMESTAMP_INVALID,          // 25
    "transaction timestamp is invalid",
    //MV_ERR_TRANSACTION_NOT_ENOUGH_FEE,             // 26
    "transaction fee is not enough",
    //MV_ERR_TRANSACTION_STAKE_REWARD_INVALID,       // 27
    "transaction stake reward is invalid",
    //MV_ERR_TRANSACTION_SIGNATURE_INVALID,          // 28
    "transaction signature is invalid",
    //MV_ERR_TRANSACTION_CONFLICTING_INPUT,          // 29
    "transaction inputs are conflicting",
    /* wallet */
    //MV_ERR_WALLET_INVALID_AMOUNT,                  // 30
    "wallet amount is invalid",
    //MV_ERR_WALLET_INSUFFICIENT_FUNDS,              // 31
    "wallet funds is insufficient",
    //MV_ERR_WALLET_SIGNATURE_FAILED,                // 32
    "wallet failed to signature",
    //MV_ERR_WALLET_TX_OVERSIZE,                     // 33
    "wallet transaction is oversize",
    //MV_ERR_WALLET_NOT_FOUND,                       // 34
    "wallet is missing",
    //MV_ERR_WALLET_IS_LOCKED,	                     // 35
    "wallet is locked",
    //MV_ERR_WALLET_IS_UNLOCKED,                     // 36
    "wallet is unlocked",
    //MV_ERR_WALLET_IS_ENCRYPTED,                    // 37
    "wallet is encrypted",
    //MV_ERR_WALLET_IS_UNENCRYPTED,                  // 38
    "wallet is unencrypted",
    //MV_ERR_WALLET_FAILED,                          // 39
    "wallet operation is failed"
};

const char* multiverse::MvErrString(const MvErr& err)
{
    if (err < MV_OK)
    {
        return _MvErrFailed;
    }
    else if (err >= MV_ERR_MAX_COUNT)
    {
        return _MvErrUnknown;
    }
    return _MvErrString[err];
}
