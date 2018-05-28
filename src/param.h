// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_PARAM_H
#define  MULTIVERSE_PARAM_H

#include "uint256.h"

static const int64 COIN = 1000000;
static const int64 CENT = 10000;
static const int64 MIN_TX_FEE = CENT / 100;
static const int64 MAX_MONEY = 1000000000 * COIN;
inline bool MoneyRange(int64 nValue) { return (nValue >= 0 && nValue <= MAX_MONEY); }

static const unsigned int MAX_BLOCK_SIZE = 2000000;
static const unsigned int MAX_TX_SIZE = (MAX_BLOCK_SIZE/20);

#endif //MULTIVERSE_PARAM_H
