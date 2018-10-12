// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "entry.h"
#include "core.h"
#include "worldline.h"
#include "txpool.h"
#include "consensus.h"
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
#include "dnseed.h"
#include "version.h"

#include <map>
#include <string>

#ifdef WIN32
#ifdef _MSC_VER
#pragma warning(disable : 4786)
#pragma warning(disable : 4804)
#pragma warning(disable : 4805)
#pragma warning(disable : 4717)
#endif
#include "shlobj.h"
#include "shlwapi.h"
#include "windows.h"
#endif

#define MINIMUM_HARD_DISK_AVAILABLE 104857600

using namespace std;
using namespace walleve;
using namespace multiverse;
using namespace boost::filesystem;

//////////////////////////////
// CMvEntry

CMvEntry& CMvEntry::GetInstance()
{
    static CMvEntry entry;
    return entry;
}

CMvEntry::CMvEntry()
{
}

CMvEntry::~CMvEntry()
{
    Exit();
}

bool CMvEntry::Initialize(int argc, char *argv[])
{
    if (!mvConfig.Load(argc, argv, GetDefaultDataDir(), "multiverse.conf") || !mvConfig.PostLoad())
    {
        cerr << "Failed to load/parse arguments and config file\n";
        return false;
    }

    // help
    if (mvConfig.GetConfig()->fHelp)
    {
        cout << mvConfig.Help() << endl;
        return false;
    }

    // version
    if (mvConfig.GetConfig()->fVersion)
    {
        cout << "Multiverse version is v" << MV_VERSION_STR << endl;
        return false;
    }

    // list config if in debug mode
    if (mvConfig.GetConfig()->fDebug)
    {
        mvConfig.ListConfig();
    }

    // path
    path& pathData = mvConfig.GetConfig()->pathData;
    if (!exists(pathData))
    {
        if (!create_directories(pathData))
        {
            cerr << "Failed create directory : " << pathData << "\n";
            return false;
        }
    }

    if (!is_directory(pathData))
    {
        cerr << "Failed to access data directory : " << pathData << "\n";
        return false;
    }

    // log
    if ((mvConfig.GetModeType() == EModeType::SERVER || mvConfig.GetModeType() == EModeType::MINER) && !walleveLog.SetLogFilePath((pathData / "multiverse.log").string()))
    {
        cerr << "Failed to open log file : " << (pathData / "multiverse.log") << "\n";
        return false;
    }

    // hard disk
    if (space(pathData).available < MINIMUM_HARD_DISK_AVAILABLE)
    {
        cerr << "Warning: hard disk available < 100M\n";
        return false;
    }

    // daemon
    if (mvConfig.GetConfig()->fDaemon &&
        (mvConfig.GetModeType() == EModeType::SERVER || mvConfig.GetModeType() == EModeType::MINER))
    {
        if (!RunInBackground(pathData))
        {
            return false;
        }
        cout << "multiverse server starting\n";
    }

    // log
    if ((mvConfig.GetModeType() == EModeType::SERVER || mvConfig.GetModeType() == EModeType::MINER || mvConfig.GetModeType() == EModeType::DNSEED) && !walleveLog.SetLogFilePath((pathData / "multiverse.log").string()))
    {
        cerr << "Failed to open log file : " << (pathData / "multiverse.log") << "\n";
        return false;
    }

    // docker
    if (!walleveDocker.Initialize(mvConfig.GetConfig(), &walleveLog))
    {
        cerr << "Failed to initialize docker\n";
        return false;
    }

    // modules
    return InitializeModules(mvConfig.GetModeType());
}

bool CMvEntry::AttachModule(IWalleveBase* pWalleveBase)
{
    if (!pWalleveBase || !walleveDocker.Attach(pWalleveBase))
    {
        delete pWalleveBase;
        return false;
    }

    return true;
}

