// Copyright (c) 2016-2018 The LoMoCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "walleve/base/base.h"
#include "walleve/event/eventproc.h"
#include <vector>
#include <boost/bind.hpp>
using namespace std;
using namespace walleve;


///////////////////////////////
// IWalleveBase

IWalleveBase::IWalleveBase()
{
    walleveStatus = WALLEVE_STATUS_OUTDOCKER;
}

IWalleveBase::IWalleveBase(const string& walleveOwnKeyIn)
{
    walleveStatus = WALLEVE_STATUS_OUTDOCKER;
    walleveOwnKey = walleveOwnKeyIn;
}

IWalleveBase::~IWalleveBase()
{
}

void IWalleveBase::WalleveSetOwnKey(const string& walleveOwnKeyIn)
{
    walleveOwnKey = walleveOwnKeyIn;
}

string& IWalleveBase::WalleveGetOwnKey()
{
    return walleveOwnKey;
}

WalleveStatus IWalleveBase::WalleveGetStatus()
{
    return walleveStatus;
}

void IWalleveBase::WalleveConnect(CWalleveDocker *pWalleveDockerIn)
{
    pWalleveDocker = pWalleveDockerIn;
    walleveStatus = WALLEVE_STATUS_ATTACHED;
}

CWalleveDocker * IWalleveBase::WalleveDocker()
{
    return pWalleveDocker;
}

bool IWalleveBase::WalleveInitialize()
{
    WalleveLog("Initializing...\n");
    if (!WalleveHandleInitialize())
    {
        WalleveLog("Failed to initialize.\n");
        return false;
    }
    WalleveLog("Initialized.\n");
    walleveStatus = WALLEVE_STATUS_INITIALIZED;
    return true;
}

void IWalleveBase::WalleveDeinitialize()
{
    WalleveLog("Deinitializing...\n");

    WalleveHandleDeinitialize();

    WalleveLog("Deinitialized...\n");
    walleveStatus = WALLEVE_STATUS_ATTACHED;
}

bool IWalleveBase::WalleveInvoke()
{
    WalleveLog("Invoking...\n");
    if (!WalleveHandleInvoke())
    {
        WalleveLog("Failed to invoke\n");
        return false;
    }
    WalleveLog("Invoked.\n");
    walleveStatus = WALLEVE_STATUS_INVOKED;
    return true;
}

void IWalleveBase::WalleveHalt()
{
    WalleveLog("Halting...\n");

    WalleveHandleHalt();

    WalleveLog("Halted.\n");
    walleveStatus = WALLEVE_STATUS_INITIALIZED;
}

bool IWalleveBase::WalleveHandleInitialize()
{
    return true;
}

void IWalleveBase::WalleveHandleDeinitialize()
{
}

bool IWalleveBase::WalleveHandleInvoke()
{
    return true;
}

void IWalleveBase::WalleveHandleHalt()
{
}

void IWalleveBase::WalleveFatalError()
{
    WalleveLog("Fatal Error!!!!!\n");
    pWalleveDocker->FatalError(walleveOwnKey.c_str());
}

void IWalleveBase::WalleveLog(const char *pszFormat,...)
{
    if (pWalleveDocker != NULL)
    {
        va_list ap;
        va_start(ap, pszFormat);
        pWalleveDocker->LogOutput(walleveOwnKey.c_str(),"", pszFormat, ap);
        va_end(ap);
    }
}

void IWalleveBase::WalleveDebug(const char *pszFormat,...)
{
    if (pWalleveDocker != NULL && pWalleveDocker->GetConfig()->fDebug)
    {
        va_list ap;
        va_start(ap, pszFormat);
        pWalleveDocker->LogOutput(walleveOwnKey.c_str(),"DEBUG: ", pszFormat, ap);
        va_end(ap);
    }
}

void IWalleveBase::WalleveVDebug(const char *pszFormat,va_list ap)
{
    if (pWalleveDocker != NULL && pWalleveDocker->GetConfig()->fDebug)
    {
        pWalleveDocker->LogOutput(walleveOwnKey.c_str(),"DEBUG: ", pszFormat, ap);
    }
}

bool IWalleveBase::WalleveThreadStart(CWalleveThread& thr)
{
    if (pWalleveDocker == NULL)
    {
        return false;
    }
    return pWalleveDocker->ThreadStart(thr);
}

bool IWalleveBase::WalleveThreadDelayStart(CWalleveThread& thr)
{
    if (pWalleveDocker == NULL)
    {
        return false;
    }
    return pWalleveDocker->ThreadDelayStart(thr);
}

void IWalleveBase::WalleveThreadExit(CWalleveThread& thr)
{
    if (pWalleveDocker != NULL)
    {
        pWalleveDocker->ThreadExit(thr);
    }
}

uint32 IWalleveBase::WalleveSetTimer(int64 nMilliSeconds,TimerCallback fnCallback)
{
    if (pWalleveDocker != NULL)
    {
        return pWalleveDocker->SetTimer(walleveOwnKey,nMilliSeconds,fnCallback);
    }
    return 0;
}

void IWalleveBase::WalleveCancelTimer(uint32 nTimerId)
{
    if (pWalleveDocker != NULL)
    {
        pWalleveDocker->CancelTimer(nTimerId);
    }
}

int64 IWalleveBase::WalleveGetTime()
{
    return (pWalleveDocker != NULL ? pWalleveDocker->GetSystemTime() : 0);
}

int64 IWalleveBase::WalleveGetNetTime()
{
    return (pWalleveDocker != NULL ? pWalleveDocker->GetNetTime() : 0);
}

void IWalleveBase::WalleveUpdateNetTime(const boost::asio::ip::address& address,int64 nTimeDelta)
{
    if (pWalleveDocker != NULL)
    {
        pWalleveDocker->UpdateNetTime(address,nTimeDelta);
    }
}

const CWalleveConfig * IWalleveBase::WalleveConfig()
{
    return (pWalleveDocker != NULL ? pWalleveDocker->GetConfig() : NULL);
}

const string IWalleveBase::WalleveGetWarnings()
{
    return (pWalleveDocker != NULL ? pWalleveDocker->GetWarnings() : "");
}
