// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_CONFIG_MACRO_H
#define MULTIVERSE_CONFIG_MACRO_H

#include <boost/program_options.hpp>

// basic config
#define MAINNET_MAGICNUM 0xd5f97d23
#define TESTNET_MAGICNUM 0xef93a7b2

// rpc config
#define DEFAULT_RPCPORT 6812
#define DEFAULT_TESTNET_RPCPORT 6814
#define DEFAULT_RPC_MAX_CONNECTIONS 5
#define DEFAULT_RPC_CONNECT_TIMEOUT 120

// dbp config
#define DEFAULT_DBPPORT 6815
#define DEFAULT_TESTNET_DBPPORT 6817
#define DEFAULT_DBP_MAX_CONNECTIONS 20
#define DEFAULT_DBP_SESSION_TIMEOUT 60 * 5

// network config
#define DEFAULT_P2PPORT 6811
#define DEFAULT_TESTNET_P2PPORT 6813
#define DEFAULT_MAX_INBOUNDS 125
#define DEFAULT_MAX_OUTBOUNDS 10
#define DEFAULT_CONNECT_TIMEOUT 5
#define DEFAULT_DNSEED_PORT 6816
#define DNSEED_DEFAULT_MAX_TIMES_CONNECT_FAIL 5

// storage config
#define DEFAULT_DB_CONNECTION 8

// add options
template <typename T>
inline void AddOpt(boost::program_options::options_description& desc,
                    const std::string& name, T& var)
{
    desc.add_options()
        (name.c_str(), boost::program_options::value<T>(&var));
}
template <typename T>
inline void AddOpt(boost::program_options::options_description& desc,
                    const std::string& name, T& var, const T& def)
{
    desc.add_options()
        (name.c_str(), boost::program_options::value<T>(&var)->default_value(def));
}
template <typename T>
inline void AddOpt(boost::program_options::options_description& desc, 
                    const std::string& name)
{
    desc.add_options()
        (name.c_str(), boost::program_options::value<T>());
}
template <typename T>
inline void AddOpt(boost::program_options::options_description& desc, 
                    const std::string& name, const T& def)
{
    desc.add_options()
        (name.c_str(), boost::program_options::value<T>()->default_value(def));
}

#endif // MULTIVERSE_CONFIG_MACRO_H