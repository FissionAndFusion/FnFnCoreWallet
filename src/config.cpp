// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "config.h"

#include <algorithm>
#include <boost/filesystem.hpp>
#include <numeric>

using namespace multiverse;

namespace po = boost::program_options;
namespace fs = boost::filesystem;

CMvConfig::CMvConfig() : emMode(EModeType::ERROR), pImpl(NULL) {}

CMvConfig::~CMvConfig()
{
    if (pImpl)
    {
        delete pImpl;
    }
}

bool CMvConfig::Load(int argc, char* argv[], const fs::path& pathDefault,
                     const std::string& strConfile)
{
    if (argc <= 0)
    {
        return false;
    }

    // call parse cmd
    std::vector<std::string> vecCmd = walleve::CWalleveConfig::ParseCmd(argc, argv);

    // determine mode type
    std::string exec = fs::path(argv[0]).filename().string();
    std::string cmd = (vecCmd.size() > 0) ? vecCmd[0] : "";

    int ignoreCmd = 0;
    if ((exec == "multiverse-server") ||
        (exec == "multiverse" && cmd == "server") ||
        (exec == "multiverse" && cmd == ""))
    {
        emMode = EModeType::SERVER;

        if (cmd == "server")
        {
            ignoreCmd = 1;
        }
    }
    else if (exec == "multiverse-miner" ||
             (exec == "multiverse" && cmd == "miner"))
    {
        emMode = EModeType::MINER;

        if (cmd == "miner")
        {
            ignoreCmd = 1;
        }
    }
    else if (exec == "multiverse-dnseed" ||
             (exec == "multiverse" && cmd == "dnseed"))
    {
        emMode = EModeType::MINER;

        if (cmd == "dnseed")
        {
            ignoreCmd = 1;
        }
    }
    else if (exec == "multiverse-console" || exec == "multiverse")
    {
        emMode = EModeType::CONSOLE;

        if (cmd == "console")
        {
            ignoreCmd = 1;
        }
    }

    if (emMode == EModeType::ERROR)
    {
        return false;
    }

    // Add desc
    pImpl = CMode::CreateConfig(emMode);

    pImpl->SetIgnoreCmd(ignoreCmd);

    // Load
    return pImpl->Load(argc, argv, pathDefault, strConfile);
}