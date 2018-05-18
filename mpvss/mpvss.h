#ifndef  MPVSS_H
#define  MPVSS_H

#include "mpu256.h"
#include "mpbox.h"

class CMPCandidate
{
public:
    CMPCandidate() {}
    CMPCandidate(const MPUInt256& nIdentIn,std::size_t nWeightIn,const CMPSealedBox& sBoxIn)
    : nIdent(nIdentIn), nWeight(nWeightIn), sBox(sBoxIn) {}
    bool Verify() const { return sBox.VerifySignature(nIdent); }
    const MPUInt256 PubKey() const { return sBox.PubKey(); }
public:
    MPUInt256 nIdent;
    std::size_t nWeight;
    CMPSealedBox sBox;
};

class CMPParticipant
{
public:
    CMPParticipant();
    CMPParticipant(const CMPCandidate& candidate,std::size_t nIndexIn,const MPUInt256& nSharedKeyIn); 
    const MPUInt256 Encrypt(const MPUInt256& data) const;
    const MPUInt256 Decrypt(const MPUInt256& cipher) const;
    bool AcceptShare(std::size_t nThresh,std::size_t nIndexIn,const std::vector<MPUInt256>& vEncrypedShare);
    bool VerifyShare(std::size_t nThresh,std::size_t nIndexIn,const std::vector<MPUInt256>& vShare);
    void PrepareVerification(std::size_t nThresh,std::size_t nLastIndex);
public:
    std::size_t nWeight;
    std::size_t nIndex;
    CMPSealedBox sBox;
    MPUInt256 nSharedKey;
    std::vector<MPUInt256> vShare;
};

class CMPSecretShare
{
public:
    CMPSecretShare();
    CMPSecretShare(const MPUInt256& nIdentIn);
    bool IsEnrolled() const { return (nIndex != 0); }
    void Setup(std::size_t nMaxThresh,CMPSealedBox& sealed);
    void Enroll(const std::vector<CMPCandidate>& vCandidate);
    void Distribute(std::map<MPUInt256,std::vector<MPUInt256> >& mapShare);
    bool Accept(const MPUInt256& nIdentFrom,const std::vector<MPUInt256>& vEncrypedShare);
    void Publish(std::map<MPUInt256,std::vector<MPUInt256> >& mapShare);
    bool Collect(const MPUInt256& nIdentFrom,const std::map<MPUInt256,std::vector<MPUInt256> >& mapShare);
    void Reconstruct(std::map<MPUInt256,MPUInt256>& mapSecret);
protected:
    virtual void RandGeneretor(MPUInt256& r);
    const MPUInt256 RandShare();
    bool GetParticipantRange(const MPUInt256& nIdentIn,std::size_t& nIndexRet,std::size_t& nWeightRet);
public:
    MPUInt256 nIdent;
    CMPOpenedBox myBox;
    std::size_t nWeight;
    std::size_t nIndex;
    std::size_t nThresh; 
    std::map<MPUInt256,CMPParticipant> mapParticipant;
    std::map<MPUInt256,std::vector<std::pair<uint32_t,MPUInt256> > > mapOpenedShare;
};

#endif //MPVSS_H

