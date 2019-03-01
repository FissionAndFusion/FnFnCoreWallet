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
    pWallet = NULL;
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
    pIoCompltUntil = NULL;
    sessionCount = 0;
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

    if(!WalleveGetObject("rpcmod", pRPCMod))
    {
        WalleveLog("Failed to request rpc mod\n");
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
            RespondConnected(event);

            for(const auto& virtualevent : mapPeerEvent)
            {
                std::string session(event.strSessionId);
                CMvEventDbpAdded eventAdd(session);
                eventAdd.data.name = "event";
                eventAdd.data.anyAddedObj = virtualevent.second;
                return pDbpServer->DispatchEvent(&eventAdd);
            }

            if(mapPeerEvent.size() == 0)
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
       
        mapPeerEvent[event.nNonce] = eventVPeer;
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
        
        mapPeerEvent.erase(event.nNonce);
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
        CMvEventPeerSubscribe eventUpSub(event.nNonce, event.hashFork);
        FilterThisSubscribeFork(event, eventUpSub);

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
        pVirtualPeerNet->DispatchEvent(&eventActive);

        mapPeerEvent[eventActive.nNonce] = event.data;
        PushEvent(event.data);
    }

    if(event.data.type == CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_DEACTIVE)
    {
        CMvEventPeerDeactive eventDeactive(0);
        ss >> eventDeactive;   
        pVirtualPeerNet->DispatchEvent(&eventDeactive);

        mapPeerEvent.erase(eventDeactive.nNonce);
        PushEvent(event.data);
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

        if(IsMyFork(eventTx.hashFork) && 
            IsThisNodeData(eventTx.hashFork, eventTx.nNonce, eventTx.data.GetHash()))
        {
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

        if(IsMyFork(eventBlock.hashFork) && 
            IsThisNodeData(eventBlock.hashFork, eventBlock.nNonce, eventBlock.data.GetHash()))
        {
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
    });
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
    if (ret == NULL)
    {
        if (sessionCount == 0 && pIoCompltUntil != NULL)
        {
            CMvRPCRouteStopRet stopRet;
            Completion(pIoCompltUntil, stopRet);
            pService->Shutdown();
        }
    }

    if (ret != NULL)
    {
        if (sessionCount == 0 && pIoCompltUntil != NULL)
        {
            CMvRPCRouteStopRet stopRet;
            Completion(pIoCompltUntil, stopRet);
            pService->Shutdown();
        }
    }
}

void CDbpService::RPCRootHandle(CMvRPCRouteGetForkCount* data, CMvRPCRouteGetForkCountRet* ret)
{
    if (ret == NULL)
    {
       if (sessionCount == 0 && pIoCompltUntil != NULL)
        {
            CMvRPCRouteGetForkCountRet ret;
            ret.count = pService->GetForkCount();
            Completion(pIoCompltUntil, ret);
        }
        return;
    }

    if (ret != NULL)
    {
        CMvRPCRouteGetForkCountRet getForkCountRetIn = *((CMvRPCRouteGetForkCountRet*)ret);
        int count = 0;

        uint64 nNonce = getForkCountRetIn.nNonce;
        auto compare = [nNonce](std::pair<uint64, boost::any>& element) { return nNonce == element.first; };
        auto iter = std::find_if(queCount.begin(), queCount.end(), compare);
        if(iter != queCount.end() && (iter->second).type() == typeid(CMvRPCRouteGetForkCountRet))
        {
            int& c = boost::any_cast<CMvRPCRouteGetForkCountRet&>(iter->second).count;
            c = c + getForkCountRetIn.count;
            count = c;
        }

        if (sessionCount == 0 && pIoCompltUntil != NULL)
        {
            CMvRPCRouteGetForkCountRet ret;
            ret.count = pService->GetForkCount() + count;
            Completion(pIoCompltUntil, ret);
        }
        return;
    }
}

void CDbpService::SwrapForks(std::vector<std::pair<uint256, CProfile>>& vFork, std::vector<CMvRPCProfile>& vRpcFork)
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
    std::sort(vFork.begin(), vFork.end(), [](CMvRPCProfile& i, CMvRPCProfile& j) { return i.strHex > j.strHex; });
    auto it = std::unique(vFork.begin(), vFork.end(), [](CMvRPCProfile& i, CMvRPCProfile& j) { return i.strHex == j.strHex; });
    vFork.resize(std::distance(vFork.begin(), it));
}

