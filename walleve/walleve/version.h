// Copyright (c) 2016-2018 The LoMoCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  LOMOCOIN_WALLEVE_VERSION_H
#define  LOMOCOIN_WALLEVE_VERSION_H

namespace walleve
{
#define WALLEVE_VERSION_MAJOR		0
#define WALLEVE_VERSION_MINOR		10


#define _TOSTR(s) #s
#define _VERSTR(major,minor)            _TOSTR(major) "." _TOSTR(minor)
#define WALLEVE_VERSION_STRING          _VERSTR(WALLEVE_VERSION_MAJOR,WALLEVE_VERSION_MINOR)

} // namespace walleve

#endif //LOMOCOIN_WALLEVE_VERSION_H

