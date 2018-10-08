// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_MINT_CONFIG_H
#define MULTIVERSE_MINT_CONFIG_H

#include <string>

#include "destination.h"
#include "uint256.h"

#include "mode/basic_config.h"

namespace multiverse
{
class CMvMintConfig : virtual public CMvBasicConfig,
                      virtual public CMvMintConfigOption
{
public:
    CMvMintConfig();
    virtual ~CMvMintConfig();
    virtual bool PostLoad();
    virtual std::string ListConfig() const;
    virtual std::string Help() const;

protected:
    void ExtractMintParamPair(const std::string& strAddress,
                              const std::string& strKey, CDestination& dest,
                              uint256& privkey);

public:
    CDestination destMPVss;
    uint256 keyMPVss;
    CDestination destBlake512;
    uint256 keyBlake512;

};

}  // namespace multiverse

#endif  // MULTIVERSE_MINT_CONFIG_H
