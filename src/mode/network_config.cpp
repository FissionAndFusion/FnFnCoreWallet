// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "mode/network_config.h"

#include "mode/config_macro.h"

namespace multiverse
{
namespace po = boost::program_options;

CMvNetworkConfig::CMvNetworkConfig()
{
    po::options_description desc("LoMoNetwork");

    CMvNetworkConfigOption::AddOptionsImpl(desc);

    AddOptions(desc);
}
CMvNetworkConfig::~CMvNetworkConfig() {}

bool CMvNetworkConfig::PostLoad()
{
    if (fHelp)
    {
        return true;
    }

    if (nPortInt <= 0 || nPortInt > 0xFFFF)
    {
        nPort = (fTestNet ? DEFAULT_TESTNET_P2PPORT : DEFAULT_P2PPORT);
    }
    else
    {
        nPort = (unsigned short)nPortInt;
    }

    nMaxOutBounds = DEFAULT_MAX_OUTBOUNDS;
    if (nMaxConnection <= DEFAULT_MAX_OUTBOUNDS)
    {
        nMaxInBounds = (fListen4 || fListen6) ? 1 : 0;
    }
    else
    {
        nMaxInBounds = nMaxConnection - DEFAULT_MAX_OUTBOUNDS;
    }

    if (nConnectTimeout == 0)
    {
        nConnectTimeout = 1;
    }

    if (!fTestNet)
    {
        // vDNSeed.push_back("118.193.83.220");
    }
    return true;
}

std::string CMvNetworkConfig::ListConfig() const
{
    std::ostringstream oss;
    oss << CMvNetworkConfigOption::ListConfigImpl();
    oss << "port: " << nPort << "\n";
    oss << "maxOutBounds: " << nMaxOutBounds << "\n";
    oss << "maxInBounds: " << nMaxInBounds << "\n";
    oss << "dnseed: ";
    for (auto& s: vDNSeed)
    {
        oss << s << " ";
    }
    oss << "\n";
    return oss.str();
}

std::string CMvNetworkConfig::Help() const
{
    return CMvNetworkConfigOption::HelpImpl();
}

}  // namespace multiverse