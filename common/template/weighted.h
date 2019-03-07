// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_TEMPLATE_WEIGHTED_H
#define MULTIVERSE_TEMPLATE_WEIGHTED_H

#include "key.h"
#include "template.h"

template <class T>
class Uint8Allocator : public std::allocator<T>
{
public:
    typedef uint8_t size_type;
};
typedef std::map<
    multiverse::crypto::CPubKey,
    uint8,
    std::less<multiverse::crypto::CPubKey>,
    Uint8Allocator<std::pair<const multiverse::crypto::CPubKey, uint8>>>
    WeightedMap;

class CTemplateWeighted : virtual public CTemplate, virtual public CSpendableTemplate
{
public:
    CTemplateWeighted(const uint8 nRequiredIn = 0, const WeightedMap& mapPubKeyWeightIn = WeightedMap());
    virtual CTemplateWeighted* clone() const;
    virtual bool GetSignDestination(const CTransaction& tx, const std::vector<uint8>& vchSig,
                                         std::set<CDestination>& setSubDest, std::vector<uint8>& vchSubSig) const;
    virtual void GetTemplateData(rpc::CTemplateResponse& obj, CDestination&& destInstance) const;

protected:
    virtual bool ValidateParam() const;
    virtual bool SetTemplateData(const std::vector<uint8>& vchDataIn);
    virtual bool SetTemplateData(const rpc::CTemplateRequest& obj, CDestination&& destInstance);
    virtual void BuildTemplateData();
    virtual bool VerifyTxSignature(const uint256& hash, const uint256& hashAnchor, const CDestination& destTo,
                                   const std::vector<uint8>& vchSig, bool& fCompleted) const;

protected:
    uint8 nRequired;
    WeightedMap mapPubKeyWeight;
};

#endif // MULTIVERSE_TEMPLATE_WEIGHTED_H