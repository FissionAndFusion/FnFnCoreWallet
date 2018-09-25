// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_MODE_NETWORK_CONFIG_H
#define MULTIVERSE_MODE_NETWORK_CONFIG_H

#include <string>
#include <vector>

#include "mode/basic_config.h"

namespace multiverse
{
class CMvNetworkConfig : virtual public CMvBasicConfig
{
public:
    CMvNetworkConfig();
    virtual ~CMvNetworkConfig();
    virtual bool PostLoad();
    virtual std::string ListConfig() const;

public:
    bool fListen;
    bool fBloom;
    unsigned short nPort;
    unsigned int nMaxInBounds;
    unsigned int nMaxOutBounds;
    unsigned int nConnectTimeout;
    std::vector<std::string> vNode;
    std::vector<std::string> vConnectTo;
    std::vector<std::string> vDNSeed;
    std::string strConfidentAddress;
    unsigned int nMaxTimes2ConnectFail;
    int nDNSeedPort;
protected:
    int nPortInt;
    int nMaxConnection;
};

}  // namespace multiverse

#endif  // MULTIVERSE_MODE_NETWORK_CONFIG_H
