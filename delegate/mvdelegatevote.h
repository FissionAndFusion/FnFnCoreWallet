// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_DELEGATE_VOTE_H
#define  MULTIVERSE_DELEGATE_VOTE_H

#include "mvdelegatecomm.h"
#include "mpvss.h"
#include <set>

namespace multiverse 
{
namespace delegate 
{

class CMvDelegateData
{
public:
    CMvDelegateData()
    {
    }
    CMvDelegateData(const MPUInt256& nIdentFromIn)
    : nIdentFrom(nIdentFromIn)
    {
    }
    void ToDataStream(walleve::CWalleveODataStream& os) const
    {
        os << nIdentFrom << mapShare << nR << nS;
    }
    void FromDataStream(walleve::CWalleveIDataStream& is)
    {
        is >> nIdentFrom >> mapShare >> nR >> nS;
    }
    const MPUInt256 GetHash() const;
    std::string ToString() const
    {
        std::ostringstream os;
        os << "CMvDelegateData: \n";
        os << nIdentFrom.ToHex() << "\n";
        os << nR.ToHex() << "\n";
        os << nS.ToHex() << "\n";
        os << mapShare.size() << "\n";
        for (std::map<MPUInt256,std::vector<MPUInt256> >::const_iterator it = mapShare.begin();
             it != mapShare.end();++it)
        {
            os << " " << (*it).first.ToHex() << " " << (*it).second.size() << "\n";
            for (int i = 0;i < (*it).second.size();i++)
            {
                os << "   " << (*it).second[i].ToHex() << "\n";
            }
        }
        return os.str();
    }
public:
    MPUInt256 nIdentFrom;
    std::map<MPUInt256,std::vector<MPUInt256> > mapShare;
    MPUInt256 nR;
    MPUInt256 nS;
};

class CMvSecretShare : public CMPSecretShare
{
public:
    CMvSecretShare();
    CMvSecretShare(const MPUInt256& nIdentIn);
    ~CMvSecretShare();
    void Distribute(CMvDelegateData& delegateData);
    void Publish(CMvDelegateData& delegateData);
protected:
    void RandGeneretor(MPUInt256& r);
};

class CMvDelegateVote
{
public:
    CMvDelegateVote();
    ~CMvDelegateVote();
    void CreateDelegate(const std::set<CDestination>& setDelegate);

    void Setup(std::size_t nMaxThresh,std::map<CDestination,std::vector<unsigned char> >& mapEnrollData);
    void Distribute(std::map<CDestination,std::vector<unsigned char> >& mapDistributeData);
    void Publish(std::map<CDestination,std::vector<unsigned char> >& mapPublishData);

    void Enroll(const std::map<CDestination,size_t>& mapWeight,
                const std::map<CDestination,std::vector<unsigned char> >& mapEnrollData);
    bool Accept(const std::vector<unsigned char>& vchDistributeData);
    bool Collect(const std::vector<unsigned char>& vchPublishData,bool& fCompleted);
    void GetAgreement(uint256& nAgreement,std::size_t& nWeight,std::map<CDestination,std::size_t>& mapBallot);
    void GetProof(std::vector<unsigned char>& vchProof);
protected:
    bool VerifySignature(const CMvDelegateData& delegateData);
protected:
    std::map<CDestination,CMvSecretShare> mapDelegate;
    CMvSecretShare witness; 

    std::vector<CMvDelegateData> vCollected;
};

} // namespace delegate
} // namespace multiverse

#endif //MULTIVERSE_DELEGATE_VOTE_H

