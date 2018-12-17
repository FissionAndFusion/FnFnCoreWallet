// Copyright (c) 2017-2018 The Multiverse developers
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
    pWallet = NULL;
    pDbpServer = NULL;
    pNetChannel = NULL;
    pVirtualPeerNet = NULL;

    std::unordered_map<std::string, IdsType> temp_map = 
        boost::assign::map_list_of(ALL_BLOCK_TOPIC, std::set<std::string>())
                                  (ALL_TX_TOPIC,    std::set<std::string>())
                                  (SYS_CMD_TOPIC,   std::set<std::string>())
                                  (TX_CMD_TOPIC,    std::set<std::string>())
                                  (BLOCK_CMD_TOPIC, std::set<std::string>())
                                  (CHANGED_TOPIC,   std::set<std::string>())
                                  (REMOVED_TOPIC,   std::set<std::string>());

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

    if (!WalleveGetObject("wallet", pWallet))
    {
        WalleveError("Failed to request wallet\n");
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

    return true;
}

void CDbpService::WalleveHandleDeinitialize()
{
    pDbpServer = NULL;
    pService = NULL;
    pCoreProtocol = NULL;
    pWallet = NULL;
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

void CDbpService::SetSupportForks(const std::vector<uint256>& vForks)
{
    vSupportFork = vForks;
}

bool CDbpService::HandleEvent(CMvEventDbpPong& event)
{
    (void)event;
    return true;
}

bool CDbpService::HandleEvent(CMvEventDbpBroken& event)
{
    mapSessionChildNodeForks.erase(event.strSessionId);
    return true;
}

bool CDbpService::HandleEvent(CMvEventDbpRemoveSession& event)
{
    RemoveSession(event.data.session);
    return true;
}

static std::string GetHex(std::string data)
{
    int n = 2 * data.length() + 1;
    std::string ret;
    const char c_map[16] = {'0', '1', '2', '3', '4', '5', '6', '7',
                            '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

    ret.reserve(n);
    for (const unsigned char &c : data)
    {
        ret.push_back(c_map[c >> 4]);
        ret.push_back(c_map[c & 15]);
    }

    return ret;
}

static void print_block(const CBlock &block)
{
   

}

static void print_tx(const CTransaction &tx)
{
    
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
            std::cout << "[<] Recved Connect [dbpservice]\n"; 
            
            RespondConnected(event);

            std::cout << "[>] Sent Connected " << event.strSessionId << " [dbpservice]\n";

            std::cout << "[>] Cached Event Size " << vCacheEvent.size() << " [dbpservice]\n";

            for(const auto& virtualevent : vCacheEvent)
            {
                std::string session(event.strSessionId);
                CMvEventDbpAdded eventAdd(session);
                eventAdd.data.name = "event";
                eventAdd.data.anyAddedObj = virtualevent;
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
    int blockHeight;

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
    std::vector<std::pair<uint256,int>> vAncestors;
    std::vector<std::pair<int,uint256>> vSublines;
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
        int nHeight = 0;
        pService->GetBlock(vAncestors[i].first,block,tempFork,nHeight);
        forkAncestors.push_back(std::make_pair(vAncestors[i].first,block.hashPrev));
    }

    path = forkAncestors;
    CBlock block;
    uint256 tempFork;
    int nHeight = 0;
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
        std::cerr << "connect fork hash is not a fork hash.\n";
        return false;
    }

    if (IsEmpty(blockHash))
    {
        blockHash = pCoreProtocol->GetGenesisBlockHash();
    }

    int blockHeight = 0;
    uint256 tempForkHash;
    if (!pService->GetBlockLocation(blockHash, tempForkHash, blockHeight))
    {
        std::cerr << "GetBlockLocation failed\n";
        return false;
    }

    if(!CalcForkPoints(connectForkHash))
    {
        std::cerr << "CalcForkPoint failed.\n";
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
            int height;
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

        auto& vSubForks = eventSub.data;
        for(const auto& fork : vSubForks)
        {
            auto key = ForkNonceKeyType(fork, eventSub.nNonce);
            if(mapChildNodeForkCount.find(key) == mapChildNodeForkCount.end())
            {
                mapChildNodeForkCount[key] = 1;
                eventUpSub.data.push_back(fork);
            }
            else
            {
                mapChildNodeForkCount[key]++;
            }
          
        }

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
                std::cout << "#############[rootnode] "  << "Subscribe Fork: [dbpservice]\n";
                for(const auto& fork : eventUpSub.data)
                {
                    std::cout << "Fork ID " << fork.ToString() << " [dbpservice]\n";
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

        auto& vUnSubForks = eventUnSub.data;
        for(const auto& fork : vUnSubForks)
        {
            auto key = ForkNonceKeyType(fork, eventUnSub.nNonce);
            if(mapChildNodeForkCount.find(key) != mapChildNodeForkCount.end())
            {
                if(mapChildNodeForkCount[key] == 1)
                {
                    mapChildNodeForkCount[key] = 0;
                    eventUpUnSub.data.push_back(fork);
                    mapChildNodeForkCount.erase(key);
                }
                else
                {
                    mapChildNodeForkCount[key]--;
                }
            }
        }

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

        if(IsMyFork(eventGetBlocks.hashFork))
        {
            eventGetBlocks.flow = "up";
            eventGetBlocks.sender = "dbpservice";
            pVirtualPeerNet->DispatchEvent(&eventGetBlocks);
        }
        else
        {
            if(IsForkNodeOfSuperNode())
            {
                CMvDbpVirtualPeerNetEvent vpeerEvent;
                vpeerEvent.type = CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_GETBLOCKS;
                vpeerEvent.data = std::vector<uint8>(eventData.begin(), eventData.end());
                SendEventToParentNode(vpeerEvent);
            }

            if(IsRootNodeOfSuperNode())
            {   
                eventGetBlocks.flow = "up";
                eventGetBlocks.sender = "dbpservice";
                pVirtualPeerNet->DispatchEvent(&eventGetBlocks);
            }
        }
    }

    if(type == CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_GETDATA)
    {
        CMvEventPeerGetData eventGetData(0,uint256());
        ss >> eventGetData;

        if(IsMyFork(eventGetData.hashFork))
        {
            eventGetData.flow = "up";
            eventGetData.sender = "dbpservice";
            pVirtualPeerNet->DispatchEvent(&eventGetData);
        }
        else
        {
            if(IsForkNodeOfSuperNode())
            {
                CMvDbpVirtualPeerNetEvent vpeerEvent;
                vpeerEvent.type = CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_GETDATA;
                vpeerEvent.data = std::vector<uint8>(eventData.begin(), eventData.end());
                SendEventToParentNode(vpeerEvent);
            }

            if(IsRootNodeOfSuperNode())
            {
                eventGetData.flow = "up";
                eventGetData.sender = "dbpservice";
                pVirtualPeerNet->DispatchEvent(&eventGetData);
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
    else
    {
        return false;
    }

    return true;
}

void CDbpService::PushBlock(const std::string& forkid, const CMvDbpBlock& block)
{
    const auto& allBlockIds = mapTopicIds[ALL_BLOCK_TOPIC];   
    for (const auto& kv : mapIdSubedSession)
    {
        std::string id = kv.first;
        std::string session = kv.second;

        if (allBlockIds.find(id) != allBlockIds.end())
        {
            CMvEventDbpAdded eventAdded(session);
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
    for (const auto& kv : mapIdSubedSession)
    {
        std::string id = kv.first;
        std::string session = kv.second;

        if (allTxIds.find(id) != allTxIds.end())
        {
            CMvEventDbpAdded eventAdded(session);
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
    int blockHeight = 0;
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
        vCacheEvent.push_back(eventVPeer);

        DeleteCache(event.nNonce, CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_DEACTIVE);
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
        vCacheEvent.push_back(eventVPeer);

        DeleteCache(event.nNonce, CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_ACTIVE);
        PushEvent(eventVPeer);
    }
    
    return true;
}

void CDbpService::DeleteCache(uint64 nNonce, int type)
{
    for (auto it = vCacheEvent.begin(); it != vCacheEvent.end(); ) 
    {
        if ((*it).type == type && (*it).nNonce == nNonce) 
        {
            it = vCacheEvent.erase(it);
        } 
        else 
        {
            ++it;
        }
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
        std::cout << "###################[forknode] generate subscribe event [dbpservice]\n";
        
        CMvEventPeerSubscribe eventUpSub(event.nNonce, event.hashFork);

        std::cout << "nonce " << event.nNonce << " [dbpservice]\n";
        std::cout << "hashfork " << event.hashFork.ToString() << " [dbpservice]\n";
        
        auto& vSubForks = event.data;
        for(const auto& fork : vSubForks)
        {
            auto key = ForkNonceKeyType(fork, event.nNonce);
            
            if(mapThisNodeForkCount.find(key) == mapThisNodeForkCount.end())
            {
                mapThisNodeForkCount[key] = 1;
                eventUpSub.data.push_back(fork);
            }
            else
            {
                mapThisNodeForkCount[key]++;
            }

            std::cout << "[forknode] Subscribe fork " << fork.ToString() << " [dbpservice]\n";
        }

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
        std::cout << "[forknode] generate unSubscribe event [dbpservice]\n";
        
        CMvEventPeerUnsubscribe eventUpUnSub(event.nNonce, event.hashFork);

        auto& vUnSubForks = event.data;
        for(const auto& fork : vUnSubForks)
        {
            auto key = ForkNonceKeyType(fork, event.nNonce);
            
            if(mapThisNodeForkCount.find(key) != mapThisNodeForkCount.end())
            {
                if(mapThisNodeForkCount[key] == 1)
                {
                    mapThisNodeForkCount[key] = 0;
                    eventUpUnSub.data.push_back(fork);
                    mapThisNodeForkCount.erase(key);
                }
                else
                {
                    mapThisNodeForkCount[key]--;
                }
            }
            std::cout << "UnSub Fork: " << fork.ToString() << " [dbpservice]\n";
        }

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
            << event.hashFork.ToString() << " [rootnode dbpservice]\n";
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
        std::cout << "[forknode] generate netclose nonce: " << event.nNonce << " [dbpservice]\n";
        std::cout << "[forknode] generate netclose type: " << event.data << " [dbpservice]\n";
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
        std::cout << "recv active event address: " << ep.address().to_string() << " [dbpservice]\n";
        std::cout << "recv active event nonce: " << eventActive.nNonce << "[dbpservice]\n";

        pVirtualPeerNet->DispatchEvent(&eventActive);

        DeleteCache(event.nNonce, CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_DEACTIVE);
        vCacheEvent.push_back(event.data);
    }

    if(event.data.type == CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_DEACTIVE)
    {
        CMvEventPeerDeactive eventDeactive(0);
        ss >> eventDeactive;   
        
        boost::asio::ip::tcp::endpoint ep;
        eventDeactive.data.ssEndpoint.GetEndpoint(ep);
        std::cout << "recv deactive event address: " << ep.address().to_string() << " [dbpservice]\n";
        
        pVirtualPeerNet->DispatchEvent(&eventDeactive);

        DeleteCache(event.nNonce, CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_ACTIVE);
        vCacheEvent.push_back(event.data);
    }

    if(event.data.type == CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_SUBSCRIBE)
    {
        CMvEventPeerSubscribe eventSub(0, uint256());
        ss >> eventSub;

        if(IsMyFork(eventSub.hashFork))
        {
            eventSub.sender = "dbpservice";
            eventSub.flow = "down";    
            pVirtualPeerNet->DispatchEvent(&eventSub);
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
    }

    if(event.data.type == CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_GETBLOCKS)
    {
        CMvEventPeerGetBlocks eventGetBlocks(0, uint256());
        ss >> eventGetBlocks;

        if(IsMyFork(eventGetBlocks.hashFork))
        {
            eventGetBlocks.sender = "dbpservice";
            eventGetBlocks.flow = "down";
            pVirtualPeerNet->DispatchEvent(&eventGetBlocks);
        }

        PushEvent(event.data);
    }

    if(event.data.type == CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_GETDATA)
    {
        CMvEventPeerGetData eventGetData(0, uint256());
        ss >> eventGetData; 

        if(IsMyFork(eventGetData.hashFork))
        {
            eventGetData.sender = "dbpservice";
            eventGetData.flow = "down";
            pVirtualPeerNet->DispatchEvent(&eventGetData);
        }

        PushEvent(event.data);
    }

    if(event.data.type == CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_INV)
    {
        CMvEventPeerInv eventInv(0, uint256());
        ss >> eventInv;

        if(IsMyFork(eventInv.hashFork))
        {
            std::cout << "[forknode] [<] Peer Inv Fork " << eventInv.hashFork.ToString() << " [dbpservice]\n"; 
            std::cout << "[forknode] [<] Peer Inv Nonce " << eventInv.nNonce << " [dbpservice]\n"; 
            
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

        if(IsMyFork(eventTx.hashFork) && IsThisNodeTx(eventTx))
        {
            std::cout << "[forknode] [<] Peer Tx Fork " << eventTx.hashFork.ToString() << " [dbpservice]\n"; 
            std::cout << "[forknode] [<] Peer Tx Nonce " << eventTx.nNonce << " [dbpservice]\n"; 
            
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

        if(IsMyFork(eventBlock.hashFork) && IsThisNodeBlock(eventBlock))
        {
            std::cout << "[forknode] [<] Peer Block Fork " << eventBlock.hashFork.ToString() << " [dbpservice]\n"; 
            std::cout << "[forknode] [<] Peer Block Nonce " << eventBlock.nNonce << " [dbpservice]\n"; 
            
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
    for(const auto& inv : event.data)
    {
        setInvHash.insert(inv.nHash);
        std::cout << "Get Data Inv Hash " << inv.nHash.ToString() << " [dbpservice]\n";
    }

    std::cout << "Get Data nonce " << nNonce << " [dbpservice]\n";
    std::cout << "Get Data hashFork " << hashFork.ToString() << " [dbpservice]\n";

    mapThisNodeGetData[std::make_pair(hashFork, nNonce)] = setInvHash;
}

bool CDbpService::IsThisNodeBlock(CMvEventPeerBlock& eventBlock)
{
    uint64 nNonce = eventBlock.nNonce;
    uint256& hashFork = eventBlock.hashFork;
    uint256 blockHash = eventBlock.data.GetHash();
    auto pairKey = std::make_pair(hashFork, nNonce);
    
    std::cout << "Pair Key " << pairKey.first.ToString() << " , " << nNonce << " [dbpservice]\n";
    std::cout << "Block Hash " << blockHash.ToString() << " \n";
    
    if(mapThisNodeGetData.find(pairKey) == mapThisNodeGetData.end())
    {
        std::cout << "pair key cannot find \n";
        return false;
    }

    auto& setInvHash = mapThisNodeGetData[pairKey];
    if(setInvHash.find(blockHash) == setInvHash.end())
    {
        std::cout << "blockHash cannot find \n";
        return false;
    }

    return true;
}

bool CDbpService::IsThisNodeTx(CMvEventPeerTx& eventTx)
{
    uint64 nNonce = eventTx.nNonce;
    uint256& hashFork = eventTx.hashFork;
    uint256 txHash = eventTx.data.GetHash();
    auto pairKey = std::make_pair(hashFork, nNonce);

    std::cout << "Pair Key " << pairKey.first.ToString() << " , " << nNonce << " [dbpservice]\n";
    std::cout << "Tx Hash " << txHash.ToString() << " \n";
    
    if(mapThisNodeGetData.find(pairKey) == mapThisNodeGetData.end())
    {
        std::cout << "pair key cannot find \n";
        return false;
    }

    auto& setInvHash = mapThisNodeGetData[pairKey];
    if(setInvHash.find(txHash) == setInvHash.end())
    {
        std::cout << "txHash cannot find \n";
        return false;
    }

    return true;
}

