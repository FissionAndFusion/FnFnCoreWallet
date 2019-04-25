// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "dbpservice.h"

#include "dbputils.h"
#include <boost/assign/list_of.hpp>
#include <boost/lexical_cast.hpp>
#include <algorithm>

using namespace multiverse;

CDbpService::CDbpService()
    : walleve::IIOModule("dbpservice")
{
    pService = NULL;
    pCoreProtocol = NULL;
    pDbpClient = NULL;
    pDbpServer = NULL;
    pNetChannel = NULL;
    pVirtualPeerNet = NULL;
    pRPCMod = NULL;

    std::unordered_map<std::string, IdsType> temp_map = 
        boost::assign::map_list_of(ALL_BLOCK_TOPIC, std::set<std::string>())
                                  (ALL_TX_TOPIC,    std::set<std::string>())
                                  (SYS_CMD_TOPIC,   std::set<std::string>())
                                  (TX_CMD_TOPIC,    std::set<std::string>())
                                  (BLOCK_CMD_TOPIC, std::set<std::string>())
                                  (CHANGED_TOPIC,   std::set<std::string>())
                                  (REMOVED_TOPIC,   std::set<std::string>())
                                  (RPC_CMD_TOPIC,   std::set<std::string>());

    mapTopicIds = temp_map;

    fEnableSuperNode = false;
    fEnableForkNode = false;
}

CDbpService::~CDbpService() noexcept
{
}

bool CDbpService::WalleveHandleInitialize()
{
    if (!WalleveGetObject("coreprotocol", pCoreProtocol))
    {
        WalleveError("Failed to request coreprotocol\n");
        return false;
    }

    if (!WalleveGetObject("service", pService))
    {
        WalleveError("Failed to request service\n");
        return false;
    }

    if (!WalleveGetObject("dbpserver", pDbpServer))
    {
        WalleveError("Failed to request dbpserver\n");
        return false;
    }

    if(!WalleveGetObject("dbpclient",pDbpClient))
    {
        WalleveError("Failed to request dbpclient\n");
        return false;
    }

    if (!WalleveGetObject("netchannel",pNetChannel))
    {
        WalleveError("Failed to request peer net datachannel\n");
        return false;
    }

    if (!WalleveGetObject("virtualpeernet",pVirtualPeerNet))
    {
        WalleveLog("Failed to request virtual peer net\n");
        return false;
    }

    if(!WalleveGetObject("rpcmod", pRPCMod))
    {
        WalleveLog("Failed to request rpc mod\n");
        return false;
    }

    return true;
}

void CDbpService::WalleveHandleDeinitialize()
{
    pDbpClient = NULL;
    pDbpServer = NULL;
    pService = NULL;
    pCoreProtocol = NULL;
    pNetChannel = NULL;
    pVirtualPeerNet = NULL;
}

void CDbpService::EnableForkNode(bool enable)
{
    fEnableForkNode = enable;
}

void CDbpService::EnableSuperNode(bool enable)
{
    fEnableSuperNode = enable;
}

void CDbpService::DeactiveNodeTree(const std::string& session)
{
    for(const auto& ele : mapPeerEventActive)
    {
        uint64 nNonce = ele.first;
        auto &event = ele.second;
   
        CWalleveBufStream ss;
        decltype(event.data) bytes = event.data;
        ss.Write((char*)bytes.data(), bytes.size());
        
        CMvEventPeerActive eventActive(0);
        ss >> eventActive;
        
        CMvEventPeerDeactive eventDeactive(nNonce);
        eventDeactive.data = eventActive.data;  
        pVirtualPeerNet->DispatchEvent(&eventDeactive);

        CWalleveBufStream ssDeactive;
        ssDeactive << eventDeactive;
        std::string data(ssDeactive.GetData(), ssDeactive.GetSize());

        CMvDbpVirtualPeerNetEvent eventVPeer;
        eventVPeer.nNonce = nNonce;
        eventVPeer.type = CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_DEACTIVE;
        eventVPeer.data = std::vector<uint8>(data.begin(), data.end());

        PushEvent(eventVPeer);
    }
}

void CDbpService::UnsubscribeChildNodeForks(const std::string& session)
{
    std::vector<ForkNonceKeyType> beDeleteKeys;
    std::vector<ForkNonceKeyType>& beFindKeys = mapSessionForkNonceKey[session];

    for(const auto& key : beFindKeys)
    {
        auto iter = mapChildNodeForkCount.find(key);
        if(iter != mapChildNodeForkCount.end())
        {
            uint64 nNonce = key.second;
            uint256 forkHash = key.first;
            uint32& forkCount = iter->second;
            if(forkCount == 1)
            {
                forkCount = 0;
                beDeleteKeys.push_back(key);
                
                CMvEventPeerUnsubscribe eventUpUnSub(nNonce, pCoreProtocol->GetGenesisBlockHash());
                eventUpUnSub.data.push_back(forkHash);

                CWalleveBufStream eventSs;
                eventSs << eventUpUnSub;
                std::string data(eventSs.GetData(), eventSs.GetSize());
                
                if(IsForkNodeOfSuperNode())
                {
                    CMvDbpVirtualPeerNetEvent vpeerEvent;
                    vpeerEvent.type = CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_UNSUBSCRIBE;
                    vpeerEvent.data = std::vector<uint8>(data.begin(), data.end());
                    SendEventToParentNode(vpeerEvent);
                }

                if(IsRootNodeOfSuperNode())
                {
                    eventUpUnSub.flow = "up";
                    eventUpUnSub.sender = "dbpservice";
                    pVirtualPeerNet->DispatchEvent(&eventUpUnSub);
                }
            }

            if(forkCount > 1)
            {
                forkCount--;
            }
        } 
    }
    
    for(const auto& key : beDeleteKeys)
    {
        mapChildNodeForkCount.erase(key);
    }
}

void CDbpService::HandleDbpClientBroken(const std::string& session)
{
    DeactiveNodeTree(session);
    mapThisNodeForkCount.clear();
    mapThisNodeGetData.clear();
}

void CDbpService::HandleDbpServerBroken(const std::string& session)
{
    UnsubscribeChildNodeForks(session);
    RemoveSession(session);
}

bool CDbpService::HandleEvent(CMvEventDbpBroken& event)
{
    if(event.data.from == "dbpserver")
    {
        HandleDbpServerBroken(event.strSessionId);
    }

    if(event.data.from == "dbpclient")
    {
        HandleDbpClientBroken(event.strSessionId);
    }

    return true;
}

bool CDbpService::HandleEvent(CMvEventDbpConnect& event)
{
    bool isReconnect = event.data.isReconnect;
    
    if (isReconnect)
    {
        RespondConnected(event);
    }
    else
    {
        if (event.data.version != 1)
        {
            RespondFailed(event);
        }
        else
        {
            std::cout << "[<] Recved Connect [dbpservice]" << std::endl; 
            
            RespondConnected(event);


            std::cout << "[>] Sent Connected " << event.strSessionId << " [dbpservice]" << std::endl;

            std::cout << "[>] MapPeerEventActive Size " << mapPeerEventActive.size() << " [dbpservice]" << std::endl;

            for(const auto& virtualevent : mapPeerEventActive)
            {
                std::string session(event.strSessionId);
                CMvEventDbpAdded eventAdd(session);
                eventAdd.data.name = "event";
                eventAdd.data.anyAddedObj = virtualevent.second;
                return pDbpServer->DispatchEvent(&eventAdd);
            }

            if(mapPeerEventActive.size() == 0)
            {
                CWalleveBufStream ss;
                CMvEventPeerActive eventAct(std::numeric_limits<uint64>::max());
                eventAct.data.nService = network::NODE_NETWORK;
                ss << eventAct;
                std::string data(ss.GetData(), ss.GetSize());
        
                CMvDbpVirtualPeerNetEvent eventVPeer;
                eventVPeer.nNonce = event.nNonce;
                eventVPeer.type = CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_ACTIVE;
                eventVPeer.data = std::vector<uint8>(data.begin(), data.end());

                std::string session(event.strSessionId);
                CMvEventDbpAdded eventAdd(session);
                eventAdd.data.name = "event";
                eventAdd.data.anyAddedObj = eventVPeer;
                return pDbpServer->DispatchEvent(&eventAdd);
            }
        }
    }

    return true;
}

bool CDbpService::HandleEvent(CMvEventDbpSub& event)
{
    std::string id = event.data.id;
    std::string topicName = event.data.name;

    if (!IsTopicExist(topicName))
    {
        RespondNoSub(event);
    }
    else
    {
        SubTopic(id, event.strSessionId, topicName);
        RespondReady(event);
    }

    return true;
}

bool CDbpService::HandleEvent(CMvEventDbpUnSub& event)
{
    UnSubTopic(event.data.id);
    return true;
}

void CDbpService::HandleGetTransaction(CMvEventDbpMethod& event)
{
    std::string id = event.data.id;
    std::string txid = boost::any_cast<std::string> 
     (event.data.params["hash"]);

    uint256 txHash(txid);
    CTransaction tx;
    uint256 forkHash;
    int32 blockHeight;

    if (pService->GetTransaction(txHash, tx, forkHash, blockHeight))
    {
        CMvDbpTransaction dbpTx;
        CDbpUtils::RawToDbpTransaction(tx, forkHash, 0, dbpTx);

        CMvEventDbpMethodResult eventResult(event.strSessionId);
        eventResult.data.id = id;
        eventResult.data.anyResultObjs.push_back(dbpTx);
        pDbpServer->DispatchEvent(&eventResult);
    }
    else
    {
        CMvEventDbpMethodResult eventResult(event.strSessionId);
        eventResult.data.id = id;
        eventResult.data.error = "404";
        pDbpServer->DispatchEvent(&eventResult);
    }
}

void CDbpService::HandleSendTransaction(CMvEventDbpMethod& event)
{
    std::string data = boost::any_cast 
        <std::string>(event.data.params["data"]);

    std::vector<unsigned char> txData(data.begin(), data.end());
    walleve::CWalleveBufStream ss;
    ss.Write((char *)&txData[0], txData.size());

    CTransaction rawTx;
    try
    {
        ss >> rawTx;
    }
    catch (const std::exception& e)
    {
        CMvEventDbpMethodResult eventResult(event.strSessionId);
        eventResult.data.id = event.data.id;
        eventResult.data.error = "400";
        pDbpServer->DispatchEvent(&eventResult);
        return;
    }

    MvErr err = pService->SendTransaction(rawTx);
    if (err == MV_OK)
    {
        CMvEventDbpMethodResult eventResult(event.strSessionId);
        eventResult.data.id = event.data.id;

        CMvDbpSendTransactionRet sendTxRet;
        sendTxRet.hash = data;
        sendTxRet.result = "succeed";
        eventResult.data.anyResultObjs.push_back(sendTxRet);

        pDbpServer->DispatchEvent(&eventResult);
    }
    else
    {
        CMvEventDbpMethodResult eventResult(event.strSessionId);
        eventResult.data.id = event.data.id;

        CMvDbpSendTransactionRet sendTxRet;
        sendTxRet.hash = data;
        sendTxRet.result = "failed";
        sendTxRet.reason = std::string(MvErrString(err));
        eventResult.data.anyResultObjs.push_back(sendTxRet);

        pDbpServer->DispatchEvent(&eventResult);
    }
}

bool CDbpService::IsTopicExist(const std::string& topic)
{
    return mapTopicIds.find(topic) != mapTopicIds.end();
}

void CDbpService::SubTopic(const std::string& id, const std::string& session, const std::string& topic)
{
    mapTopicIds[topic].insert(id);
    mapIdSubedSession.insert(std::make_pair(id, session));
}

void CDbpService::UnSubTopic(const std::string& id)
{
    for(auto& kv : mapTopicIds)
    {
        kv.second.erase(id);
    }
    mapIdSubedSession.erase(id);
}

void CDbpService::RemoveSession(const std::string& session)
{
    std::vector<std::string> vBeDeletedIds;
    for(const auto& kv : mapIdSubedSession)
    {
        std::string id = kv.first;
        std::string valueSession = kv.second;
        if(valueSession == session)
        {
            vBeDeletedIds.push_back(id);
        }
    }

    for(const auto& id : vBeDeletedIds)
    {
        UnSubTopic(id);
    }

    mapSessionForkNonceKey.erase(session);
}

bool CDbpService::IsEmpty(const uint256& hash)
{
    static const uint256 EMPTY_HASH;
    return hash == EMPTY_HASH;
}

bool CDbpService::IsForkHash(const uint256& hash)
{
    std::vector<std::pair<uint256,CProfile>> forks;
    pService->ListFork(forks);

    for(const auto& fork : forks)
    {
        if(fork.first == hash)
        {
            return true;
        }
    }

    return false;
}

bool CDbpService::IsMyFork(const uint256& hash)
{
    return pNetChannel->IsContains(hash);
}

bool CDbpService::IsForkNodeOfSuperNode()
{
    return (fEnableSuperNode && fEnableForkNode);
}

bool CDbpService::IsRootNodeOfSuperNode()
{
    return (fEnableSuperNode && !fEnableForkNode);
}

void CDbpService::TrySwitchFork(const uint256& blockHash,uint256& forkHash)
{
    auto it = mapForkPoint.find(blockHash.ToString());
    if(it != mapForkPoint.end())
    {
        auto value = it->second; 
        forkHash = value.first;
    } 
}

bool CDbpService::CalcForkPoints(const uint256& forkHash)
{
    std::vector<std::pair<uint256,int32>> vAncestors;
    std::vector<std::pair<int32,uint256>> vSublines;
    std::vector<std::pair<uint256,uint256>> path;
    if(!pService->GetForkGenealogy(forkHash,vAncestors,vSublines))
    {
        return false;
    }

    std::vector<std::pair<uint256,uint256>> forkAncestors;
    for(int i = vAncestors.size() - 1; i >= 0; i--)
    {
        CBlock block;
        uint256 tempFork;
        int32 nHeight = 0;
        pService->GetBlock(vAncestors[i].first,block,tempFork,nHeight);
        forkAncestors.push_back(std::make_pair(vAncestors[i].first,block.hashPrev));
    }

    path = forkAncestors;
    CBlock block;
    uint256 tempFork;
    int32 nHeight = 0;
    pService->GetBlock(forkHash,block,tempFork,nHeight);
    path.push_back(std::make_pair(forkHash,block.hashPrev));

    for(const auto& fork : path)
    {
        mapForkPoint.insert(std::make_pair(fork.second.ToString(), 
            std::make_pair(fork.first,fork.second)));
    }
    
    return true;
}

