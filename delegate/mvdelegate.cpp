// Copyright (c) 2017-2019 The Multiverse developers
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

void CMvDelegate::Evolve(const int32 nBlockHeight,const map<CDestination,size_t>& mapWeight,
                                          const map<CDestination,vector<unsigned char> >& mapEnrollData,
                                          CMvDelegateEvolveResult& result)
{
    const int32 nTarget = nBlockHeight + MV_CONSENSUS_INTERVAL;
    const int32 nEnrollEnd = nBlockHeight + MV_CONSENSUS_DISTRIBUTE_INTERVAL + 1;
    const int32 nPublish = nBlockHeight + 1;

    result.Clear();

    // init
    {
        auto t0 = boost::posix_time::microsec_clock::universal_time();

        CMvDelegateVote& vote = mapVote[nTarget];
        vote.CreateDelegate(setDelegate);
        vote.Setup(MAX_DELEGATE_THRESH,result.mapEnrollData);

        auto t1 = boost::posix_time::microsec_clock::universal_time();
        walleve::StdDebug("CMvDelegate", (string("Setup height:") + to_string(nTarget) + " time:" + to_string((t1-t0).ticks())).c_str());
    }
    // enroll & distribute
    {
        map<int32,CMvDelegateVote>::iterator it = mapVote.find(nEnrollEnd);
        if (it != mapVote.end())
        { 
            auto t0 = boost::posix_time::microsec_clock::universal_time();

            CMvDelegateVote& vote = (*it).second;
            vote.Enroll(mapWeight,mapEnrollData);
            vote.Distribute(result.mapDistributeData);

            auto t1 = boost::posix_time::microsec_clock::universal_time();
            walleve::StdDebug("CMvDelegate", (string("Enroll height:") + to_string(nEnrollEnd) + " time:" + to_string((t1-t0).ticks())).c_str());
        }
    }
    // publish
    {
        map<int32,CMvDelegateVote>::iterator it = mapVote.find(nPublish);
        if (it != mapVote.end())
        {
            auto t0 = boost::posix_time::microsec_clock::universal_time();

            CMvDelegateVote& vote = (*it).second;
            vote.Publish(result.mapPublishData);

            auto t1 = boost::posix_time::microsec_clock::universal_time();
            walleve::StdDebug("CMvDelegate", (string("Publish height:") + to_string(nPublish) + " time:" + to_string((t1-t0).ticks())).c_str());
        }
    }
}    

void CMvDelegate::Rollback(const int32 nBlockHeightFrom,const int32 nBlockHeightTo)
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

bool CMvDelegate::HandleDistribute(const int32 nTargetHeight,const CDestination& destFrom,
                                   const vector<unsigned char>& vchDistributeData)
{
    map<int32,CMvDelegateVote>::iterator it = mapVote.find(nTargetHeight);
    if (it != mapVote.end())
    {
        CMvDelegateVote& vote = (*it).second;

        auto t0 = boost::posix_time::microsec_clock::universal_time();

        bool ret = vote.Accept(destFrom,vchDistributeData);

        auto t1 = boost::posix_time::microsec_clock::universal_time();
        walleve::StdDebug("CMvDelegate", (string("Accept height:") + to_string(nTargetHeight) + " time:" + to_string((t1-t0).ticks())).c_str());

        return ret;
    }
    return false;
}

bool CMvDelegate::HandlePublish(const int32 nTargetHeight,const CDestination& destFrom,
                                const vector<unsigned char>& vchPublishData,bool& fCompleted)
{
    map<int32,CMvDelegateVote>::iterator it = mapVote.find(nTargetHeight);
    if (it != mapVote.end())
    {
        CMvDelegateVote& vote = (*it).second;

        auto t0 = boost::posix_time::microsec_clock::universal_time();

        bool ret = vote.Collect(destFrom,vchPublishData,fCompleted); 

        auto t1 = boost::posix_time::microsec_clock::universal_time();
        walleve::StdDebug("CMvDelegate", (string("Collect height:") + to_string(nTargetHeight) + " time:" + to_string((t1-t0).ticks())).c_str());

        return ret;
    }
    return false;
}

void CMvDelegate::GetAgreement(const int32 nTargetHeight,uint256& nAgreement,size_t& nWeight,map<CDestination,size_t>& mapBallot)
{
    nAgreement = 0;
    nWeight = 0;

    map<int32,CMvDelegateVote>::iterator it = mapVote.find(nTargetHeight);
    if (it != mapVote.end())
    {
        CMvDelegateVote& vote = (*it).second;

        auto t0 = boost::posix_time::microsec_clock::universal_time();

        vote.GetAgreement(nAgreement,nWeight,mapBallot);

        auto t1 = boost::posix_time::microsec_clock::universal_time();
        walleve::StdDebug("CMvDelegate", (string("Reconstruct height:") + to_string(nTargetHeight) + " time:" + to_string((t1-t0).ticks())).c_str());
    }
}

void CMvDelegate::GetProof(const int32 nTargetHeight,vector<unsigned char>& vchProof)
{
    map<int32,CMvDelegateVote>::iterator it = mapVote.find(nTargetHeight);
    if (it != mapVote.end())
    {
        CMvDelegateVote& vote = (*it).second;
        vote.GetProof(vchProof);
    }
}

