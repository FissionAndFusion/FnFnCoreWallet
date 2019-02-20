// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "mode/mint_config.h"

#include "mode/config_macro.h"
#include "address.h"

namespace multiverse
{
namespace po = boost::program_options;

CMvMintConfig::CMvMintConfig()
{
    po::options_description desc("LoMoMint");

    CMvMintConfigOption::AddOptionsImpl(desc);

    AddOptions(desc);
}

CMvMintConfig::~CMvMintConfig() {}

bool CMvMintConfig::PostLoad()
{
    if (fHelp)
    {
        return true;
    }

    ExtractMintParamPair(strAddressMPVss, strKeyMPVss, destMPVss, keyMPVss);
    ExtractMintParamPair(strAddressBlake512, strKeyBlake512, destBlake512,
                         keyBlake512);
    return true;
}

std::string CMvMintConfig::ListConfig() const
{
    std::ostringstream oss;
    oss << CMvMintConfigOption::ListConfigImpl();
    oss << "destMPVss: " << destMPVss.ToString() << "\n";
    oss << "keyMPVss: " << keyMPVss.GetHex() << "\n";
    oss << "destBlake512: " << destBlake512.ToString() << "\n";
    oss << "keyBlake512: " << keyBlake512.GetHex() << "\n";
    return oss.str();
}

std::string CMvMintConfig::Help() const
{
    return CMvMintConfigOption::HelpImpl();
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

////  CMvForkNodeMintConfig ///////////

CMvForkNodeMintConfig::CMvForkNodeMintConfig()
{
    po::options_description desc("ForkNodeMint");

    CMvForkNodeMintConfigOption::AddOptionsImpl(desc);

    AddOptions(desc);
}

CMvForkNodeMintConfig::~CMvForkNodeMintConfig() {}

bool CMvForkNodeMintConfig::PostLoad()
{
    if (fHelp)
    {
        return true;
    }

    ExtractMintParamPair(strForkNodeAddressMPVss, strForkNodeKeyMPVss, destMPVss, keyMPVss);
  
    return true;
}

std::string CMvForkNodeMintConfig::ListConfig() const
{
    std::ostringstream oss;
    oss << CMvForkNodeMintConfigOption::ListConfigImpl();
    oss << "destMPVss: " << destMPVss.ToString() << "\n";
    oss << "keyMPVss: " << keyMPVss.GetHex() << "\n";
    return oss.str();
}

std::string CMvForkNodeMintConfig::Help() const
{
    return CMvForkNodeMintConfigOption::HelpImpl();
}

void CMvForkNodeMintConfig::ExtractMintParamPair(const std::string& strAddress,
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

}  // namespace multiverse