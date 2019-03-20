// Copyright (c) 2016-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "docker.h"
#include "walleve/base/base.h"
#include "walleve/util.h"
#include <vector>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <boost/bind.hpp>

using namespace std;
using namespace walleve;

///////////////////////////////
// CWalleveDocker

CWalleveDocker::CWalleveDocker()
{
    pThreadTimer = NULL;
    fActived = false;
    fShutdown = false;    
}

CWalleveDocker::~CWalleveDocker()
{
    Exit();
}

bool CWalleveDocker::Initialize(CWalleveConfig* pConfigIn,CWalleveLog* pLogIn)
{
    fActived = false;
    fShutdown = false;

    pConfig = pConfigIn;
    pLog = pLogIn;

    if (pConfig == NULL)
    {
        return false;
    }

    mapTimerById.clear();
    mapTimerByExpiry.clear();
    pThreadTimer = new boost::thread(boost::bind(&CWalleveDocker::TimerProc,this));
    if (pThreadTimer == NULL)
    {
        return false;
    }

    tmNet.Clear();

    Log("\n\n\n\n\n\n\n\n");
    Log("WALL-E is being activatied...\n");

    Log("#### Configuration : \n%s\n",pConfig->ListConfig().c_str());
    Log("##################\n");

    return true;
}

bool CWalleveDocker::Attach(IWalleveBase *pWalleveBase)
{
    if (pWalleveBase == NULL)
    {
        return false;
    }

    string& walleveKey = pWalleveBase->WalleveGetOwnKey();

    boost::unique_lock<boost::mutex> lock(mtxDocker);
    if (mapWalleveObj.count(walleveKey))
    {
        return false;
    }
    pWalleveBase->WalleveConnect(this);
    mapWalleveObj.insert(make_pair(walleveKey,pWalleveBase));
    listInstalled.push_back(pWalleveBase);

    return true;
}

void CWalleveDocker::Detach(string& walleveKey)
{
    IWalleveBase *pWalleveBase = NULL;
    {
        boost::unique_lock<boost::mutex> lock(mtxDocker);
        map<string,IWalleveBase *>::iterator mi = mapWalleveObj.find(walleveKey);
        if (mi != mapWalleveObj.end())
        {
            pWalleveBase = (*mi).second;
            listInstalled.remove(pWalleveBase);
            mapWalleveObj.erase(mi);
        }
    }

    if (pWalleveBase != NULL)
    {
        if (pWalleveBase->WalleveGetStatus() == WALLEVE_STATUS_INVOKED)
        {
            pWalleveBase->WalleveHalt();
        }
        if (pWalleveBase->WalleveGetStatus() == WALLEVE_STATUS_INITIALIZED)
        {
            pWalleveBase->WalleveDeinitialize();
        }
        delete pWalleveBase;
    }
}

bool CWalleveDocker::Run()
{
    vector<IWalleveBase *> vWorkQueue;
    {
        boost::unique_lock<boost::mutex> lock(mtxDocker);
        list<IWalleveBase *>::iterator it;
        for (it = listInstalled.begin();it != listInstalled.end();++it)
        {
            WalleveStatus status = (*it)->WalleveGetStatus();
            if (status == WALLEVE_STATUS_ATTACHED)
            {
                vWorkQueue.push_back(*it);
            }
        }
    }

    vector<IWalleveBase *>::iterator it;
    for (it = vWorkQueue.begin();it != vWorkQueue.end();++it)
    {
        if (!(*it)->WalleveInitialize())
        {
            Halt(vWorkQueue);
            return false;
        }
    }

    {
        boost::unique_lock<boost::mutex> lock(mtxDocker);
        list<IWalleveBase *>::iterator it;
        for (it = listInstalled.begin();it != listInstalled.end();++it)
        {
            WalleveStatus status = (*it)->WalleveGetStatus();
            if (status == WALLEVE_STATUS_INITIALIZED
                && !count(vWorkQueue.begin(),vWorkQueue.end(),(*it)))
            {
                vWorkQueue.push_back(*it);
            }
        }
    }

    for (it = vWorkQueue.begin();it != vWorkQueue.end();++it)
    {
        if (!(*it)->WalleveInvoke())
        {
            Halt(vWorkQueue);
            return false;
        }
    }

    {
        boost::unique_lock<boost::mutex> lock(mtxDocker);
        fActived = true;
        condDocker.notify_all();
    }
    return true;
}

