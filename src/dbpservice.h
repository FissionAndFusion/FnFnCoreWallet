#ifndef MULTIVERSE_DBP_SERVICE_H
#define MULTIVERSE_DBP_SERVICE_H

#include "mvbase.h"
#include "walleve/walleve.h"
#include "event.h"

#include <map>
#include <set>
#include <vector>
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
    bool HandleEvent(walleve::CWalleveEventDbpPong& event) override;

    // notify add msg(block tx ...) to event handler
    bool HandleEvent(CMvEventDbpUpdateNewBlock& event) override;
    bool HandleEvent(CMvEventDbpUpdateNewTx& event) override;
protected:
    bool WalleveHandleInitialize() override;
    void WalleveHandleDeinitialize() override;
private:
    void CreateDbpBlock(const CBlock& blockDetail,const uint256& forkHash, 
    int blockHeight,walleve::CWalleveDbpBlock& block);
    void CreateDbpTransaction(const CTransaction& tx,walleve::CWalleveDbpTransaction& dbptx);
    bool GetBlocks(const uint256& startHash, int32 n, std::vector<walleve::CWalleveDbpBlock>& blocks);
    void HandleGetBlocks(walleve::CWalleveEventDbpMethod& event);
    void HandleGetTransaction(walleve::CWalleveEventDbpMethod& event);
    void HandleSendTransaction(walleve::CWalleveEventDbpMethod& event);
protected:
    walleve::IIOProc *pDbpServer;
    IService *pService;
    IWallet *pWallet;
private:
    typedef std::set<std::string> TopicSet;
    std::map<std::string,TopicSet> idSubedTopicsMap; // id => subed topics
    std::map<std::string,std::string> idSubedSessionMap; // id => session
    std::map<std::string,bool> currentTopicExistMap;
};



} // namespace multiverse

#endif //MULTIVERSE_DBP_SERVICE_H