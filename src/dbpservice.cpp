#include "dbpservice.h"

#ifndef WIN32
#include <unistd.h>
#endif
#include <boost/assign/list_of.hpp>
#include <boost/lexical_cast.hpp>

using namespace multiverse;

CDbpService::CDbpService()
:walleve::IIOModule("dbpservice")
{
    pService = NULL;
    pWallet = NULL;
    pDbpServer = NULL;

    std::map<std::string,bool> temp_map = boost::assign::map_list_of
                    ("all-block",true)
                    ("changed",true)
                    ("removed",true);
    
    currentTopicExistMap = temp_map;
}

CDbpService::~CDbpService()
{

}


bool CDbpService::WalleveHandleInitialize()
{
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
    pWallet = NULL;
}

bool CDbpService::HandleEvent(walleve::CWalleveEventDbpPing& event)
{
    (void)event;
    
#ifndef WIN32
    sleep(5);
#endif

    uint64 nonce = event.nNonce;
    walleve::CWalleveEventDbpPing eventConnected(nonce);
    pDbpServer->DispatchEvent(&eventConnected);
    
    return true;
}

bool CDbpService::HandleEvent(walleve::CWalleveEventDbpPong& event)
{
    (void)event;
    return true;
}

bool CDbpService::HandleEvent(walleve::CWalleveEventDbpConnect& event)
{
    bool isReconnect = event.data.isReconnect;

    WalleveLog("connect \n");
    if(isReconnect)
    {
         // reply normal
        uint64 nonce = event.nNonce;
        walleve::CWalleveEventDbpConnected eventConnected(nonce);
        eventConnected.data.session = event.data.session;
        WalleveLog("re-connect \n");
        pDbpServer->DispatchEvent(&eventConnected);
        WalleveLog("re-connect dispatched\n");
    }
    else
    {
        if(event.data.version != 1)
        {
            // reply failed
            uint64 nonce = event.nNonce;
            std::vector<int> versions{1};
            walleve::CWalleveEventDbpFailed eventFailed(nonce);
            eventFailed.data.reason = "001";
            eventFailed.data.versions = versions;
            eventFailed.data.session  = event.data.session;
            pDbpServer->DispatchEvent(&eventFailed);
        }
        else
        {
            // reply normal
            uint64 nonce = event.nNonce;
            walleve::CWalleveEventDbpConnected eventConnected(nonce);
            eventConnected.data.session = event.data.session;
            WalleveLog("connect dispatch entry\n");
            pDbpServer->DispatchEvent(&eventConnected);
            WalleveLog("connect dispatched\n");
        }
    }
    
    return true;
}

bool CDbpService::HandleEvent(walleve::CWalleveEventDbpSub& event)
{
    std::string id = event.data.id;
    std::string topicName = event.data.name;

    std::cout << "Sub topic is:" << topicName << std::endl;

    // if topic not exists
    if(currentTopicExistMap.count(topicName) == 0 )
    {
         // reply nosub
        std::cout << "Sub topic not exists: " << topicName << std::endl;
        uint64 nonce = event.nNonce;
        walleve::CWalleveEventDbpNoSub eventNoSub(nonce);
        eventNoSub.data.id = event.data.id;
        pDbpServer->DispatchEvent(&eventNoSub);
    }
    else
    {
        std::cout << "Sub topic  exists: " << topicName << std::endl;
        if(idSubedTopicsMap.count(id) == 0)
        {
            TopicSet topics{topicName};
            idSubedTopicsMap.insert(std::make_pair(id,topics));
        }
        else
        {
            auto & topics = idSubedTopicsMap[id];
            topics.insert(topicName);
        }

        idSubedNonceMap.insert(std::make_pair(id,event.nNonce));

        //reply ready
        uint64 nonce = event.nNonce;
        walleve::CWalleveEventDbpReady eventReady(nonce);
        eventReady.data.id = event.data.id;
        pDbpServer->DispatchEvent(&eventReady);
    }
    
    return true;
}

bool CDbpService::HandleEvent(walleve::CWalleveEventDbpUnSub& event)
{
    std::string id = event.data.id;    
    if(idSubedTopicsMap.count(id) != 0)
    {
        // unsub is actual delete subed topic
        std::cout << "[SubedTopicsMap] UnSub topic  success"  << std::endl;
        idSubedTopicsMap.erase(id);
    }

    if(idSubedNonceMap.count(id) != 0)
    {
        std::cout << "[SubedNonceMap] UnSub topic  success"  << std::endl;
        idSubedNonceMap.erase(id);
    }
    
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

        uint64 nonce = event.nNonce;
        walleve::CWalleveEventDbpMethodResult eventResult(nonce);
        eventResult.data.id = id;
        eventResult.data.resultType = walleve::CWalleveDbpMethodResult::ResultType::TX;
        eventResult.data.anyObjects.push_back(&dbpTx);
        pDbpServer->DispatchEvent(&eventResult);
    }
    else
    {
        uint64 nonce = event.nNonce;
        walleve::CWalleveEventDbpMethodResult eventResult(nonce);
        eventResult.data.id = id;
        eventResult.data.resultType = walleve::CWalleveDbpMethodResult::ResultType::ERROR;
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
        uint64 nonce = event.nNonce;
        walleve::CWalleveEventDbpMethodResult eventResult(nonce);
        eventResult.data.id = event.data.id;
        eventResult.data.resultType = walleve::CWalleveDbpMethodResult::ResultType::ERROR;
        eventResult.data.error = "400";
        pDbpServer->DispatchEvent(&eventResult);
        return;
    }
    
    MvErr err = pService->SendTransaction(rawTx);
    if (err == MV_OK)
    {
        uint64 nonce = event.nNonce;
        walleve::CWalleveEventDbpMethodResult eventResult(nonce);
        eventResult.data.id = event.data.id;
        eventResult.data.resultType = walleve::CWalleveDbpMethodResult::ResultType::SEND_TX;
        
        walleve::CWalleveDbpSendTxRet sendTxRet;
        sendTxRet.hash = txid;
        eventResult.data.anyObjects.push_back(&sendTxRet);
        
        pDbpServer->DispatchEvent(&eventResult);
    }
    else
    {
        uint64 nonce = event.nNonce;
        walleve::CWalleveEventDbpMethodResult eventResult(nonce);
        eventResult.data.id = event.data.id;
        eventResult.data.resultType = walleve::CWalleveDbpMethodResult::ResultType::ERROR;
        eventResult.data.error = "400";
        pDbpServer->DispatchEvent(&eventResult);
    }
}

