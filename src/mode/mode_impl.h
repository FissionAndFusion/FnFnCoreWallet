// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_MODE_IMPL_H
#define MULTIVERSE_MODE_IMPL_H

#include <algorithm>
#include <map>
#include <numeric>
#include <type_traits>
#include <vector>

#include "mode/basic_config.h"
#include "mode/mint_config.h"
#include "mode/network_config.h"
#include "mode/rpc_config.h"
#include "mode/storage_config.h"

#include "mode/config_type.h"
#include "mode/mode_type.h"
#include "mode/module_type.h"

namespace multiverse
{

namespace modeimpl
{

/**
 * Combination of inheriting all need config class.
 */
template <typename... U>
class CCombinConfig
    : virtual public std::enable_if<std::is_base_of<CMvBasicConfig, U>::value, U>::type...
{
public:
    CCombinConfig() {}
    virtual ~CCombinConfig() {}

    virtual bool PostLoad()
    {
        std::vector<bool> v = {walleve::CWalleveConfig::PostLoad(),
                               CMvBasicConfig::PostLoad(), U::PostLoad()...};
        return std::find(v.begin(), v.end(), false) == v.end();
    }

    virtual std::string ListConfig() const
    {
        std::vector<std::string> v = {walleve::CWalleveConfig::ListConfig(),
                                      CMvBasicConfig::ListConfig(),
                                      U::ListConfig()...};
        return std::accumulate(v.begin(), v.end(), std::string());
    }
};

template <>
class CCombinConfig<> : virtual public CCombinConfig<CMvBasicConfig> {};


// create entry
template <EConfigType... t>
CMvBasicConfig* create()
{
    return new CCombinConfig<
        typename std::remove_pointer<typename std::tuple_element<
            static_cast<int>(t),
            decltype(configtype::___ConfigTypeTemplate)>::type>::type...>;
}

/**
 * helper class for registering mode-module, mode-config relationship by macro
 */
class CModeHelper
{
public:
    // singleton
    static CModeHelper& GetInstance()
    {
        static CModeHelper helper;
        return helper;
    }

    /**
     * Get the mapping module types of one mode.
     */
    static const std::vector<EModuleType> GetModules(const EModeType& type) noexcept
    {
        auto mapping = GetModulesMapping();
        auto it = mapping.find(type);
        return (it != mapping.end()) ? it->second : std::vector<EModuleType>();
    }

    /**
     * Get the mapping config types of one mode.
     */
    static const std::vector<EConfigType> GetConfigs(const EModeType& type) noexcept
    {
        auto mapping = GetConfigsMapping();
        auto it = mapping.find(type);
        return (it != mapping.end()) ? it->second : std::vector<EConfigType>();
    }

private:
    template<EModeType m, EModuleType... t>
    static bool RegisterModules() noexcept
    {
        GetModulesMapping()[m] = std::vector<EModuleType>({t...});
        return true;
    }

    template<EModeType m, EConfigType... t>
    static bool RegisterConfigs() noexcept
    {
        GetConfigsMapping()[m] = std::vector<EConfigType>({t...});
        return true;
    }
    
    static inline std::map<EModeType, std::vector<EModuleType> >& GetModulesMapping() noexcept
    {
        // mode-modules mapping
        static std::map<EModeType, std::vector<EModuleType> > mapping;
        return mapping;
    }

    static inline std::map<EModeType, std::vector<EConfigType> >& GetConfigsMapping() noexcept
    {
        // mode-modules mapping
        static std::map<EModeType, std::vector<EConfigType> > mapping;
        return mapping;
    }
public:
    static CMvBasicConfig* CreateConfig(const EModeType& t) noexcept
    {
        if (t == EModeType::ERROR)
        {
            return NULL;
        }

#define GENERATE_CONFIG(type, ...) \
    else if (t == EModeType::type) \
    { \
        return create<__VA_ARGS__>(); \
    }

// ----------------------------ADD_CONFIGS begin------------------------------
// MACRO: Add mode-config mapping here.
//
// Format: GENERATE_CONFIG(mode, config1, config2 ...)
//

        GENERATE_CONFIG(SERVER, 
            EConfigType::MINT, 
            EConfigType::NETWORK,
            EConfigType::RPC, 
            EConfigType::STORAGE)

        GENERATE_CONFIG(CONSOLE,
            EConfigType::RPC)

        GENERATE_CONFIG(MINER,
            EConfigType::RPC)


// ----------------------------ADD_CONFIGS end------------------------------

#undef GENERATE_CONFIG
        else
        {
            return create<EConfigType::BASIC>();
        }
    }

private:
    CModeHelper()
    {

#define ADD_MODULES(mode, ...) RegisterModules<EModeType::mode, ##__VA_ARGS__>()
// ----------------------------ADD_MODULES begin------------------------------
// MACRO: Add new mode-modules here
// Format: ADD_MODULES(mode, module1, module2 ...);
//

// EModeType::SERVER mode-modules
ADD_MODULES(SERVER,
    EModuleType::LOCK,
    EModuleType::COREPROTOCOL,
    EModuleType::WORLDLINE,
    EModuleType::TXPOOL,
    EModuleType::WALLET,
    EModuleType::DISPATCHER,
    EModuleType::NETWORK,
    EModuleType::NETCHANNEL,
    EModuleType::SERVICE,
    EModuleType::HTTPSERVER,
    EModuleType::RPCMODE,
    EModuleType::BLOCKMAKER);

// EModeType::CONSOLE mode-modules
ADD_MODULES(CONSOLE,
    EModuleType::HTTPGET,
    EModuleType::RPCDISPATCH);

// EModeType::MINER mode-modules
ADD_MODULES(MINER,
    EModuleType::HTTPGET,
    EModuleType::MINER);

// ----------------------------ADD_MODULES end------------------------------
#undef ADD_MODULES
    }
};

static const CModeHelper& help = CModeHelper::GetInstance();

} // namespace modeimpl

} // namespace multiverse


#endif // MULTIVERSE_MODE_IMPL_H