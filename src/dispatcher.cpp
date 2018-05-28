// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "dispatcher.h"

using namespace std;
using namespace walleve;
using namespace multiverse;

//////////////////////////////
// CDispatcher 

CDispatcher::CDispatcher()
{
    pCoreProtocol = NULL;
    pWorldLine = NULL;
}

CDispatcher::~CDispatcher()
{
}

bool CDispatcher::WalleveHandleInitialize()
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

void CDispatcher::WalleveHandleDeinitialize()
{
    pCoreProtocol = NULL;
    pWorldLine = NULL;
}

bool CDispatcher::WalleveHandleInvoke()
{
    return true;
}

void CDispatcher::WalleveHandleHalt()
{
}

MvErr CDispatcher::AddNewBlock(const CBlock& block)
{
}

MvErr CDispatcher::AddNewTx(const CTransaction& tx)
{
    return MV_OK;
}
