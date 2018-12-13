// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "mvdelegate.h"

using namespace std;
using namespace walleve;
using namespace multiverse::delegate;

#define MAX_DELEGATE_THRESH		(50)
//////////////////////////////
// CMvDelegate

CMvDelegate::CMvDelegate()
{   
}   

CMvDelegate::~CMvDelegate()
{   
}   

bool CMvDelegate::Initialize()
{
    return true;
}

void CMvDelegate::Deinitialize()
{
    mapVote.clear(); 
}

void CMvDelegate::AddNewDelegate(const CDestination& destDelegate)
{
    setDelegate.insert(destDelegate); 
}

void CMvDelegate::RemoveDelegate(const CDestination& destDelegate)
{
    setDelegate.erase(destDelegate);
}

void CMvDelegate::Evolve(int nBlockHeight,const map<CDestination,size_t>& mapWeight,
                                          const map<CDestination,vector<unsigned char> >& mapEnrollData,
                                          CMvDelegateEvolveResult& result)
{
    const int nTarget = nBlockHeight + MV_CONSENSUS_INTERVAL;
    const int nEnrollEnd = nBlockHeight + MV_CONSENSUS_DISTRIBUTE_INTERVAL + 1;
    const int nPublish = nBlockHeight + 1;

    result.Clear();

    // init
    {
        CMvDelegateVote& vote = mapVote[nTarget];
        vote.CreateDelegate(setDelegate);
        vote.Setup(MAX_DELEGATE_THRESH,result.mapEnrollData);
    }
    // enroll & distribute
    {
        map<int,CMvDelegateVote>::iterator it = mapVote.find(nEnrollEnd);
        if (it != mapVote.end())
        { 
            CMvDelegateVote& vote = (*it).second;
            vote.Enroll(mapWeight,mapEnrollData);
            vote.Distribute(result.mapDistributeData);
        }
    }
    // publish
    {
        map<int,CMvDelegateVote>::iterator it = mapVote.find(nPublish);
        if (it != mapVote.end())
        {
            CMvDelegateVote& vote = (*it).second;
            vote.Publish(result.mapPublishData);
        }
    }
}    

void CMvDelegate::Rollback(int nBlockHeightFrom,int nBlockHeightTo)
{
    // init
    {
        mapVote.erase(mapVote.upper_bound(nBlockHeightTo + MV_CONSENSUS_INTERVAL),mapVote.end());
    }
    // enroll
    {
        mapVote.erase(mapVote.upper_bound(nBlockHeightTo + MV_CONSENSUS_DISTRIBUTE_INTERVAL),
                      mapVote.upper_bound(nBlockHeightFrom + MV_CONSENSUS_DISTRIBUTE_INTERVAL));
    }
    // distribute
    {
        mapVote.erase(mapVote.upper_bound(nBlockHeightTo + 1),
                      mapVote.upper_bound(nBlockHeightFrom + 1));
    }
    // publish
    {
        mapVote.erase(nBlockHeightFrom + 1);
    }
}

bool CMvDelegate::HandleDistribute(int nTargetHeight,const CDestination& destFrom,
                                   const vector<unsigned char>& vchDistributeData)
{
    map<int,CMvDelegateVote>::iterator it = mapVote.find(nTargetHeight);
    if (it != mapVote.end())
    {
        CMvDelegateVote& vote = (*it).second;
        return vote.Accept(destFrom,vchDistributeData);
    }
    return false;
}

bool CMvDelegate::HandlePublish(int nTargetHeight,const CDestination& destFrom,
                                const vector<unsigned char>& vchPublishData,bool& fCompleted)
{
    map<int,CMvDelegateVote>::iterator it = mapVote.find(nTargetHeight);
    if (it != mapVote.end())
    {
        CMvDelegateVote& vote = (*it).second;
        return vote.Collect(destFrom,vchPublishData,fCompleted); 
    }
    return false;
}

void CMvDelegate::GetAgreement(int nTargetHeight,uint256& nAgreement,size_t& nWeight,map<CDestination,size_t>& mapBallot)
{
    nAgreement = 0;
    nWeight = 0;

    map<int,CMvDelegateVote>::iterator it = mapVote.find(nTargetHeight);
    if (it != mapVote.end())
    {
        CMvDelegateVote& vote = (*it).second;
        vote.GetAgreement(nAgreement,nWeight,mapBallot);
    }
}

void CMvDelegate::GetProof(int nTargetHeight,vector<unsigned char>& vchProof)
{
    map<int,CMvDelegateVote>::iterator it = mapVote.find(nTargetHeight);
    if (it != mapVote.end())
    {
        CMvDelegateVote& vote = (*it).second;
        vote.GetProof(vchProof);
    }
}