bool CMvEntry::InitializeModules(const EModeType& mode)
{
    const std::vector<EModuleType>& modules = CMode::GetModules(mode);

    for (auto& m : modules)
    {
        switch (m)
        {
        case EModuleType::LOCK:
        {
            if (!TryLockFile((mvConfig.GetConfig()->pathData / ".lock").string()))
            {
                cerr << "Cannot obtain a lock on data directory " << mvConfig.GetConfig()->pathData << "\n"
                     << "Multiverse is probably already running.\n";
                return false;
            }
            break;
        }
        case EModuleType::BLOCKMAKER:
        {
            if (!AttachModule(new CBlockMaker()))
            {
                return false;
            }
            break;
        }
        case EModuleType::COREPROTOCOL:
        {
            if (!AttachModule(mvConfig.GetConfig()->fTestNet ? new CMvTestNetCoreProtocol : new CMvCoreProtocol()))
            {
                return false;
            }
            break;
        }
        case EModuleType::DISPATCHER:
        {
            if (!AttachModule(new CDispatcher()))
            {
                return false;
            }
            break;
        }
        case EModuleType::HTTPGET:
        {
            if (!AttachModule(new CHttpGet()))
            {
                return false;
            }
            break;
        }
        case EModuleType::HTTPSERVER:
        {
            if (!AttachModule(new CHttpServer()))
            {
                return false;
            }
            break;
        }
        case EModuleType::MINER:
        {
            if (!AttachModule(new CMiner(mvConfig.GetConfig()->vecCommand)))
            {
                return false;
            }
            break;
        }
        case EModuleType::NETCHANNEL:
        {
            if (!AttachModule(new CNetChannel()))
            {
                return false;
            }
            break;
        }
        case EModuleType::NETWORK:
        {
            if (!AttachModule(new CNetwork()))
            {
                return false;
            }
            break;
        }
        case EModuleType::RPCCLIENT:
        {
            if (!AttachModule(new CRPCClient(mvConfig.GetConfig()->vecCommand.empty())))
            {
                return false;
            }
            break;
        }
        case EModuleType::RPCMODE:
        {
            auto pBase = walleveDocker.GetObject("httpserver");
            if (!pBase)
            {
                return false;
            }
            dynamic_cast<CHttpServer*>(pBase)->AddNewHost(GetRPCHostConfig());

            if (!AttachModule(new CRPCMod()))
            {
                return false;
            }
            break;
        }
        case EModuleType::SERVICE:
        {
            if (!AttachModule(new CService()))
            {
                return false;
            }
            break;
        }
        case EModuleType::TXPOOL:
        {
            if (!AttachModule(new CTxPool()))
            {
                return false;
            }
            break;
        }
        case EModuleType::WALLET:
        {
            IWallet* pWallet;
            if (mvConfig.GetConfig()->fWallet)
            {
                pWallet = new CWallet();
            }
            else
            {
                pWallet = new CDummyWallet();
            }

            if (!AttachModule(pWallet))
            {
                return false;
            }
            break;
        }
        case EModuleType::WORLDLINE:
        {
            if (!AttachModule(new CWorldLine()))
            {
                return false;
            }
            break;
        }
        case EModuleType::CONSENSUS:
        {
            if (!AttachModule(new CConsensus()))
            {
                return false;
            }
            break;
        }
        case EModuleType::DBPSERVER:
        {
            if (!AttachModule(new CDbpServer()))
            {
                return false;
            }
            break;
        }
        case EModuleType::DBPSERVICE:
        {
            auto pBase = walleveDocker.GetObject("dbpserver");
            if (!pBase)
            {
                return false;
            }
            dynamic_cast<CDbpServer*>(pBase)->AddNewHost(GetDbpHostConfig());

            if (!AttachModule(new CDbpService()))
            {
                return false;
            }
            break;
        }
        case EModuleType::DNSEED:
        {
            if (!AttachModule(new CDNSeed()))
            {
                return false;
            }
            break;
        }
        default:
            cerr << "Unknown module:%d" << CMode::IntValue(m) << endl;
            break;
        }
    }

    return true;
}

CHttpHostConfig CMvEntry::GetRPCHostConfig()
{
    const CMvRPCServerConfig* config = CastConfigPtr<CMvRPCServerConfig *>(mvConfig.GetConfig());
    CIOSSLOption sslRPC(config->fRPCSSLEnable, config->fRPCSSLVerify,
                        config->strRPCCAFile, config->strRPCCertFile,
                        config->strRPCPKFile, config->strRPCCiphers);

    map<string, string> mapUsrRPC;
    if (!config->strRPCUser.empty())
    {
        mapUsrRPC[config->strRPCUser] = config->strRPCPass;
    }

    return CHttpHostConfig(config->epRPC, config->nRPCMaxConnections, sslRPC, mapUsrRPC,
                           config->vRPCAllowIP, "rpcmod");
}

CDbpHostConfig CMvEntry::GetDbpHostConfig()
{
    const CMvDbpServerConfig* config = CastConfigPtr<CMvDbpServerConfig*>(mvConfig.GetConfig());
    CIOSSLOption sslDbp(config->fDbpSSLEnable, config->fDbpSSLVerify,
                        config->strDbpCAFile, config->strDbpCertFile,
                        config->strDbpPKFile, config->strDbpCiphers);

    map<string, string> mapUsrDbp;
    if (!config->strDbpUser.empty())
    {
        mapUsrDbp[config->strDbpUser] = config->strDbpPass;
    }

    return CDbpHostConfig(config->epDbp, config->nDbpMaxConnections, config->nDbpSessionTimeout,
                          sslDbp, mapUsrDbp, config->vDbpAllowIP, "dbpservice");
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

    if (mvConfig.GetConfig()->fDaemon && mvConfig.GetConfig()->vecCommand.empty() && !mvConfig.GetConfig()->fHelp)
    {
        ExitBackground(mvConfig.GetConfig()->pathData);
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
    if (SHGetSpecialFolderPathA(NULL, pszPath, CSIDL_LOCAL_APPDATA, true))
    {
        return path(pszPath) / "Multiverse";
    }
    return path("C:\\Multiverse");
#else
    path pathRet;
    char *pszHome = getenv("HOME");
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
