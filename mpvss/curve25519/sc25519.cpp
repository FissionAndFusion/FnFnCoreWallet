// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "sc25519.h"

#include <cstring>
#include <iostream>

#include "base25519.h"

namespace curve25519
{
// 2**252 + 27742317777372353535851937790883648493
const uint64_t CSC25519::prime[4] = {0x5812631a5cf5d3ed, 0x14def9dea2f79cd6, 0 , 0x1000000000000000};

CSC25519::CSC25519()
{
    Zero32(value);
}

CSC25519::CSC25519(const uint64_t u64)
{
    Copy32(value, u64);
}

CSC25519::CSC25519(const uint8_t* md32)
{
    Copy32(value,md32);
    Reduce(0);
}

CSC25519::CSC25519(std::initializer_list<uint64_t> list)
{
    int i = 0;
    for (auto it = list.begin(); it != list.end() && i < 4; i++, it++)
    {
        value[i] = *it;
    }
    for (; i < 4; i++)
    {
        value[i] = 0;
    }
}

CSC25519::~CSC25519()
{
}

void CSC25519::Pack(uint8_t* md32) const
{
    Copy32(md32,value);
}

const uint8_t* CSC25519::Data() const
{
    return (const uint8_t*)value;
}

CSC25519& CSC25519::Negative()
{
    Sub32(value, prime, value);
    return *this;
}

const CSC25519 CSC25519::operator-() const
{
    CSC25519 p;
    p -= *this;
    return p;
}

CSC25519& CSC25519::operator+=(const CSC25519& b)
{
    __uint128_t sum = 0;
    uint64_t carry = 0;
    for (int i = 0;i < 4;i++)
    {
        sum = (__uint128_t)value[i] + b.value[i] + carry;
        carry = (sum >> 64) & 0xFFFFFFFFFFFFFFFF;
        value[i] = sum & 0xFFFFFFFFFFFFFFFF;
    }
    Reduce();
    return *this;
}

CSC25519& CSC25519::operator-=(const CSC25519& b)
{
    __uint128_t sum = 0;
    int carry = 0;
    for (int i = 0;i < 4;i++)
    {
        sum = (__uint128_t)prime[i] + value[i] + (~b.value[i]) + 1 + carry;
        carry = ((sum >> 64) & 0xFFFFFFFFFFFFFFFF) - 1;
        value[i] = sum;
    } 
    Reduce();
    return *this;
}

CSC25519& CSC25519::operator*=(const CSC25519& b)
{
    __uint128_t m[8] = {0};
    Mul32(m, value, b.value);

    uint64_t n[8];
    uint64_t carry = 0;
    for (int i = 0;i < 8;i++)
    {
        m[i] += carry;
        carry = m[i] >> 64;
        n[i] = m[i] & 0xFFFFFFFFFFFFFFFFUL;
    }

    BarrettReduce(n);

    return *this;
}

CSC25519& CSC25519::operator*=(const uint32_t& b)
{
    // m[i], [96, 127] = 0
    __uint128_t m[4] = {0};
    m[0] = (__uint128_t)value[0] * b;
    m[1] = (__uint128_t)value[1] * b;
    m[2] = (__uint128_t)value[2] * b;
    m[3] = (__uint128_t)value[3] * b;

    uint32_t carry = 0;
    for (int i = 0; i < 4; ++i)
    {
        m[i] += carry;
        value[i] = m[i];
        carry = m[i] >> 64;
    }

    // uint32 * uint256, [32, 63] = 0 in n[4]
    Reduce(carry);

    return *this;
}
const CSC25519 CSC25519::operator+(const CSC25519& b) const
{
    return CSC25519(*this) += b;
}

const CSC25519 CSC25519::operator-(const CSC25519& b) const
{
    return CSC25519(*this) -= b;
}

const CSC25519 CSC25519::operator*(const CSC25519& b) const
{
    return CSC25519(*this) *= b;
}

const CSC25519 CSC25519::operator*(const uint32_t& n) const
{
    return CSC25519(*this) *= n;
}

bool CSC25519::operator==(const CSC25519& b) const
{
    return Compare32(value, b.value) == 0;
}

bool CSC25519::operator!=(const CSC25519& b) const
{
    return !(*this == b);
}

void CSC25519::Reduce()
{
    if (Compare32(value, prime) >= 0)
    {
        Sub32(value, value, prime);
    }
}

void CSC25519::Reduce(const uint32_t carry)
{
    // 2**252 % prime = -27742317777372353535851937790883648493
    static const uint64_t reminder[2] = {0x5812631a5cf5d3ed, 0x14def9dea2f79cd6};
    // p - n*reminder, n = [0, 15]
    static const uint64_t reminders[16][4] = {
        {0, 0, 0, 0},
        {0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x1000000000000000},
        {0xa7ed9ce5a30a2c13, 0xeb2106215d086329, 0xffffffffffffffff, 0x0fffffffffffffff},
        {0x4fdb39cb46145826, 0xd6420c42ba10c653, 0xffffffffffffffff, 0x0fffffffffffffff},
        {0xf7c8d6b0e91e8439, 0xc16312641719297c, 0xffffffffffffffff, 0x0fffffffffffffff},
        {0x9fb673968c28b04c, 0xac84188574218ca6, 0xffffffffffffffff, 0x0fffffffffffffff},
        {0x47a4107c2f32dc5f, 0x97a51ea6d129efd0, 0xffffffffffffffff, 0x0fffffffffffffff},
        {0xef91ad61d23d0872, 0x82c624c82e3252f9, 0xffffffffffffffff, 0x0fffffffffffffff},
        {0x977f4a4775473485, 0x6de72ae98b3ab623, 0xffffffffffffffff, 0x0fffffffffffffff},
        {0x3f6ce72d18516098, 0x5908310ae843194d, 0xffffffffffffffff, 0x0fffffffffffffff},
        {0xe75a8412bb5b8cab, 0x4429372c454b7c76, 0xffffffffffffffff, 0x0fffffffffffffff},
        {0x8f4820f85e65b8be, 0x2f4a3d4da253dfa0, 0xffffffffffffffff, 0x0fffffffffffffff},
        {0x3735bdde016fe4d1, 0x1a6b436eff5c42ca, 0xffffffffffffffff, 0x0fffffffffffffff},
        {0xdf235ac3a47a10e4, 0x058c49905c64a5f3, 0xffffffffffffffff, 0x0fffffffffffffff},
        {0x8710f7a947843cf7, 0xf0ad4fb1b96d091d, 0xfffffffffffffffe, 0x0fffffffffffffff},
        {0x2efe948eea8e690a, 0xdbce55d316756c47, 0xfffffffffffffffe, 0x0fffffffffffffff},
    };

    if (carry == 0)
    {

        if (Compare32(value, prime) >= 0)
        {
            uint8_t t = ((uint8_t*)value)[31] >> 4;
            if (t == 1)
            {
                Sub32(value, value, prime);
            }
            else
            {
                ((uint8_t*)value)[31] &= 0x0f;
                Add32(value, value, reminders[t]);
                if (Compare32(value, prime) >= 0)
                {
                    Sub32(value, value, prime);
                }
            }
        }
    }
    else
    {
        // n % prime = p - (carry * 27742317777372353535851937790883648493) + n[0, 251] (then reduce once at most)
        uint64_t c = (carry << 4) + (value[3] >> 60);

        uint64_t product[4];
        __uint128_t product0 = (__uint128_t)reminder[0] * c;
        __uint128_t product1 = (__uint128_t)reminder[1] * c;

        product[0] = product0 & 0xffffffffffffffff;
        __uint128_t sum = (product0 >> 64) + (product1 & 0xffffffffffffffff);
        product[1] = sum & 0xffffffffffffffff;
        product[2] = (sum >> 64) + (product1 >> 64);
        product[3] = 0;

        Sub32(product, prime, product);
        value[3] &= 0x0fffffffffffffff;
        Add32(value, product, value);

        Reduce();
    }
}

void CSC25519::BarrettReduce(uint64_t* m)
{
    // mu = 2^506 / prime
    static const uint64_t mu[4] = {0x9fb673968c28b04c, 0xac84188574218ca6,
                                   0xffffffffffffffff, 0x3fffffffffffffff};
    
    uint64_t r1[4], r2[4];
    __uint128_t product[8] = {0};

    // r1 = m % 2^254
    Copy32(r1, m);
    r1[3] &= 0x3fffffffffffffff;

    // r2 = ((((m / 2^252) * mu) / 2^254) * prime) % 2^254
    // q1 = m / 2^252
    ShiftLeft32(r2, m + 4, 4);
    r2[0] |= m[3] >> 60;

    // q2 = q1 * mu
    Mul32(product, r2, mu);
    for (int i = 1;i < 8;i++)
    {
        product[i] += product[i-1] >> 64;
        product[i-1] &= 0xffffffffffffffff;
    }

    // q3 = q1 / 2^254
    r2[0] = (product[4] << 2) | (product[3] >> 62);
    r2[1] = (product[5] << 2) | (product[4] >> 62);
    r2[2] = (product[6] << 2) | (product[5] >> 62);
    r2[3] = (product[7] << 2) | (product[6] >> 62);

    // r2 = q3 * prime % 2^254
    Zero64(product);
    __uint128_t p;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; i + j < 4; j++)
        {
            p = (__uint128_t)r2[i] * prime[j];
            product[i + j] += p & 0xFFFFFFFFFFFFFFFF;
            product[i + j + 1] += (p >> 64) & 0xFFFFFFFFFFFFFFFF;
        }
    }

    r2[0] = product[0];
    product[1] += product[0] >> 64;
    r2[1] = product[1];
    product[2] += product[1] >> 64;
    r2[2] = product[2];
    r2[3] = product[3] + (product[2] >> 64);
    r2[3] &= 0x3fffffffffffffff;

    if (Compare32(r1, r2) < 0)
    {
        r1[3] |= 0x4000000000000000;
    }

    Sub32(value, r1, r2);

    Reduce();
    Reduce();
}

} // namespace curve25519
