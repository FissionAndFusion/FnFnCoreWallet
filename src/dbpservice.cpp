#include "dbpservice.h"

#include <boost/assign/list_of.hpp>

using namespace multiverse;

CDbpService::CDbpService()
:walleve::IIOModule("dbpservice")
{
    pService = NULL;
    pDbpServer = NULL;
}

CDbpService::~CDbpService()
{

}


bool CDbpService::WalleveHandleInitialize()
{
    if (!WalleveGetObject("service",pService))
    {
        WalleveLog("Failed to request service\n");
        return false;
    }

    if (!WalleveGetObject("dbpserver",pDbpServer))
    {
        WalleveLog("Failed to request dbpserver\n");
        return false;
    }

    return true;
}

void CDbpService::WalleveHandleDeinitialize()
{
    pDbpServer = NULL;
    pService = NULL;
}

bool CDbpService::HandleEvent(walleve::CWalleveEventDbpConnect& event)
{
    (void)event.data.client;

    if(event.data.version != 1 || event.data.session.empty())
    {
        // reply error
        uint64 nonce = event.nNonce;
        std::vector<int> versions{1};
        walleve::CWalleveEventDbpFailed eventFailed(nonce);
        eventFailed.data.versions = versions;
        pDbpServer->DispatchEvent(&eventFailed);
    }
    else
    {
        // reply normal
        uint64 nonce = event.nNonce;
        walleve::CWalleveEventDbpConnected eventConnected(nonce);
        eventConnected.data.session = event.data.session;
        pDbpServer->DispatchEvent(&eventConnected);
    }

    return true;
}

bool CDbpService::HandleEvent(walleve::CWalleveEventDbpSub& event)
{
    return true;
}

bool CDbpService::HandleEvent(walleve::CWalleveEventDbpUnSub& event)
{
    return true;
}

bool CDbpService::HandleEvent(walleve::CWalleveEventDbpMethod& event)
{
    return true;
}