void CDbpService::RPCRootHandle(CMvRPCRouteListFork* data, CMvRPCRouteListForkRet* ret)
{
    if(ret == NULL)
    {
       if (sessionCount == 0 && pIoCompltUntil != NULL)
        {
            CMvRPCRouteListForkRet ret;
            std::vector<std::pair<uint256,CProfile>> vFork;
            pService->ListFork(vFork, data->fAll);
            std::vector<CMvRPCProfile> vRpcFork;
            SwrapForks(vFork, vRpcFork);
            ret.vFork.insert(ret.vFork.end(), vRpcFork.begin(), vRpcFork.end());
            Completion(pIoCompltUntil, ret);
        }
        return;
    }

    if(ret != NULL)
    {
        CMvRPCRouteListForkRet listForkRetIn = *((CMvRPCRouteListForkRet*)ret);
        std::vector<CMvRPCProfile> vTempFork;
        uint64 nNonce = listForkRetIn.nNonce;
        auto compare = [nNonce](std::pair<uint64, boost::any>& element) { return nNonce == element.first; };
        auto iter = std::find_if(queCount.begin(), queCount.end(), compare);
        if (iter != queCount.end() && (iter->second).type() == typeid(CMvRPCRouteListForkRet))
        {
            auto& vForkSelf = boost::any_cast<CMvRPCRouteListForkRet&>(iter->second).vFork;
            vForkSelf.insert(vForkSelf.end(), listForkRetIn.vFork.begin(), listForkRetIn.vFork.end());
            ListForkUnique(vForkSelf);
            vTempFork.insert(vTempFork.end(), vForkSelf.begin(), vForkSelf.end());
        }

        if (sessionCount == 0 && pIoCompltUntil != NULL)
        {
            CMvRPCRouteListForkRet retOut;
            std::vector<std::pair<uint256, CProfile>> vFork;
            pService->ListFork(vFork, data->fAll);
            std::vector<CMvRPCProfile> vRpcFork;
            SwrapForks(vFork, vRpcFork);
            retOut.vFork.insert(retOut.vFork.end(), vTempFork.begin(), vTempFork.end());
            retOut.vFork.insert(retOut.vFork.end(), vRpcFork.begin(), vRpcFork.end());
            ListForkUnique(retOut.vFork);
            Completion(pIoCompltUntil, retOut);
        }
        return;
    }
}

void CDbpService::RPCRootHandle(CMvRPCRouteGetBlockLocation* data, CMvRPCRouteGetBlockLocationRet* ret)
{
    if (ret == NULL)
    {
        if (sessionCount == 0 && pIoCompltUntil != NULL)
        {
            CMvRPCRouteGetBlockLocationRet getBlockLocationRet;
            getBlockLocationRet.nNonce = data->nNonce;
            getBlockLocationRet.type = data->type;
            getBlockLocationRet.strFork = "";
            getBlockLocationRet.height = 0;
            Completion(pIoCompltUntil, getBlockLocationRet);
            return;
        }

        if (sessionCount != 0)
        {
            auto vRawData = RPCRouteRetToStream(*data);
            PushMsgToChild(vRawData, data->type);
            return;
        }
    }

    if (ret != NULL)
    {
        if(!ret->strFork.empty() && pIoCompltUntil != NULL)
        {
            CMvRPCRouteGetBlockLocationRet getBlockLocationRet;
            getBlockLocationRet.nNonce = data->nNonce;
            getBlockLocationRet.type = data->type;
            getBlockLocationRet.strFork = ret->strFork;
            getBlockLocationRet.height = ret->height;
            Completion(pIoCompltUntil, getBlockLocationRet);
            return;
        }

        if (sessionCount == 0 && pIoCompltUntil != NULL)
        {
            CMvRPCRouteGetBlockLocationRet getBlockLocationRet;
            getBlockLocationRet.nNonce = data->nNonce;
            getBlockLocationRet.type = data->type;
            getBlockLocationRet.strFork = "";
            getBlockLocationRet.height = 0;
            Completion(pIoCompltUntil, getBlockLocationRet);
            return;
        }

        if (sessionCount != 0)
        {
            auto vRawData = RPCRouteRetToStream(*data);
            PushMsgToChild(vRawData, data->type);
            return;
        }
    }
}

void CDbpService::RPCRootHandle(CMvRPCRouteGetBlockCount* data, CMvRPCRouteGetBlockCountRet* ret)
{
    if(ret == NULL)
    {
        if (sessionCount == 0 && pIoCompltUntil != NULL)
        {
            CMvRPCRouteGetBlockCountRet getBlockCountRet;
            getBlockCountRet.nNonce = data->nNonce;
            getBlockCountRet.type = data->type;
            getBlockCountRet.exception = 2;
            getBlockCountRet.height = 0;
            Completion(pIoCompltUntil, getBlockCountRet);
            return;
        }

        if (sessionCount != 0)
        {
            auto vRawData = RPCRouteRetToStream(*data);
            PushMsgToChild(vRawData, data->type);
            return;
        }
    }

    if(ret != NULL)
    {
        if(ret->exception == 0 && pIoCompltUntil != NULL)
        {
            CMvRPCRouteGetBlockCountRet getBlockCountRet;
            getBlockCountRet.nNonce = data->nNonce;
            getBlockCountRet.type = data->type;
            getBlockCountRet.exception = ret->exception;
            getBlockCountRet.height = ret->height;
            Completion(pIoCompltUntil, getBlockCountRet);
            return;
        }

        if (sessionCount == 0 && pIoCompltUntil != NULL)
        {
            CMvRPCRouteGetBlockCountRet getBlockCountRet;
            getBlockCountRet.nNonce = data->nNonce;
            getBlockCountRet.type = data->type;
            getBlockCountRet.exception = 2;
            getBlockCountRet.height = 0;
            Completion(pIoCompltUntil, getBlockCountRet);
            return;
        }

        if (sessionCount != 0)
        {
            auto vRawData = RPCRouteRetToStream(*data);
            PushMsgToChild(vRawData, data->type);
            return;
        }
    }
}

