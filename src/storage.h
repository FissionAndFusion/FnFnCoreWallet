// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_STORAGE_H
#define  MULTIVERSE_STORAGE_H

#include "config.h"
#include <walleve/walleve.h>
#include <boost/filesystem.hpp>

namespace multiverse
{

class CMvEntry : public walleve::CWalleveEntry
{
public:
    CMvEntry();
    ~CMvEntry();
    bool Initialize(int argc,char *argv[]);
    bool Run();
    void Exit();
protected:
    bool InitializeService();
    bool InitializeClient();
//    walleve::CHttpHostConfig GetRPCHostConfig();
//    walleve::CHttpHostConfig GetWebUIHostConfig();

    boost::filesystem::path GetDefaultDataDir();

    bool SetupEnvironment();
    bool RunInBackground(const boost::filesystem::path& pathData);
    void ExitBackground(const boost::filesystem::path& pathData);
protected:
    CMvConfig mvConfig;
    walleve::CWalleveLog walleveLog;
    walleve::CWalleveDocker walleveDocker;
};

} // namespace multiverse

#endif //MULTIVERSE_STORAGE_H

