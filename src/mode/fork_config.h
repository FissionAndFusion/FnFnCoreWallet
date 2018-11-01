// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_FORK_CONFIG_H
#define MULTIVERSE_FORK_CONFIG_H

#include <string>
#include <vector>

#include "mode/basic_config.h"

namespace multiverse
{
class CMvForkConfig : virtual public CMvBasicConfig,
                      virtual public CMvForkConfigOption
{
public:
    CMvForkConfig();
    virtual ~CMvForkConfig();
    virtual bool PostLoad();
    virtual std::string ListConfig() const;
    virtual std::string Help() const;
public:
    bool fAllowAnyFork;
};

}  // namespace multiverse

#endif  // MULTIVERSE_FORK_CONFIG_H
