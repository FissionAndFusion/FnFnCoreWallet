#include "curve25519.h"

//////////////////////////////
// 
static inline void ZeroMD32(void* md32)
{
    uint32_t* p = (uint32_t*)md32;
    p[0] = 0; p[1] = 0; p[2] = 0; p[3] = 0;
    p[4] = 0; p[5] = 0; p[6] = 0; p[7] = 0;
}

static inline void CopyMD32(void* dest,const void* src)
{
    uint32_t* p = (uint32_t*)dest;
    const uint32_t* s = (const uint32_t*)src;
    p[0] = s[0]; p[1] = s[1]; p[2] = s[2]; p[3] = s[3];
    p[4] = s[4]; p[5] = s[5]; p[6] = s[6]; p[7] = s[7];
}

//////////////////////////////
// CFP25519
const uint32_t CFP25519::prime[8] = {0xFFFFFFED,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
                                     0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0x7FFFFFFF};

CFP25519::CFP25519()
{
    ZeroMD32(value);
}

CFP25519::CFP25519(const uint32_t u32)
{
    ZeroMD32(value);
    value[0] = u32;
}

CFP25519::CFP25519(const uint8_t* md32)
{
    CopyMD32(value,md32);
    value[7]  &= 0x7FFFFFFFUL;
}

CFP25519::~CFP25519()
{
}

void CFP25519::Pack(uint8_t* md32) const
{
    CopyMD32(md32,value);

    uint64_t* u = (uint64_t*)md32;

    if (u[3] == 0x7FFFFFFFFFFFFFFFULL && u[2] == 0xFFFFFFFFFFFFFFFFULL
            && u[1] == 0xFFFFFFFFFFFFFFFFULL && u[0] >= 0xFFFFFFFFFFFFFFEDULL)
    {
        u[0] -= 0xFFFFFFFFFFFFFFEDULL;
        u[1] = u[2] = u[3] = 0;
    }
}

const CFP25519 CFP25519::Inverse() const
{
    CFP25519 z2;
    CFP25519 z9;
    CFP25519 z11;
    CFP25519 z2_5_0;
    CFP25519 z2_10_0;
    CFP25519 z2_20_0;
    CFP25519 z2_50_0;
    CFP25519 z2_100_0;
    CFP25519 r;
    
    /* 2 */ z2 = *this; z2.Square();
    /* 9 */ z9 = z2; z9.Square(); z9.Square(); z9 *= *this;
    /* 11 */ z11 = z9 * z2;
    /* 2^5 - 2^0 = 31 */ z2_5_0 = z11; z2_5_0.Square(); z2_5_0 *= z9;
    /* 2^10 - 2^0 */ z2_10_0 = z2_5_0;
                     for (int i = 0;i < 5;i++) z2_10_0.Square();
                     z2_10_0 *= z2_5_0;
    /* 2^20 - 2^0 */ z2_20_0 = z2_10_0; 
                     for (int i = 0;i < 10;i++) z2_20_0.Square();
                     z2_20_0 *= z2_10_0;
    /* 2^50 - 2^0 */ z2_50_0 = z2_20_0; 
                     for (int i = 0;i < 20;i++) z2_50_0.Square();
                     z2_50_0 *= z2_20_0;
                     for (int i = 0;i < 10;i++) z2_50_0.Square();
                     z2_50_0 *= z2_10_0;
    /* 2^100 - 2^0 */ z2_100_0 = z2_50_0;
                      for (int i = 0;i < 50;i++) z2_100_0.Square();
                      z2_100_0 *= z2_50_0;
    /* 2^250 - 2^0 */ r = z2_100_0;
                      for (int i = 0;i < 100;i++) r.Square();
                      r *= z2_100_0;
                      for (int i = 0;i < 50;i++) r.Square();
                      r *= z2_50_0;                   
    /* 2^255 - 2^5 */ for (int i = 0;i < 5;i++) r.Square();
    /* 2^255 - 21 */  r *= z11;
    return r;
}

const CFP25519 CFP25519::Power(const uint8_t* md32) const
{
// WINDOWSIZE = 4;
    const CFP25519& x = *this;
    CFP25519 pre[16];
    pre[0] = CFP25519(1);
    pre[1] = x;
    for (int i = 2;i < 16; i += 2)
    {
        pre[i] = pre[i >> 1];pre[i].Square();
        pre[i + 1] = pre[i] * x;
    }
    
    CFP25519 r(1);
    for (int i = 31;i >= 0;i--)
    {
        r.Square();r.Square();r.Square();r.Square();
        r *= pre[(md32[i] >> 4)];
        r.Square();r.Square();r.Square();r.Square();
        r *= pre[(md32[i] & 15)];
    }
    return r;
}

