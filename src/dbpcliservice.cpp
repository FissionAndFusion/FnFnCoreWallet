// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "dbpcliservice.h"

namespace multiverse
{

CDbpCliService::CDbpCliService()
    : walleve::IIOModule("dbpcliservice")
{
    pDbpClient = NULL;
}
    
CDbpCliService::~CDbpCliService()
{

}

bool CDbpCliService::WalleveHandleInitialize()
{
    if (!WalleveGetObject("dbpclient", pDbpClient))
    {
        WalleveLog("Failed to request dbpclient\n");
        return false;
    }
    return true;
}

void CDbpCliService::WalleveHandleDeinitialize()
{
    pDbpClient = NULL;
}

bool CDbpCliService::HandleEvent(CMvEventDbpBroken& event)
{
    (void)event;
    return true;
}

} // namespace multiverse