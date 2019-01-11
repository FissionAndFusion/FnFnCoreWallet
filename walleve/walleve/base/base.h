// Copyright (c) 2016-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  WALLEVE_BASE_H
#define  WALLEVE_BASE_H

#include "walleve/docker/docker.h" 
#include "walleve/event/event.h" 
#include <stdarg.h>
#include <string>
#include <boost/asio/ip/address.hpp>

namespace walleve
{

typedef enum
{
    WALLEVE_STATUS_OUTDOCKER = 0,
    WALLEVE_STATUS_ATTACHED,
    WALLEVE_STATUS_INITIALIZED,
    WALLEVE_STATUS_INVOKED
}WalleveStatus;

class IWalleveBase : virtual public CWalleveEventListener
{
public:
    IWalleveBase();
    IWalleveBase(const std::string& walleveOwnKeyIn);
    virtual ~IWalleveBase();
    void WalleveSetOwnKey(const std::string& walleveOwnKeyIn);
    std::string& WalleveGetOwnKey();
    WalleveStatus WalleveGetStatus();
    void WalleveConnect(CWalleveDocker *pWalleveDockerIn);
    CWalleveDocker * WalleveDocker();

    bool WalleveInitialize();
    void WalleveDeinitialize();
    bool WalleveInvoke();
    void WalleveHalt();
protected:
    virtual bool WalleveHandleInitialize();
    virtual void WalleveHandleDeinitialize();
    virtual bool WalleveHandleInvoke();
    virtual void WalleveHandleHalt();
    template <typename T>
    bool WalleveGetObject(const std::string& walleveKey,T*& pWalleveObj);
    void WalleveFatalError();
    void WalleveLog(const char *pszFormat,...);
    void WalleveDebug(const char *pszFormat,...);
    void WalleveVDebug(const char *pszFormat,va_list ap);
    void WalleveWarn(const char *pszFormat,...);
    void WalleveError(const char *pszFormat,...);
    bool WalleveThreadStart(CWalleveThread& thr);
    bool WalleveThreadDelayStart(CWalleveThread& thr);
    void WalleveThreadExit(CWalleveThread& thr);
    uint32 WalleveSetTimer(int64 nMilliSeconds,TimerCallback fnCallback);
    void WalleveCancelTimer(uint32 nTimerId);
    int64 WalleveGetTime();
    int64 WalleveGetNetTime();
    void  WalleveUpdateNetTime(const boost::asio::ip::address& address,int64 nTimeDelta);
    const CWalleveConfig *WalleveConfig();
    const std::string WalleveGetWarnings();
private:
    std::string walleveOwnKey;
    CWalleveDocker *pWalleveDocker;
    WalleveStatus walleveStatus;
};

template <typename T>
bool IWalleveBase::WalleveGetObject(const std::string& walleveKey,T*& pWalleveObj)
{
    pWalleveObj = NULL;
    if (pWalleveDocker != NULL)
    {
        try
        {
            pWalleveObj = dynamic_cast<T*>(pWalleveDocker->GetObject(walleveKey));
        }
        catch (...) {}
    }
    return (pWalleveObj != NULL);
}

} // namespace walleve

#endif //WALLEVE_BASE_H