void CDbpService::RPCRootHandle(CMvRPCRouteGetBlockHash * data, CMvRPCRouteGetBlockHashRet* ret)
{
    if(ret == NULL)
    {
        if (sessionCount == 0 && pIoCompltUntil != NULL)
        {
            CMvRPCRouteGetBlockHashRet getBlockHashRet;
            getBlockHashRet.nNonce = data->nNonce;
            getBlockHashRet.type = data->type;
            getBlockHashRet.exception = 2;
            Completion(pIoCompltUntil, getBlockHashRet);
            return;
        }

        if (sessionCount != 0)
        {
            auto vRawData = RPCRouteRetToStream(*data);
            PushMsgToChild(vRawData, data->type);
            return;
        }
    }

    if(ret != NULL)
    {
        if((ret->exception == 0 || ret->exception == 3) && pIoCompltUntil != NULL)
        {
            CMvRPCRouteGetBlockHashRet getBlockHashRet;
            getBlockHashRet.nNonce = data->nNonce;
            getBlockHashRet.type = data->type;
            getBlockHashRet.exception = ret->exception;
            getBlockHashRet.vHash = ret->vHash;
            Completion(pIoCompltUntil, getBlockHashRet);
            return;
        }

        if (sessionCount == 0 && pIoCompltUntil != NULL)
        {
            CMvRPCRouteGetBlockHashRet getBlockHashRet;
            getBlockHashRet.nNonce = data->nNonce;
            getBlockHashRet.type = data->type;
            getBlockHashRet.exception = 2;
            Completion(pIoCompltUntil, getBlockHashRet);
            return;
        }

        if (sessionCount != 0)
        {
            auto vRawData = RPCRouteRetToStream(*data);
            PushMsgToChild(vRawData, data->type);
            return;
        }
    }
}

void CDbpService::RPCRootHandle(CMvRPCRouteGetBlock* data, CMvRPCRouteGetBlockRet* ret)
{
    auto vRawData = RPCRouteRetToStream(*data);

    if (ret == NULL)
    {
        if (sessionCount == 0 && pIoCompltUntil != NULL)
        {
            CMvRPCRouteGetBlockRet getBlockRet;
            getBlockRet.nNonce = data->nNonce;
            getBlockRet.type = data->type;
            getBlockRet.exception = 1;
            Completion(pIoCompltUntil, getBlockRet);
            return;
        }

        if (sessionCount != 0)
        {
            PushMsgToChild(vRawData, data->type);
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
            Completion(pIoCompltUntil, getBlockRet);
            return;
        }

        if (sessionCount == 0 && pIoCompltUntil != NULL)
        {
            getBlockRet.exception = 1;
            Completion(pIoCompltUntil, getBlockRet);
            return;
        }

        if (sessionCount != 0)
        {
            PushMsgToChild(vRawData, data->type);
            return;
        }
    }
}


void CDbpService::RPCRootHandle(CMvRPCRouteGetTxPool* data, CMvRPCRouteGetTxPoolRet* ret)
{
    auto vRawData = RPCRouteRetToStream(*data);

    if(ret == NULL)
    {
        if (sessionCount == 0 && pIoCompltUntil != NULL)
        {
            CMvRPCRouteGetTxPoolRet getTxPoolRet;
            getTxPoolRet.nNonce = data->nNonce;
            getTxPoolRet.type = data->type;
            getTxPoolRet.exception = 2;
            Completion(pIoCompltUntil, getTxPoolRet);
            return;
        }

        if (sessionCount != 0)
        {
            PushMsgToChild(vRawData, data->type);
            return;
        }
    }

    if(ret != NULL)
    {
        CMvRPCRouteGetTxPoolRet getTxPoolRet;
        getTxPoolRet.nNonce = data->nNonce;
        getTxPoolRet.type = data->type;

        if (ret->exception == 0 && pIoCompltUntil != NULL)
        {
            getTxPoolRet.exception = ret->exception;
            getTxPoolRet.vTxPool = ret->vTxPool;
            Completion(pIoCompltUntil, getTxPoolRet);
            return;
        }

        if (sessionCount == 0)
        {
            getTxPoolRet.exception = 2;
            Completion(pIoCompltUntil, getTxPoolRet);
            return;
        }

        if (sessionCount != 0)
        {
            PushMsgToChild(vRawData, data->type);
            return;
        }
    }
}

void CDbpService::RPCRootHandle(CMvRPCRouteGetTransaction* data, CMvRPCRouteGetTransactionRet* ret)
{
    auto vRawData = RPCRouteRetToStream(*data);

    if(ret == NULL)
    {
        if (sessionCount == 0 && pIoCompltUntil != NULL)
        {
            CMvRPCRouteGetTransactionRet getTransactionRet;
            getTransactionRet.nNonce = data->nNonce;
            getTransactionRet.type = data->type;
            getTransactionRet.exception = 1;
            Completion(pIoCompltUntil, getTransactionRet);
            return;
        }

        if (sessionCount != 0)
        {
            PushMsgToChild(vRawData, data->type);
            return;
        }
    }

    if(ret != NULL)
    {
        CMvRPCRouteGetTransactionRet getTransactionRet;
        getTransactionRet.nNonce = data->nNonce;
        getTransactionRet.type = data->type;

        if (ret->exception == 0 && pIoCompltUntil != NULL)
        {
            getTransactionRet.exception = ret->exception;
            getTransactionRet.tx = ret->tx;
            getTransactionRet.strFork = ret->strFork;
            getTransactionRet.nDepth = ret->nDepth;
            Completion(pIoCompltUntil, getTransactionRet);
            return;
        }

        if (sessionCount == 0)
        {
            getTransactionRet.exception = 2;
            Completion(pIoCompltUntil, getTransactionRet);
            return;
        }

        if (sessionCount != 0)
        {
            PushMsgToChild(vRawData, data->type);
            return;
        }
    }
}

