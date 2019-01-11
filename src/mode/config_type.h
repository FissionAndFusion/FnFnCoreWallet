// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_TEMPLATE_CONFIG_TYPE_H
#define MULTIVERSE_TEMPLATE_CONFIG_TYPE_H

#include <tuple>

namespace multiverse
{
class CMvBasicConfig;
class CMvForkConfig;
class CMvMintConfig;
class CMvNetworkConfig;
class CMvRPCServerConfig;
class CMvRPCClientConfig;
class CMvStorageConfig;
class CMvDbpServerConfig;
class CMvDbpClientConfig;

///
// When add new config, add type to EConfigType add class pointer to ___ConfigTypeTemplate
//

// config type
enum class EConfigType
{
    BASIC,      // CMvBasicConfig
    FORK,       // CMvForkConfig
    MINT,       // CMvMintConfig
    NETWORK,    // CMvNetworkConfig
    RPCSERVER,  // CMvRPCServerConfig
    RPCCLIENT,  // CMvRPCClientConfig
    STORAGE,    // CMvStorageConfig
    DBPSERVER,  // CMvDbpServerConfig
    DBPCLIENT   // CMvDbpClientConfig
};

namespace config_type
{
// config type class template
static const auto ___ConfigTypeTemplate = std::make_tuple(
    (CMvBasicConfig*)NULL,
    (CMvForkConfig*)NULL, 
    (CMvMintConfig*)NULL, 
    (CMvNetworkConfig*)NULL,
    (CMvRPCServerConfig*)NULL, 
    (CMvRPCClientConfig*)NULL, 
    (CMvStorageConfig*)NULL,
    (CMvDbpServerConfig*)NULL,
    (CMvDbpClientConfig*)NULL);
}  // namespace config_type

}  // namespace multiverse

#endif  // MULTIVERSE_TEMPLATE_CONFIG_TYPE_H