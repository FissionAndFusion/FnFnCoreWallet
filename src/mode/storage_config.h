// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_STORAGE_CONFIG_H
#define MULTIVERSE_STORAGE_CONFIG_H

#include <string>

#include "mode/basic_config.h"

namespace multiverse
{
class CMvStorageConfig : virtual public CMvBasicConfig,
                         virtual public CMvStorageConfigOption
{
public:
    CMvStorageConfig();
    virtual ~CMvStorageConfig();
    virtual bool PostLoad();
    virtual std::string ListConfig() const;
    virtual std::string Help() const;
};

}  // namespace multiverse

#endif  // MULTIVERSE_STORAGE_CONFIG_H