void CDbpService::RPCRootHandle(CMvRPCRouteGetForkHeight* data, CMvRPCRouteGetForkHeightRet* ret)
{
    auto vRawData = RPCRouteRetToStream(*data);

    if(ret == NULL)
    {
        if (sessionCount == 0 && pIoCompltUntil != NULL)
        {
            CMvRPCRouteGetForkHeightRet getForkHeightRet;
            getForkHeightRet.nNonce = data->nNonce;
            getForkHeightRet.type = data->type;
            getForkHeightRet.exception = 2;
            Completion(pIoCompltUntil, getForkHeightRet);
            return;
        }

        if (sessionCount != 0)
        {
            PushMsgToChild(vRawData, data->type);
            return;
        }
    }

    if(ret != NULL)
    {
        CMvRPCRouteGetForkHeightRet getForkHeightRet;
        getForkHeightRet.nNonce = data->nNonce;
        getForkHeightRet.type = data->type;

        if (ret->exception == 0 && pIoCompltUntil != NULL)
        {
            getForkHeightRet.exception = ret->exception;
            getForkHeightRet.height = ret->height;
            Completion(pIoCompltUntil, getForkHeightRet);
            return;
        }

        if (sessionCount == 0)
        {
            getForkHeightRet.exception = 2;
            Completion(pIoCompltUntil, getForkHeightRet);
            return;
        }

        if (sessionCount != 0)
        {
            PushMsgToChild(vRawData, data->type);
            return;
        }
    }
}

void CDbpService::RPCRootHandle(CMvRPCRouteSendTransaction* data, CMvRPCRouteSendTransactionRet* ret)
{
    auto vRawData = RPCRouteRetToStream(*data);

    if(ret == NULL)
    {
        if (sessionCount == 0 && pIoCompltUntil != NULL)
        {
            CMvRPCRouteSendTransactionRet sendTransactionRet;
            sendTransactionRet.nNonce = data->nNonce;
            sendTransactionRet.type = data->type;
            sendTransactionRet.exception = 1;
            Completion(pIoCompltUntil, sendTransactionRet);
            return;
        }

        if (sessionCount != 0)
        {
            PushMsgToChild(vRawData, data->type);
            return;
        }
    }

    if(ret != NULL)
    {
        CMvRPCRouteSendTransactionRet  sendTransactionRet;
        sendTransactionRet.nNonce = data->nNonce;
        sendTransactionRet.type = data->type;

        if (ret->exception == 0 && pIoCompltUntil != NULL)
        {
            sendTransactionRet.exception = ret->exception;
            Completion(pIoCompltUntil, sendTransactionRet);
            return;
        }

        if (sessionCount == 0)
        {
            sendTransactionRet.exception = 1;
            sendTransactionRet.err = ret->err;
            Completion(pIoCompltUntil, sendTransactionRet);
            return;
        }

        if (sessionCount != 0)
        {
            PushMsgToChild(vRawData, data->type);
            return;
        }
    }
}

void CDbpService::RPCForkHandle(CMvRPCRouteStop* data, CMvRPCRouteStopRet* ret)
{
    if (ret == NULL)
    {
        if (sessionCount == 0)
        {
            CMvRPCRouteResult result;
            result.type = CMvRPCRoute::DBP_RPCROUTE_STOP;
            result.vRawData = RPCRouteRetToStream(*data);
            SendRPCResult(result);
            pService->Shutdown();
        }
    }

    if (ret != NULL)
    {
        if (sessionCount == 0)
        {
            CMvRPCRouteResult result;
            result.type = CMvRPCRoute::DBP_RPCROUTE_STOP;
            result.vRawData = RPCRouteRetToStream(*data);
            SendRPCResult(result);
            pService->Shutdown();
        }
    }
}

void CDbpService::RPCForkHandle(CMvRPCRouteGetForkCount* data, CMvRPCRouteGetForkCountRet* ret)
{
    if (ret == NULL)
    {
        if (sessionCount == 0)
        {
            CMvRPCRouteResult result;
            result.type = CMvRPCRoute::DBP_RPCROUTE_GET_FORK_COUNT;
            result.vRawData = RPCRouteRetToStream(*data);

            CMvRPCRouteGetForkCountRet getForkCountRet;
            getForkCountRet.nNonce = data->nNonce;
            getForkCountRet.count = pService->GetForkCount() - 1;
            result.vData = RPCRouteRetToStream(getForkCountRet);
            SendRPCResult(result);
        }
        return;
    }

    if (ret != NULL)
    {
        CMvRPCRouteGetForkCountRet getForkCountRetIn = *((CMvRPCRouteGetForkCountRet*)ret);
        CMvRPCRouteGetForkCountRet tempRet;
        int count = 0;

        uint64 nNonce = getForkCountRetIn.nNonce;
        auto compare = [nNonce](std::pair<uint64, boost::any>& element) { return nNonce == element.first; };
        auto iter = std::find_if(queCount.begin(), queCount.end(), compare);
        if (iter != queCount.end() && (iter->second).type() == typeid(CMvRPCRouteGetForkCountRet))
        {
            int &c = boost::any_cast<CMvRPCRouteGetForkCountRet&>(iter->second).count;
            c = c + getForkCountRetIn.count;
            count = c;
        }

        if (sessionCount == 0)
        {
            CMvRPCRouteResult result;
            result.type = CMvRPCRoute::DBP_RPCROUTE_GET_FORK_COUNT;
            result.vRawData = RPCRouteRetToStream(*data);

            CMvRPCRouteGetForkCountRet getForkCountRetOut;
            getForkCountRetOut.type = getForkCountRetIn.type;
            getForkCountRetOut.nNonce = getForkCountRetIn.nNonce;
            getForkCountRetOut.count = pService->GetForkCount() - 1 + count;
            result.vData = RPCRouteRetToStream(getForkCountRetOut);
            SendRPCResult(result);
        }
        return;
    }
}

