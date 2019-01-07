// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_ENTRY_H
#define  MULTIVERSE_ENTRY_H

#include "config.h"
#include "dbpserver.h"
#include "dbpclient.h"
#include <walleve/walleve.h>
#include <boost/filesystem.hpp>

namespace multiverse
{

class CMvEntry : public walleve::CWalleveEntry
{
public:
    static CMvEntry& GetInstance();
public:
    ~CMvEntry();
    bool Initialize(int argc,char *argv[]);
    bool Run() override;
    void Exit();
protected:

    bool InitializeModules(const EModeType& mode);
    bool AttachModule(walleve::IWalleveBase* pWalleveBase);

    walleve::CHttpHostConfig GetRPCHostConfig();
    CDbpHostConfig GetDbpHostConfig();
    CDbpClientConfig GetDbpClientConfig();

    void PurgeStorage();

    boost::filesystem::path GetDefaultDataDir();

    bool SetupEnvironment();
    bool RunInBackground(const boost::filesystem::path& pathData);
    void ExitBackground(const boost::filesystem::path& pathData);
protected:
    CMvConfig mvConfig;
    walleve::CWalleveLog walleveLog;
    walleve::CWalleveDocker walleveDocker;

private:
    CMvEntry();
};

} // namespace multiverse

#endif //MULTIVERSE_ENTRY_H

