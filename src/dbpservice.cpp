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

bool CDbpService::HandleEvent(walleve::CWalleveEventDbpPong& event)
{
    (void)event;
    return true;
}

bool CDbpService::HandleEvent(walleve::CWalleveEventDbpConnect& event)
{
    bool isReconnect = event.data.isReconnect;

    if(isReconnect)
    {
         // reply normal
        walleve::CWalleveEventDbpConnected eventConnected(event.session_);
        eventConnected.data.session = event.data.session;
        pDbpServer->DispatchEvent(&eventConnected);
    }
    else
    {
        if(event.data.version != 1)
        {
            // reply failed
            std::vector<int> versions{1};
            walleve::CWalleveEventDbpFailed eventFailed(event.session_);
            eventFailed.data.reason = "001";
            eventFailed.data.versions = versions;
            eventFailed.data.session  = event.data.session;
            pDbpServer->DispatchEvent(&eventFailed);
        }
        else
        {
            // reply normal
            walleve::CWalleveEventDbpConnected eventConnected(event.session_);
            eventConnected.data.session = event.data.session;
            pDbpServer->DispatchEvent(&eventConnected);
        }
    }
    
    return true;
}

bool CDbpService::HandleEvent(walleve::CWalleveEventDbpSub& event)
{
    std::string id = event.data.id;
    std::string topicName = event.data.name;

    std::cout << "Sub topic is:" << topicName << std::endl;

    if(!IsTopicExist(topicName))
    {
         // reply nosub
        std::cout << "Sub topic not exists: " << topicName << std::endl;
        walleve::CWalleveEventDbpNoSub eventNoSub(event.session_);
        eventNoSub.data.id = event.data.id;
        pDbpServer->DispatchEvent(&eventNoSub);
    }
    else
    { 
        SubTopic(id,event.session_,topicName);
        
        //reply ready
        walleve::CWalleveEventDbpReady eventReady(event.session_);
        eventReady.data.id = event.data.id;
        pDbpServer->DispatchEvent(&eventReady);
    }
    
    return true;
}

bool CDbpService::HandleEvent(walleve::CWalleveEventDbpUnSub& event)
{
    UnSubTopic(event.data.id);   
    return true;
}

void CDbpService::HandleGetTransaction(walleve::CWalleveEventDbpMethod& event)
{
    std::string id = event.data.id;
    std::string txid = event.data.params["hash"];
    
    uint256 txHash(txid);
    CTransaction tx;
    uint256 forkHash;
    int blockHeight;
    
    if(pService->GetTransaction(txHash,tx,forkHash,blockHeight))
    {
        walleve::CWalleveDbpTransaction dbpTx;
        CreateDbpTransaction(tx,dbpTx);

        walleve::CWalleveEventDbpMethodResult eventResult(event.session_);
        eventResult.data.id = id;
        eventResult.data.anyResultObjs.push_back(dbpTx);
        pDbpServer->DispatchEvent(&eventResult);
    }
    else
    {
        walleve::CWalleveEventDbpMethodResult eventResult(event.session_);
        eventResult.data.id = id;
        eventResult.data.error = "404";
        pDbpServer->DispatchEvent(&eventResult);
    }

}

