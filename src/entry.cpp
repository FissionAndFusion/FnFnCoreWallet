// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "entry.h"
#include "core.h"
#include "worldline.h"
#include "txpool.h"
#include "wallet.h"
#include "dispatcher.h"
#include "network.h"
#include "netchn.h"
#include "rpcmod.h"
#include "service.h"
#include "blockmaker.h"
#include "rpcclient.h"
#include "miner.h"
#include "dbpservice.h"

#include <map>
#include <string>

#ifdef WIN32
#ifdef _MSC_VER
#pragma warning(disable:4786)
#pragma warning(disable:4804)
#pragma warning(disable:4805)
#pragma warning(disable:4717)
#endif
#include "shlobj.h"
#include "shlwapi.h"
#include "windows.h"
#endif

using namespace std;
using namespace walleve;
using namespace multiverse;
using namespace boost::filesystem;

extern void DisplayUsage();
//////////////////////////////
// CMvEntry

CMvEntry::CMvEntry()
{
}

CMvEntry::~CMvEntry()
{
    Exit(); 
}

bool CMvEntry::Initialize(int argc,char *argv[])
{
    if (!mvConfig.Load(argc,argv,GetDefaultDataDir(),"multiverse.conf") || !mvConfig.PostLoad())
    {
        cerr << "Failed to load/parse arguments and config file\n";
        return false;
    }
  
    if (mvConfig.fHelp)
    {
        DisplayUsage();
        return false;
    }

    path& pathData = mvConfig.pathData;
    if (!exists(pathData))
    {
        create_directories(pathData);
    }

    if (!is_directory(pathData))
    {
        cerr << "Failed to access data directory : " << pathData << "\n";
        return false;
    }

    if (mvConfig.fDaemon && mvConfig.vecCommand.empty())
    {
        if (!RunInBackground(pathData))
        {
            return false;
        }
        cout << "multiverse server starting\n";
    }

    if (mvConfig.vecCommand.empty() 
        && !walleveLog.SetLogFilePath((pathData / "multiverse.log").string()))
    {
        cerr << "Failed to open log file : " << (pathData / "multiverse.log") << "\n";
        return false; 
    }

    if (!walleveDocker.Initialize(&mvConfig,&walleveLog))
    {
        cerr << "Failed to initialize docker\n";
        return false;
    }

    return (mvConfig.vecCommand.empty() ? InitializeService() : InitializeClient());
}

bool CMvEntry::InitializeService()
{
    // single instance
    if (!TryLockFile((mvConfig.pathData / ".lock").string()))
    {
        cerr << "Cannot obtain a lock on data directory " << mvConfig.pathData << "\n"
             << "Multiverse is probably already running.\n";
        return false;
    }

    ICoreProtocol *pCoreProtocol = mvConfig.fTestNet ?
                                   new CMvTestNetCoreProtocol : new CMvCoreProtocol();
    if (!pCoreProtocol || !walleveDocker.Attach(pCoreProtocol))
    {
        delete pCoreProtocol;
        return false;
    }

    CWorldLine *pWorldLine = new CWorldLine();
    if (!pWorldLine || !walleveDocker.Attach(pWorldLine))
    {
        delete pWorldLine;
        return false;
    }
    
    CTxPool *pTxPool = new CTxPool();
    if (!pTxPool || !walleveDocker.Attach(pTxPool))
    {
        delete pTxPool;
        return false;
    }

    CWallet *pWallet = new CWallet();
    if (!pWallet || !walleveDocker.Attach(pWallet))
    {
        delete pWallet;
        return false;
    }

    CDispatcher *pDispatcher = new CDispatcher();
    if (!pDispatcher || !walleveDocker.Attach(pDispatcher))
    {
        delete pDispatcher;
        return false;
    }

    CNetwork *pNetwork = new CNetwork();
    if (!pNetwork || !walleveDocker.Attach(pNetwork))
    {
        delete pNetwork;
        return false;
    }

    CNetChannel *pNetChannel = new CNetChannel();
    if (!pNetChannel || !walleveDocker.Attach(pNetChannel))
    {
        delete pNetChannel;
        return false;
    }

    //DBP Server
    CDbpServer  *pDbpServer  = new CDbpServer();
    if (!pDbpServer || !walleveDocker.Attach(pDbpServer))
    {
        delete pDbpServer;
        return false;
    }
    pDbpServer->AddNewHost(GetDbpHostConfig());

    // DBP Service
    CDbpService *pDbpService = new CDbpService();
    if (!pDbpService || !walleveDocker.Attach(pDbpService))
    {
        delete pDbpService;
        return false;
    }

    CService *pService = new CService();
    if (!pService || !walleveDocker.Attach(pService))
    {
        delete pService;
        return false;
    }

    CHttpServer *pServer = new CHttpServer();
    if (!pServer || !walleveDocker.Attach(pServer))
    {
        delete pServer;
        return false;
    }
   
    pServer->AddNewHost(GetRPCHostConfig());

    CRPCMod *pRPCMod = new CRPCMod();
    if (!pRPCMod || !walleveDocker.Attach(pRPCMod))
    {
        delete pRPCMod;
        return false;
    }

    CBlockMaker *pBlockMaker = new CBlockMaker();
    if (!pBlockMaker || !walleveDocker.Attach(pBlockMaker))
    {
        delete pBlockMaker;
        return false;
    }

    return true;
}

