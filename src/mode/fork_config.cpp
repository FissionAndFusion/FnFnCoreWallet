// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "mode/fork_config.h"

#include "mode/config_macro.h"

#include <boost/foreach.hpp>

namespace multiverse
{
namespace po = boost::program_options;

CMvForkConfig::CMvForkConfig()
{
    po::options_description desc("MvFork");

    CMvForkConfigOption::AddOptionsImpl(desc);

    AddOptions(desc);

    fAllowAnyFork = false;
}

CMvForkConfig::~CMvForkConfig() {}

bool CMvForkConfig::PostLoad()
{
    BOOST_FOREACH(const string& strFork,vFork)
    {
        if (strFork == "any")
        {
            fAllowAnyFork = true;
            break;
        }
    }
    return true;
}

std::string CMvForkConfig::ListConfig() const
{
    std::ostringstream oss;
    oss << CMvForkConfigOption::ListConfigImpl();
    return oss.str();
}

std::string CMvForkConfig::Help() const
{
    return CMvForkConfigOption::HelpImpl();
}

}  // namespace multiverse