bool CDbpService::GetLwsBlocks(const uint256& forkHash, const uint256& startHash, int32 n, std::vector<CMvDbpBlock>& blocks)
{
    uint256 connectForkHash = forkHash;
    uint256 blockHash = startHash;

    if (IsEmpty(connectForkHash))
    {
        connectForkHash = pCoreProtocol->GetGenesisBlockHash();
    }

    if(!IsForkHash(connectForkHash))
    {
        std::cerr << "connect fork hash is not a fork hash." << std::endl;
        return false;
    }

    if (IsEmpty(blockHash))
    {
        blockHash = pCoreProtocol->GetGenesisBlockHash();
    }

    int32 blockHeight = 0;
    uint256 tempForkHash;
    if (!pService->GetBlockLocation(blockHash, tempForkHash, blockHeight))
    {
        std::cerr << "GetBlockLocation failed" << std::endl;
        return false;
    }

    if(!CalcForkPoints(connectForkHash))
    {
        std::cerr << "CalcForkPoint failed." << std::endl;
        return false;
    }

    const std::size_t nonExtendBlockMaxNum = n;
    std::size_t nonExtendBlockCount = 0;
    
    pService->GetBlockLocation(blockHash, tempForkHash, blockHeight);
    
    std::vector<uint256> blocksHash;
    while (nonExtendBlockCount < nonExtendBlockMaxNum && 
            pService->GetBlockHash(tempForkHash, blockHeight, blocksHash))
    {  
        for(int i = 0; i < blocksHash.size(); ++i)
        {
            CBlockEx block;
            int32 height;
            pService->GetBlockEx(blocksHash[i], block, tempForkHash, height);
            if (block.nType != CBlock::BLOCK_EXTENDED)
            {
                nonExtendBlockCount++;
            }

            CMvDbpBlock DbpBlock;
            CDbpUtils::RawToDbpBlock(block, tempForkHash, height, DbpBlock);
            blocks.push_back(DbpBlock);
        }
        
        TrySwitchFork(blocksHash[0],tempForkHash);
        blockHeight++;
        blocksHash.clear(); blocksHash.shrink_to_fit();
       
    }

    return true;
}

void CDbpService::HandleGetBlocks(CMvEventDbpMethod& event)
{
    std::string forkid = boost::any_cast<std::string>(event.data.params["forkid"]);
    std::string blockHash = boost::any_cast<std::string>(event.data.params["hash"]);
    std::string num = boost::any_cast<std::string>(event.data.params["number"]);
    int32 blockNum = boost::lexical_cast<int32>(num);
    
    uint256 startBlockHash(std::vector<unsigned char>(blockHash.begin(), blockHash.end()));
    uint256 forkHash;
    forkHash.SetHex(forkid);
    std::vector<CMvDbpBlock> blocks;
    if (GetLwsBlocks(forkHash, startBlockHash, blockNum, blocks))
    {
        CMvEventDbpMethodResult eventResult(event.strSessionId);
        eventResult.data.id = event.data.id;

        for (auto& block : blocks)
        {
            eventResult.data.anyResultObjs.push_back(block);
        }

        pDbpServer->DispatchEvent(&eventResult);
    }
    else
    {
        CMvEventDbpMethodResult eventResult(event.strSessionId);
        eventResult.data.id = event.data.id;
        eventResult.data.error = "400";
        pDbpServer->DispatchEvent(&eventResult);
    }
}

void CDbpService::FilterChildSubscribeFork(const CMvEventPeerSubscribe& in, CMvEventPeerSubscribe& out)
{
    auto& vSubForks = in.data;
    for(const auto& fork : vSubForks)
    {
        auto key = ForkNonceKeyType(fork, in.nNonce);
        if(mapChildNodeForkCount.find(key) == mapChildNodeForkCount.end())
        {
            mapChildNodeForkCount[key] = 1;
            out.data.push_back(fork);
        }
        else
        {
            mapChildNodeForkCount[key]++;
        }
        
    }
}

void CDbpService::FilterChildUnsubscribeFork(const CMvEventPeerUnsubscribe& in, CMvEventPeerUnsubscribe& out)
{
    auto& vUnSubForks = in.data;
    for(const auto& fork : vUnSubForks)
    {
        auto key = ForkNonceKeyType(fork, in.nNonce);
        if(mapChildNodeForkCount.find(key) != mapChildNodeForkCount.end())
        {
            if(mapChildNodeForkCount[key] == 1)
            {
                mapChildNodeForkCount[key] = 0;
                out.data.push_back(fork);
                mapChildNodeForkCount.erase(key);
            }
            else
            {
                mapChildNodeForkCount[key]--;
            }
        }
    }
}

void CDbpService::CollectSessionSubForks(const std::string& session, const CMvEventPeerSubscribe& sub)
{
    if(mapSessionForkNonceKey.find(session) != mapSessionForkNonceKey.end())
    {
        for(const uint256& fork : sub.data)
        {
            mapSessionForkNonceKey[session].push_back(std::make_pair(fork, sub.nNonce));
        }
    }
    else
    {
        std::vector<ForkNonceKeyType> vForkNonceKey;
        for(const uint256& fork : sub.data)
        {
            vForkNonceKey.push_back(std::make_pair(fork, sub.nNonce));
        }
        
        mapSessionForkNonceKey[session] = vForkNonceKey;
    }
}

void CDbpService::CollectSessionUnSubForks(const std::string& session, const CMvEventPeerUnsubscribe& unsub)
{
    if(mapSessionForkNonceKey.find(session) != mapSessionForkNonceKey.end())
    {
        auto& sessionForkNonceKeys = mapSessionForkNonceKey[session];
        for(const uint256& fork : unsub.data)
        {
            auto iter = std::find(sessionForkNonceKeys.begin(), sessionForkNonceKeys.end(), 
                std::make_pair(fork, unsub.nNonce));
            if(iter != sessionForkNonceKeys.end())
            {
                sessionForkNonceKeys.erase(iter);
            }
        }
    }
}

// event from down to up
void CDbpService::HandleSendEvent(CMvEventDbpMethod& event)
{
    int type = boost::any_cast<int>(event.data.params["type"]);
    std::string eventData = boost::any_cast<std::string>(event.data.params["data"]);

    CWalleveBufStream ss;
    ss.Write(eventData.data(), eventData.size());
   
    // process reward event from down node
    if(type == CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_REWARD)
    {
        if(IsRootNodeOfSuperNode())
        {
            CWalleveSuperNodeEventPeerNetReward eventSNReward(0);
            ss >> eventSNReward;

            CWalleveEventPeerNetReward eventReward(eventSNReward.nNonce);
            eventReward.nType = eventSNReward.nType;
            eventReward.result = eventSNReward.result;
            eventReward.data = static_cast<CEndpointManager::Bonus>(eventSNReward.data);
            pVirtualPeerNet->DispatchEvent(&eventReward);
        }

        if(IsForkNodeOfSuperNode())
        {
            CMvDbpVirtualPeerNetEvent vpeerEvent;
            vpeerEvent.type = CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_REWARD;
            vpeerEvent.data = std::vector<uint8>(eventData.begin(), eventData.end());
            SendEventToParentNode(vpeerEvent);
        }
    }

    if(type == CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_CLOSE)
    {
        if(IsRootNodeOfSuperNode())
        {
            CWalleveSuperNodeEventPeerNetClose eventSNClose(0);
            ss >> eventSNClose;

            CWalleveEventPeerNetClose eventClose(eventSNClose.nNonce);
            eventClose.nType = eventSNClose.nType;
            eventClose.result = eventSNClose.result;
            eventClose.data = static_cast<CEndpointManager::CloseReason>(eventSNClose.data);
            pVirtualPeerNet->DispatchEvent(&eventClose);
        }

        if(IsForkNodeOfSuperNode())
        {
            CMvDbpVirtualPeerNetEvent vpeerEvent;
            vpeerEvent.type = CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_CLOSE;
            vpeerEvent.data = std::vector<uint8>(eventData.begin(), eventData.end());
            SendEventToParentNode(vpeerEvent);
        }
    }

    if(type == CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_SUBSCRIBE)
    {
        CMvEventPeerSubscribe eventSub(0,uint256());
        ss >> eventSub;

        CMvEventPeerSubscribe eventUpSub(eventSub.nNonce, eventSub.hashFork);
        FilterChildSubscribeFork(eventSub, eventUpSub);
        CollectSessionSubForks(event.strSessionId, eventSub);

        if(!eventUpSub.data.empty())
        {
            CWalleveBufStream eventSs;
            eventSs << eventUpSub;
            std::string data(eventSs.GetData(), eventSs.GetSize());
            
            if(IsForkNodeOfSuperNode())
            {
                CMvDbpVirtualPeerNetEvent vpeerEvent;
                vpeerEvent.type = CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_SUBSCRIBE;
                vpeerEvent.data = std::vector<uint8>(data.begin(), data.end());
                SendEventToParentNode(vpeerEvent);
            }

            if(IsRootNodeOfSuperNode())
            {
                //std::cout << "#############[rootnode] "  << "Subscribe Fork: [dbpservice]" << std::endl;
                for(const auto& fork : eventUpSub.data)
                {
                    //std::cout << "Fork ID " << fork.ToString() << " [dbpservice]" << std::endl;
                }
                eventUpSub.flow = "up";
                eventUpSub.sender = "dbpservice";
                pVirtualPeerNet->DispatchEvent(&eventUpSub);
            }
        }
    }

    if(type == CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_UNSUBSCRIBE)
    {
        CMvEventPeerUnsubscribe eventUnSub(0,uint256());
        ss >> eventUnSub;

        CMvEventPeerUnsubscribe eventUpUnSub(eventUnSub.nNonce, eventUnSub.hashFork);
        FilterChildUnsubscribeFork(eventUnSub, eventUpUnSub);
        CollectSessionUnSubForks(event.strSessionId, eventUnSub);

        if(!eventUpUnSub.data.empty())
        {
            CWalleveBufStream eventSs;
            eventSs << eventUpUnSub;
            std::string data(eventSs.GetData(), eventSs.GetSize());
            
            if(IsForkNodeOfSuperNode())
            {
                CMvDbpVirtualPeerNetEvent vpeerEvent;
                vpeerEvent.type = CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_UNSUBSCRIBE;
                vpeerEvent.data = std::vector<uint8>(data.begin(), data.end());
                SendEventToParentNode(vpeerEvent);
            }

            if(IsRootNodeOfSuperNode())
            {
                eventUpUnSub.flow = "up";
                eventUpUnSub.sender = "dbpservice";
                pVirtualPeerNet->DispatchEvent(&eventUpUnSub);
            }
        }
    }

    if(type == CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_GETBLOCKS)
    {
        CMvEventPeerGetBlocks eventGetBlocks(0,uint256());
        ss >> eventGetBlocks;


        if(IsRootNodeOfSuperNode())
        {
            eventGetBlocks.flow = "up";
            eventGetBlocks.sender = "dbpservice";
            pVirtualPeerNet->DispatchEvent(&eventGetBlocks);
        }

        if(IsForkNodeOfSuperNode())
        {
            if(IsMyFork(eventGetBlocks.hashFork)
                && eventGetBlocks.nNonce == std::numeric_limits<uint64>::max())
            {
                eventGetBlocks.flow = "up";
                eventGetBlocks.sender = "dbpservice";
                pVirtualPeerNet->DispatchEvent(&eventGetBlocks);
            }
            else
            {
                CMvDbpVirtualPeerNetEvent vpeerEvent;
                vpeerEvent.type = CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_GETBLOCKS;
                vpeerEvent.data = std::vector<uint8>(eventData.begin(), eventData.end());
                SendEventToParentNode(vpeerEvent);
            }
        }
    }

    if(type == CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_GETDATA)
    {
        CMvEventPeerGetData eventGetData(0,uint256());
        ss >> eventGetData;

        if(IsRootNodeOfSuperNode())
        {
            //std::cout << "#############[rootnode] "  << "GetData Begin [dbpservice]" << std::endl;
            //std::cout << "[rootnode] Get Data fork "  << eventGetData.hashFork.ToString() << std::endl;
            for(const auto& inv : eventGetData.data)
            {
              //std::cout << "Get Data Inv Hash " << inv.nHash.ToString() << std::endl;
            }
            //std::cout << "#############[rootnode] "  << "GetData End [dbpservice]" << std::endl;
               
            eventGetData.flow = "up";
            eventGetData.sender = "dbpservice";
            pVirtualPeerNet->DispatchEvent(&eventGetData);
        }

        if(IsForkNodeOfSuperNode())
        {
            if(IsMyFork(eventGetData.hashFork)
                && eventGetData.nNonce == std::numeric_limits<uint64>::max())
            {
                eventGetData.flow = "up";
                eventGetData.sender = "dbpservice";
                pVirtualPeerNet->DispatchEvent(&eventGetData);
            }
            else
            {
                CMvDbpVirtualPeerNetEvent vpeerEvent;
                vpeerEvent.type = CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_GETDATA;
                vpeerEvent.data = std::vector<uint8>(eventData.begin(), eventData.end());
                SendEventToParentNode(vpeerEvent);
            }

        }
    }

    if(type == CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_INV)
    {
        if(IsRootNodeOfSuperNode())
        {
            CMvEventPeerInv eventInv(0,uint256());
            ss >> eventInv;
            eventInv.sender = "dbpservice";
            eventInv.flow = "up";
            //std::cout << "from down to up Peer Inv " <<  eventInv.hashFork.ToString() << " [rootnode dbpservice]" << std::endl;
            pVirtualPeerNet->DispatchEvent(&eventInv);
        }
        
        if(IsForkNodeOfSuperNode())
        {
            CMvDbpVirtualPeerNetEvent vpeerEvent;
            vpeerEvent.type = CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_INV;
            vpeerEvent.data = std::vector<uint8>(eventData.begin(), eventData.end());
            SendEventToParentNode(vpeerEvent);
        }
    }

    if(type == CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_TX)
    {
        if(IsRootNodeOfSuperNode())
        {
            CMvEventPeerTx eventTx(0,uint256());
            ss >> eventTx;
            eventTx.sender = "dbpservice";
            eventTx.flow = "up";
            pVirtualPeerNet->DispatchEvent(&eventTx);
        }
        
        if(IsForkNodeOfSuperNode())
        {
            CMvDbpVirtualPeerNetEvent vpeerEvent;
            vpeerEvent.type = CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_TX;
            vpeerEvent.data = std::vector<uint8>(eventData.begin(), eventData.end());
            SendEventToParentNode(vpeerEvent);
        }
    }

    if(type == CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_BLOCK)
    {
        if(IsRootNodeOfSuperNode())
        {
            CMvEventPeerBlock eventBlock(0,uint256());
            ss >> eventBlock;

            std::cout << "#######from down to up Block: nonce " << std::hex << 
                eventBlock.nNonce << std::endl;

            eventBlock.sender = "dbpservice";
            eventBlock.flow = "up";
            pVirtualPeerNet->DispatchEvent(&eventBlock);
        }
        
        if(IsForkNodeOfSuperNode())
        {
            CMvDbpVirtualPeerNetEvent vpeerEvent;
            vpeerEvent.type = CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_BLOCK;
            vpeerEvent.data = std::vector<uint8>(eventData.begin(), eventData.end());
            SendEventToParentNode(vpeerEvent);
        }
    }
    
}

bool CDbpService::HandleEvent(CMvEventDbpMethod& event)
{
    if (event.data.method == CMvDbpMethod::LwsMethod::GET_BLOCKS)
    {
        HandleGetBlocks(event);
    }
    else if (event.data.method == CMvDbpMethod::LwsMethod::GET_TRANSACTION)
    {
        HandleGetTransaction(event);
    }
    else if (event.data.method == CMvDbpMethod::LwsMethod::SEND_TRANSACTION)
    {
        HandleSendTransaction(event);
    }
    else if(event.data.method == CMvDbpMethod::SnMethod::SEND_EVENT)
    {
        HandleSendEvent(event);
    }
    else if (event.data.method == CMvDbpMethod::SnMethod::RPC_ROUTE)
    {
        HandleRPCRoute(event);
    }
    else
    {
        return false;
    }

    return true;
}

