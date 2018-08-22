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