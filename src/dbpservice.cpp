// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "dbpservice.h"

#include <boost/assign/list_of.hpp>
#include <boost/lexical_cast.hpp>

using namespace multiverse;

CDbpService::CDbpService()
    : walleve::IIOModule("dbpservice")
{
    pService = NULL;
    pCoreProtocol = NULL;
    pWallet = NULL;
    pDbpServer = NULL;

    std::unordered_map<std::string, bool> temp_map = boost::assign::map_list_of("all-block", true)("all-tx", true)("changed", true)("removed", true);

    mapCurrentTopicExist = temp_map;
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

    return true;
}

void CDbpService::WalleveHandleDeinitialize()
{
    pDbpServer = NULL;
    pService = NULL;
    pCoreProtocol = NULL;
    pWallet = NULL;
}

bool CDbpService::HandleEvent(CMvEventDbpPong& event)
{
    (void)event;
    return true;
}

bool CDbpService::HandleEvent(CMvEventDbpConnect& event)
{
    bool isReconnect = event.data.isReconnect;
    
    if (isReconnect)
    {
        // reply normal
        CMvEventDbpConnected eventConnected(event.strSessionId);
        eventConnected.data.session = event.data.session;
        pDbpServer->DispatchEvent(&eventConnected);
    }
    else
    {
        if (event.data.version != 1)
        {
            // reply failed
            std::vector<int> versions{1};
            CMvEventDbpFailed eventFailed(event.strSessionId);
            eventFailed.data.reason = "001";
            eventFailed.data.versions = versions;
            eventFailed.data.session = event.data.session;
            pDbpServer->DispatchEvent(&eventFailed);
        }
        else
        {
            // reply normal
            CMvEventDbpConnected eventConnected(event.strSessionId);
            eventConnected.data.session = event.data.session;
            pDbpServer->DispatchEvent(&eventConnected);
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
        // reply nosub
        CMvEventDbpNoSub eventNoSub(event.strSessionId);
        eventNoSub.data.id = event.data.id;
        pDbpServer->DispatchEvent(&eventNoSub);
    }
    else
    {
        SubTopic(id, event.strSessionId, topicName);

        // reply ready
        CMvEventDbpReady eventReady(event.strSessionId);
        eventReady.data.id = id;
        pDbpServer->DispatchEvent(&eventReady);
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
    std::string txid = event.data.params["hash"];

    uint256 txHash(txid);
    CTransaction tx;
    uint256 forkHash;
    int blockHeight;

    if (pService->GetTransaction(txHash, tx, forkHash, blockHeight))
    {
        CMvDbpTransaction dbpTx;
        CreateDbpTransaction(tx, dbpTx);

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
    std::string data = event.data.params["data"];

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

        CMvDbpSendTxRet sendTxRet;
        sendTxRet.hash = data;
        sendTxRet.result = "succeed";
        eventResult.data.anyResultObjs.push_back(sendTxRet);

        pDbpServer->DispatchEvent(&eventResult);
    }
    else
    {
        CMvEventDbpMethodResult eventResult(event.strSessionId);
        eventResult.data.id = event.data.id;

        CMvDbpSendTxRet sendTxRet;
        sendTxRet.hash = data;
        sendTxRet.result = "failed";
        sendTxRet.reason = std::string(MvErrString(err));
        eventResult.data.anyResultObjs.push_back(sendTxRet);

        pDbpServer->DispatchEvent(&eventResult);
    }
}

bool CDbpService::IsTopicExist(const std::string& topic)
{
    return mapCurrentTopicExist.find(topic) != mapCurrentTopicExist.end();
}

bool CDbpService::IsHaveSubedTopicOf(const std::string& id)
{
    return mapIdSubedTopic.find(id) != mapIdSubedTopic.end();
}

void CDbpService::SubTopic(const std::string& id, const std::string& session, const std::string& topic)
{
    mapIdSubedTopic.insert(std::make_pair(id, topic));

    if (topic == "all-block")
        setSubedAllBlocksIds.insert(id);
    if (topic == "all-tx")
        setSubedAllTxIds.insert(id);

    mapIdSubedSession.insert(std::make_pair(id, session));
}

void CDbpService::UnSubTopic(const std::string& id)
{
    setSubedAllBlocksIds.erase(id);
    setSubedAllTxIds.erase(id);

    mapIdSubedTopic.erase(id);
    mapIdSubedSession.erase(id);
}

bool CDbpService::IsEmpty(const uint256& hash)
{
    static const std::string EMPTY_HASH("0000000000000000000000000000000000000000000000000000000000000000");
    return hash.ToString() == EMPTY_HASH;
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

void CDbpService::TrySwitchFork(const uint256& blockHash, std::vector<std::pair<uint256,uint256>>& path,uint256& forkHash)
{
    for(const auto& fork : path)
    {
        if(blockHash == fork.second)
        {
            forkHash = fork.first;
        }
    }
}

bool CDbpService::GetForkPath(const uint256& forkHash, std::vector<std::pair<uint256,uint256>>& path)
{
    std::vector<std::pair<uint256,int>> vAncestors;
    std::vector<std::pair<int,uint256>> vSublines;
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
    
    return true;
}

bool CDbpService::GetBlocks(const uint256& forkHash, const uint256& startHash, int32 n, std::vector<CMvDbpBlock>& blocks)
{
    uint256 connectForkHash = forkHash;
    uint256 blockHash = startHash;

    if (IsEmpty(connectForkHash))
    {
        connectForkHash = pCoreProtocol->GetGenesisBlockHash();
        std::cout << "fork hash is empty. default main fork\n";
    }

    if(!IsForkHash(connectForkHash))
    {
        std::cerr << "connect fork hash is not a fork hash.\n";
        return false;
    }

    if (IsEmpty(blockHash))
    {
        blockHash = connectForkHash;
        std::cout << "start block hash is empty, default value is fork hash\n";
    }

    int blockHeight = 0;
    uint256 tempForkHash;
    if (!pService->GetBlockLocation(blockHash, tempForkHash, blockHeight))
    {
        std::cerr << "GetBlockLocation failed\n";
        return false;
    }


    std::cout << "GetBocks fork hash is: " << connectForkHash.ToString() << "\n";

    // path.size() >= 1
    std::vector<std::pair<uint256,uint256>> path;
    if(!GetForkPath(connectForkHash,path))
    {
        std::cerr << "GetForkAncestors failed.\n";
        return false;
    }

    const std::size_t nonExtendBlockMaxNum = n;
    std::size_t nonExtendBlockCount = 0;
    
    
    pService->GetBlockLocation(blockHash, tempForkHash, blockHeight);
    
    std::vector<uint256> blocksHash;
    while (nonExtendBlockCount != nonExtendBlockMaxNum && 
            pService->GetBlockHash(tempForkHash, blockHeight, blocksHash))
    {
        
        for(int i = 0; i < blocksHash.size(); ++i)
        {
            CBlock block;
            pService->GetBlock(blocksHash[i], block, tempForkHash, blockHeight);
            std::cout << "block height: " << blockHeight << "\n";
            std::cout << "block fork: " << tempForkHash.ToString() << "\n"; 
            if (block.nType != CBlock::BLOCK_EXTENDED)
            {
                nonExtendBlockCount++;
            }

            CMvDbpBlock DbpBlock;
            CreateDbpBlock(block, tempForkHash, blockHeight, DbpBlock);
            blocks.push_back(DbpBlock);
        }
        
        TrySwitchFork(blockHash,path,tempForkHash);

        blockHeight++;
    }

    return true;
}

void CDbpService::HandleGetBlocks(CMvEventDbpMethod& event)
{
    std::string forkid = event.data.params["forkid"];
    std::string blockHash = event.data.params["hash"];
    int32 blockNum = boost::lexical_cast<int32>(event.data.params["number"]);

    uint256 startBlockHash(std::vector<unsigned char>(blockHash.begin(), blockHash.end()));
    uint256 forkHash;
    forkHash.SetHex(forkid);
    std::vector<CMvDbpBlock> blocks;
    if (GetBlocks(forkHash, startBlockHash, blockNum, blocks))
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

bool CDbpService::HandleEvent(CMvEventDbpMethod& event)
{
    if (event.data.method == CMvDbpMethod::Method::GET_BLOCKS)
    {
        HandleGetBlocks(event);
    }
    else if (event.data.method == CMvDbpMethod::Method::GET_TX)
    {
        HandleGetTransaction(event);
    }
    else if (event.data.method == CMvDbpMethod::Method::SEND_TX)
    {
        HandleSendTransaction(event);
    }
    else
    {
        return false;
    }

    return true;
}

void CDbpService::CreateDbpBlock(const CBlock& blockDetail, const uint256& forkHash,
                                 int blockHeight, CMvDbpBlock& block)
{
    block.nVersion = blockDetail.nVersion;
    block.nType = blockDetail.nType;
    block.nTimeStamp = blockDetail.nTimeStamp;

    walleve::CWalleveODataStream hashPrevStream(block.hashPrev);
    blockDetail.hashPrev.ToDataStream(hashPrevStream);

    walleve::CWalleveODataStream hashMerkleStream(block.hashMerkle);
    blockDetail.hashMerkle.ToDataStream(hashMerkleStream);

    block.vchProof = blockDetail.vchProof;
    block.vchSig = blockDetail.vchSig;

    // txMint
    CreateDbpTransaction(blockDetail.txMint, block.txMint);

    // vtx
    for (const auto& tx : blockDetail.vtx)
    {
        CMvDbpTransaction dbpTx;
        CreateDbpTransaction(tx, dbpTx);
        block.vtx.push_back(dbpTx);
    }

    block.nHeight = blockHeight;
    walleve::CWalleveODataStream hashStream(block.hash);
    blockDetail.GetHash().ToDataStream(hashStream);
}

void CDbpService::CreateDbpTransaction(const CTransaction& tx, CMvDbpTransaction& dbptx)
{
    dbptx.nVersion = tx.nVersion;
    dbptx.nType = tx.nType;
    dbptx.nLockUntil = tx.nLockUntil;

    walleve::CWalleveODataStream hashAnchorStream(dbptx.hashAnchor);
    tx.hashAnchor.ToDataStream(hashAnchorStream);

    for (const auto& input : tx.vInput)
    {
        CMvDbpTxIn txin;
        txin.n = input.prevout.n;

        walleve::CWalleveODataStream txInHashStream(txin.hash);
        input.prevout.hash.ToDataStream(txInHashStream);

        dbptx.vInput.push_back(txin);
    }

    dbptx.cDestination.prefix = tx.sendTo.prefix;
    dbptx.cDestination.size = tx.sendTo.DESTINATION_SIZE;

    walleve::CWalleveODataStream sendtoStream(dbptx.cDestination.data);
    tx.sendTo.data.ToDataStream(sendtoStream);

    dbptx.nAmount = tx.nAmount;
    dbptx.nTxFee = tx.nTxFee;

    dbptx.vchData = tx.vchData;
    dbptx.vchSig = tx.vchSig;

    walleve::CWalleveODataStream hashStream(dbptx.hash);
    tx.GetHash().ToDataStream(hashStream);
}

void CDbpService::PushBlock(const std::string& forkid, const CMvDbpBlock& block)
{
    for (const auto& kv : mapIdSubedSession)
    {
        std::string id = kv.first;
        std::string session = kv.second;

        if (setSubedAllBlocksIds.find(id) != setSubedAllBlocksIds.end())
        {
            CMvEventDbpAdded eventAdded(session);
            eventAdded.data.id = id;
            eventAdded.data.forkid = forkid;
            eventAdded.data.name = "all-block";
            eventAdded.data.anyAddedObj = block;
            pDbpServer->DispatchEvent(&eventAdded);
        }
    }
}

void CDbpService::PushTx(const std::string& forkid, const CMvDbpTransaction& dbptx)
{
    for (const auto& kv : mapIdSubedSession)
    {
        std::string id = kv.first;
        std::string session = kv.second;

        if (setSubedAllTxIds.find(id) != setSubedAllTxIds.end())
        {
            CMvEventDbpAdded eventAdded(session);
            eventAdded.data.id = id;
            eventAdded.data.forkid = forkid;
            eventAdded.data.name = "all-tx";
            eventAdded.data.anyAddedObj = dbptx;
            pDbpServer->DispatchEvent(&eventAdded);
        }
    }
}

bool CDbpService::HandleEvent(CMvEventDbpUpdateNewBlock& event)
{
    // get details about new block
    uint256 blockHash = event.data;
    CBlock newBlock;
    uint256 forkHash;
    int blockHeight;

    if (pService->GetBlock(blockHash, newBlock, forkHash, blockHeight))
    {
        CMvDbpBlock block;
        CreateDbpBlock(newBlock, forkHash, blockHeight, block);
        PushBlock(forkHash.ToString(),block);
    }

    return true;
}

bool CDbpService::HandleEvent(CMvEventDbpUpdateNewTx& event)
{
    decltype(event.data)& newtx = event.data;
    std::string forkid = event.hashFork.ToString();

    CMvDbpTransaction dbpTx;
    CreateDbpTransaction(newtx, dbpTx);
    PushTx(forkid,dbpTx);

    return true;
}
