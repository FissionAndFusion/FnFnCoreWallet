// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "mode/storage_config.h"

#include "mode/config_macro.h"

namespace multiverse
{
namespace po = boost::program_options;

CMvStorageConfig::CMvStorageConfig()
{
    po::options_description desc("MvStorage");

    CMvStorageConfigOption::AddOptionsImpl(desc);

    AddOptions(desc);
}

CMvStorageConfig::~CMvStorageConfig() {}

bool CMvStorageConfig::PostLoad()
{
    if (fHelp)
    {
        return true;
    }

    if (fTestNet)
    {
        strDBName += "-testnet";
    }
    if (nDBPort < 0 || nDBPort > 0xFFFF)
    {
        nDBPort = 0;
    }
    return true;
}

std::string CMvStorageConfig::ListConfig() const
{
    std::ostringstream oss;
    oss << CMvStorageConfigOption::ListConfigImpl();
    return oss.str();
}

std::string CMvStorageConfig::Help() const
{
    return CMvStorageConfigOption::HelpImpl();
}

}  // namespace multiverse