bool CMvEntry::InitializeClient()
{
    CHttpGet *pHttpGet = new CHttpGet();
    if (!pHttpGet || !walleveDocker.Attach(pHttpGet))
    {
        delete pHttpGet;
        return false;
    }

    if (mvConfig.vecCommand[0] != "miner")
    {
        CRPCDispatch *pRPCDispatch = mvConfig.vecCommand[0] == "console" ?
                                     new CRPCDispatch() : new CRPCDispatch(mvConfig.vecCommand);
        if (!pRPCDispatch || !walleveDocker.Attach(pRPCDispatch))
        {
            delete pRPCDispatch;
            return false;
        }
    }
    else
    {
        CMiner *pMiner = new CMiner(mvConfig.vecCommand);
        if (!pMiner || !walleveDocker.Attach(pMiner))
        {
            return false;
        }
    }
    return true;
}


CHttpHostConfig CMvEntry::GetRPCHostConfig()
{
    CIOSSLOption sslRPC(mvConfig.fRPCSSLEnable,mvConfig.fRPCSSLVerify,
                        mvConfig.strRPCCAFile,mvConfig.strRPCCertFile,
                        mvConfig.strRPCPKFile,mvConfig.strRPCCiphers);
    
    map<string,string> mapUsrRPC;
    if (!mvConfig.strRPCUser.empty())
    {
        mapUsrRPC[mvConfig.strRPCUser] = mvConfig.strRPCPass;
    }

    return CHttpHostConfig(mvConfig.epRPC,mvConfig.nRPCMaxConnections,sslRPC,mapUsrRPC,
                           mvConfig.vRPCAllowIP,"rpcmod");
}

CDbpHostConfig CMvEntry::GetDbpHostConfig()
{
    CIOSSLOption sslDbp(mvConfig.fDbpSSLEnable,mvConfig.fDbpSSLVerify,
                        mvConfig.strDbpCAFile,mvConfig.strDbpCertFile,
                        mvConfig.strDbpPKFile,mvConfig.strDbpCiphers);

    map<string,string> mapUsrDbp;
    if (!mvConfig.strDbpUser.empty())
    {
        mapUsrDbp[mvConfig.strDbpUser] = mvConfig.strDbpPass;
    }

    return CDbpHostConfig(mvConfig.epDbp,mvConfig.nDbpMaxConnections,sslDbp,mapUsrDbp,
                           mvConfig.vDbpAllowIP,"dbpservice");
}

bool CMvEntry::Run()
{
    if (!walleveDocker.Run())
    {
        return false;
    }

    return CWalleveEntry::Run();
}

void CMvEntry::Exit()
{
    walleveDocker.Exit();

    if (mvConfig.fDaemon && mvConfig.vecCommand.empty() && !mvConfig.fHelp)
    {
        ExitBackground(mvConfig.pathData);
    }
}


path CMvEntry::GetDefaultDataDir()
{
    // Windows: C:\Documents and Settings\username\Local Settings\Application Data\Multiverse
    // Mac: ~/Library/Application Support/Multiverse
    // Unix: ~/.multiverse

#ifdef WIN32
    // Windows
    char pszPath[MAX_PATH] = "";
    if (SHGetSpecialFolderPathA(NULL, pszPath,CSIDL_LOCAL_APPDATA,true))
    {
        return path(pszPath) / "Multiverse";
    }
    return path("C:\\Multiverse");
#else
    path pathRet;
    char* pszHome = getenv("HOME");
    if (pszHome == NULL || strlen(pszHome) == 0)
    {
        pathRet = path("/");
    }
    else
    {
        pathRet = path(pszHome);
    }
#ifdef MAC_OSX
    // Mac
    pathRet /= "Library/Application Support";
    create_directory(pathRet);
    return pathRet / "Multiverse";
#else
    // Unix
    return pathRet / ".multiverse";
#endif    
#endif   
}

bool CMvEntry::SetupEnvironment()
{
#ifdef _MSC_VER
    // Turn off microsoft heap dump noise
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, CreateFileA("NUL", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, 0));
#endif
#if _MSC_VER >= 1400
    // Disable confusing "helpful" text message on abort, ctrl-c
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif
#ifndef WIN32
    umask(077);
#endif
    return true;
}

bool CMvEntry::RunInBackground(const path& pathData)
{
#ifndef WIN32
    // Daemonize
    pid_t pid = fork();
    if (pid < 0)
    {
        cerr << "Error: fork() returned " << pid << " errno " << errno << "\n";
        return false;
    }
    if (pid > 0)
    {
        FILE* file = fopen((pathData / "multiverse.pid").string().c_str(), "w");
        if (file)
        {
            fprintf(file, "%d\n", pid);
            fclose(file);
        }
        exit(0);
    }

    pid_t sid = setsid();
    if (sid < 0)
        cerr << "Error: setsid) returned " << sid << " errno " << errno << "\n";
    return true;
#else
    HWND hwnd = GetForegroundWindow();
    cout << "daemon running, window will run in background" << endl;
    system("pause");
    ShowWindow(hwnd, SW_HIDE);
    return true;
#endif
    return false;
}

void CMvEntry::ExitBackground(const path& pathData)
{
#ifndef WIN32
    boost::filesystem::remove(pathData / "multiverse.pid");
#endif
}

