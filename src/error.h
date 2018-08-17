// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_ERROR_H
#define  MULTIVERSE_ERROR_H

namespace multiverse
{

typedef enum
{
    MV_FAILED = -1,
    MV_OK = 0,
    /* moduler */
    MV_ERR_UNAVAILABLE,   				// 1
    /* container */
    MV_ERR_NOT_FOUND,					// 2
    MV_ERR_ALREADY_HAVE,				// 3
    MV_ERR_MISSING_PREV,				// 4
    /* system */
    MV_ERR_SYS_DATABASE_ERROR,				// 5
    MV_ERR_SYS_OUT_OF_DISK_SPACE,			// 6
    MV_ERR_SYS_STORAGE_ERROR,				// 7
    MV_ERR_SYS_OUT_OF_MEMORY,				// 8
    /* block */
    MV_ERR_BLOCK_OVERSIZE,				// 9
    MV_ERR_BLOCK_PROOF_OF_WORK_INVALID,			// 10
    MV_ERR_BLOCK_PROOF_OF_STAKE_INVALID,		// 11
    MV_ERR_BLOCK_TIMESTAMP_OUT_OF_RANGE,		// 12
    MV_ERR_BLOCK_COINBASE_INVALID,			// 13
    MV_ERR_BLOCK_COINSTAKE_INVALID,			// 14
    MV_ERR_BLOCK_TRANSACTIONS_INVALID,			// 15
    MV_ERR_BLOCK_DUPLICATED_TRANSACTION,		// 16
    MV_ERR_BLOCK_SIGOPCOUNT_OUT_OF_BOUND,		// 17
    MV_ERR_BLOCK_TXHASH_MISMATCH,			// 18
    MV_ERR_BLOCK_SIGNATURE_INVALID,			// 19
    MV_ERR_BLOCK_INVALID_FORK,				// 20
    /* transaction */
    MV_ERR_TRANSACTION_INVALID,				// 21
    MV_ERR_TRANSACTION_OVERSIZE,			// 22
    MV_ERR_TRANSACTION_OUTPUT_INVALID,			// 23
    MV_ERR_TRANSACTION_INPUT_INVALID,			// 24
    MV_ERR_TRANSACTION_TIMESTAMP_INVALID,		// 25
    MV_ERR_TRANSACTION_NOT_ENOUGH_FEE,			// 26
    MV_ERR_TRANSACTION_STAKE_REWARD_INVALID,		// 27
    MV_ERR_TRANSACTION_SIGNATURE_INVALID,		// 28
    MV_ERR_TRANSACTION_CONFLICTING_INPUT,		// 29
    /* wallet */
    MV_ERR_WALLET_INVALID_AMOUNT, 			// 30
    MV_ERR_WALLET_INSUFFICIENT_FUNDS, 			// 31
    MV_ERR_WALLET_SIGNATURE_FAILED,			// 32
    MV_ERR_WALLET_TX_OVERSIZE,				// 33
    MV_ERR_WALLET_NOT_FOUND,				// 34
    MV_ERR_WALLET_IS_LOCKED,				// 35
    MV_ERR_WALLET_IS_UNLOCKED,				// 36
    MV_ERR_WALLET_IS_ENCRYPTED,				// 37
    MV_ERR_WALLET_IS_UNENCRYPTED,			// 38
    MV_ERR_WALLET_FAILED,				// 39
    /* count */
    MV_ERR_MAX_COUNT
}MvErr;

extern const char* MvErrString(const MvErr& err);

} // namespace multiverse

#endif //MULTIVERSE_ERROR_H

