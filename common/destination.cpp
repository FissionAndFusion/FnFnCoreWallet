// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "destination.h"
#include "template.h"
#include <walleve/stream/datastream.h>

using namespace std;
using namespace walleve;
using namespace multiverse::crypto;

//////////////////////////////
// CDestination

bool CDestination::VerifySignature(const uint256& hash,const vector<unsigned char>& vchSig) const
{
    if (IsPubKey())
    {
        return CPubKey(data).Verify(hash,vchSig);
    }
    else if (IsTemplate())
    {
        CTemplatePtr ptr = CTemplateGeneric::GetTemplatePtr(data,vchSig);
        if (ptr != NULL)
        {
            return ptr->VerifyTxSignature(hash,vchSig);
        }
    }
    return false;
}