void CDbpService::PushBlock(const std::string& forkid, const CMvDbpBlock& block)
{
    const auto& allBlockIds = mapTopicIds[ALL_BLOCK_TOPIC];
    for(const auto& id : allBlockIds)
    {
        auto it = mapIdSubedSession.find(id);
        if(it != mapIdSubedSession.end())
        {
            CMvEventDbpAdded eventAdded(it->second);
            eventAdded.data.id = id;
            eventAdded.data.forkid = forkid;
            eventAdded.data.name = ALL_BLOCK_TOPIC;
            eventAdded.data.anyAddedObj = block;
            pDbpServer->DispatchEvent(&eventAdded);
        }
    }
}

void CDbpService::PushTx(const std::string& forkid, const CMvDbpTransaction& dbptx)
{
    const auto& allTxIds = mapTopicIds[ALL_TX_TOPIC];
    for(const auto& id : allTxIds)
    {
        auto it = mapIdSubedSession.find(id);
        if(it != mapIdSubedSession.end())
        {
            CMvEventDbpAdded eventAdded(it->second);
            eventAdded.data.id = id;
            eventAdded.data.forkid = forkid;
            eventAdded.data.name = ALL_TX_TOPIC;
            eventAdded.data.anyAddedObj = dbptx;
            pDbpServer->DispatchEvent(&eventAdded);
        }
    }
}

bool CDbpService::PushEvent(const CMvDbpVirtualPeerNetEvent& event)
{
    std::string session;
    CMvEventDbpAdded eventAdd(session);
    eventAdd.data.name = "event";
    eventAdd.data.anyAddedObj = event;
    return pDbpServer->DispatchEvent(&eventAdd);
}

void CDbpService::RespondFailed(CMvEventDbpConnect& event)
{
    std::vector<int> versions{1};
    CMvEventDbpFailed eventFailed(event.strSessionId);
    eventFailed.data.reason = "001";
    eventFailed.data.versions = versions;
    eventFailed.data.session = event.data.session;
    pDbpServer->DispatchEvent(&eventFailed);
}

void CDbpService::RespondConnected(CMvEventDbpConnect& event)
{
    CMvEventDbpConnected eventConnected(event.strSessionId);
    eventConnected.data.session = event.data.session;
    pDbpServer->DispatchEvent(&eventConnected);
}

void CDbpService::RespondNoSub(CMvEventDbpSub& event)
{
    CMvEventDbpNoSub eventNoSub(event.strSessionId);
    eventNoSub.data.id = event.data.id;
    pDbpServer->DispatchEvent(&eventNoSub);
}

void CDbpService::RespondReady(CMvEventDbpSub& event)
{
    CMvEventDbpReady eventReady(event.strSessionId);
    eventReady.data.id = event.data.id;
    pDbpServer->DispatchEvent(&eventReady);
}

bool CDbpService::HandleEvent(CMvEventDbpUpdateNewBlock& event)
{
    // get details about new block
    CBlockEx& newBlock = event.data;
    uint256 forkHash;
    int32 blockHeight = 0;
    if (pService->GetBlockLocation(newBlock.GetHash(),forkHash,blockHeight))
    {
        CMvDbpBlock block;
        CDbpUtils::RawToDbpBlock(newBlock, forkHash, blockHeight, block);
        PushBlock(forkHash.ToString(),block);
    }

    return true;
}

bool CDbpService::HandleEvent(CMvEventDbpUpdateNewTx& event)
{
    decltype(event.data)& newtx = event.data;
    uint256& hashFork = event.hashFork;
    int64& change = event.nChange;

    CMvDbpTransaction dbpTx;
    CDbpUtils::RawToDbpTransaction(newtx, hashFork, change, dbpTx);
    PushTx(hashFork.ToString(),dbpTx);

    return true;
}

// from virtual peernet
bool CDbpService::HandleEvent(CMvEventPeerActive& event)
{
    if(IsRootNodeOfSuperNode())
    {
        CWalleveBufStream ss;
        ss << event;
        std::string data(ss.GetData(), ss.GetSize());
        
        CMvDbpVirtualPeerNetEvent eventVPeer;
        eventVPeer.nNonce = event.nNonce;
        eventVPeer.type = CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_ACTIVE;
        eventVPeer.data = std::vector<uint8>(data.begin(), data.end());
       
        mapPeerEventActive[event.nNonce] = eventVPeer;
        PushEvent(eventVPeer);
    }

    return true;
}

bool CDbpService::HandleEvent(CMvEventPeerDeactive& event)
{
    if(IsRootNodeOfSuperNode())
    {
        CWalleveBufStream ss;
        ss << event;
        std::string data(ss.GetData(), ss.GetSize());
        
        CMvDbpVirtualPeerNetEvent eventVPeer;
        eventVPeer.nNonce = event.nNonce;
        eventVPeer.type = CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_DEACTIVE;
        eventVPeer.data = std::vector<uint8>(data.begin(), data.end());
        
        mapPeerEventActive.erase(event.nNonce);
        PushEvent(eventVPeer);
    }
    
    return true;
}

void CDbpService::FilterThisSubscribeFork(const CMvEventPeerSubscribe& in, CMvEventPeerSubscribe& out)
{
    auto& vSubForks = in.data;
    for(const auto& fork : vSubForks)
    {
        auto key = ForkNonceKeyType(fork, in.nNonce);
        
        if(mapThisNodeForkCount.find(key) == mapThisNodeForkCount.end())
        {
            mapThisNodeForkCount[key] = 1;
            out.data.push_back(fork);
        }
        else
        {
            mapThisNodeForkCount[key]++;
        }

        std::cout << "[forknode] Subscribe fork " << fork.ToString() << " [dbpservice]" << std::endl;
    }
}

void CDbpService::FilterThisUnsubscribeFork(const CMvEventPeerUnsubscribe& in, CMvEventPeerUnsubscribe& out)
{
    auto& vUnSubForks = in.data;
    for(const auto& fork : vUnSubForks)
    {
        auto key = ForkNonceKeyType(fork, in.nNonce);
        
        if(mapThisNodeForkCount.find(key) != mapThisNodeForkCount.end())
        {
            if(mapThisNodeForkCount[key] == 1)
            {
                mapThisNodeForkCount[key] = 0;
                out.data.push_back(fork);
                mapThisNodeForkCount.erase(key);
            }
            else
            {
                mapThisNodeForkCount[key]--;
            }
        }

        std::cout << "UnSub Fork: " << fork.ToString() << " [dbpservice] " << std::endl;
    }
}

bool CDbpService::HandleEvent(CMvEventPeerSubscribe& event)
{
    
    
    if(IsRootNodeOfSuperNode())
    {   
        CWalleveBufStream ss;
        ss << event;
        std::string data(ss.GetData(), ss.GetSize());
        
        CMvDbpVirtualPeerNetEvent eventVPeer;
        eventVPeer.type = CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_SUBSCRIBE;
        eventVPeer.hashFork = event.hashFork;
        eventVPeer.data = std::vector<uint8>(data.begin(), data.end());
        PushEvent(eventVPeer);
    }

    if(IsForkNodeOfSuperNode())
    {
        std::cout << "###################[forknode] generate subscribe event [dbpservice]" << std::endl;
        
        CMvEventPeerSubscribe eventUpSub(event.nNonce, event.hashFork);


        std::cout << "nonce " << std::hex <<  event.nNonce << " [dbpservice] " << std::endl;
        std::cout << "hashfork " << event.hashFork.ToString() << " [dbpservice]" << std::endl;

        FilterThisSubscribeFork(event, eventUpSub);

        std::cout << "eventUpSubEmpty " << (eventUpSub.data.empty() ? "true" : "false") << std::endl;

        if(!eventUpSub.data.empty())
        {
            CWalleveBufStream UpSubSs;
            UpSubSs << eventUpSub;
            std::string data(UpSubSs.GetData(), UpSubSs.GetSize());
            CMvDbpVirtualPeerNetEvent eventVPeer;
            eventVPeer.type = CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_SUBSCRIBE;
            eventVPeer.data = std::vector<uint8>(data.begin(), data.end());
            SendEventToParentNode(eventVPeer);
        }
    }
    
    return true;
}

bool CDbpService::HandleEvent(CMvEventPeerUnsubscribe& event)
{
    if(IsRootNodeOfSuperNode())
    {
        CWalleveBufStream ss;
        ss << event;
        std::string data(ss.GetData(), ss.GetSize());
        
        CMvDbpVirtualPeerNetEvent eventVPeer;
        eventVPeer.type = CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_UNSUBSCRIBE;
        eventVPeer.hashFork = event.hashFork;
        eventVPeer.data = std::vector<uint8>(data.begin(), data.end());
        PushEvent(eventVPeer);
    }

    if(IsForkNodeOfSuperNode())
    {
        std::cout << "[forknode] generate unSubscribe event [dbpservice]" << std::endl;
        
        CMvEventPeerUnsubscribe eventUpUnSub(event.nNonce, event.hashFork);


        FilterThisUnsubscribeFork(event, eventUpUnSub);


        if(!eventUpUnSub.data.empty())
        {
            CWalleveBufStream eventSs;
            eventSs << eventUpUnSub;
            std::string data(eventSs.GetData(), eventSs.GetSize());
            
            CMvDbpVirtualPeerNetEvent vpeerEvent;
            vpeerEvent.type = CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_UNSUBSCRIBE;
            vpeerEvent.data = std::vector<uint8>(data.begin(), data.end());
            SendEventToParentNode(vpeerEvent); 
        }
    }
    
    return true;
}

bool CDbpService::HandleEvent(CMvEventPeerInv& event)
{
    CWalleveBufStream ss;
    ss << event;
    std::string data(ss.GetData(), ss.GetSize());
        
    CMvDbpVirtualPeerNetEvent eventVPeer;
    eventVPeer.type = CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_INV;
    eventVPeer.data = std::vector<uint8>(data.begin(), data.end());
    
    if(IsRootNodeOfSuperNode())
    {
        std::cout << "[rootnode] Generated PeerInv Nonce" << event.nNonce << " fork " 
            << event.hashFork.ToString() << 
            " [rootnode dbpservice]" << std::endl;
        for(const auto& inv : event.data)
        {
            std::cout << "[rootnode] Generated inv hash " << inv.nHash.ToString() << std::endl;
        }
        PushEvent(eventVPeer);
    }

    if(IsForkNodeOfSuperNode())
    {
        if(event.nNonce == std::numeric_limits<uint64>::max())
        {
            PushEvent(eventVPeer);
        }
        else
        {
            std::cout << "[forknode] Generated PeerInv Nonce" << event.nNonce << " fork " 
             << event.hashFork.ToString() << " [forknode dbpservice]" << std::endl;
            
            SendEventToParentNode(eventVPeer);
        }
    }
    
    return true;
}

bool CDbpService::HandleEvent(CMvEventPeerBlock& event)
{
    CWalleveBufStream ss;
    ss << event;
    std::string data(ss.GetData(), ss.GetSize());
        
    CMvDbpVirtualPeerNetEvent eventVPeer;
    eventVPeer.type = CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_BLOCK;
    eventVPeer.data = std::vector<uint8>(data.begin(), data.end());

    if(IsRootNodeOfSuperNode())
    {
        PushEvent(eventVPeer);
    }

    if(IsForkNodeOfSuperNode())
    {
        if(event.nNonce == std::numeric_limits<uint64>::max())
        {
            PushEvent(eventVPeer);
        }
        else
        {
            SendEventToParentNode(eventVPeer);
        }
    }
    
    
    return true;
}

bool CDbpService::HandleEvent(CMvEventPeerTx& event)
{
    CWalleveBufStream ss;
    ss << event;
    std::string data(ss.GetData(), ss.GetSize());
        
    CMvDbpVirtualPeerNetEvent eventVPeer;
    eventVPeer.type = CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_TX;
    eventVPeer.data = std::vector<uint8>(data.begin(), data.end());

    if(IsRootNodeOfSuperNode())
    {
        PushEvent(eventVPeer);
    }

    if(IsForkNodeOfSuperNode())
    {
        if(event.nNonce == std::numeric_limits<uint64>::max())
        {
            PushEvent(eventVPeer);
        }
        else
        {
            SendEventToParentNode(eventVPeer);
        }
    }
    
    return true;
}

bool CDbpService::HandleEvent(CMvEventPeerGetBlocks& event)
{
    CWalleveBufStream ss;
    ss << event;
    std::string data(ss.GetData(), ss.GetSize());
        
    CMvDbpVirtualPeerNetEvent eventVPeer;
    eventVPeer.type = CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_GETBLOCKS;
    eventVPeer.data = std::vector<uint8>(data.begin(), data.end());
    
    if(IsRootNodeOfSuperNode())
    {
        PushEvent(eventVPeer);
    }

    if(IsForkNodeOfSuperNode())
    {
       SendEventToParentNode(eventVPeer);
    }

    return true;
}

bool CDbpService::HandleEvent(CMvEventPeerGetData& event)
{
    CWalleveBufStream ss;
    ss << event;
    std::string data(ss.GetData(), ss.GetSize());
        
    CMvDbpVirtualPeerNetEvent eventVPeer;
    eventVPeer.type = CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_GETDATA;
    eventVPeer.data = std::vector<uint8>(data.begin(), data.end());
    
    if(IsRootNodeOfSuperNode())
    {
        PushEvent(eventVPeer);
    }

    if(IsForkNodeOfSuperNode())
    {
        UpdateGetDataEventRecord(event);
        SendEventToParentNode(eventVPeer);
    }
    
    return true;
}

bool CDbpService::HandleEvent(CWalleveEventPeerNetReward& event)
{
    CWalleveSuperNodeEventPeerNetReward eventSNReward(event.nNonce);
    eventSNReward.result = event.result;
    eventSNReward.nType = event.nType;
    eventSNReward.data = event.data;
    
    CWalleveBufStream ss;
    ss << eventSNReward;
    std::string data(ss.GetData(), ss.GetSize());
        
    CMvDbpVirtualPeerNetEvent eventVPeer;
    eventVPeer.type = CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_REWARD;
    eventVPeer.data = std::vector<uint8>(data.begin(), data.end());

    if(IsForkNodeOfSuperNode())
    {
        SendEventToParentNode(eventVPeer);
    }
    
    return true;
}
    
bool CDbpService::HandleEvent(CWalleveEventPeerNetClose& event)
{
    CWalleveSuperNodeEventPeerNetClose eventSNNetClose(event.nNonce);
    eventSNNetClose.result = event.result;
    eventSNNetClose.nType = event.nType;
    eventSNNetClose.data = event.data;
    
    CWalleveBufStream ss;
    ss << eventSNNetClose;
    std::string data(ss.GetData(), ss.GetSize());
        
    CMvDbpVirtualPeerNetEvent eventVPeer;
    eventVPeer.type = CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_CLOSE;
    eventVPeer.data = std::vector<uint8>(data.begin(), data.end());

    if(IsForkNodeOfSuperNode())
    {
        std::cout << "[forknode] generate netclose nonce: " << event.nNonce << " [dbpservice]" << std::endl;
        std::cout << "[forknode] generate netclose type: " << event.data << " [dbpservice]" << std::endl;
        SendEventToParentNode(eventVPeer);
    }
    
    return true;
}

