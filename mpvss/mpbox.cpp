#include "mpbox.h"
#include <stdexcept>
#include <string>

using namespace std;

//////////////////////////////
// CMPSealedBox

static inline const MPUInt256 MPEccPubkey(const MPUInt256& s)
{
    MPUInt256 pubkey;
    CEdwards25519 P;
    P.Generate(s);
    P.Pack(pubkey.Data());
    return pubkey;
}

static inline bool MPEccPubkeyValidate(const MPUInt256& pubkey)
{
    CEdwards25519 P;
    return (!pubkey.IsZero() && P.Unpack(pubkey.Data()) && P != CEdwards25519());
}

static inline const MPUInt256 MPEccSign(const MPUInt256& key,const MPUInt256& r,const MPUInt256& hash)
{
    CSC25519 sign = key.ToSC25519() + r.ToSC25519() * hash.ToSC25519();
    return MPUInt256(sign);
}

static inline bool MPEccVerify(const MPUInt256& pubkey,const MPUInt256& rG,const MPUInt256& signature,const MPUInt256& hash)
{
    CEdwards25519 P,R,S;
    if (P.Unpack(pubkey.Data()) && R.Unpack(rG.Data()))
    {
        S.Generate(signature);
        return (S == (P + R.ScalarMult(hash.ToSC25519())));
    }
    return false;
}

static inline const MPUInt256 MPEccSharedKey(const MPUInt256& key,const MPUInt256& other)
{
    MPUInt256 shared;
    CEdwards25519 P;
    if (P.Unpack(other.Data()))
    {
        P.ScalarMult(key.ToSC25519()).Pack(shared.Data());
    }
    return shared;
}

//////////////////////////////
// CMPOpenedBox
CMPOpenedBox::CMPOpenedBox()
{
}

CMPOpenedBox::CMPOpenedBox(const std::vector<MPUInt256>& vCoeffIn)
: vCoeff(vCoeffIn)
{
}

const MPUInt256 CMPOpenedBox::PrivKey() const
{
    if (IsNull())
    {
        throw runtime_error("Box is null");
    }
    return vCoeff[0];
}

const MPUInt256 CMPOpenedBox::SharedKey(const MPUInt256& pubkey) const
{
    return MPEccSharedKey(PrivKey(),pubkey);
}

const MPUInt256 CMPOpenedBox::Polynomial(std::size_t nThresh,uint32_t nX) const
{
    if (IsNull() || nThresh > vCoeff.size())
    {
        throw runtime_error("Box is null or insufficient");
    }

    CSC25519 f = vCoeff[0].ToSC25519();
    CSC25519 x(nX);
    for (size_t i = 1;i < nThresh;i++)
    {
        f += vCoeff[i].ToSC25519() * x;
        x *= nX;
    }
    return MPUInt256(f);
}

//////////////////////////////
// CMPSealedBox

CMPSealedBox::CMPSealedBox()
{
}

CMPSealedBox::CMPSealedBox(const vector<MPUInt256>& vEncryptedCoeffIn,const MPUInt256& nRIn,const MPUInt256& nSIn)
: vEncryptedCoeff(vEncryptedCoeffIn),nR(nRIn),nS(nSIn)
{
}

const MPUInt256 CMPSealedBox::PubKey() const
{
    if (vEncryptedCoeff.empty())
    {
        throw runtime_error("Box is empty");
    }
    return vEncryptedCoeff[0];
}

bool CMPSealedBox::Make(const MPUInt256& nIdent,CMPOpenedBox& opened,const MPUInt256& r)
{
    if (!opened.Validate() || r.IsZero())
    {
        return false;
    } 

    try
    {
        MPUInt256 key = opened.PrivKey();

        vEncryptedCoeff.resize(opened.vCoeff.size());
        for (int i = 0;i < opened.vCoeff.size();i++)
        {
            vEncryptedCoeff[i] = MPEccPubkey(opened.vCoeff[i]);
        }
        nR = MPEccPubkey(r);
        nS = MPEccSign(key,r,nIdent);
        return true;
    }
    catch (...) {}
    return false;
}

bool CMPSealedBox::VerifySignature(const MPUInt256& nIdent) const
{
    if (vEncryptedCoeff.empty())
    {
        return false;
    }
    for (size_t i = 0;i < vEncryptedCoeff.size();i++)
    {
        if (!MPEccPubkeyValidate(vEncryptedCoeff[i]))
        {
            return false;
        }
    }
    return MPEccVerify(vEncryptedCoeff[0],nR,nS,nIdent);
}

bool CMPSealedBox::VerifyPolynomial(uint32_t nX,const MPUInt256& v)
{
    if (nX >= vEncryptedShare.size())
    {
        return false;
    }
    return (vEncryptedShare[nX] == MPEccPubkey(v));
}

void CMPSealedBox::PrecalcPolynomial(size_t nThresh,size_t nLastIndex)
{
    vector<CEdwards25519> vP;
    vP.resize(nThresh);
    vEncryptedShare.resize(nLastIndex);
    for (int i = 0;i < nThresh;i++)
    {
        vP[i].Unpack(vEncryptedCoeff[i].Data());
    }
    
    for (uint32_t nX = 1; nX < nLastIndex; nX++)
    {
        CEdwards25519 P = vP[0];
        CSC25519 x(nX);
        for (size_t i = 1;i < nThresh;i++)
        {
            P += vP[i].ScalarMult(x);
            x *= nX;
        }
        P.Pack(vEncryptedShare[nX].Data());
    }
}

