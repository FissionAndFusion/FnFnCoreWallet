/*
 * Curve 25519 C++ implementation
 * Derived from public domain C code by Daniel J. Bernstein <djb@cr.yp.to>
 * More information can be found : http://cr.yp.to/ecdh.html
 */

#ifndef CURVE25519_H
#define CURVE25519_H

#include <stdint.h>
#include <vector>

class CFP25519
{
public:
    CFP25519();
    CFP25519(const uint32_t u32);
    CFP25519(const uint8_t* md32);
    virtual ~CFP25519();
    void Pack(uint8_t* md32) const; 
    const CFP25519 Inverse() const;
    const CFP25519 Power(const uint8_t* md32) const;
    const CFP25519 Sqrt() const;
    bool IsZero() const
    {
        uint64_t* u = (uint64_t*)value;
        return ((u[3] == 0 && u[2] == 0 && u[1] == 0 && u[0] == 0)
                || (u[3] == 0x7FFFFFFFFFFFFFFFULL && u[2] == 0xFFFFFFFFFFFFFFFFULL 
                    && u[1] == 0xFFFFFFFFFFFFFFFFULL && u[0] == 0xFFFFFFFFFFFFFFEDULL));
    }
    bool IsOne() const
    {
        uint64_t* u = (uint64_t*)value;
        return ((u[3] == 0 && u[2] == 0 && u[1] == 0 && u[0] == 1)
                || (u[3] == 0x7FFFFFFFFFFFFFFFULL && u[2] == 0xFFFFFFFFFFFFFFFFULL 
                    && u[1] == 0xFFFFFFFFFFFFFFFFULL && u[0] == 0xFFFFFFFFFFFFFFEEULL));
    }
    uint8_t Parity() const
    {
        uint64_t* u = (uint64_t*)value;
        bool over = (u[3] == 0x7FFFFFFFFFFFFFFFULL && u[2] == 0xFFFFFFFFFFFFFFFFULL 
                      && u[1] == 0xFFFFFFFFFFFFFFFFULL && u[0] >= 0xFFFFFFFFFFFFFFEDULL);
        return ((uint8_t)((u[0] ^ over) & 1));
    }
    const CFP25519 operator-() const
    {
        CFP25519 p;
        p -= *this;
        return p;
    }
    CFP25519& operator+=(const CFP25519& b)
    {
        uint64_t sum = 0;
        uint32_t carry = 0;
        for (int i = 0;i < 8;i++)
        {
            sum = (uint64_t)value[i] + b.value[i] + carry;
            carry = sum >> 32;value[i] = sum;
        }
        Range(carry);
        return *this;
    }
    CFP25519& operator-=(const CFP25519& b)
    {
        uint64_t sum = 0;
        uint32_t carry = 0;
        for (int i = 0;i < 8;i++)
        {
            sum = (((uint64_t)prime[i]) << 1) + value[i] - b.value[i] + carry;
            carry = sum >> 32;value[i] = sum;
        }
        Range(carry);
        return *this;
    }
    CFP25519& operator*=(const CFP25519& b)
    {
        uint64_t product = 0;
        uint64_t m[16] = {0,};
        for (int i = 0;i < 8;i++)
        {
            for (int j = 0;j < 8;j++)
            {
                product = (uint64_t)value[i] * b.value[j];
                m[i + j] += product & 0xFFFFFFFF;
                m[i + j + 1] += product >> 32;
            }
        }

        uint32_t carry = 0;
        for (int i = 0;i < 8;i++)
        {
            m[i] += carry + m[i + 8] * 38;
            carry =  m[i] >> 32; value[i] = m[i];
        }
        Range(carry);
        return *this;
    }
    CFP25519& Square();
    const CFP25519 operator+(const CFP25519& b) const { return CFP25519(*this) += b; }
    const CFP25519 operator-(const CFP25519& b) const { return CFP25519(*this) -= b; }
    const CFP25519 operator*(const CFP25519& b) const { return CFP25519(*this) *= b; }
    const CFP25519 operator/(const CFP25519& b) const { return CFP25519(*this) *= b.Inverse(); }
    bool operator==(const CFP25519& b) const {return (CFP25519(*this) -= b).IsZero();} 
    bool operator!=(const CFP25519& b) const {return (!(*this == b));} 
protected:
    void Range(uint32_t carry)
    {
        while ((carry = ((carry << 1) + (value[7] >> 31)) * 19))
        {
            value[7] &= 0x7FFFFFFF;
            for (int i = 0;i < 8;i++)
            {
                uint64_t sum = (uint64_t)value[i] + carry;
                carry = sum >> 32;value[i] = sum;
            }
        }
    }
    void Reduce()
    {
        uint64_t* u = (uint64_t*)value;
        if (u[3] == 0x7FFFFFFFFFFFFFFFULL && u[2] == 0xFFFFFFFFFFFFFFFFULL 
            && u[1] == 0xFFFFFFFFFFFFFFFFULL && u[0] >= 0xFFFFFFFFFFFFFFEDULL)
        {
            u[0] -= 0xFFFFFFFFFFFFFFEDULL;
            u[1] = u[2] = u[3] = 0;
        }
    }
public:
    uint32_t value[8];
protected:
    static const uint32_t prime[8]; 
};