bool CWalleveDocker::Run(string walleveKey)
{
    IWalleveBase *pWalleveBase = NULL;
    {
        boost::unique_lock<boost::mutex> lock(mtxDocker);
        map<string,IWalleveBase *>::iterator mi = mapWalleveObj.find(walleveKey);
        if (mi != mapWalleveObj.end())
        {
            pWalleveBase = (*mi).second;
        }
    }

    if (pWalleveBase == NULL)
    {
        return false;
    }

    if (pWalleveBase->WalleveGetStatus() == WALLEVE_STATUS_ATTACHED)
    {
        pWalleveBase->WalleveInitialize();
    }

    if (pWalleveBase->WalleveGetStatus() == WALLEVE_STATUS_INITIALIZED)
    {
        pWalleveBase->WalleveInvoke();
    }

    if (pWalleveBase->WalleveGetStatus() == WALLEVE_STATUS_INVOKED)
    {
        return true;
    }

    if (pWalleveBase->WalleveGetStatus() != WALLEVE_STATUS_ATTACHED)
    {
        Halt(walleveKey);
    }
    return false;
}

void CWalleveDocker::Halt()
{
    {
        boost::unique_lock<boost::mutex> lock(mtxDocker);
        fActived = false;
        condDocker.notify_all();
    }

    vector<IWalleveBase *> vWorkQueue;
    {
        boost::unique_lock<boost::mutex> lock(mtxDocker);
        vWorkQueue.assign(listInstalled.begin(),listInstalled.end());
        
    }

    Halt(vWorkQueue);
}

void CWalleveDocker::Halt(string walleveKey)
{
    IWalleveBase *pWalleveBase = NULL;
    {
        boost::unique_lock<boost::mutex> lock(mtxDocker);
        map<string,IWalleveBase *>::iterator mi = mapWalleveObj.find(walleveKey);
        if (mi != mapWalleveObj.end())
        {
            pWalleveBase = (*mi).second;
        }
    }

    if (pWalleveBase->WalleveGetStatus() == WALLEVE_STATUS_INVOKED)
    {
        pWalleveBase->WalleveHalt();
    }

    if (pWalleveBase->WalleveGetStatus() == WALLEVE_STATUS_INITIALIZED)
    {
        pWalleveBase->WalleveDeinitialize();
    }

    CancelTimers(walleveKey);
}

void CWalleveDocker::Halt(vector<IWalleveBase *>& vWorkQueue)
{
    vector<IWalleveBase *>::reverse_iterator it;
    for (it = vWorkQueue.rbegin();it != vWorkQueue.rend();++it)
    {
        if ((*it)->WalleveGetStatus() == WALLEVE_STATUS_INVOKED)
        {
            (*it)->WalleveHalt();
        }
    }

    for (it = vWorkQueue.rbegin();it != vWorkQueue.rend();++it)
    {
        if ((*it)->WalleveGetStatus() == WALLEVE_STATUS_INITIALIZED)
        {
            (*it)->WalleveDeinitialize();
        }
    }

    for (it = vWorkQueue.rbegin();it != vWorkQueue.rend();++it)
    {
        CancelTimers((*it)->WalleveGetOwnKey());
    }
}

void CWalleveDocker::Exit()
{
    if (fShutdown)
    {
        return;
    }

    fShutdown = true;

    Halt();

    {
        boost::unique_lock<boost::mutex> lock(mtxDocker);
        while(!listInstalled.empty())
        {
            IWalleveBase *pWalleveBase = listInstalled.back();
            listInstalled.pop_back();
            delete pWalleveBase;
        }
    }

    if (pThreadTimer)
    {
        {
            boost::unique_lock<boost::mutex> lock(mtxTimer);
            condTimer.notify_all();
        }
        pThreadTimer->join();
        delete pThreadTimer;
        pThreadTimer = NULL;        
    }

    Log("WALL-E is deactivatied.\n");
}

