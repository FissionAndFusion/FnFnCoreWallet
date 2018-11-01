// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_DBP_SERVICE_H
#define MULTIVERSE_DBP_SERVICE_H

#include "mvbase.h"
#include "dbpserver.h"
#include "event.h"
#include "mvpeernet.h"
#include "walleve/walleve.h"

#include <set>
#include <utility>
#include <unordered_map>


namespace multiverse
{

using namespace network;

class CDbpService : public walleve::IIOModule, virtual public CDBPEventListener, virtual public CMvDBPEventListener
{
public:
    CDbpService();
    virtual ~CDbpService() noexcept;

    bool HandleEvent(CMvEventDbpConnect& event) override;
    bool HandleEvent(CMvEventDbpSub& event) override;
    bool HandleEvent(CMvEventDbpUnSub& event) override;
    bool HandleEvent(CMvEventDbpMethod& event) override;
    bool HandleEvent(CMvEventDbpPong& event) override;
    bool HandleEvent(CMvEventDbpBroken& event) override;
    bool HandleEvent(CMvEventDbpAdded& event) override;

    // notify add msg(block tx ...) to event handler
    bool HandleEvent(CMvEventDbpUpdateNewBlock& event) override;
    bool HandleEvent(CMvEventDbpUpdateNewTx& event) override;

protected:
    bool WalleveHandleInitialize() override;
    void WalleveHandleDeinitialize() override;

private:
    void CreateDbpBlock(const CBlockEx& blockDetail, const uint256& forkHash,
                      int blockHeight, CMvDbpBlock& block);
    void CreateDbpTransaction(const CTransaction& tx, int64 nChange, CMvDbpTransaction& dbptx);
    bool CalcForkPoints(const uint256& forkHash);
    void TrySwitchFork(const uint256& blockHash, uint256& forkHash);
    bool GetBlocks(const uint256& forkHash, const uint256& startHash, int32 n, std::vector<CMvDbpBlock>& blocks);
    bool IsEmpty(const uint256& hash);
    bool IsForkHash(const uint256& hash);
    void HandleGetBlocks(CMvEventDbpMethod& event);
    void HandleGetTransaction(CMvEventDbpMethod& event);
    void HandleSendTransaction(CMvEventDbpMethod& event);
    void HandleRegisterFork(CMvEventDbpMethod& event);
    void HandleSendBlock(CMvEventDbpMethod& event);
    void HandleSendTx(CMvEventDbpMethod& event);

    bool IsTopicExist(const std::string& topic);
    bool IsHaveSubedTopicOf(const std::string& id);

    void SubTopic(const std::string& id, const std::string& session, const std::string& topic);
    void UnSubTopic(const std::string& id);

    void PushBlock(const std::string& forkid, const CMvDbpBlock& block);
    void PushTx(const std::string& forkid, const CMvDbpTransaction& dbptx);

    ///////////  super node  ////////////
    void UpdateChildNodeForks(const std::string& session, const std::string& forks);
protected:
    walleve::IIOProc* pDbpServer;
    walleve::IIOProc* pDbpClient;
    IService* pService;
    ICoreProtocol* pCoreProtocol;
    IWallet* pWallet;
    IMvNetChannel* pNetChannel;

private:
    std::map<std::string, std::string> mapIdSubedTopic; // id => subed topic

    std::set<std::string> setSubedAllBlocksIds; // block ids
    std::set<std::string> setSubedAllTxIds;     // tx ids

    typedef std::set<std::string> ForksType;
    std::map<std::string, ForksType> mapSessionChildNodeForks; // session => child node forks

    std::map<std::string, std::string> mapIdSubedSession;       // id => session
    std::unordered_map<std::string, bool> mapCurrentTopicExist; // topic => enabled

    std::unordered_map<std::string, std::pair<uint256,uint256>> mapForkPoint; // fork point hash => (fork hash, fork point hash)
};

} // namespace multiverse

#endif //MULTIVERSE_DBP_SERVICE_H
