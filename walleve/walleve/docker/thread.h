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
    CWalleveThread(const std::string& strNameIn,ThreadFunc fnCallbackIn)
    : strThreadName(strNameIn),pThread(NULL),fnCallback(fnCallbackIn),fRunning(false)
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

protected:
    const std::string strThreadName;
    boost::thread* pThread;
    ThreadFunc fnCallback;
    bool fRunning;
};

} // namespace walleve

#endif //WALLEVE_THREAD_H

