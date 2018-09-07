// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "mode/rpc_config.h"

#include <boost/algorithm/algorithm.hpp>

namespace po = boost::program_options;
using tcp = boost::asio::ip::tcp;
namespace fs = boost::filesystem;
using namespace multiverse;

#define DEFAULT_RPCPORT 6812
#define DEFAULT_TESTNET_RPCPORT 6814
#define DEFAULT_RPC_MAX_CONNECTIONS 5
#define DEFAULT_RPC_CONNECT_TIMEOUT 120

CMvRPCConfig::CMvRPCConfig()
{
    po::options_description desc("LoMoRPC");

    AddOpt<std::string>(desc, "rpcconnect", strRPCConnect, "127.0.0.1");
    AddOpt<int>(desc, "rpcport", nRPCPortInt, 0);
    AddOpt<unsigned int>(desc, "rpctimeout", nRPCConnectTimeout,
                         DEFAULT_RPC_CONNECT_TIMEOUT);
    AddOpt<unsigned int>(desc, "rpcmaxconnections", nRPCMaxConnections,
                         DEFAULT_RPC_MAX_CONNECTIONS);
    AddOpt<std::vector<std::string> >(desc, "rpcallowip", vRPCAllowIP);

    AddOpt<std::string>(desc, "rpcwallet", strRPCWallet, "");
    AddOpt<std::string>(desc, "rpcuser", strRPCUser, "");
    AddOpt<std::string>(desc, "rpcpassword", strRPCPass, "");

    AddOpt<bool>(desc, "rpcssl", fRPCSSLEnable, false);
    AddOpt<bool>(desc, "rpcsslverify", fRPCSSLVerify, true);
    AddOpt<std::string>(desc, "rpcsslcafile", strRPCCAFile, "ca.crt");
    AddOpt<std::string>(desc, "rpcsslcertificatechainfile", strRPCCertFile,
                        "server.crt");
    AddOpt<std::string>(desc, "rpcsslprivatekeyfile", strRPCPKFile,
                        "server.key");
    AddOpt<std::string>(desc, "rpcsslciphers", strRPCCiphers,
                        "TLSv1+HIGH:!SSLv2:!aNULL:!eNULL:!AH:!3DES:@STRENGTH");

    AddOptions(desc);
}

CMvRPCConfig::~CMvRPCConfig() {}

bool CMvRPCConfig::PostLoad()
{
    if (nRPCPortInt <= 0 || nRPCPortInt > 0xFFFF)
    {
        nRPCPort = (fTestNet ? DEFAULT_TESTNET_RPCPORT : DEFAULT_RPCPORT);
    }
    else
    {
        nRPCPort = (unsigned short)nRPCPortInt;
    }

    if (nRPCConnectTimeout == 0)
    {
        nRPCConnectTimeout = 1;
    }

    epRPC = tcp::endpoint(!vRPCAllowIP.empty()
                              ? boost::asio::ip::address_v4::any()
                              : boost::asio::ip::address_v4::loopback(),
                          nRPCPort);

    if (!fs::path(strRPCCAFile).is_complete())
    {
        strRPCCAFile = (pathRoot / strRPCCAFile).string();
    }

    if (!fs::path(strRPCCertFile).is_complete())
    {
        strRPCCertFile = (pathRoot / strRPCCertFile).string();
    }

    if (!fs::path(strRPCPKFile).is_complete())
    {
        strRPCPKFile = (pathRoot / strRPCPKFile).string();
    }

    return true;
}

std::string CMvRPCConfig::ListConfig() const
{
    std::ostringstream oss;
    oss << "rpc config:\n";
    return oss.str();
}