//Event from up to down
bool CDbpService::HandleEvent(CMvEventDbpVirtualPeerNet& event)
{
    CWalleveBufStream ss;
    decltype(event.data.data) bytes = event.data.data;
    ss.Write((char*)bytes.data(), bytes.size());

    // process and classify and dispatch to vpeernet
    if(event.data.type == CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_ACTIVE)
    {
        CMvEventPeerActive eventActive(0);
        ss >> eventActive;  
        
        boost::asio::ip::tcp::endpoint ep;
        eventActive.data.ssEndpoint.GetEndpoint(ep);
        std::cout << "recv active event address: " << ep.address().to_string() << " [dbpservice]" << std::endl;
        std::cout << "recv active event nonce: " << std::hex << eventActive.nNonce << "[dbpservice]" << std::endl;

        pVirtualPeerNet->DispatchEvent(&eventActive);

        mapPeerEventActive[eventActive.nNonce] = event.data;
        PushEvent(event.data);
    }

    if(event.data.type == CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_DEACTIVE)
    {
        CMvEventPeerDeactive eventDeactive(0);
        ss >> eventDeactive;   
        
        boost::asio::ip::tcp::endpoint ep;
        eventDeactive.data.ssEndpoint.GetEndpoint(ep);
        std::cout << "recv deactive event address: " << ep.address().to_string() << " [dbpservice]" << std::endl;
        
        pVirtualPeerNet->DispatchEvent(&eventDeactive);

        mapPeerEventActive.erase(eventDeactive.nNonce);
        PushEvent(event.data);
    }

    if(event.data.type == CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_SUBSCRIBE)
    {
        CMvEventPeerSubscribe eventSub(0, uint256());
        ss >> eventSub;

        std::cout << "####### from up to down SUBSCRIBE :" << std::endl;
        for(const auto& fork : eventSub.data)
        {
            std::cout << "Subscribed fork " << fork.ToString() << std::endl;
        }
        if(IsMyFork(eventSub.hashFork))
        {
            eventSub.sender = "dbpservice";
            eventSub.flow = "down";    
            pVirtualPeerNet->DispatchEvent(&eventSub);
        }
        else
        {
            PushEvent(event.data);
        }
    }

    if(event.data.type == CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_UNSUBSCRIBE)
    {
        CMvEventPeerUnsubscribe eventUnSub(0, uint256());
        ss >> eventUnSub;
        
        if(IsMyFork(eventUnSub.hashFork))
        {
            eventUnSub.flow = "down";
            eventUnSub.sender = "dbpservice";
            pVirtualPeerNet->DispatchEvent(&eventUnSub);
        }
        else
        {
            PushEvent(event.data);
        }
    }

    if(event.data.type == CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_GETBLOCKS)
    {
        CMvEventPeerGetBlocks eventGetBlocks(0, uint256());
        ss >> eventGetBlocks;

        std::cout << "####### from up to down GETBLOCKS " << 
            eventGetBlocks.hashFork.ToString() << std::endl;
        if(IsMyFork(eventGetBlocks.hashFork))
        {
            eventGetBlocks.sender = "dbpservice";
            eventGetBlocks.flow = "down";
            pVirtualPeerNet->DispatchEvent(&eventGetBlocks);
        }
        else
        {
            PushEvent(event.data);
        }
    }

    if(event.data.type == CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_GETDATA)
    {
        CMvEventPeerGetData eventGetData(0, uint256());
        ss >> eventGetData; 

        std::cout << "####### from up to down GETDATA " << 
            eventGetData.hashFork.ToString() << std::endl;
        if(IsMyFork(eventGetData.hashFork))
        {
            eventGetData.sender = "dbpservice";
            eventGetData.flow = "down";
            pVirtualPeerNet->DispatchEvent(&eventGetData);
        }
        else
        {
            PushEvent(event.data);
        }
    }

    if(event.data.type == CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_INV)
    {
        CMvEventPeerInv eventInv(0, uint256());
        ss >> eventInv;

        if(IsMyFork(eventInv.hashFork))
        {
            //std::cout << "[forknode] [<] Peer Inv Fork " << eventInv.hashFork.ToString() << " [dbpservice]" << std::endl; 
            //std::cout << "[forknode] [<] Peer Inv Nonce " << eventInv.nNonce << " [dbpservice]" << std::endl;
            
            for(const auto& inv : eventInv.data)
            {
               //std::cout << "[forknode] [<] inv hash " << inv.nHash.ToString() << std::endl;
            }
            eventInv.sender = "dbpservice";
            eventInv.flow = "down";
            pVirtualPeerNet->DispatchEvent(&eventInv);
        }

        PushEvent(event.data);
    }

    if(event.data.type == CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_TX)
    {
        CMvEventPeerTx eventTx(0, uint256());
        ss >> eventTx;

        //std::cout << "[forknode] From up to down:  Peer Tx Hash " << eventTx.data.GetHash().ToString() << " [dbpservice]" << std::endl; 
        
        if(IsMyFork(eventTx.hashFork) && 
            IsThisNodeData(eventTx.hashFork, eventTx.nNonce, eventTx.data.GetHash()))
        {
            //std::cout << "[forknode] [<] Peer Tx Fork " << eventTx.hashFork.ToString() << " [dbpservice]" << std::endl; 
            //std::cout << "[forknode] [<] Peer Tx Nonce " << eventTx.nNonce << " [dbpservice]" << std::endl;
            //std::cout << "[forknode] [<] Peer Tx Hash " << eventTx.data.GetHash().ToString() << " [dbpservice]" << std::endl; 
            
            eventTx.sender = "dbpservice";
            eventTx.flow = "down";
            pVirtualPeerNet->DispatchEvent(&eventTx);
        }

        PushEvent(event.data);
    }

    if(event.data.type == CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_BLOCK)
    {
        CMvEventPeerBlock eventBlock(0, uint256());
        ss >> eventBlock;

        //std::cout << "[forknode] From up to down:  Peer Block Hash " << eventBlock.data.GetHash().ToString() << " [dbpservice]" << std::endl; 
        
        if(IsMyFork(eventBlock.hashFork) && 
            IsThisNodeData(eventBlock.hashFork, eventBlock.nNonce, eventBlock.data.GetHash()))
        {
            //std::cout << "[forknode] [<] Peer Block Fork " << eventBlock.hashFork.ToString() << " [dbpservice]" << std::endl; 
            //std::cout << "[forknode] [<] Peer Block Nonce " << eventBlock.nNonce << " [dbpservice]" << std::endl;
            //std::cout << "[forknode] [<] Peer Block Hask " << eventBlock.data.GetHash().ToString() << " [dbpservice]" << std::endl; 
            
            eventBlock.sender = "dbpservice";
            eventBlock.flow = "down";
            pVirtualPeerNet->DispatchEvent(&eventBlock);
        }

        PushEvent(event.data);
    }

    return true;
}

void CDbpService::SendEventToParentNode(CMvDbpVirtualPeerNetEvent& event)
{
    CMvEventDbpVirtualPeerNet eventVirtualPeerNet("");
    eventVirtualPeerNet.data.type = event.type;
    eventVirtualPeerNet.data.data = event.data;
    pDbpClient->DispatchEvent(&eventVirtualPeerNet);
}

void CDbpService::UpdateGetDataEventRecord(const CMvEventPeerGetData& event)
{
    uint64 nNonce = event.nNonce;
    const uint256& hashFork = event.hashFork;
    
    std::set<uint256> setInvHash;
    std::for_each(event.data.begin(), event.data.end(), [&](const CInv& inv) {
        setInvHash.insert(inv.nHash);
        //std::cout << "Get Data Inv Hash " << inv.nHash.ToString() << " [dbpservice]" << std::endl;
    });

    //std::cout << "Get Data nonce " << nNonce << " [dbpservice]" << std::endl;
    //std::cout << "Get Data hashFork " << hashFork.ToString() << " [dbpservice]" << std::endl;

    mapThisNodeGetData[std::make_pair(hashFork, nNonce)] = setInvHash;
}

bool CDbpService::IsThisNodeData(const uint256& hashFork, uint64 nNonce, const uint256& dataHash)
{
    auto pairKey = std::make_pair(hashFork, nNonce);

    if(mapThisNodeGetData.find(pairKey) == mapThisNodeGetData.end())
    {
        return false;
    }

    auto& setInvHash = mapThisNodeGetData[pairKey];
    if(setInvHash.find(dataHash) == setInvHash.end())
    {
        return false;
    }

    setInvHash.erase(dataHash);

    return true;
}

//rpc route

void CDbpService::RPCRootHandle(CMvRPCRouteStop* data, CMvRPCRouteStopRet* ret)
{
    if (GetSessionCountByNonce(data->nNonce) == 0)
    {
        CMvRPCRouteStopRet stopRet;
        CompletionByNonce(data->nNonce, stopRet);
        DeleteSessionCountByNonce(data->nNonce);
        pService->Shutdown();
    }
}

void CDbpService::TransformForks(std::vector<std::pair<uint256, CProfile>>& vFork, std::vector<CMvRPCProfile>& vRpcFork)
{
    for (auto& fork : vFork)
    {
        CMvRPCProfile profile;
        profile.strHex = fork.first.GetHex();
        profile.strName = fork.second.strName;
        profile.strSymbol = fork.second.strSymbol;
        profile.fIsolated = fork.second.IsIsolated();
        profile.fPrivate = fork.second.IsPrivate();
        profile.fEnclosed = fork.second.IsEnclosed();
        profile.address = CMvAddress(fork.second.destOwner).ToString();
        vRpcFork.push_back(profile);
    }
}

void CDbpService::ListForkUnique(std::vector<CMvRPCProfile>& vFork)
{
    std::sort(vFork.begin(), vFork.end(), [](const CMvRPCProfile& i, const CMvRPCProfile& j) { return i.strHex > j.strHex; });
    auto it = std::unique(vFork.begin(), vFork.end(), [](const CMvRPCProfile& i, const CMvRPCProfile& j) { return i.strHex == j.strHex; });
    vFork.resize(std::distance(vFork.begin(), it));
}

void CDbpService::RPCRootHandle(CMvRPCRouteListFork* data, CMvRPCRouteListForkRet* ret)
{
    if(ret == NULL)
    {
        if (GetSessionCountByNonce(data->nNonce) == 0)
        {
            CMvRPCRouteListForkRet ret;
            std::vector<std::pair<uint256, CProfile>> vFork;
            pService->ListFork(vFork, data->fAll);
            std::vector<CMvRPCProfile> vRpcFork;
            TransformForks(vFork, vRpcFork);
            ret.vFork.insert(ret.vFork.end(), vRpcFork.begin(), vRpcFork.end());
            CompletionByNonce(data->nNonce, ret);
            DeleteSessionCountByNonce(data->nNonce);
            DeleteQueCount(data->nNonce);
        }
        return;
    }

    if(ret != NULL)
    {
        CMvRPCRouteListForkRet listForkRetIn = *((CMvRPCRouteListForkRet*)ret);
        for(auto &i : listForkRetIn.vFork)
        {
            std::cout << "0-.hex:" << i.strHex << std::endl;
        }
        std::vector<CMvRPCProfile> vTempFork;
        uint64 nNonce = listForkRetIn.nNonce;
        auto compare = [nNonce](const std::pair<uint64, boost::any>& element) { return nNonce == element.first; };
        auto iter = std::find_if(queCount.begin(), queCount.end(), compare);
        if (iter != queCount.end() && (iter->second).type() == typeid(CMvRPCRouteListForkRet))
        {
            auto& vForkSelf = boost::any_cast<CMvRPCRouteListForkRet&>(iter->second).vFork;
            std::cout << "1.vTempFork size:" << vTempFork.size() << ", vForkSelf size:" << vForkSelf.size() << std::endl;
            vForkSelf.insert(vForkSelf.end(), listForkRetIn.vFork.begin(), listForkRetIn.vFork.end());
            ListForkUnique(vForkSelf);
            vTempFork.insert(vTempFork.end(), vForkSelf.begin(), vForkSelf.end());
            std::cout << "2.vTempFork size:" << vTempFork.size() << ", vForkSelf size:" << vForkSelf.size() << std::endl;
            for(auto &i : vForkSelf)
            {
                std::cout << "2-.hex:" << i.strHex << std::endl;
            }
        }

        if (GetSessionCountByNonce(data->nNonce) == 0)
        {
            CMvRPCRouteListForkRet retOut;
            std::vector<std::pair<uint256, CProfile>> vFork;
            pService->ListFork(vFork, data->fAll);
            std::vector<CMvRPCProfile> vRpcFork;
            TransformForks(vFork, vRpcFork);
            retOut.vFork.insert(retOut.vFork.end(), vTempFork.begin(), vTempFork.end());
            retOut.vFork.insert(retOut.vFork.end(), vRpcFork.begin(), vRpcFork.end());
            ListForkUnique(retOut.vFork);
            std::cout << "3.retOut.vFork size:" << retOut.vFork.size() << ", vTempFork size:" << vTempFork.size() << ", vRpcFork size:" << vRpcFork.size() << std::endl;
            for(auto &i : retOut.vFork)
            {
                std::cout << "4.hex:" << i.strHex << std::endl;
            }
            CompletionByNonce(data->nNonce, retOut);
            DeleteSessionCountByNonce(data->nNonce);
            DeleteQueCount(data->nNonce);
        }
        return;
    }
}

void CDbpService::RPCRootHandle(CMvRPCRouteGetBlockLocation* data, CMvRPCRouteGetBlockLocationRet* ret)
{
    if (ret == NULL)
    {
        if (GetSessionCountByNonce(data->nNonce) == 0)
        {
            CMvRPCRouteGetBlockLocationRet getBlockLocationRet;
            getBlockLocationRet.nNonce = data->nNonce;
            getBlockLocationRet.type = data->type;
            getBlockLocationRet.strFork = "";
            getBlockLocationRet.height = 0;
            CompletionByNonce(data->nNonce, getBlockLocationRet);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) != 0)
        {
            auto vRawData = RPCRouteRetToStream(*data);
            PushMsgToChild(vRawData, data->type, data->nNonce);
            return;
        }
    }

    if (ret != NULL)
    {
        if(!ret->strFork.empty())
        {
            CMvRPCRouteGetBlockLocationRet getBlockLocationRet;
            getBlockLocationRet.nNonce = data->nNonce;
            getBlockLocationRet.type = data->type;
            getBlockLocationRet.strFork = ret->strFork;
            getBlockLocationRet.height = ret->height;
            CompletionByNonce(data->nNonce, getBlockLocationRet);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) == 0)
        {
            CMvRPCRouteGetBlockLocationRet getBlockLocationRet;
            getBlockLocationRet.nNonce = data->nNonce;
            getBlockLocationRet.type = data->type;
            getBlockLocationRet.strFork = "";
            getBlockLocationRet.height = 0;
            CompletionByNonce(data->nNonce, getBlockLocationRet);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) != 0)
        {
            auto vRawData = RPCRouteRetToStream(*data);
            PushMsgToChild(vRawData, data->type, data->nNonce);
            return;
        }
    }
}

void CDbpService::RPCRootHandle(CMvRPCRouteGetBlockCount* data, CMvRPCRouteGetBlockCountRet* ret)
{
    if(ret == NULL)
    {
        if (GetSessionCountByNonce(data->nNonce) == 0)
        {
            CMvRPCRouteGetBlockCountRet getBlockCountRet;
            getBlockCountRet.nNonce = data->nNonce;
            getBlockCountRet.type = data->type;
            getBlockCountRet.exception = 2;
            getBlockCountRet.height = 0;
            CompletionByNonce(data->nNonce, getBlockCountRet);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) != 0)
        {
            auto vRawData = RPCRouteRetToStream(*data);
            PushMsgToChild(vRawData, data->type, data->nNonce);
            return;
        }
    }

    if(ret != NULL)
    {
        if(ret->exception == 0)
        {
            CMvRPCRouteGetBlockCountRet getBlockCountRet;
            getBlockCountRet.nNonce = data->nNonce;
            getBlockCountRet.type = data->type;
            getBlockCountRet.exception = ret->exception;
            getBlockCountRet.height = ret->height;
            CompletionByNonce(data->nNonce, getBlockCountRet);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) == 0)
        {
            CMvRPCRouteGetBlockCountRet getBlockCountRet;
            getBlockCountRet.nNonce = data->nNonce;
            getBlockCountRet.type = data->type;
            getBlockCountRet.exception = 2;
            getBlockCountRet.height = 0;
            CompletionByNonce(data->nNonce, getBlockCountRet);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) != 0)
        {
            auto vRawData = RPCRouteRetToStream(*data);
            PushMsgToChild(vRawData, data->type, data->nNonce);
            return;
        }
    }
}

