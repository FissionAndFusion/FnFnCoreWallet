// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_TEMPLATE_MULTISIG_H
#define  MULTIVERSE_TEMPLATE_MULTISIG_H

#include "weighted.h"

class CTemplateMultiSig : virtual public CTemplateWeighted
{
public:
    CTemplateMultiSig(const uint8 nRequiredIn = 0, const WeightedMap& mapPubKeyWeightIn = WeightedMap());
    virtual CTemplateMultiSig* clone() const;
    virtual void GetTemplateData(rpc::CTemplateResponse& obj, CDestination&& destInstance) const;
protected:
    virtual bool SetTemplateData(const rpc::CTemplateRequest& obj, CDestination&& destInstance);
};

#endif // MULTIVERSE_TEMPLATE_MULTISIG_H