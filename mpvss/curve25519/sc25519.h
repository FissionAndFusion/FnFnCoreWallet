// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SC25519_H
#define SC25519_H

#include <cstdint>
#include <initializer_list>

namespace curve25519
{

class CSC25519
{
public:
    CSC25519();
    CSC25519(const uint64_t u64);
    CSC25519(const uint8_t* md32);
    CSC25519(std::initializer_list<uint64_t> list);
    virtual ~CSC25519();
    void Pack(uint8_t* md32) const; 
    const uint8_t* Data() const;

    CSC25519& Negative();
    const CSC25519 operator-() const;
    CSC25519& operator+=(const CSC25519& b);
    CSC25519& operator-=(const CSC25519& b);
    CSC25519& operator*=(const CSC25519& b);
    CSC25519& operator*=(const uint32_t& n);
    const CSC25519 operator+(const CSC25519& b) const;
    const CSC25519 operator-(const CSC25519& b) const;
    const CSC25519 operator*(const CSC25519& b) const;
    const CSC25519 operator*(const uint32_t& n) const;
    bool operator==(const CSC25519& b) const;
    bool operator!=(const CSC25519& b) const;
protected:
    void Reduce();
    void Reduce(const uint32_t carry);
    void BarrettReduce(uint64_t* m);

public:
    uint64_t value[4];

    static const CSC25519 naturalPowTable[27][27];

protected:
    static const uint64_t prime[4];
};

} // namespace curve25519

#endif  // SC25519_H