void CDbpService::RPCRootHandle(CMvRPCRouteGetBlockHash * data, CMvRPCRouteGetBlockHashRet* ret)
{
    if(ret == NULL)
    {
        if (GetSessionCountByNonce(data->nNonce) == 0)
        {
            CMvRPCRouteGetBlockHashRet getBlockHashRet;
            getBlockHashRet.nNonce = data->nNonce;
            getBlockHashRet.type = data->type;
            getBlockHashRet.exception = 2;
            CompletionByNonce(data->nNonce, getBlockHashRet);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) != 0)
        {
            auto vRawData = RPCRouteRetToStream(*data);
            PushMsgToChild(vRawData, data->type, data->nNonce);
            return;
        }
    }

    if(ret != NULL)
    {
        if(ret->exception == 0 || ret->exception == 3)
        {
            CMvRPCRouteGetBlockHashRet getBlockHashRet;
            getBlockHashRet.nNonce = data->nNonce;
            getBlockHashRet.type = data->type;
            getBlockHashRet.exception = ret->exception;
            getBlockHashRet.vHash = ret->vHash;
            CompletionByNonce(data->nNonce, getBlockHashRet);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) == 0)
        {
            CMvRPCRouteGetBlockHashRet getBlockHashRet;
            getBlockHashRet.nNonce = data->nNonce;
            getBlockHashRet.type = data->type;
            getBlockHashRet.exception = 2;
            CompletionByNonce(data->nNonce, getBlockHashRet);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) != 0)
        {
            auto vRawData = RPCRouteRetToStream(*data);
            PushMsgToChild(vRawData, data->type, data->nNonce);
            return;
        }
    }
}

void CDbpService::RPCRootHandle(CMvRPCRouteGetBlock* data, CMvRPCRouteGetBlockRet* ret)
{
    auto vRawData = RPCRouteRetToStream(*data);

    if (ret == NULL)
    {
        if (GetSessionCountByNonce(data->nNonce) == 0)
        {
            CMvRPCRouteGetBlockRet getBlockRet;
            getBlockRet.nNonce = data->nNonce;
            getBlockRet.type = data->type;
            getBlockRet.exception = 1;
            CompletionByNonce(data->nNonce, getBlockRet);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) != 0)
        {
            PushMsgToChild(vRawData, data->type, data->nNonce);
            return;
        }
    }

    if (ret != NULL)
    {
        CMvRPCRouteGetBlockRet getBlockRet;
        getBlockRet.nNonce = data->nNonce;
        getBlockRet.type = data->type;

        if(ret->exception == 0)
        {
            getBlockRet.exception = ret->exception;
            getBlockRet.strFork = ret->strFork;
            getBlockRet.height = ret->height;
            getBlockRet.block = ret->block;
            CompletionByNonce(data->nNonce, getBlockRet);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) == 0)
        {
            getBlockRet.exception = 1;
            CompletionByNonce(data->nNonce, getBlockRet);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) != 0)
        {
            PushMsgToChild(vRawData, data->type, data->nNonce);
            return;
        }
    }
}

void CDbpService::RPCRootHandle(CMvRPCRouteGetTxPool* data, CMvRPCRouteGetTxPoolRet* ret)
{
    auto vRawData = RPCRouteRetToStream(*data);

    if(ret == NULL)
    {
        if (GetSessionCountByNonce(data->nNonce) == 0)
        {
            CMvRPCRouteGetTxPoolRet getTxPoolRet;
            getTxPoolRet.nNonce = data->nNonce;
            getTxPoolRet.type = data->type;
            getTxPoolRet.exception = 2;
            CompletionByNonce(data->nNonce, getTxPoolRet);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) != 0)
        {
            PushMsgToChild(vRawData, data->type, data->nNonce);
            return;
        }
    }

    if(ret != NULL)
    {
        CMvRPCRouteGetTxPoolRet getTxPoolRet;
        getTxPoolRet.nNonce = data->nNonce;
        getTxPoolRet.type = data->type;

        if (ret->exception == 0)
        {
            getTxPoolRet.exception = ret->exception;
            getTxPoolRet.vTxPool = ret->vTxPool;
            CompletionByNonce(data->nNonce, getTxPoolRet);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) == 0)
        {
            getTxPoolRet.exception = 2;
            CompletionByNonce(data->nNonce, getTxPoolRet);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) != 0)
        {
            PushMsgToChild(vRawData, data->type, data->nNonce);
            return;
        }
    }
}

void CDbpService::RPCRootHandle(CMvRPCRouteGetTransaction* data, CMvRPCRouteGetTransactionRet* ret)
{
    auto vRawData = RPCRouteRetToStream(*data);

    if(ret == NULL)
    {
        if (GetSessionCountByNonce(data->nNonce) == 0)
        {
            CMvRPCRouteGetTransactionRet getTransactionRet;
            getTransactionRet.nNonce = data->nNonce;
            getTransactionRet.type = data->type;
            getTransactionRet.exception = 1;
            CompletionByNonce(data->nNonce, getTransactionRet);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) != 0)
        {
            PushMsgToChild(vRawData, data->type, data->nNonce);
            return;
        }
    }

    if(ret != NULL)
    {
        CMvRPCRouteGetTransactionRet getTransactionRet;
        getTransactionRet.nNonce = data->nNonce;
        getTransactionRet.type = data->type;

        if (ret->exception == 0)
        {
            getTransactionRet.exception = ret->exception;
            getTransactionRet.tx = ret->tx;
            getTransactionRet.strFork = ret->strFork;
            getTransactionRet.nDepth = ret->nDepth;
            CompletionByNonce(data->nNonce, getTransactionRet);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) == 0)
        {
            getTransactionRet.exception = 2;
            CompletionByNonce(data->nNonce, getTransactionRet);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) != 0)
        {
            PushMsgToChild(vRawData, data->type, data->nNonce);
            return;
        }
    }
}

void CDbpService::RPCRootHandle(CMvRPCRouteGetForkHeight* data, CMvRPCRouteGetForkHeightRet* ret)
{
    auto vRawData = RPCRouteRetToStream(*data);

    if(ret == NULL)
    {
        if (GetSessionCountByNonce(data->nNonce) == 0)
        {
            CMvRPCRouteGetForkHeightRet getForkHeightRet;
            getForkHeightRet.nNonce = data->nNonce;
            getForkHeightRet.type = data->type;
            getForkHeightRet.exception = 2;
            CompletionByNonce(data->nNonce, getForkHeightRet);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) != 0)
        {
            PushMsgToChild(vRawData, data->type, data->nNonce);
            return;
        }
    }

    if(ret != NULL)
    {
        CMvRPCRouteGetForkHeightRet getForkHeightRet;
        getForkHeightRet.nNonce = data->nNonce;
        getForkHeightRet.type = data->type;

        if (ret->exception == 0)
        {
            getForkHeightRet.exception = ret->exception;
            getForkHeightRet.height = ret->height;
            CompletionByNonce(data->nNonce, getForkHeightRet);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) == 0)
        {
            getForkHeightRet.exception = 2;
            CompletionByNonce(data->nNonce, getForkHeightRet);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) != 0)
        {
            PushMsgToChild(vRawData, data->type, data->nNonce);
            return;
        }
    }
}

void CDbpService::RPCRootHandle(CMvRPCRouteSendTransaction* data, CMvRPCRouteSendTransactionRet* ret)
{
    auto vRawData = RPCRouteRetToStream(*data);

    if(ret == NULL)
    {
        if (GetSessionCountByNonce(data->nNonce) == 0)
        {
            CMvRPCRouteSendTransactionRet sendTransactionRet;
            sendTransactionRet.nNonce = data->nNonce;
            sendTransactionRet.type = data->type;
            sendTransactionRet.exception = 1;
            CompletionByNonce(data->nNonce, sendTransactionRet);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) != 0)
        {
            PushMsgToChild(vRawData, data->type, data->nNonce);
            return;
        }
    }

    if(ret != NULL)
    {
        CMvRPCRouteSendTransactionRet  sendTransactionRet;
        sendTransactionRet.nNonce = data->nNonce;
        sendTransactionRet.type = data->type;

        if (ret->exception == 0)
        {
            sendTransactionRet.exception = ret->exception;
            CompletionByNonce(data->nNonce, sendTransactionRet);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) == 0)
        {
            sendTransactionRet.exception = 1;
            sendTransactionRet.err = ret->err;
            CompletionByNonce(data->nNonce, sendTransactionRet);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) != 0)
        {
            PushMsgToChild(vRawData, data->type, data->nNonce);
            return;
        }
    }
}

void CDbpService::RPCForkHandle(CMvRPCRouteStop* data, CMvRPCRouteStopRet* ret)
{
    if (GetSessionCountByNonce(data->nNonce) == 0)
    {
        CMvRPCRouteResult result;
        result.type = CMvRPCRoute::DBP_RPCROUTE_STOP;
        result.vRawData = RPCRouteRetToStream(*data);
        SendRPCResult(result);
        DeleteSessionCountByNonce(data->nNonce);
        pService->Shutdown();
    }
}

void CDbpService::RPCForkHandle(CMvRPCRouteListFork* data, CMvRPCRouteListForkRet* ret)
{
    if(ret == NULL)
    {
        if (GetSessionCountByNonce(data->nNonce) == 0)
        {
            CMvRPCRouteResult result;
            result.type = CMvRPCRoute::DBP_RPCROUTE_LIST_FORK;
            result.vRawData = RPCRouteRetToStream(*data);

            CMvRPCRouteListForkRet retOut;
            retOut.nNonce = data->nNonce;
            retOut.type = data->type;

            std::vector<std::pair<uint256, CProfile>> vFork;
            pService->ListFork(vFork, data->fAll);
            std::vector<CMvRPCProfile> vRpcFork;
            TransformForks(vFork, vRpcFork);
            retOut.vFork.insert(retOut.vFork.end(), vRpcFork.begin(), vRpcFork.end());

            result.vData = RPCRouteRetToStream(retOut);
            SendRPCResult(result);
            DeleteSessionCountByNonce(data->nNonce);
            DeleteQueCount(data->nNonce);
        }
        return;
    }

    if(ret != NULL)
    {
        CMvRPCRouteListForkRet listForkRetIn = *ret;
        std::vector<CMvRPCProfile> vTempFork;
        uint64 nNonce = listForkRetIn.nNonce;
        auto compare = [nNonce](const std::pair<uint64, boost::any>& element) { return nNonce == element.first; };
        auto iter = std::find_if(queCount.begin(), queCount.end(), compare);
        if (iter != queCount.end() && (iter->second).type() == typeid(CMvRPCRouteListForkRet))
        {
            auto& vForkSelf = boost::any_cast<CMvRPCRouteListForkRet&>(iter->second).vFork;
            vForkSelf.insert(vForkSelf.end(), listForkRetIn.vFork.begin(), listForkRetIn.vFork.end());
            ListForkUnique(vForkSelf);
            vTempFork.insert(vTempFork.end(), vForkSelf.begin(), vForkSelf.end());
        }

        if (GetSessionCountByNonce(data->nNonce) == 0)
        {
            CMvRPCRouteResult result;
            result.type = CMvRPCRoute::DBP_RPCROUTE_LIST_FORK;
            result.vRawData = RPCRouteRetToStream(*data);

            CMvRPCRouteListForkRet listForkRetOut;
            listForkRetOut.type = listForkRetIn.type;
            listForkRetOut.nNonce = listForkRetIn.nNonce;

            std::vector<std::pair<uint256, CProfile>> vFork;
            pService->ListFork(vFork, data->fAll);
            std::vector<CMvRPCProfile> vRpcFork;
            TransformForks(vFork, vRpcFork);

            listForkRetOut.vFork.insert(listForkRetOut.vFork.end(), vTempFork.begin(), vTempFork.end());
            listForkRetOut.vFork.insert(listForkRetOut.vFork.end(), vRpcFork.begin(), vRpcFork.end());
            ListForkUnique(listForkRetOut.vFork);
            result.vData = RPCRouteRetToStream(listForkRetOut);
            SendRPCResult(result);
            DeleteSessionCountByNonce(data->nNonce);
            DeleteQueCount(data->nNonce);
        }
        return;
    }
}

void CDbpService::RPCForkHandle(CMvRPCRouteGetBlockLocation* data, CMvRPCRouteGetBlockLocationRet* ret)
{
    auto vRawData = RPCRouteRetToStream(*data);

    if (ret == NULL)
    {
        if (GetSessionCountByNonce(data->nNonce) == 0)
        {
            CMvRPCRouteGetBlockLocationRet getBlockLocationRet;
            CMvRPCRouteResult result;
            result.type = CMvRPCRoute::DBP_RPCROUTE_GET_BLOCK_LOCATION;
            result.vRawData = vRawData;
            getBlockLocationRet.nNonce = data->nNonce;
            getBlockLocationRet.type = data->type;
            getBlockLocationRet.strFork = "";
            getBlockLocationRet.height = 0;
            result.vData = RPCRouteRetToStream(getBlockLocationRet);
            SendRPCResult(result);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) != 0)
        {
            PushMsgToChild(vRawData, data->type, data->nNonce);
            return;
        }
    }

    if (ret != NULL)
    {
        if (!ret->strFork.empty())
        {
            CMvRPCRouteGetBlockLocationRet getBlockLocationRet;
            CMvRPCRouteResult result;
            result.type = CMvRPCRoute::DBP_RPCROUTE_GET_BLOCK_LOCATION;
            result.vRawData = vRawData;
            getBlockLocationRet.nNonce = data->nNonce;
            getBlockLocationRet.type = data->type;
            getBlockLocationRet.strFork = ret->strFork;
            getBlockLocationRet.height = ret->height;
            result.vData = RPCRouteRetToStream(getBlockLocationRet);
            SendRPCResult(result);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) == 0)
        {
            CMvRPCRouteGetBlockLocationRet getBlockLocationRet;
            CMvRPCRouteResult result;
            result.type = CMvRPCRoute::DBP_RPCROUTE_GET_BLOCK_LOCATION;
            result.vRawData = vRawData;
            getBlockLocationRet.nNonce = data->nNonce;
            getBlockLocationRet.type = data->type;
            getBlockLocationRet.strFork = "";
            getBlockLocationRet.height = 0;
            result.vData = RPCRouteRetToStream(getBlockLocationRet);
            SendRPCResult(result);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) != 0)
        {
            PushMsgToChild(vRawData, data->type, data->nNonce);
            return;
        }
    }
}

