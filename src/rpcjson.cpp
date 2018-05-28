// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "rpcjson.h"
#include "param.h"

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

} // namespace multiverse