bool CDbpService::GetBlocks(const uint256& startHash, int32 n, std::vector<walleve::CWalleveDbpBlock>& blocks)
{
     // first, find start block from block hash
    CBlock startBlock;
    uint256 forkHash;
    int currentHeight;
    
    if(pService->GetBlock(startHash,startBlock,forkHash,currentHeight))
    {
        walleve::CWalleveDbpBlock firstBlock;
        CreateDbpBlock(startBlock,forkHash,currentHeight,firstBlock);
        blocks.push_back(firstBlock);
        int nextHeight = currentHeight + 1;
        
        for(int i = 0; i < n - 1; ++i)
        {
            uint256 nextHash;
            CBlock block;
            if(pService->GetBlockHash(forkHash,nextHeight,nextHash) && 
                pService->GetBlock(nextHash,block,forkHash,currentHeight))
            {
                walleve::CWalleveDbpBlock dbpBlock;
                CreateDbpBlock(block,forkHash,currentHeight,dbpBlock);
                blocks.push_back(dbpBlock);

                nextHeight = currentHeight + 1;
            }
            else
            {
                break;
            }
        }
        
        return true;  
    }
    else
    {
        return false;
    } 
}

void CDbpService::HandleGetBlocks(walleve::CWalleveEventDbpMethod& event)
{
    std::string blockHash = event.data.params["hash"];
    int32 blockNum = boost::lexical_cast<int32>(event.data.params["number"]);
    
    std::vector<unsigned char> hashData = walleve::ParseHexString(blockHash);
    uint256 startBlockHash(hashData);

    std::vector<walleve::CWalleveDbpBlock> blocks;
    if(GetBlocks(startBlockHash,blockNum,blocks))
    {
        uint64 nonce = event.nNonce;
        walleve::CWalleveEventDbpMethodResult eventResult(nonce);
        eventResult.data.id = event.data.id;
        eventResult.data.resultType = walleve::CWalleveDbpMethodResult::ResultType::BLOCKS;
        
        for(auto& block : blocks)
        {
            eventResult.data.anyObjects.push_back(&block);
        }
    
        pDbpServer->DispatchEvent(&eventResult);
    }
    else
    {
        uint64 nonce = event.nNonce;
        walleve::CWalleveEventDbpMethodResult eventResult(nonce);
        eventResult.data.id = event.data.id;
        eventResult.data.resultType = walleve::CWalleveDbpMethodResult::ResultType::ERROR;
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
}

bool CDbpService::HandleEvent(CMvEventDbpUpdateNewBlock& event)
{
    // get details about new block
    uint256 blockHash = event.data;
    CBlock newBlock;
    uint256 forkHash;
    int blockHeight;

   // std::cout << "new block event added,block hash is: " <<  blockHash.ToString() << std::endl;
   
    if(pService->GetBlock(blockHash,newBlock,forkHash,blockHeight))
    {
        walleve::CWalleveDbpBlock block;
        CreateDbpBlock(newBlock,forkHash,blockHeight,block);
     //   std::cout << "get dbp block success: " <<  blockHash.ToString() << std::endl;
        
        // push new block to dbpclient when new-block-event comes 
        for(const auto& kv : idSubedNonceMap)
        {
            std::string id = kv.first;
            uint64 nonce = kv.second;
            
            walleve::CWalleveEventDbpAdded eventAdded(nonce);
            eventAdded.data.id = id;
            eventAdded.data.name = "all-block";
            eventAdded.data.type = walleve::CWalleveDbpAdded::BLOCK;
            eventAdded.data.anyObject = &block;

            pDbpServer->DispatchEvent(&eventAdded);

        }
    } 
    return true;
}

bool CDbpService::HandleEvent(CMvEventDbpUpdateNewTx& event)
{
    decltype(event.data) & newtx = event.data;

    walleve::CWalleveDbpTransaction dbpTx;
    CreateDbpTransaction(newtx,dbpTx);

   // std::cout << "new tx event added,tx hash is: " <<  newtx.GetHash().ToString() << std::endl;

    // push new tx to dbpclient when new-tx-event comes
    for(const auto& kv : idSubedNonceMap)
    {
        std::string id = kv.first;
        int nonce = kv.second;
        
        walleve::CWalleveEventDbpAdded eventAdded(nonce);
        eventAdded.data.id = id;
        eventAdded.data.name = "all-tx";
        eventAdded.data.type = walleve::CWalleveDbpAdded::TX;
        eventAdded.data.anyObject = &dbpTx;

        pDbpServer->DispatchEvent(&eventAdded);
    }

    return true;
}