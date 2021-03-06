// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_DELEGATE_H
#define  MULTIVERSE_DELEGATE_H

#include "mvdelegatecomm.h"
#include "mvdelegatevote.h"
#include "destination.h"

namespace multiverse 
{
namespace delegate 
{

class CMvDelegateEvolveResult
{
public:
    void Clear()
    {
        mapEnrollData.clear();
        mapDistributeData.clear();
        mapPublishData.clear();
    }
public:
    std::map<CDestination,std::vector<unsigned char> > mapEnrollData;
    std::map<CDestination,std::vector<unsigned char> > mapDistributeData;
    std::map<CDestination,std::vector<unsigned char> > mapPublishData;
};

class CMvDelegate
{
public:
    CMvDelegate();
    ~CMvDelegate();
    bool Initialize();
    void Deinitialize();
    void AddNewDelegate(const CDestination& destDelegate);
    void RemoveDelegate(const CDestination& destDelegate);
    
    void Evolve(const int32 nBlockHeight,const std::map<CDestination,std::size_t>& mapWeight,
                                 const std::map<CDestination,std::vector<unsigned char> >& mapEnrollData,
                                 CMvDelegateEvolveResult& result);
    void Rollback(const int32 nBlockHeightFrom,const int32 nBlockHeightTo);
    bool HandleDistribute(const int32 nTargetHeight,const CDestination& destFrom,
                          const std::vector<unsigned char>& vchDistributeData);
    bool HandlePublish(const int32 nTargetHeight,const CDestination& destFrom,
                       const std::vector<unsigned char>& vchPublishData,bool& fCompleted);
    void GetAgreement(const int32 nTargetHeight,uint256& nAgreement,std::size_t& nWeight,std::map<CDestination,std::size_t>& mapBallot);
    void GetProof(const int32 nTargetHeight,std::vector<unsigned char>& vchProof);
protected:
    std::set<CDestination> setDelegate;
    std::map<int32,CMvDelegateVote> mapVote;
};

} // namespace delegate
} // namespace multiverse

#endif //MULTIVERSE_DELEGATE_H

