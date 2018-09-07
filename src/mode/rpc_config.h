// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_MODE_RPC_CONFIG_H
#define MULTIVERSE_MODE_RPC_CONFIG_H

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <string>
#include <vector>

#include "mode/basic_config.h"

namespace multiverse
{
class CMvRPCConfig : virtual public CMvBasicConfig
{
public:
    CMvRPCConfig();
    virtual ~CMvRPCConfig();
    virtual bool PostLoad();
    virtual std::string ListConfig() const;

public:
    boost::asio::ip::tcp::endpoint epRPC;
    std::string strRPCConnect;
    unsigned int nRPCPort;
    unsigned int nRPCConnectTimeout;
    unsigned int nRPCMaxConnections;
    std::vector<std::string> vRPCAllowIP;
    std::string strRPCWallet;
    std::string strRPCUser;
    std::string strRPCPass;
    bool fRPCSSLEnable;
    bool fRPCSSLVerify;
    std::string strRPCCAFile;
    std::string strRPCCertFile;
    std::string strRPCPKFile;
    std::string strRPCCiphers;

protected:
    int nRPCPortInt;
};

}  // namespace multiverse

#endif  // MULTIVERSE_MODE_RPC_CONFIG_H
