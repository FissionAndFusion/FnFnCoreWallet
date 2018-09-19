// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "dbpservice.h"

#include <boost/assign/list_of.hpp>
#include <boost/lexical_cast.hpp>

using namespace multiverse;

CDbpService::CDbpService()
:walleve::IIOModule("dbpservice")
{
    pService = NULL;
    pCoreProtocol = NULL;
    pWallet = NULL;
    pDbpServer = NULL;

    std::unordered_map<std::string,bool> temp_map = boost::assign::map_list_of
                    ("all-block",true)
                    ("all-tx",true)
                    ("changed",true)
                    ("removed",true);
    
    currentTopicExistMap = temp_map;
}

CDbpService::~CDbpService()
{

}


bool CDbpService::WalleveHandleInitialize()
{
    if (!WalleveGetObject("coreprotocol",pCoreProtocol))
    {
        WalleveLog("Failed to request coreprotocol\n");
        return false;
    }
    
    
    if (!WalleveGetObject("service",pService))
    {
        WalleveLog("Failed to request service\n");
        return false;
    }

    if (!WalleveGetObject("wallet",pWallet))
    {
        WalleveLog("Failed to request wallet\n");
        return false;
    }

    if (!WalleveGetObject("dbpserver",pDbpServer))
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

    if(isReconnect)
    {
         // reply normal
        CMvEventDbpConnected eventConnected(event.session_);
        eventConnected.data.session = event.data.session;
        pDbpServer->DispatchEvent(&eventConnected);
    }
    else
    {
        if(event.data.version != 1)
        {
            // reply failed
            std::vector<int> versions{1};
            CMvEventDbpFailed eventFailed(event.session_);
            eventFailed.data.reason = "001";
            eventFailed.data.versions = versions;
            eventFailed.data.session  = event.data.session;
            pDbpServer->DispatchEvent(&eventFailed);
        }
        else
        {
            // reply normal
            CMvEventDbpConnected eventConnected(event.session_);
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

    std::cout << "Sub topic is:" << topicName << std::endl;

    if(!IsTopicExist(topicName))
    {
         // reply nosub
        std::cout << "Sub topic not exists: " << topicName << std::endl;
        CMvEventDbpNoSub eventNoSub(event.session_);
        eventNoSub.data.id = event.data.id;
        pDbpServer->DispatchEvent(&eventNoSub);
    }
    else
    { 
        SubTopic(id,event.session_,topicName);

        std::cout << "subed topic: " << topicName << "id: " 
        << id << std::endl;
        
        // reply ready
        CMvEventDbpReady eventReady(event.session_);
        eventReady.data.id = id;
        pDbpServer->DispatchEvent(&eventReady);

        std::cout << "sended ready: " << topicName << "id: " 
        << id << std::endl;
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
    
    if(pService->GetTransaction(txHash,tx,forkHash,blockHeight))
    {
        CMvDbpTransaction dbpTx;
        CreateDbpTransaction(tx,dbpTx);

        CMvEventDbpMethodResult eventResult(event.session_);
        eventResult.data.id = id;
        eventResult.data.anyResultObjs.push_back(dbpTx);
        pDbpServer->DispatchEvent(&eventResult);
    }
    else
    {
        CMvEventDbpMethodResult eventResult(event.session_);
        eventResult.data.id = id;
        eventResult.data.error = "404";
        pDbpServer->DispatchEvent(&eventResult);
    }

}

void CDbpService::HandleSendTransaction(CMvEventDbpMethod& event)
{
    std::string data = event.data.params["data"];

    std::vector<unsigned char> txData(data.begin(),data.end());
    walleve::CWalleveBufStream ss;
    ss.Write((char *)&txData[0],txData.size());
    
    CTransaction rawTx;
    try
    {
        ss >> rawTx;
    }
    catch (const std::exception &e)
    {
        
        std::cout << "stream to CTransaction failed " << std::endl;
        
        CMvEventDbpMethodResult eventResult(event.session_);
        eventResult.data.id = event.data.id;
        eventResult.data.error = "400";
        pDbpServer->DispatchEvent(&eventResult);
        return;
    }

    std::cout << "stream to CTransaction successed " << std::endl;
    std::cout << "tx amount: " << rawTx.nAmount << std::endl;
    std::cout << "tx version: " << rawTx.nVersion << std::endl;
    std::cout << "tx fee: " << rawTx.nTxFee << std::endl;
    
    MvErr err = pService->SendTransaction(rawTx);
    if (err == MV_OK)
    {
        
        std::cout << "tx send success" << std::endl;
        
        CMvEventDbpMethodResult eventResult(event.session_);
        eventResult.data.id = event.data.id;
        
        CMvDbpSendTxRet sendTxRet;
        sendTxRet.hash = data;
        sendTxRet.result = "succeed";
        eventResult.data.anyResultObjs.push_back(sendTxRet);
        
        pDbpServer->DispatchEvent(&eventResult);
    }
    else
    {
        std::cout << "tx send failed" << std::endl;
        
        CMvEventDbpMethodResult eventResult(event.session_);
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
    return currentTopicExistMap.find(topic) != currentTopicExistMap.end();
}

bool CDbpService::IsHaveSubedTopicOf(const std::string& id)
{
    return idSubedTopicMap.find(id) != idSubedTopicMap.end();
}

void CDbpService::SubTopic(const std::string& id, const std::string& session,const std::string& topic)
{
    idSubedTopicMap.insert(std::make_pair(id,topic));

    if(topic == "all-block") subedAllBlocksIds.insert(id);
    if(topic == "all-tx") subedAllTxIds.insert(id);

    idSubedSessionMap.insert(std::make_pair(id,session));
}

void CDbpService::UnSubTopic(const std::string& id)
{ 
    subedAllBlocksIds.erase(id);
    subedAllTxIds.erase(id);
    
    idSubedTopicMap.erase(id);
    idSubedSessionMap.erase(id);
}

bool CDbpService::IsEmpty(const uint256& hash)
{
    static const std::string EMPTY_HASH
    ("0000000000000000000000000000000000000000000000000000000000000000");
    return hash.ToString() == EMPTY_HASH;
}

bool CDbpService::GetBlocks(const uint256& startHash, int32 n, std::vector<CMvDbpBlock>& blocks)
{
    uint256 blockHash = startHash;
    std::cout << "blockhash: " << blockHash.ToString() << std::endl;

    if(IsEmpty(blockHash))
    {
       blockHash = pCoreProtocol->GetGenesisBlockHash();
       std::cout << "Genesis Block Hash: " << blockHash.ToString() << std::endl;
    }

    uint256 forkHash;
    int blockHeight = 0;
    if(!pService->GetBlockLocation(blockHash,forkHash,blockHeight)) 
    {
        std::cout << "block not exists" << std::endl;
        return false;
    }

    const std::size_t primaryBlockMaxNum = n;
    std::size_t primaryBlockCount = 0;
    while(primaryBlockCount != primaryBlockMaxNum
    && pService->GetBlockHash(forkHash,blockHeight,blockHash))
    {
        CBlock block;
        pService->GetBlock(blockHash,block,forkHash,blockHeight);
        if(block.nType == CBlock::BLOCK_PRIMARY)
        {
            primaryBlockCount++;
        }
        
        CMvDbpBlock DbpBlock;
        CreateDbpBlock(block,forkHash,blockHeight,DbpBlock);
        blocks.push_back(DbpBlock);
        
        blockHeight++;
    }
    
    return true; 
}

void CDbpService::HandleGetBlocks(CMvEventDbpMethod& event)
{
    std::string blockHash = event.data.params["hash"];
    int32 blockNum = boost::lexical_cast<int32>(event.data.params["number"]);
    
    uint256 startBlockHash(std::vector<unsigned char>(blockHash.begin(),blockHash.end()));
    std::vector<CMvDbpBlock> blocks;
    if(GetBlocks(startBlockHash,blockNum,blocks))
    {
        std::cout << "Get Blocks success[service]: " << blocks.size() << std::endl;
        
        CMvEventDbpMethodResult eventResult(event.session_);
        eventResult.data.id = event.data.id;
        
        for(auto& block : blocks)
        {
           eventResult.data.anyResultObjs.push_back(block);
        }
    
        pDbpServer->DispatchEvent(&eventResult);
    }
    else
    {
        
        CMvEventDbpMethodResult eventResult(event.session_);
        eventResult.data.id = event.data.id;
        eventResult.data.error = "400";
        pDbpServer->DispatchEvent(&eventResult);
    }
}

bool CDbpService::HandleEvent(CMvEventDbpMethod& event)
{
    if(event.data.method == CMvDbpMethod::Method::GET_BLOCKS)
    {
        HandleGetBlocks(event);    
    }
    else if(event.data.method == CMvDbpMethod::Method::GET_TX)
    {
        HandleGetTransaction(event);
    }
    else if(event.data.method == CMvDbpMethod::Method::SEND_TX)
    {
        HandleSendTransaction(event);
    }
    else
    {
        return false;
    }
    
    return true;
}

void CDbpService::CreateDbpBlock(const CBlock& blockDetail,const uint256& forkHash, 
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
    CreateDbpTransaction(blockDetail.txMint,block.txMint);
   
    // vtx
    for(const auto& tx : blockDetail.vtx)
    {
        CMvDbpTransaction dbpTx;
        CreateDbpTransaction(tx,dbpTx);
        block.vtx.push_back(dbpTx);
    }

    block.nHeight = blockHeight;
    walleve::CWalleveODataStream hashStream(block.hash);
    blockDetail.GetHash().ToDataStream(hashStream);
}

void CDbpService::CreateDbpTransaction(const CTransaction& tx,CMvDbpTransaction& dbptx)
{
    dbptx.nVersion = tx.nVersion;
    dbptx.nType = tx.nType;
    dbptx.nLockUntil = tx.nLockUntil;

    walleve::CWalleveODataStream hashAnchorStream(dbptx.hashAnchor);
    tx.hashAnchor.ToDataStream(hashAnchorStream);

    for(const auto& input : tx.vInput)
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

void CDbpService::PushBlock(const CMvDbpBlock& block)
{
    for(const auto& kv : idSubedSessionMap)
    {
        std::string id = kv.first;
        std::string session = kv.second;

        if(subedAllBlocksIds.find(id) != subedAllBlocksIds.end())
        {
            CMvEventDbpAdded eventAdded(session);
            eventAdded.data.id = id;
            eventAdded.data.name = "all-block";
            eventAdded.data.anyAddedObj = block;
            pDbpServer->DispatchEvent(&eventAdded);
        }

    }
}

void CDbpService::PushTx(const CMvDbpTransaction& dbptx)
{
    for(const auto& kv : idSubedSessionMap)
    {
        std::string id = kv.first;
        std::string session = kv.second;

        if(subedAllTxIds.find(id) != subedAllTxIds.end())
        {
            CMvEventDbpAdded eventAdded(session);
            eventAdded.data.id = id;
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
   
    if(pService->GetBlock(blockHash,newBlock,forkHash,blockHeight))
    {
        CMvDbpBlock block;
        CreateDbpBlock(newBlock,forkHash,blockHeight,block);
        PushBlock(block);
    } 
    
    return true;
}

bool CDbpService::HandleEvent(CMvEventDbpUpdateNewTx& event)
{
    decltype(event.data) & newtx = event.data;

    CMvDbpTransaction dbpTx;
    CreateDbpTransaction(newtx,dbpTx);
    PushTx(dbpTx);

    return true;
}