const CFP25519 CFP25519::Sqrt() const
{
    const uint8_t e[32] = {0xfd,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                           0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x0f}; /* (p-5)/8 */
    const uint8_t b[32] = {0xb0,0xa0,0x0e,0x4a,0x27,0x1b,0xee,0xc4,0x78,0xe4,0x2f,0xad,0x06,0x18,0x43,0x2f,
                           0xa7,0xd7,0xfb,0x3d,0x99,0x00,0x4d,0x2b,0x0b,0xdf,0xc1,0x4f,0x80,0x24,0x83,0x2b}; /* sqrt(-1) */

    if (!IsZero())
    {
        CFP25519 z58,z38,z14,z12;
        z58 = Power(e);
        z38 = z58; z38 *= *this;
        z14 = z58; z14 *= z38;
        z12 = z14; z12.Square();
    
        if (z12.IsOne())
        {
            return (z14.IsOne() ? z38 : z38 * CFP25519(b));
        }
    }
    return CFP25519();
}

CFP25519& CFP25519::Square()
{
    uint64_t product = 0;
    uint64_t m[16] = {0,};
    for (int i = 0;i < 8;i++)
    {
        product = (uint64_t)value[i] * value[i];
        m[i + i] += product & 0xFFFFFFFF;
        m[i + i + 1] += product >> 32;
            
        for (int j = i + 1;j < 8;j++)
        {
            product = (uint64_t)value[i] * value[j];
            m[i + j] += (product & 0x7FFFFFFF) << 1;
            m[i + j + 1] += product >> 31;
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

//////////////////////////////
// CSC25519
const uint32_t CSC25519::prime[8] = {0x5CF5D3ED,0x5812631A,0xA2F79CD6,0x14DEF9DE,0,0,0,0x10000000};

CSC25519::CSC25519()
{
    ZeroMD32(value);
}

CSC25519::CSC25519(const uint32_t u32)
{
    ZeroMD32(value);
    value[0] = u32;
}

CSC25519::CSC25519(const uint8_t* md32)
{
    if (md32[31] & 0xC0) 
    {
         uint64_t m[8];
         for (int i = 0;i < 8;i++) 
         {
             m[i] = *(const uint32_t*)(md32 + 4 * i);
         } 
         BarrettReduce(m,8);
    }
    else
    {
        CopyMD32(value,md32);
        Reduce();
    }
}

CSC25519::~CSC25519()
{
}

void CSC25519::Pack(uint8_t* md32) const
{
    CopyMD32(md32,value);
}

//////////////////////////////
// CEdwards25519

static const uint8_t _ecd[32] = {0xA3,0x78,0x59,0x13,0xCA,0x4D,0xEB,0x75,0xAB,0xD8,0x41,0x41,0x4D,0x0A,0x70,0x00,
                                 0x98,0xE8,0x79,0x77,0x79,0x40,0xC7,0x8C,0x73,0xFE,0x6F,0x2B,0xEE,0x6C,0x03,0x52};
static const uint8_t _base_x[32] = {0x1A,0xD5,0x25,0x8F,0x60,0x2D,0x56,0xC9,0xB2,0xA7,0x25,0x95,0x60,0xC7,0x2C,0x69, 
                                    0x5C,0xDC,0xD6,0xFD,0x31,0xE2,0xA4,0xC0,0xFE,0x53,0x6E,0xCD,0xD3,0x36,0x69,0x21};
static const uint8_t _base_y[32] = {0x58,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,
                                    0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66};

const CFP25519 CEdwards25519::ecd = CFP25519(_ecd);

const CEdwards25519 CEdwards25519::base = CEdwards25519(CFP25519(_base_x),CFP25519(_base_y),true);

CEdwards25519::CEdwards25519()
: fX(CFP25519()),fY(CFP25519(1)),fZ(CFP25519(1)),fT(CFP25519())
{
}

CEdwards25519::CEdwards25519(const CFP25519& x,const CFP25519& y,bool fPrescalar)
: fX(x),fY(y),fZ(CFP25519(1)),fT(fX * fY)
{
    if (fPrescalar)
    {
        CalcPrescalar();
    }
}

CEdwards25519::CEdwards25519(const CFP25519& x,const CFP25519& y,const CFP25519& z)
: fX(x),fY(y),fZ(z),fT(fX * fY)
{
}

CEdwards25519::CEdwards25519(const CFP25519& x,const CFP25519& y,const CFP25519& z,const CFP25519& t)
: fX(x),fY(y),fZ(z),fT(t)
{
}

CEdwards25519::~CEdwards25519()
{
}

bool CEdwards25519::Unpack(const uint8_t* md32)
{
    fZ = CFP25519(1);
    fY = CFP25519(md32);
    CFP25519 y2 = CFP25519(md32).Square();
    CFP25519 x2 = (y2 - fZ) / (y2 * ecd + fZ);
    fX = x2.Sqrt();
    if (fX.IsZero())
    {
        fT = CFP25519();
        return x2.IsZero();
    }

    if (fX.Parity() != (md32[31] >> 7))
    {
        fX = -fX;
    }
    fT = fX * fY;
    return true;
}

void CEdwards25519::Pack(uint8_t* md32) const
{
    CFP25519 zi = fZ.Inverse();
    CFP25519 tx = fX * zi;
    CFP25519 ty = fY * zi;
    ty.Pack(md32);
    md32[31] ^= tx.Parity() << 7;
}

CEdwards25519& CEdwards25519::Add(const CEdwards25519& q)
{
    CFP25519 a,b,c,d;
    a = (fY - fX) * (q.fY - q.fX);
    b = (fY + fX) * (q.fY + q.fX);
    c = fT * q.fT * ecd; c += c;
    d = fZ * q.fZ; d += d;
    
    FromP1P1(b - a,b + a,d + c,d - c);
    return *this;
}

CEdwards25519& CEdwards25519::Double()
{
    CFP25519 a,b,c,d,e,g;
    a = fX;a.Square();
    b = fY;b.Square();
    c = fZ;c.Square();c += c;
    d = -a;
    e = fX + fY;e.Square();
    g = d + b;

    FromP1P1(e - a - b,d - b,g,g - c);
    return *this;
}

void CEdwards25519::AddPrescalar(const CEdwards25519& q)
{
    CFP25519 a,b,c,d,e;
    CFP25519 x,y,z,t;
    // Add
    {
        a = (fY - fX) * (q.fY - q.fX);
        b = (fY + fX) * (q.fY + q.fX);
        c = fT * q.fT * ecd; c += c;
        d = fZ * q.fZ; d += d;
        x = b - a; y = b + a; z = d + c; t = d - c;

        fX = x * t;
        fY = y * z;
        fZ = z * t;
    }
    // Double 4
    for (int i = 0;i < 4;i++)
    {
        a = fX;a.Square();
        b = fY;b.Square();
        c = fZ;c.Square();c += c;
        d = -a;
        e = fX + fY;e.Square();
        x = e - a - b; y = d - b; z = d + b; t = z - c;
        
        fX = x * t;
        fY = y * z;
        fZ = z * t;
    }
    fT = x * y;
}

const CEdwards25519 CEdwards25519::ScalarMult(const uint8_t* u8,std::size_t size) const
{
    if (preScalar.size() != 16 || preScalar[1].fX != fX || preScalar[1].fY != fY)
    {
        CalcPrescalar();
    }

    CEdwards25519 r;
    int i;
    for (i = size - 1;i > 0 && u8[i] == 0;i--);
    for (;i > 0;i--)
    {
        r.AddPrescalar(preScalar[u8[i] >> 4]);
        r.AddPrescalar(preScalar[u8[i] & 15]);
    }
    r.AddPrescalar(preScalar[u8[0] >> 4]);
    r.Add(preScalar[u8[0] & 15]);

    return r;
}

void CEdwards25519::FromP1P1(const CFP25519& x,const CFP25519& y,const CFP25519& z,const CFP25519& t)
{
    fX = x * t;
    fY = y * z;
    fZ = z * t;
    fT = x * y;
}

void CEdwards25519::CalcPrescalar() const
{
    preScalar.resize(16);
    preScalar[0] = CEdwards25519();
    preScalar[1] = CEdwards25519(fX,fY,fZ,fT);
    for (int i = 2;i < 16;i += 2)
    {
        preScalar[i] = preScalar[i >> 1]; preScalar[i].Double();
        preScalar[i + 1] = preScalar[i]; preScalar[i + 1].Add(*this);
    } 
}