class CSC25519
{
public:
    CSC25519();
    CSC25519(const uint32_t u32);
    CSC25519(const uint8_t* md32);
    virtual ~CSC25519();
    void Pack(uint8_t* md32) const; 
    const uint8_t* Data() const
    {
        return (const uint8_t*)value;
    }
    const CSC25519 operator-() const
    {
        CSC25519 p;
        p -= *this;
        return p;
    }
    CSC25519& operator+=(const CSC25519& b)
    {
        uint64_t sum = 0;
        uint32_t carry = 0;
        for (int i = 0;i < 8;i++)
        {
            sum = (uint64_t)value[i] + b.value[i] + carry;
            carry = sum >> 32;value[i] = sum;
        }
        Reduce();
        return *this;
    }
    CSC25519& operator-=(const CSC25519& b)
    {
        uint64_t sum = 0;
        int carry = 0;
        for (int i = 0;i < 8;i++)
        {
            sum = (uint64_t)prime[i] + value[i] + (~b.value[i]) + 1 + carry;
            carry = (sum >> 32) - 1;value[i] = sum;
        } 
        Reduce();
        return *this;
    }
    CSC25519& operator*=(const CSC25519& b)
    {
        uint64_t product = 0;
        uint64_t m[16] = {0,};
        for (int i = 0;i < 8;i++)
        {
            for (int j = 0;j < 8;j++)
            {
                product = (uint64_t)value[i] * b.value[j];
                m[i + j] += product & 0xFFFFFFFFUL;
                m[i + j + 1] += product >> 32;
            }
        }

        uint32_t carry = 0;
        for (int i = 0;i < 16;i++)
        {
            m[i] += carry; carry =  m[i] >> 32; m[i] &= 0xFFFFFFFFUL;
        }
        BarrettReduce(m,16);

        return *this;
    }        
    CSC25519& operator*=(const uint32_t& n)
    {
        uint64_t product = 0;
        uint64_t m[9] = {0,};
        uint32_t carry = 0;
        for (int i = 0;i < 8;i++)
        {
            product = (uint64_t)value[i] * n + carry;
            m[i] = product & 0xFFFFFFFFUL; 
            carry = product >> 32;
        }
        m[8] = carry;

        BarrettReduce(m,9);
        return *this;
    }
    const CSC25519 operator+(const CSC25519& b) const { return CSC25519(*this) += b; }
    const CSC25519 operator-(const CSC25519& b) const { return CSC25519(*this) -= b; }
    const CSC25519 operator*(const CSC25519& b) const { return CSC25519(*this) *= b; }
    const CSC25519 operator*(const uint32_t& n) const { return CSC25519(*this) *= n; }
protected:
    void Reduce()
    {
        uint32_t d[8];
        uint64_t diff,borrow = 0;
        for (int i = 0;i < 8;i++)
        {
            diff = value[i] + (~((uint64_t)prime[i])) + 1 - borrow;
            borrow = diff >> 63; d[i] = diff & 0xFFFFFFFFUL;
        }
        if (!borrow)
        {
            for (int i = 0;i < 8;i++)
            {
                value[i] = d[i];
            }
        }
    }
    void BarrettReduce(uint64_t *m,std::size_t count)
    { 
        // barrett_reduce
        const uint32_t mu[9] = {0x0A2C131B,0xED9CE5A3,0x086329A7,0x2106215D,
                                0xFFFFFFEB,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0x0F};
        uint64_t product = 0;
        uint64_t q2[18] = {0,};
        int n = count + 2;
        for (int i = 0;i < count - 7;i++)
        {
            for (int j = 0;j < 9;j++)
            {
                product = m[i + 7] * mu[j];
                q2[i + j] += product & 0xFFFFFFFFUL;
                q2[i + j + 1] += product >> 32;
            }
        }
        uint32_t carry = 0;
        for (int i = 0;i < n;i++)
        {
            q2[i] += carry; carry =  q2[i] >> 32; q2[i] &= 0xFFFFFFFFUL;
        }
         
        uint64_t r2[10] = {0,};
        for (int i = 0;i < 9 && i + 9 < n;i++)
        {
            for (int j = 0;j < 8 && i + j < 9;j++)
            {
                product = q2[i + 9] * prime[j];
                r2[i + j] += product & 0xFFFFFFFFUL;
                r2[i + j + 1] += product >> 32;
            }
        }        

        uint64_t diff,borrow = 0;
        carry = 0;
        for (int i = 0;i < 8;i++)
        {
            r2[i] += carry; carry = r2[i] >> 32; r2[i] &= 0xFFFFFFFFUL;
            diff = m[i] + (~(r2[i] & 0xFFFFFFFFUL)) + 1 - borrow;
            borrow = diff >> 63; value[i] = diff & 0xFFFFFFFFUL;
        }
        Reduce();
        Reduce();
    }
public:
    uint32_t value[8];
protected:
    static const uint32_t prime[8];
};

