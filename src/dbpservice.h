// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_DBP_SERVICE_H
#define MULTIVERSE_DBP_SERVICE_H

#include "mvbase.h"
#include "dbpserver.h"
#include "event.h"
#include "virtualpeernetevent.h"
#include "mvpeernet.h"
#include "walleve/walleve.h"

#include <set>
#include <utility>
#include <unordered_map>


namespace multiverse
{

using namespace network;

class CDbpService : public walleve::IIOModule, virtual public CDBPEventListener, 
                    virtual public CMvDBPEventListener, virtual public CFkNodeEventListener
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
    bool HandleEvent(CMvEventDbpRemoveSession& event) override;
    // notify add msg(block tx ...) to event handler
    bool HandleEvent(CMvEventDbpUpdateNewBlock& event) override;
    bool HandleEvent(CMvEventDbpUpdateNewTx& event) override;

protected:
    bool WalleveHandleInitialize() override;
    void WalleveHandleDeinitialize() override;

private:
    typedef std::set<std::string> ForksType;
    typedef std::set<std::string> IdsType;
    typedef std::tuple<int, std::string> ForkStates; // (lastHeight, lastBlockHash)
    typedef std::tuple<std::string, std::string, int> ParentForkInfo;  // (parentForkHash, parentJointHash, parentJointPointHeight)
    typedef std::vector<ParentForkInfo>  ForkTopology;
    
    bool CalcForkPoints(const uint256& forkHash);
    void TrySwitchFork(const uint256& blockHash, uint256& forkHash);
    bool GetLwsBlocks(const uint256& forkHash, const uint256& startHash, int32 n, std::vector<CMvDbpBlock>& blocks);
    bool IsEmpty(const uint256& hash);
    bool IsForkHash(const uint256& hash);
  
    
    void HandleGetBlocks(CMvEventDbpMethod& event);
    void HandleGetTransaction(CMvEventDbpMethod& event);
    void HandleSendTransaction(CMvEventDbpMethod& event);
   

    bool IsTopicExist(const std::string& topic);

    void SubTopic(const std::string& id, const std::string& session, const std::string& topic);
    void UnSubTopic(const std::string& id);
    void RemoveSession(const std::string& session);

    void PushBlock(const std::string& forkid, const CMvDbpBlock& block);
    void PushTx(const std::string& forkid, const CMvDbpTransaction& dbptx);
   

    void RespondFailed(CMvEventDbpConnect& event);
    void RespondConnected(CMvEventDbpConnect& event);
    void RespondNoSub(CMvEventDbpSub& event);
    void RespondReady(CMvEventDbpSub& event);


protected:
    walleve::IIOProc* pDbpServer;
    walleve::IIOProc* pDbpClient;
    walleve::IIOProc* pVirtualPeerNet;
    IService* pService;
    ICoreProtocol* pCoreProtocol;
    IWallet* pWallet;
    IMvNetChannel* pNetChannel;
    IForkManager* pForkManager;

private:
    std::map<std::string, ForksType> mapSessionChildNodeForks; // session => child node forks

    std::map<std::string, std::string> mapIdSubedSession;       // id => session
    std::unordered_map<std::string, IdsType> mapTopicIds;       // topic => ids

    std::unordered_map<std::string, std::pair<uint256,uint256>> mapForkPoint; // fork point hash => (fork hash, fork point hash)
};

} // namespace multiverse

#endif //MULTIVERSE_DBP_SERVICE_H
