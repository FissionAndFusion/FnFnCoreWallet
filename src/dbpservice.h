#ifndef MULTIVERSE_DBP_SERVICE_H
#define MULTIVERSE_DBP_SERVICE_H

#include "mvbase.h"
#include "walleve/walleve.h"
#include "event.h"

#include <vector>
#include <utility>
#include <unordered_map>

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

    bool IsTopicExist(const std::string& topic);
    bool IsHaveSubedTopicOf(const std::string& id);
    
    void SubTopic(const std::string& id, const std::string& session, const std::string& topic);
    void UnSubTopic(const std::string& id);

    void PushTopic(const std::string& topic);
protected:
    walleve::IIOProc *pDbpServer;
    IService *pService;
    IWallet *pWallet;
private:
    std::map<std::string,std::string> idSubedTopicMap; // id => subed topic
    std::map<std::string,std::string> subedTopicIdMap; // subed topic => id
    std::map<std::string,std::string> idSubedSessionMap; // id => session
    std::unordered_map<std::string,bool> currentTopicExistMap; // topic => enabled
};



} // namespace multiverse

#endif //MULTIVERSE_DBP_SERVICE_H