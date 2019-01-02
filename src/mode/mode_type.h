// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_MODE_TYPE_H
#define MULTIVERSE_MODE_TYPE_H

namespace multiverse
{
// mode type
enum class EModeType
{
    ERROR = 0,  // ERROR type
    SERVER,     // server
    CONSOLE,    // console
    MINER,      // miner
    DNSEED,     // dnseed
};

}  // namespace multiverse

#endif  // MULTIVERSE_MODE_TYPE_H