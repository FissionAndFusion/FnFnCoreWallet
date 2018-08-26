#ifndef DBP_UTILS_HPP
#define DBP_UTILS_HPP

#include <arpa/inet.h>
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

    static void writeLenToMsgHeader(uint32_t len,char* header,int size)
    {
        uint32_t lenNetWorkOrder = htonl(len);
        std::memcpy(header,&lenNetWorkOrder,4);
    }
};

#endif