IWalleveBase * CWalleveDocker::GetObject(const string& walleveKey)
{
    {
        boost::unique_lock<boost::mutex> lock(mtxDocker);
        map<string,IWalleveBase *>::iterator mi = mapWalleveObj.find(walleveKey);
        if (mi != mapWalleveObj.end())
        {
            return (*mi).second;
        }
    }
    return NULL;
}

void CWalleveDocker::FatalError(const std::string& walleveKey)
{
}

void CWalleveDocker::LogOutput(const char *walleveKey,const char *strPrefix,const char *pszFormat,va_list ap)
{
    if (pLog != NULL)
    {
        (*pLog)(walleveKey,strPrefix,pszFormat,ap);
    }
}

bool CWalleveDocker::ThreadStart(CWalleveThread& thr)
{
    try
    {
        boost::thread::attributes attr;
        attr.set_stack_size(16 * 1024 * 1024);
        thr.pThread = new boost::thread(attr,boost::bind(&CWalleveDocker::ThreadRun,this,boost::ref(thr)));
        return (thr.pThread != NULL);
    }
    catch (exception& e)
    {
        LogException(thr.strThreadName.c_str(), &e);
    }
    return false;
}

bool CWalleveDocker::ThreadDelayStart(CWalleveThread& thr)
{
    try
    {
        thr.pThread = new boost::thread(boost::bind(&CWalleveDocker::ThreadDelayRun,this,boost::ref(thr)));
        return (thr.pThread != NULL);
    }
    catch (exception& e)
    {
        LogException(thr.strThreadName.c_str(), &e);
    }
    return false;
}

void CWalleveDocker::ThreadExit(CWalleveThread& thr)
{
    thr.Exit();
}

void CWalleveDocker::ThreadRun(CWalleveThread& thr)
{
    Log("Thread %s started\n",thr.strThreadName.c_str());
    thr.SetAffinity();
    try
    {
        thr.fRunning = true;
        thr.fnCallback();
        thr.fRunning = false;
    }
    catch (std::exception& e)
    {
        thr.fRunning = false;
        LogException(thr.strThreadName.c_str(),&e);
    }
    catch (...)
    {
        thr.fRunning = false;
        throw; // support pthread_cancel()
    }
    Log("Thread %s exiting\n",thr.strThreadName.c_str());
}

void CWalleveDocker::ThreadDelayRun(CWalleveThread& thr)
{
    thr.SetAffinity();
    {    
        boost::unique_lock<boost::mutex> lock(mtxDocker);
        if (!fActived)
        {
            Log("Thread %s delay to invoke\n",thr.strThreadName.c_str());
        }
        while (!fActived && !fShutdown)
        {
            condDocker.wait(lock);
        }
    }
    if (!fShutdown)
    {
        ThreadRun(thr);
    }
    else
    {
        Log("Thread %s is not running before shutdown\n",thr.strThreadName.c_str());
    }
}

uint32 CWalleveDocker::SetTimer(const string& walleveKey,int64 nMilliSeconds,TimerCallback fnCallback)
{
    static uint32 nTimerIdAlloc = 0;
    uint32 nTimerId = 0;
    bool fUpdate = false;
    {
        boost::unique_lock<boost::mutex> lock(mtxTimer);

        boost::system_time tExpiryAt = boost::get_system_time() 
                                        + boost::posix_time::milliseconds(nMilliSeconds);
        pair<map<uint32,CWalleveTimer>::iterator,bool> r;
        do
        {
            nTimerId = ++nTimerIdAlloc;
            if (nTimerId == 0)
            {
                continue;
            }
            r = mapTimerById.insert(make_pair(nTimerId,CWalleveTimer(walleveKey,nTimerId,tExpiryAt,fnCallback)));
        }
        while (!r.second);

        multimap<boost::system_time,uint32>::iterator it;    
        it = mapTimerByExpiry.insert(make_pair(tExpiryAt,nTimerId));
        fUpdate = (it == mapTimerByExpiry.begin());
    }
    if (fUpdate)
    {
        condTimer.notify_all();
    }
    return nTimerId;
}

