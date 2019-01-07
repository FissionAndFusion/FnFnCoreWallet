// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_RPC_CONFIG_H
#define MULTIVERSE_RPC_CONFIG_H

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <string>
#include <vector>

#include "mode/basic_config.h"

namespace multiverse
{

class CMvRPCBasicConfig : virtual public CMvBasicConfig, 
                          virtual public CMvRPCBasicConfigOption
{
public:
    CMvRPCBasicConfig();
    virtual ~CMvRPCBasicConfig();
    virtual bool PostLoad();
    virtual std::string ListConfig() const;
    virtual std::string Help() const;

public:
    unsigned int nRPCPort;
};

class CMvRPCServerConfig : virtual public CMvRPCBasicConfig, 
                           virtual public CMvRPCServerConfigOption
{
public:
    CMvRPCServerConfig();
    virtual ~CMvRPCServerConfig();
    virtual bool PostLoad();
    virtual std::string ListConfig() const;
    virtual std::string Help() const;

public:
    boost::asio::ip::tcp::endpoint epRPC;
};

class CMvRPCClientConfig : virtual public CMvRPCBasicConfig,
                           virtual public CMvRPCClientConfigOption
{
public:
    CMvRPCClientConfig();
    virtual ~CMvRPCClientConfig();
    virtual bool PostLoad();
    virtual std::string ListConfig() const;
    virtual std::string Help() const;
};

}  // namespace multiverse

#endif  // MULTIVERSE_RPC_CONFIG_H
