// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "mode/network_config.h"

namespace po = boost::program_options;
using namespace multiverse;

#define DEFAULT_P2PPORT 6811
#define DEFAULT_TESTNET_P2PPORT 6813
#define DEFAULT_DNSEED_PORT             6816
#define DEFAULT_MAX_INBOUNDS 125
#define DEFAULT_MAX_OUTBOUNDS 10
#define DEFAULT_CONNECT_TIMEOUT 5
#define DNSEED__DEFAULT_MAX_TIMES_CONNECT_FAIL 5

CMvNetworkConfig::CMvNetworkConfig()
{
    po::options_description desc("LoMoNetwork");

    AddOpt<bool>(desc, "listen", fListen, false);
    AddOpt<bool>(desc, "bloom", fBloom, true);
    AddOpt<int>(desc, "port", nPortInt, 0);
    AddOpt<int>(desc, "maxconnections", nMaxConnection,
                DEFAULT_MAX_OUTBOUNDS + DEFAULT_MAX_INBOUNDS);
    AddOpt<unsigned int>(desc, "timeout", nConnectTimeout,
                         DEFAULT_CONNECT_TIMEOUT);
    AddOpt<std::vector<std::string> >(desc, "addnode", vNode);
    AddOpt<std::vector<std::string> >(desc, "connect", vConnectTo);
    AddOpt<std::vector<std::string> >(desc, "dnseednode", vDNSeed);
    AddOpt<int>(desc, "dnseedport",nDNSeedPort,DEFAULT_DNSEED_PORT);
    AddOpt<unsigned int>(desc, "dnseedmaxtimes",nMaxTimes2ConnectFail,
                                DNSEED__DEFAULT_MAX_TIMES_CONNECT_FAIL);
    AddOpt<std::string>(desc, "confidentAddress", strConfidentAddress, "");                              
    AddOptions(desc);
}
CMvNetworkConfig::~CMvNetworkConfig() {}

bool CMvNetworkConfig::PostLoad()
{
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
        nMaxInBounds = fListen ? 1 : 0;
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
        vDNSeed.push_back("113.105.146.22");
        //        vDNSeed.push_back("123.57.22.233");
    }
    return true;
}

std::string CMvNetworkConfig::ListConfig() const
{
    std::ostringstream oss;
    oss << "network config:\n";
    return oss.str();
}
