// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "dbpclient.h"

namespace multiverse
{

CMvDbpClient::CMvDbpClient()
  : walleve::IIOModule("dbpclient")
{
}

CMvDbpClient::~CMvDbpClient() {}

bool CMvDbpClient::WalleveHandleInitialize()
{
    return true;
}

void CMvDbpClient::WalleveHandleDeinitialize()
{
}

} // namespace multiverse