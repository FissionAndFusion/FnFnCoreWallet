#ifndef  MPBOX_H
#define  MPBOX_H

#include "mpu256.h"
#include <vector>
#include <map>

class CMPSealedBox;

class CMPOpenedBox
{
public:
    CMPOpenedBox();
    CMPOpenedBox(const std::vector<MPUInt256>& vCoeffIn,const MPUInt256& nPrivKeyIn);
    bool IsNull() const { return (vCoeff.empty() || nPrivKey.IsZero()); }
    bool Validate() const 
    { 
        for (int i = 0;i < vCoeff.size();i++)
            if (vCoeff[i].IsZero())
                return false; 
        return (!vCoeff.empty());
    }
    const MPUInt256 PrivKey() const;
    const MPUInt256 PubKey() const;
    const MPUInt256 SharedKey(const MPUInt256& pubkey) const;
    const MPUInt256 Polynomial(std::size_t nThresh,uint32_t nX) const;
    void Signature(const MPUInt256& hash,const MPUInt256& r,MPUInt256& nR,MPUInt256& nS) const;
    bool VerifySignature(const MPUInt256& hash,const MPUInt256& nR,const MPUInt256& nS) const;
    bool MakeSealedBox(CMPSealedBox& sealed,const MPUInt256& nIdent,const MPUInt256& r) const;
public:
    std::vector<MPUInt256> vCoeff;
    MPUInt256 nPrivKey;
};

class CMPSealedBox
{
public:
    CMPSealedBox();
    CMPSealedBox(const std::vector<MPUInt256>& vEncryptedCoeffIn,const MPUInt256& nPubKeyIn,const MPUInt256& nRIn,const MPUInt256& nSIn);
    bool IsNull() const { return (vEncryptedCoeff.empty() || nPubKey.IsZero()); }
    const MPUInt256 PubKey() const;
    bool VerifySignature(const MPUInt256& nIdent) const;
    bool VerifySignature(const MPUInt256& hash,const MPUInt256& nR,const MPUInt256& nS) const;
    bool VerifyPolynomial(uint32_t nX,const MPUInt256& v);
    void PrecalcPolynomial(std::size_t nThresh,std::size_t nLastIndex);
protected:
    CEdwards25519& CachedEdPoint(const MPUInt256& pubkey);
public:
    std::vector<MPUInt256> vEncryptedCoeff;
    std::vector<MPUInt256> vEncryptedShare;
    MPUInt256 nPubKey;
    MPUInt256 nR;
    MPUInt256 nS;
};

#endif //MPBOX_H
