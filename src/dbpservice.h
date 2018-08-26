#ifndef MULTIVERSE_DBP_SERVICE_H
#define MULTIVERSE_DBP_SERVICE_H

#include "mvbase.h"
#include "walleve/walleve.h"

namespace multiverse{

class CDbpService : public walleve::IIOModule, virtual public walleve::CWalleveDBPEventListener
{
public:
    CDbpService();
    virtual ~CDbpService();
protected:
    bool WalleveHandleInitialize() override;
    void WalleveHandleDeinitialize() override;
protected:
    walleve::IIOProc *pDbpServer;
    IService *pService;
};



} // namespace multiverse

#endif //MULTIVERSE_DBP_SERVICE_H