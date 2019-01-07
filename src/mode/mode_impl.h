// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_MODE_IMPL_H
#define MULTIVERSE_MODE_IMPL_H

#include <set>
#include <numeric>
#include <type_traits>

#include "mode/config_type.h"
#include "mode/basic_config.h"

namespace multiverse
{
namespace mode_impl
{
/**
 * Combination of inheriting all need config class.
 */
template <typename... U>
class CCombinConfig
    : virtual public std::enable_if<std::is_base_of<CMvBasicConfig, U>::value,
                                    U>::type...
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

    virtual std::string Help() const
    {
        std::vector<std::string> v = {walleve::CWalleveConfig::Help(),
                                      CMvBasicConfig::Help(),
                                      U::Help()...};
        return std::accumulate(v.begin(), v.end(), std::string());
    }
};

template <>
class CCombinConfig<> : virtual public CCombinConfig<CMvBasicConfig>
{
};

/**
 * check if type exists in t...
 */
template <EConfigType... t>
class CCheckType
{
public:
    bool Exist(EConfigType type)
    {
        std::set<EConfigType> s = {t...};
        return s.find(type) != s.end();
    }
};

}  // namespace mode_impl

}  // namespace multiverse

#endif  // MULTIVERSE_MODE_IMPL_H