// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "dbpclient.h"
#include "walleve/netio/netio.h"
namespace multiverse
{

CMvDbpClient::CMvDbpClient()
  : walleve::CIOProc("dbpclient")
{
}

CMvDbpClient::~CMvDbpClient() noexcept {}

bool CMvDbpClient::WalleveHandleInitialize()
{
    // init client config

    return true;
}

void CMvDbpClient::WalleveHandleDeinitialize()
{
    // delete client config
}

void CMvDbpClient::EnterLoop()
{
    // start resource
    WalleveLog("Dbp Client starting:\n");
}

void CMvDbpClient::LeaveLoop()
{
    WalleveLog("Dbp Client stop\n");
    // destory resource
}

} // namespace multiverse