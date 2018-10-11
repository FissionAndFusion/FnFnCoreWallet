// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_NETWORK_CONFIG_H
#define MULTIVERSE_NETWORK_CONFIG_H

#include <string>
#include <vector>

#include "mode/basic_config.h"

namespace multiverse
{
class CMvNetworkConfig : virtual public CMvBasicConfig,
                         virtual public CMvNetworkConfigOption
{
public:
    CMvNetworkConfig();
    virtual ~CMvNetworkConfig();
    virtual bool PostLoad();
    virtual std::string ListConfig() const;
    virtual std::string Help() const;

public:
    unsigned short nPort;
    unsigned int nMaxInBounds;
    unsigned int nMaxOutBounds;
    std::vector<std::string> vDNSeed;
protected:
    int nPortInt;
    int nMaxConnection;
};

}  // namespace multiverse

#endif  // MULTIVERSE_NETWORK_CONFIG_H
