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

inline std::string GetLocalTime()
{
using namespace boost::posix_time;
    time_facet *facet = new time_facet("%Y-%m-%d %H:%M:%S");
    std::stringstream ss;
    ss.imbue(std::locale(std::locale("C"), facet));
    ss << second_clock::universal_time();
    return ss.str();
}

inline void StdLog(const char* pszName, const char* pszErr)
{
    std::cerr << GetLocalTime() << " [INFO] <" << pszName << "> " << pszErr << std::endl;
}

inline void StdWarn(const char* pszName, const char* pszErr)
{
    std::cerr << GetLocalTime() << " [WARN] <" << pszName << "> " << pszErr << std::endl;
}

inline void StdError(const char* pszName, const char* pszErr)
{
    std::cerr << GetLocalTime() << " [ERROR] <" << pszName << "> " << pszErr << std::endl;
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

inline std::string ToHexString(const std::vector<unsigned char>& vch)
{
    return ToHexString(&vch[0],vch.size());
}

template<typename T>
inline std::string UIntToHexString(const T& t)
{
    const char hexc[17] = "0123456789abcdef";
    char hex[sizeof(T) * 2 + 1];
    for (std::size_t i = 0;i < sizeof(T);i++)
    {
        int byte = (t >> ((sizeof(T) - i - 1)) * 8) & 0xFF;
        hex[i * 2]     = hexc[byte >> 4];
        hex[i * 2 + 1] = hexc[byte & 15];
    }
    hex[sizeof(T) * 2] = 0;
    return std::string(hex);
}

inline int CharToHex(char c)
{
    if (c >= '0' && c <= '9') return (c - '0');
    if (c >= 'a' && c <= 'f') return (c - 'a' + 10);
    if (c >= 'A' && c <= 'F') return (c - 'A' + 10);
    return -1;
}

inline std::vector<unsigned char> ParseHexString(const char* psz)
{
    std::vector<unsigned char> vch;
    vch.reserve(128);
    while (*psz)
    {
        int h = CharToHex(*psz++);
        int l = CharToHex(*psz++);
        if (h < 0 || l < 0) break;
        vch.push_back((unsigned char)((h << 4) | l));
    }
    return vch; 
}

inline std::size_t ParseHexString(const char* psz,unsigned char* p,std::size_t n)
{
    unsigned char* end = p + n;
    while (*psz && p != end)
    {
        int h = CharToHex(*psz++);
        int l = CharToHex(*psz++);
        if (h < 0 || l < 0) break;
        *p++ = (unsigned char)((h << 4) | l);
    }
    return (n - (end - p));
}

inline std::vector<unsigned char> ParseHexString(const std::string& str)
{
    return ParseHexString(str.c_str());
}

inline std::size_t ParseHexString(const std::string& str,unsigned char* p,std::size_t n)
{
    return ParseHexString(str.c_str(),p,n);
}

#ifdef __GNUG__
#include <cxxabi.h>
inline const char* TypeName(const std::type_info& info)
{
    int status = 0;
    return abi::__cxa_demangle(info.name(), 0, 0, &status);
}
#else
inline const char* TypeName(const std::type_info& info)
{
    return info.name();
}
#endif

} // namespace walleve

#endif //WALLEVE_UTIL_H

