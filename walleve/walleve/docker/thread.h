// Copyright (c) 2016-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  WALLEVE_THREAD_H
#define  WALLEVE_THREAD_H

#include <string>
#include <boost/function.hpp>
#include <boost/thread/thread.hpp>

namespace walleve
{

class CWalleveDock;

class CWalleveThread
{
    friend class CWalleveDocker;
public:
    typedef boost::function<void()> ThreadFunc;
    CWalleveThread(const std::string& strNameIn,ThreadFunc fnCallbackIn, const int nCPUIn = -1)
    : strThreadName(strNameIn),pThread(NULL),fnCallback(fnCallbackIn),fRunning(false), nCPU(nCPUIn)
    {
    }

    virtual ~CWalleveThread()
    {
        delete pThread;
    }

    bool IsRunning()
    {
        return fRunning;
    }

    void Interrupt()
    {
        if (pThread)
        {
            pThread->interrupt();
        }
    }

    void Exit()
    {
        if (pThread)
        {
            pThread->join();
            delete pThread;
            pThread = NULL;
        }
    }

    void SetAffinity()
    {
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifdef __USE_GNU
        if (pThread && nCPU >= 0)
        {
            cpu_set_t set;
            CPU_ZERO(&set);
            CPU_SET(nCPU, &set);
            pthread_setaffinity_np(pThread->native_handle(), sizeof(set), &set);
        }
#endif
    }

protected:
    const std::string strThreadName;
    boost::thread* pThread;
    ThreadFunc fnCallback;
    bool fRunning;
    int nCPU;
};

} // namespace walleve

#endif //WALLEVE_THREAD_H

