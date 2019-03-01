// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_TEMPLATE_FORK_H
#define MULTIVERSE_TEMPLATE_FORK_H

#include "template.h"

class CTemplateFork : virtual public CTemplate
{
public:
    CTemplateFork(const CDestination& destRedeemIn = CDestination(), const uint256& hashForkIn = uint256());
    virtual CTemplateFork* clone() const;
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
    CDestination destRedeem;
    uint256 hashFork;
};

#endif // MULTIVERSE_TEMPLATE_FORK_H