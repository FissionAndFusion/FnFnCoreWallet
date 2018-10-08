// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "mode/basic_config.h"

#include "mode/config_macro.h"

namespace multiverse
{
namespace po = boost::program_options;

CMvBasicConfig::CMvBasicConfig()
{
    po::options_description desc("MvBasic");

    CMvBasicConfigOption::AddOptionsImpl(desc);

    AddOptions(desc);
}

CMvBasicConfig::~CMvBasicConfig() {}

bool CMvBasicConfig::PostLoad()
{
    if (fHelp)
    {
        return true;
    }

    if (fTestNet)
    {
        pathData /= "testnet";
    }
    nMagicNum = fTestNet ? TESTNET_MAGICNUM : MAINNET_MAGICNUM;

    return true;
}

std::string CMvBasicConfig::ListConfig() const
{
    std::ostringstream oss;
    oss << CMvBasicConfigOption::ListConfigImpl();
    oss << "magicNum: " << nMagicNum << "\n";
    return oss.str();
}

std::string CMvBasicConfig::Help() const
{
    return CMvBasicConfigOption::HelpImpl();
}


}  // namespace multiverse