void CDbpService::RPCForkHandle(CMvRPCRouteListFork* data, CMvRPCRouteListForkRet* ret)
{
    if(ret == NULL)
    {
        if (sessionCount == 0)
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
            SwrapForks(vFork, vRpcFork);
            retOut.vFork.insert(retOut.vFork.end(), vRpcFork.begin(), vRpcFork.end());

            result.vData = RPCRouteRetToStream(retOut);
            SendRPCResult(result);
        }
        return;
    }

    if(ret != NULL)
    {
        CMvRPCRouteListForkRet listForkRetIn = *ret;
        std::vector<CMvRPCProfile> vTempFork;
        uint64 nNonce = listForkRetIn.nNonce;
        auto compare = [nNonce](std::pair<uint64, boost::any>& element) { return nNonce == element.first; };
        auto iter = std::find_if(queCount.begin(), queCount.end(), compare);
        if (iter != queCount.end() && (iter->second).type() == typeid(CMvRPCRouteListForkRet))
        {
            auto& vForkSelf = boost::any_cast<CMvRPCRouteListForkRet&>(iter->second).vFork;
            vForkSelf.insert(vForkSelf.end(), listForkRetIn.vFork.begin(), listForkRetIn.vFork.end());
            ListForkUnique(vForkSelf);
            vTempFork.insert(vTempFork.end(), vForkSelf.begin(), vForkSelf.end());
        }

        if (sessionCount == 0)
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
            SwrapForks(vFork, vRpcFork);

            listForkRetOut.vFork.insert(listForkRetOut.vFork.end(), vTempFork.begin(), vTempFork.end());
            listForkRetOut.vFork.insert(listForkRetOut.vFork.end(), vRpcFork.begin(), vRpcFork.end());
            ListForkUnique(listForkRetOut.vFork);
            result.vData = RPCRouteRetToStream(listForkRetOut);
            SendRPCResult(result);
        }
        return;
    }
}

void CDbpService::RPCForkHandle(CMvRPCRouteGetBlockLocation* data, CMvRPCRouteGetBlockLocationRet* ret)
{
    auto vRawData = RPCRouteRetToStream(*data);

    if (ret == NULL)
    {
        if (sessionCount == 0)
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
            return;
        }

        if (sessionCount != 0)
        {
            PushMsgToChild(vRawData, data->type);
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
            return;
        }

        if (sessionCount == 0)
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
            return;
        }

        if (sessionCount != 0)
        {
            PushMsgToChild(vRawData, data->type);
            return;
        }
    }
}

void CDbpService::RPCForkHandle(CMvRPCRouteGetBlockCount* data, CMvRPCRouteGetBlockCountRet* ret)
{
    auto vRawData = RPCRouteRetToStream(*data);

    if (ret == NULL)
    {
        if (sessionCount == 0)
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
            return;
        }

        if (sessionCount != 0)
        {
            PushMsgToChild(vRawData, data->type);
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
            return;
        }

        if (sessionCount == 0)
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
            return;
        }

        if (sessionCount != 0)
        {
            PushMsgToChild(vRawData, data->type);
            return;
        }
    }
}

void CDbpService::RPCForkHandle(CMvRPCRouteGetBlockHash* data, CMvRPCRouteGetBlockHashRet* ret)
{
    auto vRawData = RPCRouteRetToStream(*data);

    if (ret == NULL)
    {
        if (sessionCount == 0)
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
            return;
        }

        if (sessionCount != 0)
        {
            PushMsgToChild(vRawData, data->type);
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
            return;
        }

        if (sessionCount == 0)
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
            return;
        }

        if (sessionCount != 0)
        {
            PushMsgToChild(vRawData, data->type);
            return;
        }
    }
}

void CDbpService::RPCForkHandle(CMvRPCRouteGetBlock* data, CMvRPCRouteGetBlockRet* ret)
{
    auto vRawData = RPCRouteRetToStream(*data);

    if (ret == NULL)
    {
        if (sessionCount == 0)
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
            return;
        }

        if (sessionCount != 0)
        {
            PushMsgToChild(vRawData, data->type);
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
            return;
        }

        if (sessionCount == 0)
        {
            getBlockRet.exception = 1;
            result.vData = RPCRouteRetToStream(getBlockRet);
            SendRPCResult(result);
            return;
        }

        if (sessionCount != 0)
        {
            PushMsgToChild(vRawData, data->type);
            return;
        }
    }
}

void CDbpService::RPCForkHandle(CMvRPCRouteGetTxPool* data, CMvRPCRouteGetTxPoolRet* ret)
{
    auto vRawData = RPCRouteRetToStream(*data);

    if (ret == NULL)
    {
        if (sessionCount == 0)
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
            return;
        }

        if (sessionCount != 0)
        {
            PushMsgToChild(vRawData, data->type);
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
            return;
        }

        if (sessionCount == 0)
        {
            getTxPoolRet.exception = 2;
            result.vData = RPCRouteRetToStream(getTxPoolRet);
            SendRPCResult(result);
            return;
        }

        if (sessionCount != 0)
        {
            PushMsgToChild(vRawData, data->type);
            return;
        }
    }
}

