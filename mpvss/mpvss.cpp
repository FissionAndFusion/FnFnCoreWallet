#include "mpvss.h"
#include "mplagrange.h"
#include <stdlib.h>

using namespace std;

//////////////////////////////
// CMPParticipant
CMPParticipant::CMPParticipant()
{
}

CMPParticipant::CMPParticipant(const CMPCandidate& candidate,size_t nIndexIn,const MPUInt256& nSharedKeyIn)
: nWeight(candidate.nWeight),nIndex(nIndexIn),sBox(candidate.sBox),nSharedKey(nSharedKeyIn)
{
}

const MPUInt256 CMPParticipant::Encrypt(const MPUInt256& data) const
{
    return (nSharedKey ^ data); 
}

const MPUInt256 CMPParticipant::Decrypt(const MPUInt256& cipher) const
{
    return (nSharedKey ^ cipher); 
}

bool CMPParticipant::AcceptShare(size_t nThresh,size_t nIndexIn,const vector<MPUInt256>& vEncrypedShare)
{
    vShare.resize(vEncrypedShare.size());
    for (size_t i = 0; i < vEncrypedShare.size(); i++)
    {
        vShare[i] = Decrypt(vEncrypedShare[i]);
        if (!sBox.VerifyPolynomial(nIndexIn + i,vShare[i]))
        {
            vShare.clear();
            return false;
        }
    }
    return true;
}

bool CMPParticipant::VerifyShare(size_t nThresh,size_t nIndexIn,const vector<MPUInt256>& vShare)
{
    for (size_t i = 0; i < vShare.size(); i++)
    {
        if (!sBox.VerifyPolynomial(nIndexIn + i,vShare[i]))
        {
            return false;
        }
    }
    return true;
}

void CMPParticipant::PrepareVerification(std::size_t nThresh,std::size_t nLastIndex)
{
    sBox.PrecalcPolynomial(nThresh,nLastIndex);
}

//////////////////////////////
// CMPSecretShare

CMPSecretShare::CMPSecretShare()
{
    nIndex = 0;
    nThresh = 0;
    nWeight = 0;    
}

CMPSecretShare::CMPSecretShare(const MPUInt256& nIdentIn)
: nIdent(nIdentIn)
{
    nIndex = 0;
    nThresh = 0;
    nWeight = 0;    
}

void CMPSecretShare::RandGeneretor(MPUInt256& r)
{
    uint8_t *p = r.Data();
    for (int i = 0;i < 32;i++)
    {
        *p++ = rand();
    }
}

const MPUInt256 CMPSecretShare::RandShare()
{
    MPUInt256 r;
    RandGeneretor(r);
    r.u64[3] &= 0x0FFFFFFFFFFFFFFFULL;
    return r;
}

bool CMPSecretShare::GetParticipantRange(const MPUInt256& nIdentIn,size_t& nIndexRet,size_t& nWeightRet)
{
    if (nIdentIn == nIdent)
    {
        nIndexRet = nIndex;
        nWeightRet = nWeight;
        return true;
    }

    map<MPUInt256,CMPParticipant>::iterator it = mapParticipant.find(nIdentIn);
    if (it == mapParticipant.end())
    {
        return false;
    }

    nIndexRet = (*it).second.nIndex;
    nWeightRet = (*it).second.nWeight;
    return true;
}

void CMPSecretShare::Setup(size_t nMaxThresh,CMPSealedBox& sealed)
{
    myBox.vCoeff.clear();
    myBox.vCoeff.resize(nMaxThresh);
    do
    {
        myBox.nPrivKey = RandShare();
        for (int i = 0;i < nMaxThresh;i++)
        {
            myBox.vCoeff[i] = RandShare();
        }
    } while (!myBox.MakeSealedBox(sealed,nIdent,RandShare()));

    nIndex = 0;
    nThresh = 0;
    nWeight = 0; 
    mapParticipant.clear();
    mapOpenedShare.clear();
}

void CMPSecretShare::SetupWitness()
{
    myBox.vCoeff.clear();
    nIdent = 0;
    nIndex = 0;
    nThresh = 0;
    nWeight = 0;    
    mapParticipant.clear();
    mapOpenedShare.clear();
}

void CMPSecretShare::Enroll(const vector<CMPCandidate>& vCandidate)
{
    size_t nLastIndex = 1;
    for (size_t i = 0;i < vCandidate.size();i++)
    {
        const CMPCandidate& candidate = vCandidate[i];
        if (candidate.nIdent == nIdent)
        {
            nIndex = nLastIndex;
            nWeight = candidate.nWeight; 
            nLastIndex += candidate.nWeight;
        }
        else if (!mapParticipant.count(candidate.nIdent) && candidate.Verify())
        {
            try
            {
                MPUInt256 shared = myBox.SharedKey(candidate.PubKey());
                mapParticipant[candidate.nIdent] = CMPParticipant(candidate,nLastIndex,shared);
                nLastIndex += candidate.nWeight;
            }
            catch (...) {}
        } 
    }
    nThresh = (nLastIndex - 1) / 2 + 1;

    for (map<MPUInt256,CMPParticipant>::iterator it = mapParticipant.begin();it != mapParticipant.end();++it)
    {
        (*it).second.PrepareVerification(nThresh,nLastIndex);
    }
}

