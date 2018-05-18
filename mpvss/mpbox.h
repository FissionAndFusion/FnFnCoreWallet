#ifndef  MPBOX_H
#define  MPBOX_H

#include "mpu256.h"
#include <vector>
#include <map>

class CMPOpenedBox
{
public:
    CMPOpenedBox();
    CMPOpenedBox(const std::vector<MPUInt256>& vCoeffIn);
    bool IsNull() const { return (vCoeff.empty() || vCoeff[0].IsZero()); }
    bool Validate() const 
    { 
        for (int i = 0;i < vCoeff.size();i++)
            if (vCoeff[i].IsZero())
                return false; 
        return (!vCoeff.empty());
    }
    const MPUInt256 PrivKey() const;
    const MPUInt256 SharedKey(const MPUInt256& pubkey) const;
    const MPUInt256 Polynomial(std::size_t nThresh,uint32_t nX) const;
public:
    std::vector<MPUInt256> vCoeff;
};

class CMPSealedBox
{
public:
    CMPSealedBox();
    CMPSealedBox(const std::vector<MPUInt256>& vEncryptedCoeffIn,const MPUInt256& nRIn,const MPUInt256& nSIn);
    const MPUInt256 PubKey() const;
    bool Make(const MPUInt256& nIdent,CMPOpenedBox& opened,const MPUInt256& r);
    bool VerifySignature(const MPUInt256& nIdent) const;
    bool VerifyPolynomial(uint32_t nX,const MPUInt256& v);
    void PrecalcPolynomial(std::size_t nThresh,std::size_t nLastIndex);
protected:
    CEdwards25519& CachedEdPoint(const MPUInt256& pubkey);
public:
    std::vector<MPUInt256> vEncryptedCoeff;
    std::vector<MPUInt256> vEncryptedShare;
    MPUInt256 nR;
    MPUInt256 nS;
};

#endif //MPBOX_H
