// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "mode/basic_config.h"

namespace po = boost::program_options;
using namespace multiverse;

#define MAINNET_MAGICNUM 0xd5f97d23
#define TESTNET_MAGICNUM 0xef93a7b2

CMvBasicConfig::CMvBasicConfig()
{
    po::options_description desc("MvBasic");

    AddOpt<bool>(desc, "testnet", fTestNet, false);

    AddOptions(desc);
}

CMvBasicConfig::~CMvBasicConfig() {}

bool CMvBasicConfig::PostLoad()
{
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
    oss << "TestNet: " << (fTestNet ? "Y" : "N") << "\n";
    return oss.str();
}