void CDbpService::RPCForkHandle(CMvRPCRouteGetBlockCount* data, CMvRPCRouteGetBlockCountRet* ret)
{
    auto vRawData = RPCRouteRetToStream(*data);

    if (ret == NULL)
    {
        if (GetSessionCountByNonce(data->nNonce) == 0)
        {
            CMvRPCRouteGetBlockCountRet getBlockCountRet;
            CMvRPCRouteResult result;
            result.type = CMvRPCRoute::DBP_RPCROUTE_GET_BLOCK_COUNT;
            result.vRawData = vRawData;
            getBlockCountRet.nNonce = data->nNonce;
            getBlockCountRet.type = data->type;
            getBlockCountRet.exception = 2;
            getBlockCountRet.height = 0;
            result.vData = RPCRouteRetToStream(getBlockCountRet);
            SendRPCResult(result);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) != 0)
        {
            PushMsgToChild(vRawData, data->type, data->nNonce);
            return;
        }
    }

    if (ret != NULL)
    {
        if(ret->exception == 0)
        {
            CMvRPCRouteGetBlockCountRet getBlockCountRet;
            CMvRPCRouteResult result;
            result.type = CMvRPCRoute::DBP_RPCROUTE_GET_BLOCK_COUNT;
            result.vRawData = vRawData;
            getBlockCountRet.nNonce = data->nNonce;
            getBlockCountRet.type = data->type;
            getBlockCountRet.exception = ret->exception;
            getBlockCountRet.height = ret->height;
            result.vData = RPCRouteRetToStream(getBlockCountRet);
            SendRPCResult(result);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) == 0)
        {
            CMvRPCRouteGetBlockCountRet getBlockCountRet;
            CMvRPCRouteResult result;
            result.type = CMvRPCRoute::DBP_RPCROUTE_GET_BLOCK_COUNT;
            result.vRawData = vRawData;
            getBlockCountRet.nNonce = data->nNonce;
            getBlockCountRet.type = data->type;
            getBlockCountRet.exception = 2;
            getBlockCountRet.height = 0;
            result.vData = RPCRouteRetToStream(getBlockCountRet);
            SendRPCResult(result);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) != 0)
        {
            PushMsgToChild(vRawData, data->type, data->nNonce);
            return;
        }
    }
}

void CDbpService::RPCForkHandle(CMvRPCRouteGetBlockHash* data, CMvRPCRouteGetBlockHashRet* ret)
{
    auto vRawData = RPCRouteRetToStream(*data);

    if (ret == NULL)
    {
        if (GetSessionCountByNonce(data->nNonce) == 0)
        {
            CMvRPCRouteGetBlockHashRet getBlockHashRet;
            CMvRPCRouteResult result;
            result.type = CMvRPCRoute::DBP_RPCROUTE_GET_BLOCK_HASH;
            result.vRawData = vRawData;
            getBlockHashRet.nNonce = data->nNonce;
            getBlockHashRet.type = data->type;
            getBlockHashRet.exception = 2;
            result.vData = RPCRouteRetToStream(getBlockHashRet);
            SendRPCResult(result);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) != 0)
        {
            PushMsgToChild(vRawData, data->type, data->nNonce);
            return;
        }
    }

    if (ret != NULL)
    {
        if(ret->exception == 0 || ret->exception == 3)
        {
            CMvRPCRouteGetBlockHashRet getBlockHashRet;
            CMvRPCRouteResult result;
            result.type = CMvRPCRoute::DBP_RPCROUTE_GET_BLOCK_HASH;
            result.vRawData = vRawData;
            getBlockHashRet.nNonce = data->nNonce;
            getBlockHashRet.type = data->type;
            getBlockHashRet.exception = ret->exception;
            getBlockHashRet.vHash = ret->vHash;
            result.vData = RPCRouteRetToStream(getBlockHashRet);
            SendRPCResult(result);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) == 0)
        {
            CMvRPCRouteGetBlockHashRet getBlockHashRet;
            CMvRPCRouteResult result;
            result.type = CMvRPCRoute::DBP_RPCROUTE_GET_BLOCK_HASH;
            result.vRawData = vRawData;
            getBlockHashRet.nNonce = data->nNonce;
            getBlockHashRet.type = data->type;
            getBlockHashRet.exception = 2;
            result.vData = RPCRouteRetToStream(getBlockHashRet);
            SendRPCResult(result);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) != 0)
        {
            PushMsgToChild(vRawData, data->type, data->nNonce);
            return;
        }
    }
}

void CDbpService::RPCForkHandle(CMvRPCRouteGetBlock* data, CMvRPCRouteGetBlockRet* ret)
{
    auto vRawData = RPCRouteRetToStream(*data);

    if (ret == NULL)
    {
        if (GetSessionCountByNonce(data->nNonce) == 0)
        {
            CMvRPCRouteGetBlockRet getBlockRet;
            CMvRPCRouteResult result;
            result.type = CMvRPCRoute::DBP_RPCROUTE_GET_BLOCK;
            result.vRawData = vRawData;
            getBlockRet.nNonce = data->nNonce;
            getBlockRet.type = data->type;
            getBlockRet.exception = 1;
            result.vData = RPCRouteRetToStream(getBlockRet);
            SendRPCResult(result);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) != 0)
        {
            PushMsgToChild(vRawData, data->type, data->nNonce);
            return;
        }
    }

    if (ret != NULL)
    {
        CMvRPCRouteGetBlockRet getBlockRet;
        CMvRPCRouteResult result;
        result.type = CMvRPCRoute::DBP_RPCROUTE_GET_BLOCK;
        result.vRawData = vRawData;
        getBlockRet.nNonce = data->nNonce;
        getBlockRet.type = data->type;

        if(ret->exception == 0)
        {
            getBlockRet.exception = ret->exception;
            getBlockRet.strFork = ret->strFork;
            getBlockRet.height = ret->height;
            getBlockRet.block = ret->block;
            result.vData = RPCRouteRetToStream(getBlockRet);
            SendRPCResult(result);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) == 0)
        {
            getBlockRet.exception = 1;
            result.vData = RPCRouteRetToStream(getBlockRet);
            SendRPCResult(result);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) != 0)
        {
            PushMsgToChild(vRawData, data->type, data->nNonce);
            return;
        }
    }
}

void CDbpService::RPCForkHandle(CMvRPCRouteGetTxPool* data, CMvRPCRouteGetTxPoolRet* ret)
{
    auto vRawData = RPCRouteRetToStream(*data);

    if (ret == NULL)
    {
        if (GetSessionCountByNonce(data->nNonce) == 0)
        {
            CMvRPCRouteGetTxPoolRet getTxPoolRet;
            CMvRPCRouteResult result;
            result.type = CMvRPCRoute::DBP_RPCROUTE_GET_TXPOOL;
            result.vRawData = vRawData;
            getTxPoolRet.nNonce = data->nNonce;
            getTxPoolRet.type = data->type;
            getTxPoolRet.exception = 2;
            result.vData = RPCRouteRetToStream(getTxPoolRet);
            SendRPCResult(result);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) != 0)
        {
            PushMsgToChild(vRawData, data->type, data->nNonce);
            return;
        }
    }

    if (ret != NULL)
    {
        CMvRPCRouteGetTxPoolRet getTxPoolRet;
        CMvRPCRouteResult result;
        result.type = CMvRPCRoute::DBP_RPCROUTE_GET_TXPOOL;
        result.vRawData = vRawData;
        getTxPoolRet.nNonce = data->nNonce;
        getTxPoolRet.type = data->type;

        if(ret->exception == 0)
        {
            getTxPoolRet.exception = ret->exception;
            getTxPoolRet.vTxPool = ret->vTxPool;
            result.vData = RPCRouteRetToStream(getTxPoolRet);
            SendRPCResult(result);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) == 0)
        {
            getTxPoolRet.exception = 2;
            result.vData = RPCRouteRetToStream(getTxPoolRet);
            SendRPCResult(result);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) != 0)
        {
            PushMsgToChild(vRawData, data->type, data->nNonce);
            return;
        }
    }
}

void CDbpService::RPCForkHandle(CMvRPCRouteGetTransaction* data, CMvRPCRouteGetTransactionRet* ret)
{
    auto vRawData = RPCRouteRetToStream(*data);

    if (ret == NULL)
    {
        if (GetSessionCountByNonce(data->nNonce) == 0)
        {
            CMvRPCRouteGetTransactionRet getTransactionRet;
            CMvRPCRouteResult result;
            result.type = CMvRPCRoute::DBP_RPCROUTE_GET_TRANSACTION;
            result.vRawData = vRawData;
            getTransactionRet.nNonce = data->nNonce;
            getTransactionRet.type = data->type;
            getTransactionRet.exception = 1;
            result.vData = RPCRouteRetToStream(getTransactionRet);
            SendRPCResult(result);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) != 0)
        {
            PushMsgToChild(vRawData, data->type, data->nNonce);
            return;
        }
    }

    if (ret != NULL)
    {
        CMvRPCRouteGetTransactionRet getTransactionRet;
        CMvRPCRouteResult result;
        result.type = CMvRPCRoute::DBP_RPCROUTE_GET_TRANSACTION;
        result.vRawData = vRawData;
        getTransactionRet.nNonce = data->nNonce;
        getTransactionRet.type = data->type;

        if(ret->exception == 0)
        {
            getTransactionRet.exception = ret->exception;
            getTransactionRet.tx = ret->tx;
            getTransactionRet.strFork = ret->strFork;
            getTransactionRet.nDepth = ret->nDepth;
            result.vData = RPCRouteRetToStream(getTransactionRet);
            SendRPCResult(result);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) == 0) 
        {
            getTransactionRet.exception = 2;
            result.vData = RPCRouteRetToStream(getTransactionRet);
            SendRPCResult(result);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) != 0)
        {
            PushMsgToChild(vRawData, data->type, data->nNonce);
            return;
        }
    }
}

void CDbpService::RPCForkHandle(CMvRPCRouteGetForkHeight* data, CMvRPCRouteGetForkHeightRet* ret)
{
    auto vRawData = RPCRouteRetToStream(*data);

    if (ret == NULL)
    {
        if (GetSessionCountByNonce(data->nNonce) == 0)
        {
            CMvRPCRouteGetForkHeightRet getForkHeightRet;
            CMvRPCRouteResult result;
            result.type = CMvRPCRoute::DBP_RPCROUTE_GET_FORK_HEIGHT;
            result.vRawData = vRawData;
            getForkHeightRet.nNonce = data->nNonce;
            getForkHeightRet.type = data->type;
            getForkHeightRet.exception = 2;
            result.vData = RPCRouteRetToStream(getForkHeightRet);
            SendRPCResult(result);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) != 0)
        {
            PushMsgToChild(vRawData, data->type, data->nNonce);
            return;
        }
    }

    if (ret != NULL)
    {
        CMvRPCRouteGetForkHeightRet getForkHeightRet;
        CMvRPCRouteResult result;
        result.type = CMvRPCRoute::DBP_RPCROUTE_GET_FORK_HEIGHT;
        result.vRawData = vRawData;
        getForkHeightRet.nNonce = data->nNonce;
        getForkHeightRet.type = data->type;

        if(ret->exception == 0)
        {
            getForkHeightRet.exception = ret->exception;
            getForkHeightRet.height = ret->height;
            result.vData = RPCRouteRetToStream(getForkHeightRet);
            SendRPCResult(result);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) == 0)
        {
            getForkHeightRet.exception = 2;
            result.vData = RPCRouteRetToStream(getForkHeightRet);
            SendRPCResult(result);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) != 0)
        {
            PushMsgToChild(vRawData, data->type, data->nNonce);
            return;
        }
    }
}

void CDbpService::RPCForkHandle(CMvRPCRouteSendTransaction* data, CMvRPCRouteSendTransactionRet* ret)
{
    auto vRawData = RPCRouteRetToStream(*data);

    if (ret == NULL)
    {
        if (GetSessionCountByNonce(data->nNonce) == 0)
        {
            CMvRPCRouteSendTransactionRet sendTransactionRet;
            CMvRPCRouteResult result;
            result.type = CMvRPCRoute::DBP_RPCROUTE_SEND_TRANSACTION;
            result.vRawData = vRawData;
            sendTransactionRet.nNonce = data->nNonce;
            sendTransactionRet.type = data->type;
            sendTransactionRet.exception = 1;
            result.vData = RPCRouteRetToStream(sendTransactionRet);
            SendRPCResult(result);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) != 0)
        {
            PushMsgToChild(vRawData, data->type, data->nNonce);
            return;
        }
    }

    if (ret != NULL)
    {
        CMvRPCRouteSendTransactionRet sendTransactionRet;
        CMvRPCRouteResult result;
        result.type = CMvRPCRoute::DBP_RPCROUTE_SEND_TRANSACTION;
        result.vRawData = vRawData;
        sendTransactionRet.nNonce = data->nNonce;
        sendTransactionRet.type = data->type;

        if(ret->exception == 0)
        {
            sendTransactionRet.exception = ret->exception;
            result.vData = RPCRouteRetToStream(sendTransactionRet);
            SendRPCResult(result);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) == 0)
        {
            sendTransactionRet.exception = 1;
            result.vData = RPCRouteRetToStream(sendTransactionRet);
            SendRPCResult(result);
            DeleteSessionCountByNonce(data->nNonce);
            return;
        }

        if (GetSessionCountByNonce(data->nNonce) != 0)
        {
            PushMsgToChild(vRawData, data->type, data->nNonce);
            return;
        }
    }
}

void CDbpService::InsertQueCount(uint64 nNonce, boost::any obj)
{
    if(queCount.size() > 100)
    {
        queCount.pop_back();
    }
    std::pair<uint64, boost::any> pair(nNonce, obj);
    queCount.push_front(pair);
}

void CDbpService::DeleteQueCount(uint64 nNonce)
{
    auto compare = [nNonce](const std::pair<uint64, boost::any>& element) { return nNonce == element.first; };
    auto iter = std::find_if(queCount.begin(), queCount.end(), compare);
    if (iter != queCount.end())
    {
        queCount.erase(iter);
    }
}

void CDbpService::PushMsgToChild(std::vector<uint8>& data, int& type, uint64& nNonce)
{
    auto compare = [nNonce](const std::pair<uint64, std::vector<std::string>>& element) {
        return nNonce == element.first;
    };

    auto iter = std::find_if(vRPCTopicId.begin(), vRPCTopicId.end(), compare);
    if (iter != vRPCTopicId.end())
    {
        if (!iter->second.empty())
        {
            auto id = iter->second.back();
            PushRPCOnce(id, data, type);
            iter->second.pop_back();
        }

        if (iter->second.empty())
        {
            vRPCTopicId.erase(iter);
        }
    }
}

void CDbpService::SendRPCResult(CMvRPCRouteResult& result)
{
    CMvEventRPCRouteResult event(""); 
    event.data.type = result.type;
    event.data.vData = result.vData;
    event.data.vRawData = result.vRawData;
    pDbpClient->DispatchEvent(&event);
}

void CDbpService::PushRPCOnce(std::string id, std::vector<uint8>& data, int type)
{
    auto it = mapIdSubedSession.find(id);
    if (it != mapIdSubedSession.end())
    {
        CMvEventRPCRouteAdded eventAdded(it->second);
        eventAdded.data.id = id;
        eventAdded.data.name = RPC_CMD_TOPIC;
        eventAdded.data.type = type;
        eventAdded.data.vData = data;
        pDbpServer->DispatchEvent(&eventAdded);
    }
}

void CDbpService::PushRPC(std::vector<uint8>& data, int type)
{
    const auto& allIds = mapTopicIds[RPC_CMD_TOPIC];
    for (const auto& id : allIds)
    {
        auto it = mapIdSubedSession.find(id);
        if (it != mapIdSubedSession.end())
        {
            CMvEventRPCRouteAdded eventAdded(it->second);
            eventAdded.data.id = id;
            eventAdded.data.name = RPC_CMD_TOPIC;
            eventAdded.data.type = type;
            eventAdded.data.vData = data;
            pDbpServer->DispatchEvent(&eventAdded);
        }
    }
}

