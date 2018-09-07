// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "mode/mint_config.h"

#include "address.h"

namespace po = boost::program_options;
using namespace multiverse;

CMvMintConfig::CMvMintConfig()
{
    po::options_description desc("LoMoMint");

    AddOpt<std::string>(desc, "mpvssaddress", strAddressMPVss, "");
    AddOpt<std::string>(desc, "mpvsskey", strKeyMPVss, "");
    AddOpt<std::string>(desc, "blake512address", strAddressBlake512, "");
    AddOpt<std::string>(desc, "blake512key", strKeyBlake512, "");

    AddOptions(desc);
}

CMvMintConfig::~CMvMintConfig() {}

bool CMvMintConfig::PostLoad()
{
    ExtractMintParamPair(strAddressMPVss, strKeyMPVss, destMPVss, keyMPVss);
    ExtractMintParamPair(strAddressBlake512, strKeyBlake512, destBlake512,
                         keyBlake512);
    return true;
}

std::string CMvMintConfig::ListConfig() const
{
    std::ostringstream oss;
    oss << "mint config:\n";
    return oss.str();
}

void CMvMintConfig::ExtractMintParamPair(const std::string& strAddress,
                                         const std::string& strKey,
                                         CDestination& dest, uint256& privkey)
{
    CMvAddress address;
    if (address.ParseString(strAddress) && !address.IsNull() &&
        strKey.size() == 64)
    {
        dest = address;
        privkey.SetHex(strKey);
    }
}