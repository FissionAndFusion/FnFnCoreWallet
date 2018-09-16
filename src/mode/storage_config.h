// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_MODE_STORAGE_CONFIG_H
#define MULTIVERSE_MODE_STORAGE_CONFIG_H

#include <string>

#include "mode/basic_config.h"

namespace multiverse
{
class CMvStorageConfig : virtual public CMvBasicConfig
{
public:
    CMvStorageConfig();
    virtual ~CMvStorageConfig();
    virtual bool PostLoad();
    virtual std::string ListConfig() const;

public:
    std::string strDBHost;
    std::string strDBName;
    std::string strDBUser;
    std::string strDBPass;
    int nDBPort;
    int nDBConn;

};

}  // namespace multiverse

#endif  // MULTIVERSE_MODE_STORAGE_CONFIG_H
