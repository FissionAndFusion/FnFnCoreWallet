// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "wallet.h"

using namespace std;
using namespace walleve;
using namespace multiverse;

//////////////////////////////
// CWallet 

CWallet::CWallet()
{
    pCoreProtocol = NULL;
    pWorldLine = NULL;
}

CWallet::~CWallet()
{
}

bool CWallet::WalleveHandleInitialize()
{
    if (!WalleveGetObject("coreprotocol",pCoreProtocol))
    {
        WalleveLog("Failed to request coreprotocol\n");
        return false;
    }

    if (!WalleveGetObject("worldline",pWorldLine))
    {
        WalleveLog("Failed to request worldline\n");
        return false;
    }

    return true;
}

void CWallet::WalleveHandleDeinitialize()
{
    pCoreProtocol = NULL;
    pWorldLine = NULL;
}

bool CWallet::WalleveHandleInvoke()
{
    return true;
}

void CWallet::WalleveHandleHalt()
{
}

