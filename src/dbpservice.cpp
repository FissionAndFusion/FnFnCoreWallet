#include "dbpservice.h"

using namespace multiverse;


CDbpService::CDbpService()
:walleve::CDbpServer()
{

}

CDbpService::~CDbpService()
{

}


bool CDbpService::WalleveHandleInitialize()
{
    if(!walleve::CDbpServer::WalleveHandleInitialize())
    {
        return false;
    }

    if (!WalleveGetObject("service",pService))
    {
        WalleveLog("Failed to request service\n");
        return false;
    }

    return true;
}

void CDbpService::WalleveHandleDeinitialize()
{
    walleve::CDbpServer::WalleveHandleDeinitialize();
}