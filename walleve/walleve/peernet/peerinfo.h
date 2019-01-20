// Copyright (c) 2016-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  WALLEVE_PEERINFO_H
#define  WALLEVE_PEERINFO_H

#include <string>

namespace walleve
{

class CPeerInfo
{
public:
    virtual ~CPeerInfo() {}
public:
    std::string strAddress;
    bool fInBound;
    int64 nActive;
    int64 nLastRecv;
    int64 nLastSend;
    int nScore;
};

} // namespace walleve

#endif //WALLEVE_PEERINFO_H