void CDbpService::CreateCompletion(uint64 nNonce, std::shared_ptr<walleve::CIOCompletionUntil> sPtr)
{
    std::pair<uint64, std::shared_ptr<walleve::CIOCompletionUntil>> pair(nNonce, sPtr);
    vCompletion.push_back(pair);
}

void CDbpService::CompletionByNonce(uint64& nNonce, boost::any obj)
{
    auto compare = [nNonce](const std::pair<uint64, std::shared_ptr<walleve::CIOCompletionUntil>>& element) {
        return nNonce == element.first;
    };

    auto iter = std::find_if(vCompletion.begin(), vCompletion.end(), compare);
    if (iter != vCompletion.end())
    {
        iter->second->obj = obj;
        iter->second->Completed(true);
    }
}

void CDbpService::DeleteCompletionByNonce(uint64 nNonce)
{
    auto compare = [nNonce](const std::pair<uint64, std::shared_ptr<walleve::CIOCompletionUntil>>& element) {
        return nNonce == element.first;
    };

    auto iter = std::find_if(vCompletion.begin(), vCompletion.end(), compare);
    if (iter != vCompletion.end())
    {
        vCompletion.erase(iter);
    }
}

void CDbpService::InitRPCTopicId(uint64 nNonce)
{
    if (vRPCTopicId.size() >= 100)
    {
        vRPCTopicId.erase(vRPCTopicId.begin());
    }

    std::vector<std::string> vTopicId;
    const auto& allIds = mapTopicIds[RPC_CMD_TOPIC];
    for (const auto id : allIds)
    {
        auto it = mapIdSubedSession.find(id);
        if (it != mapIdSubedSession.end())
        {
            vTopicId.push_back(id);
        }
    }

    std::pair<uint64, std::vector<std::string>> pair(nNonce, vTopicId);
    vRPCTopicId.push_back(pair);
    // std::cout << "vRPCTopicId size:" << vRPCTopicId.size() << ", vTopicId size:" << vTopicId.size() << std::endl;
}

void CDbpService::InitSessionCount(uint64 nNonce)
{
    if (vSessionCount.size() >= 100)
    {
        vSessionCount.erase(vSessionCount.begin());
    }

    std::set<std::string> setSession;
    const auto& allIds = mapTopicIds[RPC_CMD_TOPIC];
    for (const auto id : allIds)
    {
        auto it = mapIdSubedSession.find(id);
        if (it != mapIdSubedSession.end())
        {
            setSession.insert(it->second);
        }
    }
    std::pair<uint64, int> pair(nNonce, setSession.size());
    vSessionCount.push_back(pair);
    std::cout << "vSessionCount size:" << vSessionCount.size() << ", setSession size:" << setSession.size() << std::endl;
}

int CDbpService::GetSessionCountByNonce(uint64 nNonce)
{
    auto compare = [nNonce](const std::pair<uint64, int>& element) {
        return nNonce == element.first;
    };

    auto iter = std::find_if(vSessionCount.begin(), vSessionCount.end(), compare);
    if (iter != vSessionCount.end())
    {
        return iter->second;
    }
    return -1;
}

void CDbpService::CountDownSessionCountByNonce(uint64 nNonce)
{
    auto compare = [nNonce](const std::pair<uint64, int>& element) {
        return nNonce == element.first;
    };

    auto iter = std::find_if(vSessionCount.begin(), vSessionCount.end(), compare);
    if (iter != vSessionCount.end())
    {
        int& count = iter->second;
        count--;
    }
}

void CDbpService::DeleteSessionCountByNonce(uint64 nNonce)
{
    auto compare = [nNonce](const std::pair<uint64, int>& element) {
        return nNonce == element.first;
    };

    auto iter = std::find_if(vSessionCount.begin(), vSessionCount.end(), compare);
    if (iter != vSessionCount.end())
    {
        vSessionCount.erase(iter);
    }
}

bool CDbpService::HandleEvent(CMvEventRPCRouteStop& event)
{
    event.data.type = CMvRPCRoute::DBP_RPCROUTE_STOP;
    CreateCompletion(event.data.nNonce, event.data.spIoCompltUntil);
    CMvRPCRouteStopRet stopRet;

    InitSessionCount(event.data.nNonce);
    if(GetSessionCountByNonce(event.data.nNonce) > 0)
    {
        auto data = RPCRouteRetToStream(event.data);
        PushRPC(data, CMvRPCRoute::DBP_RPCROUTE_STOP);
        return true;
    }

    RPCRootHandle(&event.data, NULL);
    return true;
}

bool CDbpService::HandleEvent(CMvEventRPCRouteListFork& event)
{
    event.data.type = CMvRPCRoute::DBP_RPCROUTE_LIST_FORK;
    CreateCompletion(event.data.nNonce, event.data.spIoCompltUntil);
    CMvRPCRouteListForkRet listForkRet;
    InsertQueCount(event.data.nNonce, listForkRet);

    InitSessionCount(event.data.nNonce);
    if(GetSessionCountByNonce(event.data.nNonce) > 0)
    {
        auto data = RPCRouteRetToStream(event.data);
        PushRPC(data, CMvRPCRoute::DBP_RPCROUTE_LIST_FORK);
        return true;
    }
    RPCRootHandle(&event.data, NULL);
    return true;
}

bool CDbpService::HandleEvent(CMvEventRPCRouteGetBlockLocation& event)
{
    event.data.type = CMvRPCRoute::DBP_RPCROUTE_GET_BLOCK_LOCATION;
    CreateCompletion(event.data.nNonce, event.data.spIoCompltUntil);

    CMvRPCRouteGetBlockLocationRet getBlockLocationRet;
    getBlockLocationRet.strFork = "";
    getBlockLocationRet.height = 0;

    uint256 hashBlock;
    hashBlock.SetHex(event.data.strBlock);
    uint256 fork;
    int height;
    if (pService->GetBlockLocation(hashBlock, fork, height))
    {
        getBlockLocationRet.height = height;
        getBlockLocationRet.strFork = fork.GetHex();
        CompletionByNonce(event.data.nNonce, getBlockLocationRet);
        return true;
    }

    InitRPCTopicId(event.data.nNonce);
    InitSessionCount(event.data.nNonce);
    RPCRootHandle(&event.data, NULL);
    return true;
}

bool CDbpService::GetForkHashOfDef(const rpc::CRPCString& hex, uint256& hashFork)
{
    if (!hex.empty())
    {
        if (hashFork.SetHex(hex) != hex.size())
        {
            return false;
        }
    }
    else
    {
        hashFork = pCoreProtocol->GetGenesisBlockHash();
    }
    return true;
}

bool CDbpService::HandleEvent(CMvEventRPCRouteGetBlockCount& event)
{
    event.data.type = CMvRPCRoute::DBP_RPCROUTE_GET_BLOCK_COUNT;
    CreateCompletion(event.data.nNonce, event.data.spIoCompltUntil);

    CMvRPCRouteGetBlockCountRet getBlockCountRet;
    getBlockCountRet.nNonce = event.data.nNonce;

    uint256 hashFork;
    if (!GetForkHashOfDef(event.data.strFork, hashFork))
    {
        getBlockCountRet.height = 0;
        getBlockCountRet.exception = 1;
        CompletionByNonce(event.data.nNonce, getBlockCountRet);
        return true;
    }

    if (pService->HaveFork(hashFork))
    {
        getBlockCountRet.height = pService->GetBlockCount(hashFork);
        getBlockCountRet.exception = 0;
        CompletionByNonce(event.data.nNonce, getBlockCountRet);
        return true;
    }

    InitRPCTopicId(event.data.nNonce);
    InitSessionCount(event.data.nNonce);
    RPCRootHandle(&event.data, NULL);
    return true;
}

bool CDbpService::HandleEvent(CMvEventRPCRouteGetBlockHash& event)
{
    event.data.type = CMvRPCRoute::DBP_RPCROUTE_GET_BLOCK_HASH;
    CreateCompletion(event.data.nNonce, event.data.spIoCompltUntil);

    CMvRPCRouteGetBlockHashRet getBlockHashRet;
    getBlockHashRet.nNonce = event.data.nNonce;

    uint256 hashFork;
    if (!GetForkHashOfDef(event.data.strFork, hashFork))
    {
        getBlockHashRet.exception = 1;
        CompletionByNonce(event.data.nNonce, getBlockHashRet);
        return true;
    }

    if (pService->HaveFork(hashFork))
    {
        vector<uint256> vBlockHash;
        if (!pService->GetBlockHash(hashFork, event.data.height, vBlockHash))
        {
            getBlockHashRet.exception = 3;
            CompletionByNonce(event.data.nNonce, getBlockHashRet);
            return true;
        }

        for (const uint256& hash : vBlockHash)
        {
            getBlockHashRet.vHash.push_back(hash.GetHex());
        }

        getBlockHashRet.exception = 0;
        CompletionByNonce(event.data.nNonce, getBlockHashRet);
        return true;
    }

    InitRPCTopicId(event.data.nNonce);
    InitSessionCount(event.data.nNonce);
    RPCRootHandle(&event.data, NULL);
    return true;
}

bool CDbpService::HandleEvent(CMvEventRPCRouteGetBlock& event)
{
    event.data.type = CMvRPCRoute::DBP_RPCROUTE_GET_BLOCK;
    CreateCompletion(event.data.nNonce, event.data.spIoCompltUntil);

    CMvRPCRouteGetBlockRet getBlockRet;
    getBlockRet.nNonce = event.data.nNonce;

    uint256 hashBlock;
    hashBlock.SetHex(event.data.hash);

    CBlock block;
    uint256 fork;
    int height;
    if (pService->GetBlock(hashBlock, block, fork, height))
    {
        getBlockRet.exception = 0;
        getBlockRet.strFork = fork.GetHex();
        getBlockRet.height = height;
        getBlockRet.block = block;
        CompletionByNonce(event.data.nNonce, getBlockRet);
        return true;
    }

    InitRPCTopicId(event.data.nNonce);
    InitSessionCount(event.data.nNonce);
    RPCRootHandle(&event.data, NULL);
    return true;
}

bool CDbpService::HandleEvent(CMvEventRPCRouteGetTxPool& event)
{
    event.data.type = CMvRPCRoute::DBP_RPCROUTE_GET_TXPOOL;
    CreateCompletion(event.data.nNonce, event.data.spIoCompltUntil);

    CMvRPCRouteGetTxPoolRet getTxPoolRet;
    getTxPoolRet.nNonce = event.data.nNonce;

    uint256 hashFork;
    if (!GetForkHashOfDef(event.data.strFork, hashFork))
    {
        getTxPoolRet.exception = 1;
        CompletionByNonce(event.data.nNonce, getTxPoolRet);
        return true;
    }

    if (pService->HaveFork(hashFork))
    {
        std::vector<std::pair<uint256, size_t>> vTxPool;
        pService->GetTxPool(hashFork, vTxPool);
        for (auto& txPool : vTxPool)
        {
            getTxPoolRet.vTxPool.push_back({txPool.first.GetHex(), txPool.second});
        }

        getTxPoolRet.exception = 0;
        CompletionByNonce(event.data.nNonce, getTxPoolRet);
        return true;
    }

    InitRPCTopicId(event.data.nNonce);
    InitSessionCount(event.data.nNonce);
    RPCRootHandle(&event.data, NULL);
    return true;
}

bool CDbpService::HandleEvent(CMvEventRPCRouteGetTransaction& event) 
{
    event.data.type = CMvRPCRoute::DBP_RPCROUTE_GET_TRANSACTION;
    CreateCompletion(event.data.nNonce, event.data.spIoCompltUntil);

    CMvRPCRouteGetTransactionRet getTransactionRet;
    getTransactionRet.nNonce = event.data.nNonce;

    uint256 txid;
    txid.SetHex(event.data.strTxid);

    CTransaction tx;
    uint256 hashFork;
    int nHeight;

    if (pService->GetTransaction(txid, tx, hashFork, nHeight))
    {
        getTransactionRet.exception = 0;
        getTransactionRet.tx = tx;
        getTransactionRet.strFork = hashFork.GetHex();
        getTransactionRet.nDepth = nHeight < 0 ? 0 : pService->GetBlockCount(hashFork) - nHeight;
        CompletionByNonce(event.data.nNonce, getTransactionRet);
        return true;
    }

    InitRPCTopicId(event.data.nNonce);
    InitSessionCount(event.data.nNonce);
    RPCRootHandle(&event.data, NULL);
    return true;
}

bool CDbpService::HandleEvent(CMvEventRPCRouteGetForkHeight& event)
{
    event.data.type = CMvRPCRoute::DBP_RPCROUTE_GET_FORK_HEIGHT;
    CreateCompletion(event.data.nNonce, event.data.spIoCompltUntil);

    CMvRPCRouteGetForkHeightRet getForkHeightRet;
    getForkHeightRet.nNonce = event.data.nNonce;

    uint256 hashFork;
    if (!GetForkHashOfDef(event.data.strFork, hashFork))
    {
        getForkHeightRet.exception = 1;
        CompletionByNonce(event.data.nNonce, getForkHeightRet);
        return true;
    }

    if (pService->HaveFork(hashFork))
    {
        getForkHeightRet.exception = 0;
        getForkHeightRet.height = pService->GetForkHeight(hashFork);
        CompletionByNonce(event.data.nNonce, getForkHeightRet);
        return true;
    }

    InitRPCTopicId(event.data.nNonce);
    InitSessionCount(event.data.nNonce);
    RPCRootHandle(&event.data, NULL);
    return true;
}

bool CDbpService::HandleEvent(CMvEventRPCRouteSendTransaction& event)
{
    event.data.type = CMvRPCRoute::DBP_RPCROUTE_SEND_TRANSACTION;
    CreateCompletion(event.data.nNonce, event.data.spIoCompltUntil);

    CMvRPCRouteSendTransactionRet sendTransactionRet;
    sendTransactionRet.nNonce = event.data.nNonce;

    MvErr err = pService->SendTransaction(event.data.rawTx);
    if (err == MV_OK)
    {
        sendTransactionRet.exception = 0;
        CompletionByNonce(event.data.nNonce, sendTransactionRet);
        return true;
    }

    InitRPCTopicId(event.data.nNonce);
    InitSessionCount(event.data.nNonce);
    RPCRootHandle(&event.data, NULL);
    return true;
}

bool CDbpService::HandleAddedEventStop(CMvEventRPCRouteAdded& event, walleve::CWalleveBufStream& ss)
{
    CMvRPCRouteStop stop;
    ss >> stop;
    CMvRPCRouteStopRet stopRet;
    InitSessionCount(stop.nNonce);
    if (GetSessionCountByNonce(stop.nNonce) > 0)
    {
        PushRPC(event.data.vData, event.data.type);
        return true;
    }
    RPCForkHandle(&stop, NULL);
    return true;
}

bool CDbpService::HandleAddedEventListFork(CMvEventRPCRouteAdded& event, walleve::CWalleveBufStream& ss)
{
    CMvRPCRouteListFork listFork;
    ss >> listFork;

    CMvRPCRouteListForkRet listForkRet;
    InitSessionCount(listFork.nNonce);
    InsertQueCount(listFork.nNonce, listForkRet);
    if (GetSessionCountByNonce(listFork.nNonce) > 0)
    {
        PushRPC(event.data.vData, event.data.type);
        return true;
    }
    RPCForkHandle(&listFork, NULL);
    return true;
}

