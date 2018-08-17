// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_ADDRESS_H
#define  MULTIVERSE_ADDRESS_H

#include "destination.h"

namespace multiverse 
{

class CMvAddress : public CDestination
{
public:
    enum { ADDRESS_LEN=57 };
    CMvAddress(const CDestination& destIn = CDestination()) : CDestination(destIn) {}
    CMvAddress(const std::string& strAddress) { ParseString(strAddress); }
    bool ParseString(const std::string& strAddress);
    std::string ToString() const;
};

} // namespace multiverse

#endif //MULTIVERSE_ADDRESS_H

