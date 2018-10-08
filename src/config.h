// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_CONFIG_H
#define MULTIVERSE_CONFIG_H

#include <string>
#include <type_traits>
#include <vector>

#include "mode/mode.h"
#include "walleve/walleve.h"

namespace multiverse
{
class CMvConfig
{
public:
    CMvConfig();
    ~CMvConfig();
    bool Load(int argc,char *argv[],const boost::filesystem::path& pathDefault,
              const std::string& strConfile);
    bool PostLoad();
    void ListConfig() const;
    std::string Help() const;

    inline CMvBasicConfig* GetConfig() { return pImpl; }
    inline EModeType GetModeType() { return emMode; }
protected:
    EModeType emMode;
    std::string subCmd;
    CMvBasicConfig* pImpl;
};

}  // namespace multiverse

#endif  // MULTIVERSE_CONFIG_H
