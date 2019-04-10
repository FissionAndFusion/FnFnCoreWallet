// Copyright (c) 2016-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "eventproc.h"

#include <boost/bind.hpp>
#include <thread>

using namespace std;
using namespace walleve;

///////////////////////////////
// CWalleveEventProc

CWalleveEventProc::CWalleveEventProc(const string& walleveOwnKeyIn, const size_t nThreadNumIn, const bool fAffinityIn)
 : IWalleveBase(walleveOwnKeyIn), nThreadNum(nThreadNumIn), fAffinity(fAffinityIn)
{
    vecThrEventQue.reserve(nThreadNum);
    auto nextCPU = fAffinity
        ? [](int& x) -> int { return x++ % thread::hardware_concurrency(); }
        : [](int& x) -> int { return -1; };
    
    int nCPU = 0;
    for (size_t i = 0; i < nThreadNum; ++i)
    {
        vecThrEventQue.emplace_back(walleveOwnKeyIn + "-eventq-" + to_string(i),
            boost::bind(&CWalleveEventProc::EventThreadFunc,this), nextCPU(nCPU));
    }
}

bool CWalleveEventProc::WalleveHandleInvoke()
{   
    queEvent.Reset();

    for (CWalleveThread& thrEventQue : vecThrEventQue)
    {
        if (!WalleveThreadStart(thrEventQue))
        {
            return false;
        }
    }
    return true;
}   

void CWalleveEventProc::WalleveHandleHalt()
{
    queEvent.Interrupt();
    
    for (CWalleveThread& thrEventQue : vecThrEventQue)
    {
        WalleveThreadExit(thrEventQue);
    }
}

void CWalleveEventProc::PostEvent(CWalleveEvent * pEvent)
{
    queEvent.AddNew(pEvent);
}

void CWalleveEventProc::EventThreadFunc()
{
    CWalleveEvent *pEvent = NULL;
    while ((pEvent = queEvent.Fetch()) != NULL)
    {
        if (!pEvent->Handle(*this))
        {
            queEvent.Reset();
        }
        pEvent->Free();
    }
}