void CDbpService::HandleSendTransaction(walleve::CWalleveEventDbpMethod& event)
{
    std::string txid = event.data.params["hash"];

    std::vector<unsigned char> txData = walleve::ParseHexString(txid);
    walleve::CWalleveBufStream ss;
    ss.Write((char *)&txData[0],txData.size());
    
    CTransaction rawTx;
    try
    {
        ss >> rawTx;
    }
    catch (const std::exception &e)
    {
        walleve::CWalleveEventDbpMethodResult eventResult(event.session_);
        eventResult.data.id = event.data.id;
        eventResult.data.error = "400";
        pDbpServer->DispatchEvent(&eventResult);
        return;
    }
    
    MvErr err = pService->SendTransaction(rawTx);
    if (err == MV_OK)
    {
        walleve::CWalleveEventDbpMethodResult eventResult(event.session_);
        eventResult.data.id = event.data.id;
        
        walleve::CWalleveDbpSendTxRet sendTxRet;
        sendTxRet.hash = txid;
        eventResult.data.anyResultObjs.push_back(sendTxRet);
        
        pDbpServer->DispatchEvent(&eventResult);
    }
    else
    {
        walleve::CWalleveEventDbpMethodResult eventResult(event.session_);
        eventResult.data.id = event.data.id;
        eventResult.data.error = "400";
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
    if(topic == "all-tx") subedAllTxIdIds.insert(id);

    idSubedSessionMap.insert(std::make_pair(id,session));
}

void CDbpService::UnSubTopic(const std::string& id)
{ 
    subedAllBlocksIds.erase(id);
    subedAllTxIdIds.erase(id);
    
    idSubedTopicMap.erase(id);
    idSubedSessionMap.erase(id);
}

void CDbpService::PushTopic(const std::string& topic)
{
    
}

bool CDbpService::GetBlocks(const uint256& startHash, int32 n, std::vector<walleve::CWalleveDbpBlock>& blocks)
{
     // first, find start block from block hash
    CBlock startBlock;
    uint256 forkHash;
    int currentHeight;

    std::cout << "HEx string start hash: " << startHash.ToString() << std::endl;

    // Get Genesis block if hash is empty
    uint256 start = startHash;
    if(startHash.ToString().empty())
    {
       start = pCoreProtocol->GetGenesisBlockHash();
    }
   
    if(pService->GetBlock(start,startBlock,forkHash,currentHeight))
    {
        
        std::cout << "Get First Block height: " << currentHeight << std::endl;

        walleve::CWalleveDbpBlock firstBlock;
        CreateDbpBlock(startBlock,forkHash,currentHeight,firstBlock);
        blocks.push_back(firstBlock);
        int nextHeight = currentHeight + 1;
    
        
        for(int i = 0; i < n - 1; ++i)
        {
            std::cout << "i: " << i << " height: " << nextHeight <<std::endl;
            
            uint256 nextHash;
            CBlock block;
            if(pService->GetBlockHash(forkHash,nextHeight,nextHash) && 
                pService->GetBlock(nextHash,block,forkHash,currentHeight))
            {
                
                std::cout << "height: " << nextHeight << "success" << std::endl;
                
                walleve::CWalleveDbpBlock dbpBlock;
                CreateDbpBlock(block,forkHash,currentHeight,dbpBlock);
                blocks.push_back(dbpBlock);

                nextHeight = currentHeight + 1;
            }
            else
            {
                std::cout << "i: " << i << "break" << std::endl;
                break;
            }
        }
        
        return true;  
    }
    else
    {
        std::cout << "GetBlocks return false" << std::endl;
        return false;
    }  
}

void CDbpService::HandleGetBlocks(walleve::CWalleveEventDbpMethod& event)
{
    std::string blockHash = event.data.params["hash"];
    reverse(blockHash.begin(), blockHash.end());
    uint256 *startBlockHash = (uint256 *)blockHash.c_str();
    int32 blockNum = boost::lexical_cast<int32>(event.data.params["number"]);

    // std::cout << "block hash[service]: " << blockHash << " block number: " << blockNum << std::endl;
    
    // uint256 startBlockHash(blockHash);  

    std::vector<walleve::CWalleveDbpBlock> blocks;
    if(GetBlocks(*startBlockHash,blockNum,blocks))
    {
        std::cout << "Get Blocks success[service]: " << blocks.size() << std::endl;
        
        walleve::CWalleveEventDbpMethodResult eventResult(event.session_);
        eventResult.data.id = event.data.id;
        
        for(auto& block : blocks)
        {
           eventResult.data.anyResultObjs.push_back(block);
        }
    
        pDbpServer->DispatchEvent(&eventResult);
    }
    else
    {
        
        walleve::CWalleveEventDbpMethodResult eventResult(event.session_);
        eventResult.data.id = event.data.id;
        eventResult.data.error = "400";
        pDbpServer->DispatchEvent(&eventResult);
    }
}

bool CDbpService::HandleEvent(walleve::CWalleveEventDbpMethod& event)
{
    if(event.data.method == walleve::CWalleveDbpMethod::Method::GET_BLOCKS)
    {
        HandleGetBlocks(event);    
    }
    else if(event.data.method == walleve::CWalleveDbpMethod::Method::GET_TX)
    {
        HandleGetTransaction(event);
    }
    else if(event.data.method == walleve::CWalleveDbpMethod::Method::SEND_TX)
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
int blockHeight, walleve::CWalleveDbpBlock& block)
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
        walleve::CWalleveDbpTransaction dbpTx;
        CreateDbpTransaction(tx,dbpTx);
        block.vtx.push_back(dbpTx);
    }

    block.nHeight = blockHeight;
    walleve::CWalleveODataStream hashStream(block.hash);
    blockDetail.GetHash().ToDataStream(hashStream);
}

void CDbpService::CreateDbpTransaction(const CTransaction& tx,walleve::CWalleveDbpTransaction& dbptx)
{
    dbptx.nVersion = tx.nVersion;
    dbptx.nType = tx.nType;
    dbptx.nLockUntil = tx.nLockUntil;

    walleve::CWalleveODataStream hashAnchorStream(dbptx.hashAnchor);
    tx.hashAnchor.ToDataStream(hashAnchorStream);

    for(const auto& input : tx.vInput)
    {
        walleve::CWalleveDbpTxIn txin;
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

bool CDbpService::HandleEvent(CMvEventDbpUpdateNewBlock& event)
{
    // get details about new block
    uint256 blockHash = event.data;
    CBlock newBlock;
    uint256 forkHash;
    int blockHeight;
   
    if(pService->GetBlock(blockHash,newBlock,forkHash,blockHeight))
    {
        walleve::CWalleveDbpBlock block;
        CreateDbpBlock(newBlock,forkHash,blockHeight,block);

        // push new block to dbpclient when new-block-event comes 
        for(const auto& kv : idSubedSessionMap)
        {
            std::string id = kv.first;
            std::string session = kv.second;

            if(subedAllBlocksIds.find(id) != subedAllBlocksIds.end())
            {
                walleve::CWalleveEventDbpAdded eventAdded(session);
                eventAdded.data.id = id;
                eventAdded.data.name = "all-block";
                eventAdded.data.anyAddedObj = block;
                pDbpServer->DispatchEvent(&eventAdded);
            }

        }
    } 
    return true;
}

bool CDbpService::HandleEvent(CMvEventDbpUpdateNewTx& event)
{
    decltype(event.data) & newtx = event.data;

    walleve::CWalleveDbpTransaction dbpTx;
    CreateDbpTransaction(newtx,dbpTx);

    // push new tx to dbpclient when new-tx-event comes
    for(const auto& kv : idSubedSessionMap)
    {
        std::string id = kv.first;
        std::string session = kv.second;

        if(subedAllTxIdIds.find(id) != subedAllTxIdIds.end())
        {
            walleve::CWalleveEventDbpAdded eventAdded(session);
            eventAdded.data.id = id;
            eventAdded.data.name = "all-tx";
            eventAdded.data.anyAddedObj = dbpTx;
            pDbpServer->DispatchEvent(&eventAdded);
        }
    }

    return true;
}