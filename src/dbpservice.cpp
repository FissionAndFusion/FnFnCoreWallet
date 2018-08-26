#include "dbpservice.h"

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
    event.data.session;
    event.data.version;

    if(event.data.version != 1)
    {
        // reply error
    }
    else
    {
        // reply normal
        
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