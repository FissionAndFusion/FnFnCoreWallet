#ifndef MULTIVERSE_DBP_SERVICE_H
#define MULTIVERSE_DBP_SERVICE_H

#include "mvbase.h"
#include "walleve/walleve.h"

namespace multiverse{

class CDbpService : public walleve::CDbpServer
{
public:
    CDbpService();
    virtual ~CDbpService();
protected:
    bool WalleveHandleInitialize();
    void WalleveHandleDeinitialize();
protected:
    IService *pService;
};



} // namespace multiverse

#endif //MULTIVERSE_DBP_SERVICE_H