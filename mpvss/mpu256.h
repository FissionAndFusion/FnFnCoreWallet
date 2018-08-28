#ifndef  MPU256_H
#define  MPU256_H

#include "curve25519.h"
#include <walleve/stream/datastream.h>

#include <stdint.h>
#include <string>

class MPUInt256
{
public:
    MPUInt256() { u64[0] = u64[1] = u64[2] = u64[3] = 0; }
    MPUInt256(uint32_t u32) { u64[0] = u32; u64[1] = u64[2] = u64[3] = 0; }
    MPUInt256(const std::string& hex) { SetHex(hex); }
    MPUInt256(const CSC25519& s) { s.Pack(Data()); }
    uint8_t* Data() { return ((uint8_t*)u64); }
    const uint8_t* Data() const { return ((const uint8_t*)u64); }
    const CSC25519 ToSC25519() const { return CSC25519(Data()); }
    bool IsZero() const { return (u64[0] == 0 && u64[1] == 0 && u64[2] == 0 && u64[3] == 0); }
    bool operator==(const MPUInt256& b) const { return (u64[0] == b.u64[0] && u64[1] == b.u64[1]
                                                        && u64[2] == b.u64[2] && u64[3] == b.u64[3]); }
    bool operator!=(const MPUInt256& b) const { return (!(*this == b)); }    
    bool operator<(const MPUInt256& b) const 
    {
        return (u64[3] < b.u64[3] 
                || (u64[3] == b.u64[3] && (u64[2] < b.u64[2]
                                           || (u64[2] == b.u64[2] && (u64[1] < b.u64[1]
                                                                      || (u64[1] == b.u64[1] && u64[0] < b.u64[0]))))));
    }
    const MPUInt256 operator^(const MPUInt256& b) const
    {
        MPUInt256 c;
        c.u64[0] = u64[0]^b.u64[0]; c.u64[1] = u64[1]^b.u64[1];
        c.u64[2] = u64[2]^b.u64[2]; c.u64[3] = u64[3]^b.u64[3];
        return c;
    }
    void SetHex(const std::string& hex)
    {
        u64[0] = u64[1] = u64[2] = u64[3] = 0;
        
        uint8_t* d = Data();
        int n = 0;
        for (int i = hex.size() - 1; i >= 0 && n < 64; i--,n++)
        {
            int c = hex[i];
            int v = -1;
            if (c >= '0' && c <= '9') v = c - '0';
            else if (c >= 'a' && c <= 'f') v = c - 'a' + 10;
            else if (c >= 'A' && c <= 'F') v = c - 'A' + 10;
            else break;
            
            d[n >> 1] += v << ((n & 1) << 2);
        }
    }
    std::string ToHex() const
    {
        const char chex[17] = "0123456789abcdef";
        const uint8_t* p = Data() + 31;
        std::string s;
        s.reserve(32);
        for (int i = 0;i < 32;i++)
        {
            s.push_back(chex[(*p) >> 4]);
            s.push_back(chex[(*p--) & 15]);
        }
        return s;
    }
    void ToDataStream(walleve::CWalleveODataStream& os) const
    {
        os.Push(Data(),sizeof(u64));
    }
    void FromDataStream(walleve::CWalleveIDataStream& is)
    {
        is.Pop(Data(),sizeof(u64));
    }
public:
    uint64_t u64[4];
};

#endif //MPU256_H
