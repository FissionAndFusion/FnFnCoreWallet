// Copyright (c) 2016-2018 The LoMoCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  WALLEVE_UTIL_H
#define  WALLEVE_UTIL_H

#include "walleve/type.h"
#include <boost/date_time.hpp>
#include <boost/asio/ip/address.hpp>

namespace walleve
{

inline int64 GetTime()
{
using namespace boost::posix_time;
    static ptime epoch(boost::gregorian::date(1970, 1, 1));
    return int64((second_clock::universal_time() - epoch).total_seconds());
}

inline int64 GetTimeMillis()
{
using namespace boost::posix_time;
    static ptime epoch(boost::gregorian::date(1970, 1, 1));
    return int64((microsec_clock::universal_time() - epoch).total_milliseconds());
}

inline bool IsRoutable(const boost::asio::ip::address& address)
{
    if (address.is_loopback() || address.is_unspecified())
    {
        return false;
    }
    if (address.is_v4())
    {        
        unsigned long u = address.to_v4().to_ulong();
        //RFC1918
        if ((u & 0xFF000000) == 0x0A000000 || (u & 0xFFFF0000) == 0xC0A80000 
            || (u >= 0xAC100000 && u < 0xAC200000))
        {
            return false;
        }
        //RFC3927
        if ((u & 0xFFFF0000) == 0xA9FE0000)
        {
            return false;
        }
    }
    else
    {
        boost::asio::ip::address_v6::bytes_type b = address.to_v6().to_bytes();
        //RFC4862
        if (b[0] == 0xFE && b[1] == 0x80 && b[2] == 0 && b[3] == 0 
            && b[4] == 0 && b[5] == 0 && b[6] == 0 && b[7] == 0)
        {
            return false;
        }
        //RFC4193
        if ((b[0] & 0xFE) == 0xFC)
        {
            return false;
        }
        //RFC4843
        if (b[0] == 0x20 && b[1] == 0x01 && b[2] == 0x00 && (b[3] & 0xF0) == 0x10)
        {
            return false;
        }
    }
    return true;
}

inline std::string ToHexString(const unsigned char *p,std::size_t size)
{
    const char hexc[17] = "0123456789abcdef";
    char hex[128];
    std::string strHex;
    strHex.reserve(size * 2);

    for (size_t i = 0;i < size;i += 64)
    {
        size_t k;
        for (k = 0;k < 64 && k + i < size;k++)
        {
            int c = *p++;
            hex[k * 2]     = hexc[c >> 4];
            hex[k * 2 + 1] = hexc[c & 15];
        }
        strHex.append(hex,k * 2);
    }
    return strHex;    
}

} // namespace walleve

#endif //WALLEVE_UTIL_H

