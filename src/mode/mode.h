// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_MODE_MODE_H
#define MULTIVERSE_MODE_MODE_H

#include <vector>

#include "mode/config_type.h"
#include "mode/mode_impl.h"
#include "mode/mode_type.h"
#include "mode/module_type.h"
#include "mode/basic_config.h"

namespace multiverse
{

/**
 * CMode class. Container of mode-module mapping and mode-config mapping.
 */
class CMode
{
public:
    static inline int TypeValue(EModeType type)
    {
        return static_cast<int>(type);
    }
    static inline int TypeValue(EModuleType type)
    {
        return static_cast<int>(type);
    }
    static inline int TypeValue(EConfigType type)
    {
        return static_cast<int>(type);
    }

public:
    /**
     * Create a multiple inheritance config class
     */
    static CMvBasicConfig* CreateConfig(const EModeType& type) noexcept
    {
        return modeimpl::CModeHelper::CreateConfig(type);
    }

    /**
     * Get the mapping module types of one mode.
     */
    static const std::vector<EModuleType> GetModules(const EModeType& type) noexcept
    {
        return modeimpl::CModeHelper::GetModules(type);
    }

    /**
     * Get the mapping config types of one mode.
     */
    static const std::vector<EConfigType> GetConfigs(const EModeType& type) noexcept
    {
        return modeimpl::CModeHelper::GetConfigs(type);
    }
};



}  // namespace multiverse

#endif  // MULTIVERSE_MODE_MODE_H