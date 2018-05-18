// Copyright (c) 2016-2018 The LoMoCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "event.h"

using namespace std;
using namespace walleve;

///////////////////////////////
// CWalleveEventListener

bool CWalleveEventListener::DispatchEvent(CWalleveEvent* pEvent)
{
    return pEvent->Handle(*this);
}

