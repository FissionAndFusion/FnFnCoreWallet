// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "mvdelegatevote.h"
#include "crypto.h"
#include <boost/foreach.hpp>

using namespace std;
using namespace walleve;
using namespace multiverse::delegate;

//////////////////////////////
// CMvDelegateData

const uint256 CMvDelegateData::GetHash() const
{
    vector<unsigned char> vch;
    CWalleveODataStream os(vch);
    os << mapShare;
    return crypto::CryptoHash(&vch[0],vch.size());
}

//////////////////////////////
// CMvSecretShare

CMvSecretShare::CMvSecretShare()
{
}

CMvSecretShare::CMvSecretShare(const uint256& nIdentIn)
: CMPSecretShare(nIdentIn)
{

}

CMvSecretShare::~CMvSecretShare()
{
}

void CMvSecretShare::Distribute(CMvDelegateData& delegateData)
{
    delegateData.nIdentFrom = nIdent;
    CMPSecretShare::Distribute(delegateData.mapShare);
    Signature(delegateData.GetHash(),delegateData.nR,delegateData.nS);
}

void CMvSecretShare::Publish(CMvDelegateData& delegateData)
{
    delegateData.nIdentFrom = nIdent;
    CMPSecretShare::Publish(delegateData.mapShare);
    Signature(delegateData.GetHash(),delegateData.nR,delegateData.nS);
}

void CMvSecretShare::RandGeneretor(uint256& r)
{
    crypto::CryptoGetRand256(r);
}

//////////////////////////////
// CMvDelegateVote

CMvDelegateVote::CMvDelegateVote()
{   
    witness.SetupWitness();
}

CMvDelegateVote::~CMvDelegateVote()
{   
}   

void CMvDelegateVote::CreateDelegate(const set<CDestination>& setDelegate)
{
    for (set<CDestination>::const_iterator it = setDelegate.begin();it != setDelegate.end();++it)
    {
        const CDestination& dest = (*it);
        mapDelegate.insert(make_pair(dest,CMvSecretShare(DestToIdentUInt256(dest))));
    } 
}

void CMvDelegateVote::Setup(size_t nMaxThresh,map<CDestination,vector<unsigned char> >& mapEnrollData)
{
    for (map<CDestination,CMvSecretShare>::iterator it = mapDelegate.begin();it != mapDelegate.end();++it)
    {
        CMvSecretShare& delegate = (*it).second;
        CMPSealedBox sealed;
        delegate.Setup(nMaxThresh,sealed);
        
        CWalleveODataStream os(mapEnrollData[(*it).first]);
        os << sealed.nPubKey <<  sealed.vEncryptedCoeff << sealed.nR << sealed.nS;
    }
}

void CMvDelegateVote::Distribute(map<CDestination,std::vector<unsigned char> >& mapDistributeData)
{
    for (map<CDestination,CMvSecretShare>::iterator it = mapDelegate.begin();it != mapDelegate.end();++it)
    {
        CMvSecretShare& delegate = (*it).second;
        if (delegate.IsEnrolled())
        {
            CMvDelegateData delegateData;
            delegate.Distribute(delegateData);
            
            CWalleveODataStream os(mapDistributeData[(*it).first]);
            os << delegateData;
        }
    }
}

void CMvDelegateVote::Publish(map<CDestination,vector<unsigned char> >& mapPublishData)
{
    for (map<CDestination,CMvSecretShare>::iterator it = mapDelegate.begin();it != mapDelegate.end();++it)
    {
        CMvSecretShare& delegate = (*it).second;
        if (delegate.IsEnrolled())
        {
            CMvDelegateData delegateData;
            delegate.Publish(delegateData);
            
            CWalleveODataStream os(mapPublishData[(*it).first]);
            os << delegateData;
        }
    }
}