void CDbpService::RPCForkHandle(CMvRPCRouteGetTransaction* data, CMvRPCRouteGetTransactionRet* ret)
{
    auto vRawData = RPCRouteRetToStream(*data);

    if (ret == NULL)
    {
        if (sessionCount == 0)
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
            return;
        }

        if (sessionCount != 0)
        {
            PushMsgToChild(vRawData, data->type);
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
            return;
        }

        if (sessionCount == 0) 
        {
            getTransactionRet.exception = 2;
            result.vData = RPCRouteRetToStream(getTransactionRet);
            SendRPCResult(result);
            return;
        }

        if (sessionCount != 0)
        {
            PushMsgToChild(vRawData, data->type);
            return;
        }
    }
}


void CDbpService::RPCForkHandle(CMvRPCRouteGetForkHeight* data, CMvRPCRouteGetForkHeightRet* ret)
{
    auto vRawData = RPCRouteRetToStream(*data);

    if (ret == NULL)
    {
        if (sessionCount == 0)
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
            return;
        }

        if (sessionCount != 0)
        {
            PushMsgToChild(vRawData, data->type);
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
            return;
        }

        if (sessionCount == 0)
        {
            getForkHeightRet.exception = 2;
            result.vData = RPCRouteRetToStream(getForkHeightRet);
            SendRPCResult(result);
            return;
        }

        if (sessionCount != 0)
        {
            PushMsgToChild(vRawData, data->type);
            return;
        }
    }
}

