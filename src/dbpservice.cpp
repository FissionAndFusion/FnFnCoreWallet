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
    pForkManager = NULL;

    std::unordered_map<std::string, IdsType> temp_map = 
        boost::assign::map_list_of(ALL_BLOCK_TOPIC, std::set<std::string>())
                                  (ALL_TX_TOPIC,    std::set<std::string>())
                                  (SYS_CMD_TOPIC,   std::set<std::string>())
                                  (TX_CMD_TOPIC,    std::set<std::string>())
                                  (BLOCK_CMD_TOPIC, std::set<std::string>())
                                  (CHANGED_TOPIC,   std::set<std::string>())
                                  (REMOVED_TOPIC,   std::set<std::string>());

    mapTopicIds = temp_map;
}

CDbpService::~CDbpService()
{
}

bool CDbpService::WalleveHandleInitialize()
{
    if (!WalleveGetObject("coreprotocol", pCoreProtocol))
    {
        WalleveLog("Failed to request coreprotocol\n");
        return false;
    }

    if (!WalleveGetObject("service", pService))
    {
        WalleveLog("Failed to request service\n");
        return false;
    }

    if (!WalleveGetObject("wallet", pWallet))
    {
        WalleveLog("Failed to request wallet\n");
        return false;
    }

    if (!WalleveGetObject("dbpserver", pDbpServer))
    {
        WalleveLog("Failed to request dbpserver\n");
        return false;
    }

    if(!WalleveGetObject("dbpclient",pDbpClient))
    {
        WalleveLog("Failed to request dbpclient\n");
        return false;
    }

    if (!WalleveGetObject("netchannel",pNetChannel))
    {
        WalleveLog("Failed to request peer net datachannel\n");
        return false;
    }

    if (!WalleveGetObject("virtualpeernet",pVirtualPeerNet))
    {
        WalleveLog("Failed to request virtual peer net\n");
        return false;
    }
    
    if(!WalleveGetObject("forkmanager", pForkManager))
    {
        WalleveLog("Failed to request fork manager\n");
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
    pForkManager = NULL;
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

bool CDbpService::HandleEvent(CMvEventDbpAdded& event)
{
    if(true)
    {

    }
    else
    {
        return false;
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
            RespondConnected(event);
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

void CDbpService::HandleSendEvent(CMvEventDbpMethod& event)
{
    int type = boost::any_cast<int>(event.data.params["type"]);
    std::string eventData = boost::any_cast<std::string>(event.data.params["data"]);

    
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

void CDbpService::PushEvent(const CMvDbpVirtualPeerNetEvent& event)
{
    std::string session;
    CMvEventDbpAdded eventAdd(session);
    eventAdd.data.name = "event";
    eventAdd.data.anyAddedObj = event;
    pDbpServer->DispatchEvent(&eventAdd);
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

bool CDbpService::HandleEvent(CMvEventPeerActive& event)
{
    return true;
}

bool CDbpService::HandleEvent(CMvEventPeerDeactive& event)
{
    return true;
}

bool CDbpService::HandleEvent(CMvEventPeerSubscribe& event)
{
    return true;
}

bool CDbpService::HandleEvent(CMvEventPeerUnsubscribe& event)
{
    return true;
}

bool CDbpService::HandleEvent(CMvEventPeerInv& event)
{
    return true;
}

bool CDbpService::HandleEvent(CMvEventPeerBlock& event)
{
    return true;
}

bool CDbpService::HandleEvent(CMvEventPeerTx& event)
{
    return true;
}

bool CDbpService::HandleEvent(CMvEventPeerGetBlocks& event)
{
    return true;
}

bool CDbpService::HandleEvent(CMvEventPeerGetData& event)
{
    return true;
}

bool CDbpService::HandleEvent(CWalleveEventPeerNetReward& event)
{
    return true;
}
    
bool CDbpService::HandleEvent(CWalleveEventPeerNetClose& event)
{
    return true;
}

// from up node Event
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
    }

    if(event.data.type == CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_DEACTIVE)
    {
        CMvEventPeerDeactive eventDeactive(0);
        ss >> eventDeactive;   
        pVirtualPeerNet->DispatchEvent(&eventDeactive);
    }

    if(event.data.type == CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_SUBSCRIBE)
    {
        CMvEventPeerSubscribe eventSub(0, uint256());
        ss >> eventSub;    
        pVirtualPeerNet->DispatchEvent(&eventSub);
    }

    if(event.data.type == CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_UNSUBSCRIBE)
    {
        CMvEventPeerUnsubscribe eventUnSub(0, uint256());
        ss >> eventUnSub;
        pVirtualPeerNet->DispatchEvent(&eventUnSub);
    }

    if(event.data.type == CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_GETBLOCKS)
    {
        CMvEventPeerGetBlocks eventGetBlocks(0, uint256());
        ss >> eventGetBlocks;   
        pVirtualPeerNet->DispatchEvent(&eventGetBlocks);
    }

    if(event.data.type == CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_GETDATA)
    {
        CMvEventPeerGetData eventGetData(0, uint256());
        ss >> eventGetData;   
        pVirtualPeerNet->DispatchEvent(&eventGetData);
    }

    if(event.data.type == CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_INV)
    {
        CMvEventPeerInv eventInv(0, uint256());
        ss >> eventInv;   
        pVirtualPeerNet->DispatchEvent(&eventInv);
    }

    if(event.data.type == CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_TX)
    {
        CMvEventPeerTx eventTx(0, uint256());
        ss >> eventTx;   
        pVirtualPeerNet->DispatchEvent(&eventTx);
    }

    if(event.data.type == CMvDbpVirtualPeerNetEvent::EventType::DBP_EVENT_PEER_BLOCK)
    {
        CMvEventPeerTx eventTx(0, uint256());
        ss >> eventTx;   
        pVirtualPeerNet->DispatchEvent(&eventTx);
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

