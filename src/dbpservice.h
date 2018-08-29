#ifndef MULTIVERSE_DBP_SERVICE_H
#define MULTIVERSE_DBP_SERVICE_H

#include "mvbase.h"
#include "walleve/walleve.h"
#include "event.h"

#include <map>
#include <set>
#include <utility>

namespace multiverse{

class CDbpService : public walleve::IIOModule, virtual public walleve::CWalleveDBPEventListener,
virtual public CMvDBPEventListener
{
public:
    CDbpService();
    virtual ~CDbpService();

    bool HandleEvent(walleve::CWalleveEventDbpConnect& event) override;
    bool HandleEvent(walleve::CWalleveEventDbpSub& event) override;
    bool HandleEvent(walleve::CWalleveEventDbpUnSub& event) override;
    bool HandleEvent(walleve::CWalleveEventDbpMethod& event) override;

    // notify add msg(block tx ...) to event handler
    bool HandleEvent(CMvEventDbpUpdateNewBlock& event) override;
    bool HandleEvent(CMvEventDbpUpdateNewTx& event) override;
protected:
    bool WalleveHandleInitialize() override;
    void WalleveHandleDeinitialize() override;
protected:
    walleve::IIOProc *pDbpServer;
    IService *pService;
private:
    typedef std::set<std::string> TopicSet;
    std::map<std::string,TopicSet> idSubedTopicsMap; // id => subed topics
    std::map<std::string,int> idSubedNonceMap; // id => nonce
    std::map<std::string,bool> currentTopicExistMap;
};



} // namespace multiverse

#endif //MULTIVERSE_DBP_SERVICE_H