bool CDbpService::HandleAddedEventGetBlockLocation(CMvEventRPCRouteAdded& event, walleve::CWalleveBufStream& ss)
{
    CMvRPCRouteGetBlockLocation getBlockLocation;
    ss >> getBlockLocation;
    CMvRPCRouteGetBlockLocationRet getBlockLocationRet;

    InitSessionCount(getBlockLocation.nNonce);
    InitRPCTopicId(getBlockLocation.nNonce);

    uint256 hashBlock;
    hashBlock.SetHex(getBlockLocation.strBlock);
    uint256 fork;
    int height;
    if (pService->GetBlockLocation(hashBlock, fork, height))
    {
        CMvRPCRouteResult result;
        result.type = CMvRPCRoute::DBP_RPCROUTE_GET_BLOCK_LOCATION;
        result.vRawData = event.data.vData;
        getBlockLocationRet.nNonce = getBlockLocation.nNonce;
        getBlockLocationRet.height = height;
        getBlockLocationRet.strFork = fork.GetHex();
        result.vData = RPCRouteRetToStream(getBlockLocationRet);
        SendRPCResult(result);
        return true;
    }
    RPCForkHandle(&getBlockLocation, NULL);
    return true;
}

bool CDbpService::HandleAddedEventGetBlockCount(CMvEventRPCRouteAdded& event, walleve::CWalleveBufStream& ss)
{
    CMvRPCRouteGetBlockCount getBlockCount;
    ss >> getBlockCount;
    CMvRPCRouteGetBlockCountRet getBlockCountRet;

    InitSessionCount(getBlockCount.nNonce);
    InitRPCTopicId(getBlockCount.nNonce);

    uint256 hashFork;
    if (!GetForkHashOfDef(getBlockCount.strFork, hashFork))
    {
        CMvRPCRouteResult result;
        result.type = CMvRPCRoute::DBP_RPCROUTE_GET_BLOCK_COUNT;
        result.vRawData = event.data.vData;
        getBlockCountRet.nNonce = getBlockCount.nNonce;
        getBlockCountRet.height = 0;
        getBlockCountRet.exception = 1;
        result.vData = RPCRouteRetToStream(getBlockCountRet);
        SendRPCResult(result);
        return true;
    }

    if (pService->HaveFork(hashFork))
    {
        CMvRPCRouteResult result;
        result.type = CMvRPCRoute::DBP_RPCROUTE_GET_BLOCK_COUNT;
        result.vRawData = event.data.vData;
        getBlockCountRet.nNonce = getBlockCount.nNonce;
        getBlockCountRet.height = pService->GetBlockCount(hashFork);
        getBlockCountRet.exception = 0;
        result.vData = RPCRouteRetToStream(getBlockCountRet);
        SendRPCResult(result);
        return true;
    }
    RPCForkHandle(&getBlockCount, NULL);
    return true;
}

bool CDbpService::HandleAddedEventGetBlockHash(CMvEventRPCRouteAdded& event, walleve::CWalleveBufStream& ss)
{
    CMvRPCRouteGetBlockHash getBlockHash;
    ss >> getBlockHash;
    CMvRPCRouteGetBlockHashRet getBlockHashRet;

    CMvRPCRouteResult result;
    result.type = CMvRPCRoute::DBP_RPCROUTE_GET_BLOCK_HASH;
    result.vRawData = event.data.vData;
    getBlockHashRet.nNonce = getBlockHash.nNonce;

    InitSessionCount(getBlockHash.nNonce);
    InitRPCTopicId(getBlockHash.nNonce);

    uint256 hashFork;
    if (!GetForkHashOfDef(getBlockHash.strFork, hashFork))
    {
        getBlockHashRet.exception = 1;
        result.vData = RPCRouteRetToStream(getBlockHashRet);
        SendRPCResult(result);
        return true;
    }

    if (pService->HaveFork(hashFork))
    {
        vector<uint256> vBlockHash;
        if (!pService->GetBlockHash(hashFork, getBlockHash.height, vBlockHash))
        {
            getBlockHashRet.exception = 3;
            result.vData = RPCRouteRetToStream(getBlockHashRet);
            SendRPCResult(result);
            return true;
        }

        for (const uint256& hash : vBlockHash)
        {
            getBlockHashRet.vHash.push_back(hash.GetHex());
        }

        getBlockHashRet.exception = 0;
        result.vData = RPCRouteRetToStream(getBlockHashRet);
        SendRPCResult(result);
        return true;
    }
    RPCForkHandle(&getBlockHash, NULL);
    return true;
}

bool CDbpService::HandleAddedEventGetBlock(CMvEventRPCRouteAdded& event, walleve::CWalleveBufStream& ss)
{
    CMvRPCRouteGetBlock getBlock;
    ss >> getBlock;
    CMvRPCRouteGetBlockRet getBlockRet;

    CMvRPCRouteResult result;
    result.type = CMvRPCRoute::DBP_RPCROUTE_GET_BLOCK;
    result.vRawData = event.data.vData;
    getBlockRet.nNonce = getBlock.nNonce;

    InitSessionCount(getBlock.nNonce);
    InitRPCTopicId(getBlock.nNonce);

    uint256 hashBlock, fork;
    hashBlock.SetHex(getBlock.hash);
    CBlock block;
    int height;
    if (pService->GetBlock(hashBlock, block, fork, height))
    {
        getBlockRet.exception = 0;
        getBlockRet.strFork = fork.GetHex();
        getBlockRet.height = height;
        getBlockRet.block = block;
        result.vData = RPCRouteRetToStream(getBlockRet);
        SendRPCResult(result);
        return true;
    }
    RPCForkHandle(&getBlock, NULL);
    return true;
}

bool CDbpService::HandleAddedEventGetTxPool(CMvEventRPCRouteAdded& event, walleve::CWalleveBufStream& ss)
{
    CMvRPCRouteGetTxPool getTxPool;
    ss >> getTxPool;
    CMvRPCRouteGetTxPoolRet getTxPoolRet;

    CMvRPCRouteResult result;
    result.type = CMvRPCRoute::DBP_RPCROUTE_GET_TXPOOL;
    result.vRawData = event.data.vData;
    getTxPoolRet.nNonce = getTxPool.nNonce;

    InitSessionCount(getTxPool.nNonce);
    InitRPCTopicId(getTxPool.nNonce);

    uint256 hashFork;
    hashFork.SetHex(getTxPool.strFork);

    if (pService->HaveFork(hashFork))
    {
        std::vector<std::pair<uint256, size_t>> vTxPool;
        pService->GetTxPool(hashFork, vTxPool);
        for (auto& txPool : vTxPool)
        {
            getTxPoolRet.vTxPool.push_back(
              { txPool.first.GetHex(), txPool.second });
        }

        getTxPoolRet.exception = 0;
        result.vData = RPCRouteRetToStream(getTxPoolRet);
        SendRPCResult(result);
        return true;
    }
    RPCForkHandle(&getTxPool, NULL);
    return true;
}

bool CDbpService::HandleAddedEventGetTransaction(CMvEventRPCRouteAdded& event, walleve::CWalleveBufStream& ss)
{
    CMvRPCRouteGetTransaction getTransaction;
    ss >> getTransaction;
    CMvRPCRouteGetTransactionRet getTransactionRet;

    CMvRPCRouteResult result;
    result.type = CMvRPCRoute::DBP_RPCROUTE_GET_TRANSACTION;
    result.vRawData = event.data.vData;
    getTransactionRet.nNonce = getTransaction.nNonce;

    InitSessionCount(getTransaction.nNonce);
    InitRPCTopicId(getTransaction.nNonce);

    uint256 txid, hashFork;
    txid.SetHex(getTransaction.strTxid);
    CTransaction tx;
    int nHeight;
    if (pService->GetTransaction(txid, tx, hashFork, nHeight))
    {
        getTransactionRet.exception = 0;
        getTransactionRet.tx = tx;
        getTransactionRet.strFork = hashFork.GetHex();
        getTransactionRet.nDepth =
          nHeight < 0 ? 0 : pService->GetBlockCount(hashFork) - nHeight;
        result.vData = RPCRouteRetToStream(getTransactionRet);
        SendRPCResult(result);
        return true;
    }
    RPCForkHandle(&getTransaction, NULL);
    return true;
}

bool CDbpService::HandleAddedEventGetForkHeight(CMvEventRPCRouteAdded& event, walleve::CWalleveBufStream& ss)
{
    CMvRPCRouteGetForkHeight getForkHeight;
    ss >> getForkHeight;
    CMvRPCRouteGetForkHeightRet getForkHeightRet;

    CMvRPCRouteResult result;
    result.type = CMvRPCRoute::DBP_RPCROUTE_GET_FORK_HEIGHT;
    result.vRawData = event.data.vData;
    getForkHeightRet.nNonce = getForkHeight.nNonce;

    InitSessionCount(getForkHeight.nNonce);
    InitRPCTopicId(getForkHeight.nNonce);

    uint256 hashFork;
    hashFork.SetHex(getForkHeight.strFork);

    if (pService->HaveFork(hashFork))
    {
        getForkHeightRet.exception = 0;
        getForkHeightRet.height = pService->GetForkHeight(hashFork);
        result.vData = RPCRouteRetToStream(getForkHeightRet);
        SendRPCResult(result);
        return true;
    }
    RPCForkHandle(&getForkHeight, NULL);
    return true;
}

bool CDbpService::HandleAddedEventSendTransaction(CMvEventRPCRouteAdded& event, walleve::CWalleveBufStream& ss)
{
    CMvRPCRouteSendTransaction sendTransaction;
    ss >> sendTransaction;
    CMvRPCRouteSendTransactionRet sendTransactionRet;

    CMvRPCRouteResult result;
    result.type = CMvRPCRoute::DBP_RPCROUTE_SEND_TRANSACTION;
    result.vRawData = event.data.vData;
    sendTransactionRet.nNonce = sendTransaction.nNonce;

    InitSessionCount(sendTransaction.nNonce);
    InitRPCTopicId(sendTransaction.nNonce);

    MvErr err = pService->SendTransaction(sendTransaction.rawTx);
    if (err == MV_OK)
    {
        sendTransactionRet.exception = 0;
        result.vData = RPCRouteRetToStream(sendTransactionRet);
        SendRPCResult(result);
        return true;
    }
    RPCForkHandle(&sendTransaction, NULL);
    return true;
}

bool CDbpService::HandleEvent(CMvEventRPCRouteAdded& event)
{
    walleve::CWalleveBufStream ss;
    ss.Write((char*)event.data.vData.data(), event.data.vData.size());

    if (event.data.type == CMvRPCRoute::DBP_RPCROUTE_STOP)
    {
        return HandleAddedEventStop(event, ss);
    }

    if(event.data.type == CMvRPCRoute::DBP_RPCROUTE_LIST_FORK)
    {
        return HandleAddedEventListFork(event, ss);
    }

    if(event.data.type == CMvRPCRoute::DBP_RPCROUTE_GET_BLOCK_LOCATION)
    {
        return HandleAddedEventGetBlockLocation(event, ss);
    }

    if (event.data.type == CMvRPCRoute::DBP_RPCROUTE_GET_BLOCK_COUNT)
    {
        return HandleAddedEventGetBlockCount(event, ss);
    }

    if(event.data.type == CMvRPCRoute::DBP_RPCROUTE_GET_BLOCK_HASH)
    {
        return HandleAddedEventGetBlockHash(event, ss);
    }

    if (event.data.type == CMvRPCRoute::DBP_RPCROUTE_GET_BLOCK)
    {
        return HandleAddedEventGetBlock(event, ss);
    }

    if (event.data.type == CMvRPCRoute::DBP_RPCROUTE_GET_TXPOOL)
    {
        return HandleAddedEventGetTxPool(event, ss);
    }

    if (event.data.type == CMvRPCRoute::DBP_RPCROUTE_GET_TRANSACTION)
    {
        return HandleAddedEventGetTransaction(event, ss);
    }

    if (event.data.type == CMvRPCRoute::DBP_RPCROUTE_GET_FORK_HEIGHT)
    {
        return HandleAddedEventGetForkHeight(event, ss);
    }

    if (event.data.type == CMvRPCRoute::DBP_RPCROUTE_SEND_TRANSACTION)
    {
        return HandleAddedEventSendTransaction(event, ss);
    }
    return true;
}

bool CDbpService::HandleEvent(CMvEventRPCRouteDelCompltUntil& event)
{
    DeleteCompletionByNonce(event.data.nNonce);
    return true;
}

void CDbpService::HandleRPCRoute(CMvEventDbpMethod& event)
{
    int type = boost::any_cast<int>(event.data.params["type"]);
    std::string data = boost::any_cast<std::string>(event.data.params["data"]);
    std::string rawData = boost::any_cast<std::string>(event.data.params["rawdata"]);

    walleve::CWalleveBufStream ss, ssRaw;
    ss.Write(data.data(), data.size());
    ssRaw.Write(rawData.data(), rawData.size());

    if (type == CMvRPCRoute::DBP_RPCROUTE_STOP)
    {
        HANDLE_RPC_ROUTE(CMvRPCRouteStop, CMvRPCRouteStopRet);
    }

    if(type == CMvRPCRoute::DBP_RPCROUTE_LIST_FORK)
    {
        HANDLE_RPC_ROUTE(CMvRPCRouteListFork, CMvRPCRouteListForkRet);
    }

    if(type == CMvRPCRoute::DBP_RPCROUTE_GET_BLOCK_LOCATION)
    {
        HANDLE_RPC_ROUTE(CMvRPCRouteGetBlockLocation, CMvRPCRouteGetBlockLocationRet);
    }

    if (type == CMvRPCRoute::DBP_RPCROUTE_GET_BLOCK_COUNT)
    {
        HANDLE_RPC_ROUTE(CMvRPCRouteGetBlockCount, CMvRPCRouteGetBlockCountRet);
    }

    if(type == CMvRPCRoute::DBP_RPCROUTE_GET_BLOCK_HASH)
    {
        HANDLE_RPC_ROUTE(CMvRPCRouteGetBlockHash, CMvRPCRouteGetBlockHashRet);
    }

    if(type == CMvRPCRoute::DBP_RPCROUTE_GET_BLOCK)
    {
        HANDLE_RPC_ROUTE(CMvRPCRouteGetBlock, CMvRPCRouteGetBlockRet);
    }

    if(type == CMvRPCRoute::DBP_RPCROUTE_GET_TXPOOL)
    {
        HANDLE_RPC_ROUTE(CMvRPCRouteGetTxPool, CMvRPCRouteGetTxPoolRet);
    }

    if(type == CMvRPCRoute::DBP_RPCROUTE_GET_TRANSACTION)
    {
        HANDLE_RPC_ROUTE(CMvRPCRouteGetTransaction, CMvRPCRouteGetTransactionRet);
    }

    if(type == CMvRPCRoute::DBP_RPCROUTE_GET_FORK_HEIGHT)
    {
        HANDLE_RPC_ROUTE(CMvRPCRouteGetForkHeight, CMvRPCRouteGetForkHeightRet);
    }

    if(type == CMvRPCRoute::DBP_RPCROUTE_SEND_TRANSACTION)
    {
        HANDLE_RPC_ROUTE(CMvRPCRouteSendTransaction, CMvRPCRouteSendTransactionRet);
    }
}

//