// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "multisig.h"

#include "rpc/auto_protocol.h"
#include "template.h"

using namespace std;
using namespace walleve;
using namespace multiverse::crypto;

//////////////////////////////
// CTemplateMultiSig

CTemplateMultiSig::CTemplateMultiSig(const uint8 nRequiredIn, const WeightedMap& mapPubKeyWeightIn)
  : CTemplate(TEMPLATE_MULTISIG), CTemplateWeighted(nRequiredIn, mapPubKeyWeightIn)
{
}

CTemplateMultiSig* CTemplateMultiSig::clone() const
{
    return new CTemplateMultiSig(*this);
}

void CTemplateMultiSig::GetTemplateData(rpc::CTemplateResponse& obj, CDestination&& destInstance) const
{
    obj.multisig.nRequired = nRequired;
    for (const auto& keyweight : mapPubKeyWeight)
    {
        obj.multisig.vecAddresses.push_back(destInstance.SetPubKey(keyweight.first).ToString());
    }
}

bool CTemplateMultiSig::SetTemplateData(const rpc::CTemplateRequest& obj, CDestination&& destInstance)
{
    if (obj.strType != GetTypeName(TEMPLATE_MULTISIG))
    {
        return false;
    }

    if (obj.multisig.nRequired < 1 || obj.multisig.nRequired > 255)
    {
        return false;
    }
    nRequired = obj.multisig.nRequired;

    for (const string& key : obj.multisig.vecPubkeys)
    {
        if (!destInstance.SetPubKey(key))
        {
            return false;
        }
        mapPubKeyWeight.insert(make_pair(destInstance.GetPubKey(), 1));
    }

    if (mapPubKeyWeight.size() != obj.multisig.vecPubkeys.size())
    {
        return false;
    }

    return true;
}