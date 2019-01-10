// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "mvdelegateverify.h"

using namespace std;
using namespace walleve;
using namespace multiverse::delegate;

//////////////////////////////
// CMvDelegateVerify

CMvDelegateVerify::CMvDelegateVerify(const map<CDestination,size_t>& mapWeight,
                                     const map<CDestination,vector<unsigned char> >& mapEnrollData)
{   
    Enroll(mapWeight,mapEnrollData);
}   

bool CMvDelegateVerify::VerifyProof(const vector<unsigned char>& vchProof,uint256& nAgreement,
                                    size_t& nWeight,map<CDestination,size_t>& mapBallot)
{
    uint256 nAgreementParse;
    try
    {
        unsigned char nWeightParse;
        vector<CMvDelegateData> vPublish;
        CWalleveIDataStream is(vchProof);
        is >> nWeightParse >> nAgreementParse;
        if (nWeightParse == 0 && nAgreementParse == 0)
        {
            return true;
        }
        is >> vPublish;
        bool fCompleted = false;
        for (int i = 0;i < vPublish.size();i++)
        {
            const CMvDelegateData& delegateData = vPublish[i];
            if (!VerifySignature(delegateData) 
                || !witness.Collect(delegateData.nIdentFrom,delegateData.mapShare,fCompleted))
            {
                return false;
            }
        }
    }
    catch (exception& e) 
    {
        StdError(__PRETTY_FUNCTION__, e.what());
        return false;
    } 

    GetAgreement(nAgreement,nWeight,mapBallot);
    
    return (nAgreement == nAgreementParse);
}
