// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "profile.h"
#include <walleve/stream/datastream.h>
#include <walleve/compacttv.h>

using namespace std;
using namespace walleve;

//////////////////////////////
// CProfile

bool CProfile::Save(std::vector<unsigned char>& vchProfile)
{
    try
    {
        CWalleveCompactTagValue encoder;
        encoder.Push(PROFILE_VERSION,nVersion);
        encoder.Push(PROFILE_NAME,strName);
        encoder.Push(PROFILE_SYMBOL,strSymbol);
        encoder.Push(PROFILE_FLAG,nFlag);
        encoder.Push(PROFILE_MINTXFEE,nMinTxFee);
        encoder.Push(PROFILE_MINTREWARD,nMintReward);
        
        vector<unsigned char> vchDestOwner;
        CWalleveODataStream os(vchDestOwner);
        os << destOwner.prefix << destOwner.data;
        encoder.Push(PROFILE_OWNER,vchDestOwner);

        if (hashParent != 0)
        {
            encoder.Push(PROFILE_PARENT,hashParent);
        }

        encoder.Encode(vchProfile);
    }
    catch (...)
    {
        return false;
    }
    return true;
}

bool CProfile::Load(const vector<unsigned char>& vchProfile)
{
    SetNull();
    CWalleveIDataStream is(vchProfile);
    try
    {
        CWalleveCompactTagValue decoder;
        decoder.Decode(vchProfile);
        if (!decoder.Get(PROFILE_VERSION,nVersion))
        {
            return false;
        }
        if (nVersion != 1)
        {
            return false;
        }
        if (!decoder.Get(PROFILE_NAME,strName))
        {
            return false;
        }
        if (!decoder.Get(PROFILE_SYMBOL,strSymbol))
        {
            return false;
        }
        if (!decoder.Get(PROFILE_FLAG,nFlag))
        {
            return false;
        }
        if (!decoder.Get(PROFILE_MINTXFEE,nMinTxFee))
        {
            return false;
        }
        if (!decoder.Get(PROFILE_MINTREWARD,nMintReward))
        {
            return false;
        }

        if (!decoder.Get(PROFILE_PARENT,hashParent))
        {
            hashParent = 0;
        }

        vector<unsigned char> vchDestOwner;
        if (decoder.Get(PROFILE_OWNER,vchDestOwner))
        {
            CWalleveIDataStream is(vchDestOwner);
            is >> destOwner.prefix >> destOwner.data;
        }
    }
    catch (...) 
    {
        return false;
    } 
    return true;
}