void CDbpService::RPCForkHandle(CMvRPCRouteSendTransaction* data, CMvRPCRouteSendTransactionRet* ret)
{
    auto vRawData = RPCRouteRetToStream(*data);

    if (ret == NULL)
    {
        if (sessionCount == 0)
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
            return;
        }

        if (sessionCount != 0)
        {
            PushMsgToChild(vRawData, data->type);
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
            return;
        }

        if (sessionCount == 0)
        {
            sendTransactionRet.exception = 1;
            result.vData = RPCRouteRetToStream(sendTransactionRet);
            SendRPCResult(result);
            return;
        }

        if (sessionCount != 0)
        {
            PushMsgToChild(vRawData, data->type);
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

void CDbpService::PushMsgToChild(std::vector<uint8>& data, int& type)
{
    if (!vRPCTopicIds.empty())
    {
        auto id = vRPCTopicIds.back();
        PushRPCOnece(id, data, type);
        vRPCTopicIds.pop_back();
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

void CDbpService::PushRPCOnece(std::string id, std::vector<uint8>& data, int type)
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

void CDbpService::Completion(CIOCompletionUntil* ioCompltUntil, boost::any obj)
{
    ioCompltUntil->obj = obj;
    ioCompltUntil->Completed(true);
}

void CDbpService::CreateCompletion(uint64 nNonce, walleve::CIOCompletionUntil* ptr)
{
    std::pair<uint64, std::shared_ptr<walleve::CIOCompletionUntil>> pair(nNonce, ptr);
    vCompletionPtr.push_back(pair);
}

void CDbpService::CompletionByNonce(uint64& nNonce, boost::any obj)
{
    auto compare = [nNonce](std::pair<uint64, std::shared_ptr<walleve::CIOCompletionUntil>>& element) {
        return nNonce == element.first;
    };
    auto iter = std::find_if(vCompletionPtr.begin(), vCompletionPtr.end(), compare);
    if (iter != vCompletionPtr.end())
    {
        iter->second->obj = obj;
        iter->second->Completed(true);
    }
}

void CDbpService::DeleteCompletionByNonce(uint64 nNonce)
{
    auto compare = [nNonce](std::pair<uint64, std::shared_ptr<walleve::CIOCompletionUntil>>& element) {
        return nNonce == element.first;
    };
    auto iter = std::find_if(vCompletionPtr.begin(), vCompletionPtr.end(), compare);
    if (iter != vCompletionPtr.end())
    {
        vCompletionPtr.erase(iter);
    }
}

void CDbpService::InitRPCTopicIds()
{
    vRPCTopicIds.clear();
    const auto& allIds = mapTopicIds[RPC_CMD_TOPIC];
    for (const auto id : allIds)
    {
        auto it = mapIdSubedSession.find(id);
        if (it != mapIdSubedSession.end())
        {
            vRPCTopicIds.push_back(id);
        }
    }
    std::cout << "[<] topic id count:" << vRPCTopicIds.size() << std::endl;
}

void CDbpService::InitSessionCount()
{
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
    sessionCount = setSession.size();
    std::cout << "[<] session count:" << setSession.size() << std::endl;
}

bool CDbpService::HandleEvent(CMvEventRPCRouteStop& event)
{
    event.data.type = CMvRPCRoute::DBP_RPCROUTE_STOP;
    pIoCompltUntil = event.data.pIoCompltUntil;
    CMvRPCRouteStopRet stopRet;
    InsertQueCount(event.data.nNonce, stopRet);

    InitSessionCount();
    if(sessionCount > 0)
    {
        auto data = RPCRouteRetToStream(event.data);
        PushRPC(data, CMvRPCRoute::DBP_RPCROUTE_STOP);
        return true;
    }

    RPCRootHandle(&event.data, NULL);
    return true;
}

bool CDbpService::HandleEvent(CMvEventRPCRouteGetForkCount& event)
{
    event.data.type = CMvRPCRoute::DBP_RPCROUTE_GET_FORK_COUNT;
    pIoCompltUntil = event.data.pIoCompltUntil;
    CMvRPCRouteGetForkCountRet getForkCountRet;
    getForkCountRet.count = 0;
    InsertQueCount(event.data.nNonce, getForkCountRet);

    InitSessionCount();
    if(sessionCount > 0)
    {
        auto data = RPCRouteRetToStream(event.data);
        PushRPC(data, CMvRPCRoute::DBP_RPCROUTE_GET_FORK_COUNT);
        return true;
    }
    RPCRootHandle(&event.data, NULL);
    return true;
}

bool CDbpService::HandleEvent(CMvEventRPCRouteListFork& event)
{
    event.data.type = CMvRPCRoute::DBP_RPCROUTE_LIST_FORK;
    pIoCompltUntil = event.data.pIoCompltUntil;
    CMvRPCRouteListForkRet listForkRet;
    InsertQueCount(event.data.nNonce, listForkRet);

    InitSessionCount();
    if(sessionCount > 0)
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
    pIoCompltUntil = event.data.pIoCompltUntil;

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
        Completion(pIoCompltUntil, getBlockLocationRet);
        return true;
    }

    InitRPCTopicIds();
    InitSessionCount();
    InsertQueCount(event.data.nNonce, getBlockLocationRet);
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
    pIoCompltUntil = event.data.pIoCompltUntil;

    CMvRPCRouteGetBlockCountRet getBlockCountRet;
    getBlockCountRet.nNonce = event.data.nNonce;

    uint256 hashFork;
    if (!GetForkHashOfDef(event.data.strFork, hashFork))
    {
        getBlockCountRet.height = 0;
        getBlockCountRet.exception = 1;
        Completion(pIoCompltUntil, getBlockCountRet);
        return true;
    }

    if (pService->HaveFork(hashFork))
    {
        getBlockCountRet.height = pService->GetBlockCount(hashFork);
        getBlockCountRet.exception = 0;
        Completion(pIoCompltUntil, getBlockCountRet);
        return true;
    }

    InitRPCTopicIds();
    InitSessionCount();
    InsertQueCount(event.data.nNonce, getBlockCountRet);
    RPCRootHandle(&event.data, NULL);
    return true;
}

bool CDbpService::HandleEvent(CMvEventRPCRouteGetBlockHash& event)
{
    event.data.type = CMvRPCRoute::DBP_RPCROUTE_GET_BLOCK_HASH;
    pIoCompltUntil = event.data.pIoCompltUntil;

    CMvRPCRouteGetBlockHashRet getBlockHashRet;
    getBlockHashRet.nNonce = event.data.nNonce;

    uint256 hashFork;
    if (!GetForkHashOfDef(event.data.strFork, hashFork))
    {
        getBlockHashRet.exception = 1;
        Completion(pIoCompltUntil, getBlockHashRet);
        return true;
    }

    if (pService->HaveFork(hashFork))
    {
        vector<uint256> vBlockHash;
        if (!pService->GetBlockHash(hashFork, event.data.height, vBlockHash))
        {
            getBlockHashRet.exception = 3;
            Completion(pIoCompltUntil, getBlockHashRet);
            return true;
        }

        for (const uint256& hash : vBlockHash)
        {
            getBlockHashRet.vHash.push_back(hash.GetHex());
        }

        getBlockHashRet.exception = 0;
        Completion(pIoCompltUntil, getBlockHashRet);
        return true;
    }

    InitRPCTopicIds();
    InitSessionCount();
    InsertQueCount(event.data.nNonce, getBlockHashRet);
    RPCRootHandle(&event.data, NULL);
    return true;
}

bool CDbpService::HandleEvent(CMvEventRPCRouteGetBlock& event)
{
    event.data.type = CMvRPCRoute::DBP_RPCROUTE_GET_BLOCK;
    pIoCompltUntil = event.data.pIoCompltUntil;

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
        Completion(pIoCompltUntil, getBlockRet);
        return true;
    }

    InitRPCTopicIds();
    InitSessionCount();
    InsertQueCount(event.data.nNonce, getBlockRet);
    RPCRootHandle(&event.data, NULL);
    return true;
}

bool CDbpService::HandleEvent(CMvEventRPCRouteGetTxPool& event)
{
    event.data.type = CMvRPCRoute::DBP_RPCROUTE_GET_TXPOOL;
    pIoCompltUntil = event.data.pIoCompltUntil;

    CMvRPCRouteGetTxPoolRet getTxPoolRet;
    getTxPoolRet.nNonce = event.data.nNonce;

    uint256 hashFork;
    if (!GetForkHashOfDef(event.data.strFork, hashFork))
    {
        getTxPoolRet.exception = 1;
        Completion(pIoCompltUntil, getTxPoolRet);
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
        Completion(pIoCompltUntil, getTxPoolRet);
        return true;
    }

    InitRPCTopicIds();
    InitSessionCount();
    InsertQueCount(event.data.nNonce, getTxPoolRet);
    RPCRootHandle(&event.data, NULL);
    return true;
}

bool CDbpService::HandleEvent(CMvEventRPCRouteGetTransaction& event) 
{
    event.data.type = CMvRPCRoute::DBP_RPCROUTE_GET_TRANSACTION;
    pIoCompltUntil = event.data.pIoCompltUntil;

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
        Completion(pIoCompltUntil, getTransactionRet);
        return true;
    }

    InitRPCTopicIds();
    InitSessionCount();
    InsertQueCount(event.data.nNonce, getTransactionRet);
    RPCRootHandle(&event.data, NULL);
    return true;
}

bool CDbpService::HandleEvent(CMvEventRPCRouteGetForkHeight& event)
{
    event.data.type = CMvRPCRoute::DBP_RPCROUTE_GET_FORK_HEIGHT;
    pIoCompltUntil = event.data.pIoCompltUntil;

    CMvRPCRouteGetForkHeightRet getForkHeightRet;
    getForkHeightRet.nNonce = event.data.nNonce;

    uint256 hashFork;
    if (!GetForkHashOfDef(event.data.strFork, hashFork))
    {
        getForkHeightRet.exception = 1;
        Completion(pIoCompltUntil, getForkHeightRet);
        return true;
    }

    if (pService->HaveFork(hashFork))
    {
        getForkHeightRet.exception = 0;
        getForkHeightRet.height = pService->GetForkHeight(hashFork);
        Completion(pIoCompltUntil, getForkHeightRet);
        return true;
    }

    InitRPCTopicIds();
    InitSessionCount();
    InsertQueCount(event.data.nNonce, getForkHeightRet);
    RPCRootHandle(&event.data, NULL);
    return true;
}

bool CDbpService::HandleEvent(CMvEventRPCRouteSendTransaction& event)
{
    event.data.type = CMvRPCRoute::DBP_RPCROUTE_SEND_TRANSACTION;
    pIoCompltUntil = event.data.pIoCompltUntil;

    CMvRPCRouteSendTransactionRet sendTransactionRet;
    sendTransactionRet.nNonce = event.data.nNonce;

    MvErr err = pService->SendTransaction(event.data.rawTx);
    if (err == MV_OK)
    {
        sendTransactionRet.exception = 0;
        Completion(pIoCompltUntil, sendTransactionRet);
        return true;
    }

    InitRPCTopicIds();
    InitSessionCount();
    InsertQueCount(event.data.nNonce, sendTransactionRet);
    RPCRootHandle(&event.data, NULL);
    return true;
}

bool CDbpService::HandleAddedEventStop(CMvEventRPCRouteAdded& event, walleve::CWalleveBufStream& ss)
{
    CMvRPCRouteStop stop;
    ss >> stop;
    CMvRPCRouteStopRet stopRet;
    InsertQueCount(stop.nNonce, stopRet);
    if (sessionCount > 0)
    {
        PushRPC(event.data.vData, event.data.type);
        return true;
    }
    RPCForkHandle(&stop, NULL);
    return true;
}

bool CDbpService::HandleAddedEventGetForkCount(CMvEventRPCRouteAdded& event, walleve::CWalleveBufStream& ss)
{
    CMvRPCRouteGetForkCount getForkCount;
    ss >> getForkCount;
    CMvRPCRouteGetForkCountRet getForkCountRet;
    getForkCountRet.count = 0;
    InsertQueCount(getForkCount.nNonce, getForkCountRet);
    if (sessionCount > 0)
    {
        PushRPC(event.data.vData, event.data.type);
        return true;
    }
    RPCForkHandle(&getForkCount, NULL);
    return true;
}

bool CDbpService::HandleAddedEventListFork(CMvEventRPCRouteAdded& event, walleve::CWalleveBufStream& ss)
{
    CMvRPCRouteListFork listFork;
    ss >> listFork;
    CMvRPCRouteListForkRet listForkRet;
    InsertQueCount(listFork.nNonce, listForkRet);
    if (sessionCount > 0)
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

    InitRPCTopicIds();
    InsertQueCount(getBlockLocation.nNonce, getBlockLocationRet);

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

    InitRPCTopicIds();
    InsertQueCount(getBlockCount.nNonce, getBlockCountRet);

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

    InitRPCTopicIds();
    InsertQueCount(getBlockHash.nNonce, getBlockHashRet);

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

    InitRPCTopicIds();
    InsertQueCount(getBlock.nNonce, getBlockRet);

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

    InitRPCTopicIds();
    InsertQueCount(getTxPool.nNonce, getTxPoolRet);

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

    InitRPCTopicIds();
    InsertQueCount(getTransaction.nNonce, getTransactionRet);

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

    InitRPCTopicIds();
    InsertQueCount(getForkHeight.nNonce, getForkHeightRet);

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

    InitRPCTopicIds();
    InsertQueCount(sendTransaction.nNonce, sendTransactionRet);

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
    InitSessionCount();

    walleve::CWalleveBufStream ss;
    ss.Write((char*)event.data.vData.data(), event.data.vData.size());

    if (event.data.type == CMvRPCRoute::DBP_RPCROUTE_STOP)
    {
        return HandleAddedEventStop(event, ss);
    }

    if(event.data.type == CMvRPCRoute::DBP_RPCROUTE_GET_FORK_COUNT)
    {
        return HandleAddedEventGetForkCount(event, ss);
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
}

void CDbpService::HandleRPCRoute(CMvEventDbpMethod& event)
{
    sessionCount--;
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

    if (type == CMvRPCRoute::DBP_RPCROUTE_GET_FORK_COUNT)
    {
        HANDLE_RPC_ROUTE(CMvRPCRouteGetForkCount, CMvRPCRouteGetForkCountRet);
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