void CWalleveDocker::CancelTimer(const uint32 nTimerId)
{
    bool fUpdate = false;
    {
        boost::unique_lock<boost::mutex> lock(mtxTimer);
        map<uint32,CWalleveTimer>::iterator it = mapTimerById.find(nTimerId);
        if (it != mapTimerById.end())
        {
            CWalleveTimer &tmr = (*it).second;
            multimap<boost::system_time,uint32>::iterator mi;
            for (mi = mapTimerByExpiry.lower_bound(tmr.tExpiryAt);
                 mi != mapTimerByExpiry.upper_bound(tmr.tExpiryAt); ++mi)
            {
                if ((*mi).second == nTimerId)
                {
                    fUpdate = (mi == mapTimerByExpiry.begin());
                    mapTimerByExpiry.erase(mi);
                    break;
                }
            }
            mapTimerById.erase(it);
        }
    }
    if (fUpdate)
    {
        condTimer.notify_all();
    }
}

void CWalleveDocker::CancelTimers(const string& walleveKey)
{
    bool fUpdate = false;
    {
        boost::unique_lock<boost::mutex> lock(mtxTimer);
        multimap<boost::system_time,uint32>::iterator mi = mapTimerByExpiry.begin();
        while(mi != mapTimerByExpiry.end())
        {
            map<uint32,CWalleveTimer>::iterator it = mapTimerById.find((*mi).second);
            if (it != mapTimerById.end() && (*it).second.walleveKey == walleveKey)
            {
                if (mi == mapTimerByExpiry.begin())
                {
                    fUpdate = true;
                }
                mapTimerByExpiry.erase(mi++);
                mapTimerById.erase(it);
            }
            else
            {
                ++mi;
            }
        }
    }
    if (fUpdate)
    {
        condTimer.notify_all();
    }
}

int64 CWalleveDocker::GetSystemTime()
{
    return GetTime();
}

int64 CWalleveDocker::GetNetTime()
{
    return (GetTime() + tmNet.GetTimeOffset());
}

void CWalleveDocker::UpdateNetTime(const boost::asio::ip::address& address,int64 nTimeDelta)
{
    if (!tmNet.AddNew(address,nTimeDelta))
    {
    // warning..
    }
}

const CWalleveConfig *CWalleveDocker::GetConfig()
{
    return pConfig;
}

const string CWalleveDocker::GetWarnings()
{
    return "";
}    

void CWalleveDocker::Log(const char *pszFormat,...)
{
    va_list ap;
    va_start(ap, pszFormat);
    LogOutput("docker","[INFO]",pszFormat, ap);
    va_end(ap);
}

void CWalleveDocker::LogException(const char* pszThread, std::exception* pex)
{
    const char* pszError = (pex != NULL) ? pex->what() : "";

    ostringstream oss;
    oss << "(" << (pszThread != NULL ? pszThread : "") << "): " << ((pex != NULL) ? pex->what() : "unknown") << '\n';
    va_list ap;
    LogOutput("docker","[ERROR]", oss.str().c_str(), ap);
}

void CWalleveDocker::TimerProc()
{
    while(!fShutdown)
    {
        vector<uint32> vWorkQueue;
        {
            boost::unique_lock<boost::mutex> lock(mtxTimer);
            if (fShutdown)
            {
                break;
            }
            multimap<boost::system_time,uint32>::iterator mi = mapTimerByExpiry.begin();
            if (mi == mapTimerByExpiry.end())
            {
                condTimer.wait(lock);
                continue;
            }
            boost::system_time const now = boost::get_system_time(); 
            if ((*mi).first > now)
            {
                condTimer.timed_wait(lock,(*mi).first);
            }
            else
            {
                while(mi != mapTimerByExpiry.end() && (*mi).first <= now)
                {
                    vWorkQueue.push_back((*mi).second);
                    mapTimerByExpiry.erase(mi++);
                }
            }
        }
        BOOST_FOREACH(const uint32& nTimerId,vWorkQueue)
        {
            if (fShutdown)
            {
                break;
            }
            TimerCallback fnCallback = NULL;
            {
                boost::unique_lock<boost::mutex> lock(mtxTimer);
                map<uint32,CWalleveTimer>::iterator it = mapTimerById.find(nTimerId);
                if (it != mapTimerById.end())
                {
                    fnCallback = (*it).second.fnCallback;
                    mapTimerById.erase(it);
                }
            }
            if (fnCallback != NULL)
            {
                fnCallback(nTimerId);
            }
        }
    }
}
