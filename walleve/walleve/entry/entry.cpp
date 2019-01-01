// Copyright (c) 2016-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "entry.h"
#include <signal.h>
#include <stdio.h>
#include <boost/bind.hpp>
using namespace std;
using namespace walleve;


///////////////////////////////
// CWalleveEntry

CWalleveEntry::CWalleveEntry()
: ioService(),ipcSignals(ioService)
{
    ipcSignals.add(SIGINT);
    ipcSignals.add(SIGTERM);
#if defined(SIGQUIT)
    ipcSignals.add(SIGQUIT);
#endif // defined(SIGQUIT)
#if defined(SIGHUP)
    ipcSignals.add(SIGHUP);
#endif // defined(SIGHUP)

    ipcSignals.async_wait(boost::bind(&CWalleveEntry::HandleSignal,this,_1,_2));
}

CWalleveEntry::~CWalleveEntry()
{
}

bool CWalleveEntry::TryLockFile(const string& strLockFile)
{
    FILE* fp = fopen(strLockFile.c_str(), "a");
    if (fp) fclose(fp);
    lockFile = boost::interprocess::file_lock(strLockFile.c_str());
    return lockFile.try_lock();
}

bool CWalleveEntry::Run()
{
    try
    {
        ioService.run();
    }
    catch (...)
    {
        return false;
    }
    return true;
}

void CWalleveEntry::Stop()
{
    ioService.stop();
}

void CWalleveEntry::HandleSignal(const boost::system::error_code& error,int signal_number)
{
    if (signal_number == SIGINT || signal_number == SIGTERM)
    {
        Stop();
    }
}


