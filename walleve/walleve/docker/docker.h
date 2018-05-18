// Copyright (c) 2016-2018 The LoMoCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  WALLEVE_DOCKER_H
#define  WALLEVE_DOCKER_H

#include "walleve/docker/config.h"
#include "walleve/docker/log.h"
#include "walleve/docker/thread.h"
#include "walleve/docker/timer.h"
#include "walleve/docker/nettime.h"

#include <stdarg.h>
#include <string>
#include <map>
#include <list>
#include <boost/thread/thread.hpp>
#include <boost/asio/ip/address.hpp>

namespace walleve
{

class IWalleveBase;

class CWalleveDocker
{
public:
    CWalleveDocker();
    ~CWalleveDocker();
    bool Initialize(CWalleveConfig* pConfigIn,CWalleveLog* pLogIn = NULL);

    bool Attach(IWalleveBase *pWalleveBase);
    void Detach(std::string& walleveKey);

    bool Run();
    bool Run(std::string walleveKey);
    void Halt();
    void Halt(std::string walleveKey);

    void Exit();

    IWalleveBase * GetObject(const std::string& walleveKey);
    void FatalError(const std::string& walleveKey);
    void LogOutput(const char *walleveKey,const char *strPrefix,const char *pszFormat,va_list ap);
    bool ThreadStart(CWalleveThread& thr);
    bool ThreadDelayStart(CWalleveThread& thr);
    void ThreadExit(CWalleveThread& thr);
    void ThreadRun(CWalleveThread& thr);
    void ThreadDelayRun(CWalleveThread& thr);
    uint32 SetTimer(const std::string& walleveKey,int64 nMilliSeconds,TimerCallback fnCallback);
    void CancelTimer(const uint32 nTimerId);
    void CancelTimers(const std::string& walleveKey);
    int64 GetSystemTime(); 
    int64 GetNetTime();
    void  UpdateNetTime(const boost::asio::ip::address& address,int64 nTimeDelta);
    const CWalleveConfig *GetConfig();
    const std::string GetWarnings();
protected:
    void Halt(std::vector<IWalleveBase *>& vWorkQueue);
    void Log(const char *pszFormat,...);
    void LogException(const char* pszThread,std::exception* pex);
    void ListConfig();
    void TimerProc();
protected:
    boost::mutex mtxDocker;
    boost::condition_variable condDocker;    
    std::map<std::string,IWalleveBase *> mapWalleveObj;
    std::list<IWalleveBase *> listInstalled;

    CWalleveNetTime tmNet;        
    CWalleveConfig* pConfig;
    CWalleveLog* pLog;

    bool fActived;
    bool fShutdown;

    boost::thread *pThreadTimer;
    boost::mutex mtxTimer;
    boost::condition_variable condTimer;    
    std::map<uint32,CWalleveTimer> mapTimerById;
    std::multimap<boost::system_time,uint32> mapTimerByExpiry;
};

} // namespace walleve

#endif //WALLEVE_DOCKER_H