void CMvDelegateVote::Enroll(const map<CDestination,size_t>& mapWeight,
                             const map<CDestination,vector<unsigned char> >& mapEnrollData)
{
    vector<CMPCandidate> vCandidate;
    vCandidate.reserve(mapWeight.size());
    for (map<CDestination,size_t>::const_iterator it = mapWeight.begin();it != mapWeight.end();++it)
    {
        map<CDestination,vector<unsigned char> >::const_iterator mi = mapEnrollData.find((*it).first);
        if (mi != mapEnrollData.end())
        {
            try
            {
                vector<uint256> vEncryptedCoeff;
                uint256 nPubKey,nR,nS;

                CWalleveIDataStream is((*mi).second);
                is >> nPubKey >> vEncryptedCoeff >> nR >> nS;

                vCandidate.push_back(CMPCandidate(DestToIdentUInt256((*it).first),(*it).second,
                                                  CMPSealedBox(vEncryptedCoeff,nPubKey,nR,nS)));
            }
            catch (...)
            {
            }
        }
    }
    witness.Enroll(vCandidate);
    for (map<CDestination,CMvSecretShare>::iterator it = mapDelegate.begin();it != mapDelegate.end();++it)
    {
        CMvSecretShare& delegate = (*it).second;
        delegate.Enroll(vCandidate);
    }
}

bool CMvDelegateVote::Accept(const CDestination& destFrom,const vector<unsigned char>& vchDistributeData)
{
    CMvDelegateData delegateData;
    try
    {
        CWalleveIDataStream is(vchDistributeData);
        is >> delegateData;
        if (delegateData.nIdentFrom != DestToIdentUInt256(destFrom) || !VerifySignature(delegateData))
        {
            return false;
        }
    }
    catch (...)
    {
        return false;
    }

    for (map<CDestination,CMvSecretShare>::iterator it = mapDelegate.begin();it != mapDelegate.end();++it)
    {
        CMvSecretShare& delegate = (*it).second;
        if (delegate.IsEnrolled())
        {
            map<uint256,vector<uint256> >::iterator mi = delegateData.mapShare.find(delegate.GetIdent());
            if (mi != delegateData.mapShare.end())
            {
                if (!delegate.Accept(delegateData.nIdentFrom,(*mi).second))
                {
                    return false;
                }
            }
        } 
    }
    return true;
}

bool CMvDelegateVote::Collect(const CDestination& destFrom,const vector<unsigned char>& vchPublishData,bool& fCompleted)
{
    try
    {
        CMvDelegateData delegateData;
        CWalleveIDataStream is(vchPublishData);
        is >> delegateData;
        if (delegateData.nIdentFrom == DestToIdentUInt256(destFrom) && VerifySignature(delegateData))
        {
            if (witness.Collect(delegateData.nIdentFrom,delegateData.mapShare,fCompleted))
            {
                vCollected.push_back(delegateData);
                return true;
            }
        }
    }
    catch (...) {}
    return false;
}

void CMvDelegateVote::GetAgreement(uint256& nAgreement,size_t& nWeight,map<CDestination,size_t>& mapBallot)
{
    nAgreement = 0;
    nWeight = 0;
    mapBallot.clear();

    map<uint256,pair<uint256,size_t> > mapSecret;

    witness.Reconstruct(mapSecret);
     
    if (!mapSecret.empty())
    {
        vector<unsigned char> vch;
        CWalleveODataStream os(vch);
        for (map<uint256,pair<uint256,size_t> >::iterator it = mapSecret.begin();
             it != mapSecret.end(); ++it)
        {
            os << (*it).second.first;
            nWeight += (*it).second.second;
            mapBallot.insert(make_pair(DestFromIdentUInt256((*it).first),(*it).second.second));
        }
        nAgreement = crypto::CryptoHash(&vch[0],vch.size());
    }
}

void CMvDelegateVote::GetProof(vector<unsigned char>& vchProof)
{
    CWalleveODataStream os(vchProof);
    os << vCollected;
}

bool CMvDelegateVote::VerifySignature(const CMvDelegateData& delegateData)
{
    return witness.VerifySignature(delegateData.nIdentFrom,delegateData.GetHash(),
                                   delegateData.nR,delegateData.nS);
}

