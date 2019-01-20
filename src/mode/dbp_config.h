// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_DBP_CONFIG_H
#define MULTIVERSE_DBP_CONFIG_H

#include "mode/basic_config.h"

namespace multiverse
{
class CMvDbpBasicConfig : virtual public CMvBasicConfig,
                          virtual public CMvDbpBasicConfigOption
{
public:
    CMvDbpBasicConfig();
    virtual ~CMvDbpBasicConfig();
    virtual bool PostLoad();
    virtual std::string ListConfig() const;
    virtual std::string Help() const;

public:
    unsigned int nDbpPort;
};

class CMvDbpServerConfig : virtual public CMvDbpBasicConfig,
                           virtual public CMvDbpServerConfigOption
{
public:
    CMvDbpServerConfig();
    virtual ~CMvDbpServerConfig();
    virtual bool PostLoad();
    virtual std::string ListConfig() const;
    virtual std::string Help() const;

public:
    boost::asio::ip::tcp::endpoint epDbp;
};

class CMvDbpClientConfig : virtual public CMvDbpBasicConfig,
                           virtual public CMvDbpClientConfigOption
{
public:
    CMvDbpClientConfig();
    virtual ~CMvDbpClientConfig();
    virtual bool PostLoad();
    virtual std::string ListConfig() const;
    virtual std::string Help() const;
public:
    boost::asio::ip::tcp::endpoint epParentHost;
};
} // namespace multiverse

#endif  // MULTIVERSE_DBP_CONFIG_H