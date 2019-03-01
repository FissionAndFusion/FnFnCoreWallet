// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_TEMPLATE_MINT_H
#define MULTIVERSE_TEMPLATE_MINT_H

#include <boost/shared_ptr.hpp>

#include "template.h"

class CTemplateMint;
typedef boost::shared_ptr<CTemplateMint> CTemplateMintPtr;

class CBlock;

class CTemplateMint : virtual public CTemplate
{
public:
    // Construct by CTemplateMint pointer.
    static const CTemplateMintPtr CreateTemplatePtr(CTemplateMint* ptr);

    // Verify block signature.
    static bool VerifyBlockSignature(const CTemplateId& nIdIn, const uint256& hash, const std::vector<uint8>& vchSig);

    // Build block signature by concrete template.
    bool BuildBlockSignature(const uint256& hash, const std::vector<uint8>& vchPreSig, std::vector<uint8>& vchSig) const;

protected:
    // Verify block signature by concrete template.
    virtual bool VerifyBlockSignature(const uint256& hash, const std::vector<uint8>& vchSig) const = 0;
};

#endif // MULTIVERSE_TEMPLATE_MINT_H