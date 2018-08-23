#ifndef DBP_UTILS_HPP
#define DBP_UTILS_HPP

#include <arpa/inet.h>
#include <cstdint>
#include <cstring>

class CDbpUtils
{
public:
    static uint32_t parseLenFromMsgHeader(const char* header, int size)
    {
        uint32_t lenNetWorkOrder = 0;
        std::memcpy(&lenNetWorkOrder,header,4);
        return ntohl(lenNetWorkOrder);
    }
};

#endif