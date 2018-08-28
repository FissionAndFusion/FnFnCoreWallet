#ifndef MULTIVERSE_DBP_SERVICE_H
#define MULTIVERSE_DBP_SERVICE_H

#include "mvbase.h"
#include "walleve/walleve.h"

#include <map>
#include <utility>

namespace multiverse{

class CDbpService : public walleve::IIOModule, virtual public walleve::CWalleveDBPEventListener
{
public:
    CDbpService();
    virtual ~CDbpService();

    bool HandleEvent(walleve::CWalleveEventDbpConnect& event) override;
    bool HandleEvent(walleve::CWalleveEventDbpSub& event) override;
    bool HandleEvent(walleve::CWalleveEventDbpUnSub& event) override;
    bool HandleEvent(walleve::CWalleveEventDbpMethod& event) override;
protected:
    bool WalleveHandleInitialize() override;
    void WalleveHandleDeinitialize() override;
protected:
    walleve::IIOProc *pDbpServer;
    IService *pService;
private:
    typedef std::pair<std::string, std::string> IdTopicKeyPair;
    std::map<IdTopicKeyPair,int> idTopicMap;
    std::map<std::string,bool> currentTopicExistMap;
};



} // namespace multiverse

#endif //MULTIVERSE_DBP_SERVICE_H