// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
//
//
// Introduction about Mode:
// 
// Concept:
//  - Mode: Program execution mode. e.g. server, console, miner, dnseed...
//  - Module: Some mostly independent function. e.g. txpool, httpserver, worldline...
//  - Config: program running depends on configurations from command-line and configuration file.
//            Divided into several configuration class by need. e.g. network, mint...
//
// Relationship:
//  - One mode conrresponds on multiple modules, and multiple configures.
//  - If there are dependencies in implementation among modules or configurations, configure them by order.
//
// Operation:
//  - Add new mode: In "mode_type.h", add a new mode type. 
//  - Add new module: In "module_type.h", add a new module type.
//  - Add new config: In "config_type.h", add a new config type, 
//                    and add corresponding class point into ___ConfigTypeTemplate. 
//  - Add new mode-module relationship: In "mode.h" file bottom, add into "CMode::CreateConfig" function.
//          format: case mode_type:
//                  {
//                      return create<config_type1, config_type2, ...>();
//                  }
//          e.g:    case EModeType::SERVER:
//                  {
//                      return create<
//                                      EConfigType::MINT, 
//                                      EConfigType::NETWORK,
//                                      EConfigType::RPC, 
//                                      EConfigType::STORAGE
//                                   >();
//                  }
//  - Add new mode-config relationship: In "mode.h" file bottom, add into "CMode::GetModules" function,
//                                        "mapping" variable definition.
//          format: { 
//                      mode_type, 
//                      {
//                          module_type1,
//                          module_type2,
//                          ...
//                      }
//                  },
//          e.g.:   { 
//                      EModeType::MINER, 
//                      {
//                          EModuleType::HTTPGET,
//                          EModuleType::MINER
//                      }
//                  },

#ifndef MULTIVERSE_MODE_H
#define MULTIVERSE_MODE_H

#include <tuple>
#include <vector>

#include "mode/mode_impl.h"
#include "mode/mode_type.h"
#include "mode/module_type.h"
#include "mode/basic_config.h"
#include "mode/mint_config.h"
#include "mode/network_config.h"
#include "mode/rpc_config.h"
#include "mode/storage_config.h"

#include "rpc/auto_rpc.h"

namespace multiverse
{
/**
 * CMode class. Container of mode-module mapping and mode-config mapping.
 */
class CMode
{
public:
    static inline int IntValue(EModeType type)
    {
        return static_cast<int>(type);
    }
    static inline int IntValue(EModuleType type)
    {
        return static_cast<int>(type);
    }
    static inline int IntValue(EConfigType type)
    {
        return static_cast<int>(type);
    }

private:
    template <EConfigType... t>
    static CMvBasicConfig* Create(const std::string& cmd)
    {
        if (mode_impl::CCheckType<t...>().Exist(EConfigType::RPCCLIENT))
        {
            return rpc::CreateConfig<
                typename std::remove_pointer<typename std::tuple_element<
                    static_cast<int>(t),
                    decltype(config_type::___ConfigTypeTemplate)>::type>::type...>(cmd);
        }
        else
        {
            return new mode_impl::CCombinConfig<
                typename std::remove_pointer<typename std::tuple_element<
                    static_cast<int>(t),
                    decltype(config_type::___ConfigTypeTemplate)>::type>::type...>;
        }
    }

public:
    /**
     * Create a multiple inheritance config class.
     */
    static CMvBasicConfig* CreateConfig(const EModeType& type, const std::string& cmd) noexcept
    {
        switch (type)
        {
            case EModeType::ERROR:
            {
                return NULL;
            }
            case EModeType::SERVER:
            {
                return Create<
                                EConfigType::MINT, 
                                EConfigType::NETWORK,
                                EConfigType::RPCSERVER, 
                                EConfigType::STORAGE
                             >(cmd);
            }
            case EModeType::CONSOLE:
            {
                return Create<
                                EConfigType::RPCCLIENT
                             >(cmd);
            }
            case EModeType::MINER:
            {
                return Create<
                                EConfigType::RPCCLIENT
                             >(cmd);
            }
            case EModeType::DNSEED:
            {
                return Create<
                                EConfigType::NETWORK,
                                EConfigType::STORAGE
                             >(cmd);
            }
            // Add new mode-module relationship here.
            default:
            {
                return Create<>(cmd);
            }
        }
    }

    /**
     * Get the mapping module types of one mode.
     */
    static const std::vector<EModuleType> GetModules(const EModeType& mode) noexcept
    {
        static std::map<EModeType, std::vector<EModuleType> > mapping = {
            {
                EModeType::SERVER, 
                {
                    EModuleType::LOCK,
                    EModuleType::COREPROTOCOL,
                    EModuleType::WORLDLINE,
                    EModuleType::TXPOOL,
                    EModuleType::CONSENSUS,
                    EModuleType::WALLET,
                    EModuleType::DISPATCHER,
                    EModuleType::NETWORK,
                    EModuleType::NETCHANNEL,
                    EModuleType::SERVICE,
                    EModuleType::DBPSOCKET,
                    EModuleType::HTTPSERVER,
                    EModuleType::RPCMODE,
                    EModuleType::BLOCKMAKER
                }
            },
            {
                EModeType::CONSOLE, 
                {
                    EModuleType::HTTPGET,
                    EModuleType::RPCCLIENT
                }
            },
            {
                EModeType::MINER, 
                {
                    EModuleType::HTTPGET,
                    EModuleType::MINER
                }
            },
            {
                EModeType::DNSEED, 
                {
                    EModuleType::LOCK,
                    EModuleType::DNSEED,
                }
            }
            // Add new mode-config relationship here.
        };

        auto it = mapping.find(mode);
        return (it == mapping.end()) ? std::vector<EModuleType>() : it->second;
    }

    /**
     * Entry of auto_rpc function "Help"
     */
    static std::string Help(EModeType type, const std::string& subCmd, const std::string& options = "")
    {
        return rpc::Help(type, subCmd, options);
    }
};

}  // namespace multiverse

#endif  // MULTIVERSE_MODE_H