void CMPSecretShare::Distribute(map<MPUInt256,vector<MPUInt256> >& mapShare)
{
    for (map<MPUInt256,CMPParticipant>::iterator it = mapParticipant.begin();it != mapParticipant.end();++it)
    {
        CMPParticipant& participant = (*it).second;
        vector<MPUInt256>& vShare = mapShare[(*it).first];
        vShare.resize(participant.nWeight);
        for (size_t i = 0;i < participant.nWeight;i++)
        {
            vShare[i] = participant.Encrypt(myBox.Polynomial(nThresh,participant.nIndex + i));
        }
    }
}

bool CMPSecretShare::Accept(const MPUInt256& nIdentFrom,const vector<MPUInt256>& vEncryptedShare)
{
    if (vEncryptedShare.size() == nWeight)
    {
        map<MPUInt256,CMPParticipant>::iterator it = mapParticipant.find(nIdentFrom);
        if (it != mapParticipant.end())
        {
            return (*it).second.AcceptShare(nThresh,nIndex,vEncryptedShare);
        }
    }
    return false;
}

void CMPSecretShare::Publish(map<MPUInt256,vector<MPUInt256> >& mapShare)
{
    mapShare.clear();

    for (map<MPUInt256,CMPParticipant>::iterator it = mapParticipant.begin();it != mapParticipant.end();++it)
    {
        if (!(*it).second.vShare.empty())
        {
            mapShare.insert(make_pair((*it).first,(*it).second.vShare));
        }
    }

    vector<MPUInt256>& myShare = mapShare[nIdent];
    for (size_t i = 0;i < nWeight;i++)
    {
        myShare.push_back(myBox.Polynomial(nThresh,nIndex + i));
    }
}

bool CMPSecretShare::Collect(const MPUInt256& nIdentFrom,const map<MPUInt256,vector<MPUInt256> >& mapShare,bool& fCompleted)
{
    size_t nIndexFrom,nWeightFrom;

    fCompleted = false;

    if (!GetParticipantRange(nIdentFrom,nIndexFrom,nWeightFrom))
    {
        return false;
    }
    for (map<MPUInt256,vector<MPUInt256> >::const_iterator mi = mapShare.begin();mi != mapShare.end();++mi)
    {
        const vector<MPUInt256>& vShare = (*mi).second;
        if (nWeightFrom != vShare.size())
        {
            return false;
        } 

        if ((*mi).first == nIdent)
        {
            for (size_t i = 0;i < vShare.size();i++)
            {
                if (myBox.Polynomial(nThresh,nIndexFrom + i) != vShare[i])
                {
                    return false;
                } 
            }
            continue;
        }

        map<MPUInt256,CMPParticipant>::iterator it = mapParticipant.find((*mi).first);
        if (it == mapParticipant.end())
        {
            return false;
        }
        CMPParticipant& participant = (*it).second;
        if (!participant.VerifyShare(nThresh,nIndexFrom,vShare))
        {
            return false;
        }
    }

    size_t nCompleteCollect = 0;
    for (map<MPUInt256,vector<MPUInt256> >::const_iterator mi = mapShare.begin();mi != mapShare.end();++mi)
    {
        vector<pair<uint32_t,MPUInt256> >& vOpenedShare = mapOpenedShare[(*mi).first];
        for (size_t i = 0;i < nWeightFrom && vOpenedShare.size() < nThresh;i++)
        {
            vOpenedShare.push_back(make_pair(nIndexFrom + i,(*mi).second[i]));
        }
        if (vOpenedShare.size() == nThresh)
        {
            nCompleteCollect++;
        }
    }
    fCompleted = (nCompleteCollect >= mapShare.size());
    return true;
}

void CMPSecretShare::Reconstruct(map<MPUInt256,pair<MPUInt256,size_t> >& mapSecret)
{
    map<MPUInt256,vector<pair<uint32_t,MPUInt256> > >::iterator it;
    for (it = mapOpenedShare.begin(); it != mapOpenedShare.end(); ++it)
    {
        if ((*it).second.size() == nThresh)
        {
            const MPUInt256& nIdentAvail = (*it).first; 
            size_t nIndexRet,nWeightRet;
            if (GetParticipantRange(nIdentAvail,nIndexRet,nWeightRet))
            {
                mapSecret[nIdentAvail] = make_pair(MPLagrange((*it).second),nWeightRet);
            }
        }
    }
}

void CMPSecretShare::Signature(const MPUInt256& hash,MPUInt256& nR,MPUInt256& nS)
{
    MPUInt256 r = RandShare();
    myBox.Signature(hash,r,nR,nS);
}
bool CMPSecretShare::VerifySignature(const MPUInt256& nIdentFrom,const MPUInt256& hash,const MPUInt256& nR,const MPUInt256& nS)
{
    if (nIdentFrom == nIdent)
    {
        return myBox.VerifySignature(hash,nR,nS);
    }
    map<MPUInt256,CMPParticipant>::iterator it = mapParticipant.find(nIdentFrom);
    if (it == mapParticipant.end())
    {
        return false;
    }
    CMPParticipant& participant = (*it).second;
    return participant.sBox.VerifySignature(hash,nR,nS);
}
