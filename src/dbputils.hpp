// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_DBP_UTILS_HPP
#define MULTIVERSE_DBP_UTILS_HPP

#include <arpa/inet.h>
#include <cstring>
#include <random>


namespace multiverse{
class CDbpUtils
{
public:
    static uint32_t ParseLenFromMsgHeader(const char* header, int size)
    {
        uint32_t lenNetWorkOrder = 0;
        std::memcpy(&lenNetWorkOrder,header,4);
        return ntohl(lenNetWorkOrder);
    }

    static void WriteLenToMsgHeader(uint32_t len,char* header,int size)
    {
        uint32_t lenNetWorkOrder = htonl(len);
        std::memcpy(header,&lenNetWorkOrder,4);
    }

    static uint64 CurrentUTC()
    {
        boost::posix_time::ptime epoch(boost::gregorian::date(1970, boost::gregorian::Jan, 1));
        boost::posix_time::time_duration time_from_epoch =
        boost::posix_time::second_clock::universal_time() - epoch;
        return time_from_epoch.total_seconds();
    }

    static std::string RandomString()
    {
        static auto& chrs = "0123456789"
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

        thread_local static std::mt19937 rg{std::random_device{}()};
        thread_local static std::uniform_int_distribution<std::string::size_type> pick(0, sizeof(chrs) - 2);

        std::string s;

        int length = 20;

        s.reserve(length);

        while(length--)
            s += chrs[pick(rg)];

        return s;
    }
};
} // namespace multiverse
#endif // MULTIVERSE_DBP_UTILS_HPP