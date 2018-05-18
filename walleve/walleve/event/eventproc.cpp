// Copyright (c) 2016-2018 The LoMoCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "eventproc.h"

#include <boost/bind.hpp>

using namespace std;
using namespace walleve;

///////////////////////////////
// CWalleveEventProc

CWalleveEventProc::CWalleveEventProc(const string& walleveOwnKeyIn)
 : IWalleveBase(walleveOwnKeyIn) ,
   thrEventQue(walleveOwnKeyIn + "-eventq",boost::bind(&CWalleveEventProc::EventThreadFunc,this))
{
}

bool CWalleveEventProc::WalleveHandleInvoke()
{   
    queEvent.Reset();

    return WalleveThreadStart(thrEventQue);
}   

void CWalleveEventProc::WalleveHandleHalt()
{
    queEvent.Interrupt();
    
    WalleveThreadExit(thrEventQue);
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

