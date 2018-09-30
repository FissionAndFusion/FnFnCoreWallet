// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_DELEGATE_VERIFY_H
#define  MULTIVERSE_DELEGATE_VERIFY_H

#include "mvdelegatevote.h"

namespace multiverse 
{
namespace delegate 
{

class CMvDelegateVerify : public CMvDelegateVote
{
public:
    CMvDelegateVerify(const std::map<CDestination,std::size_t>& mapWeight,
                      const std::map<CDestination,std::vector<unsigned char> >& mapEnrollData);
    bool VerifyProof(const std::vector<unsigned char>& vchProof,
                     std::size_t& nWeight,std::map<CDestination,std::size_t>& mapBallot);
};

} // namespace delegate
} // namespace multiverse

#endif //MULTIVERSE_DELEGATE_VERIFY_H