class CEdwards25519
{
public:
    CEdwards25519();
    CEdwards25519(const CFP25519& x,const CFP25519& y,bool fPrescalar = false);
    CEdwards25519(const CFP25519& x,const CFP25519& y,const CFP25519& z);
    CEdwards25519(const CFP25519& x,const CFP25519& y,const CFP25519& z,const CFP25519& t);
    ~CEdwards25519();
    template <typename T>
    void Generate(const T& t)
    {
        *this = base.ScalarMult(t);
    }
    bool Unpack(const uint8_t* md32);
    void Pack(uint8_t* md32) const;
    const CEdwards25519 ScalarMult(const uint8_t* u8,std::size_t size) const;
    template <typename T>
    const CEdwards25519 ScalarMult(const T& t) const
    {
        return ScalarMult((const uint8_t*)&t,sizeof(T));
    }
    const CEdwards25519 ScalarMult(const CSC25519& s) const
    {
        return ScalarMult((const uint8_t*)s.value,32);
    }
    const CEdwards25519 operator-() const { return CEdwards25519(-fX,fY,fZ,-fT); }
    CEdwards25519& operator+=(const CEdwards25519& q) { return Add(q); }
    CEdwards25519& operator-=(const CEdwards25519& q) { return Add(-q); }
    const CEdwards25519 operator+(const CEdwards25519& q) const { return (CEdwards25519(*this).Add(q)); }
    const CEdwards25519 operator-(const CEdwards25519& q) const { return (CEdwards25519(*this).Add(-q)); }
    bool operator==(const CEdwards25519& q) const { return (fX * q.fZ == q.fX * fZ && fY * q.fZ == q.fY * fZ); }
    bool operator!=(const CEdwards25519& q) const { return (!(*this == q)); }
protected:
    CEdwards25519& Add(const CEdwards25519& q);
    CEdwards25519& Double();
    void FromP1P1(const CFP25519& x,const CFP25519& y,const CFP25519& z,const CFP25519& t);
    void CalcPrescalar() const;
    void AddPrescalar(const CEdwards25519& q);
public:
    CFP25519 fX;
    CFP25519 fY;
    CFP25519 fZ;
    CFP25519 fT;
protected:
    mutable std::vector<CEdwards25519> preScalar;
    static const CFP25519 ecd;
    static const CEdwards25519 base;
};

#endif //CURVE25519_H
