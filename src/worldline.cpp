// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "worldline.h"

using namespace std;
using namespace walleve;
using namespace multiverse;

//////////////////////////////
// CWorldLine 

CWorldLine::CWorldLine()
{
    pCoreProtocol = NULL;
}

CWorldLine::~CWorldLine()
{
}

bool CWorldLine::WalleveHandleInitialize()
{
    if (!WalleveGetObject("coreprotocol",pCoreProtocol))
    {
        WalleveLog("Failed to request coreprotocol\n");
        return false;
    }

    return true;
}

void CWorldLine::WalleveHandleDeinitialize()
{
    pCoreProtocol = NULL;
}

bool CWorldLine::WalleveHandleInvoke()
{
    storage::CMvDBConfig dbConfig(StorageConfig()->strDBHost,StorageConfig()->nDBPort,
                                  StorageConfig()->strDBName,StorageConfig()->strDBUser,StorageConfig()->strDBPass);

    if (!cntrBlock.Initialize(dbConfig,StorageConfig()->nDBConn,WalleveConfig()->pathData,WalleveConfig()->fDebug))
    {
        WalleveLog("Failed to initalize container\n");
        return false;
    }

    if (!CheckContainer())
    {
        cntrBlock.Clear();
        WalleveLog("Block container is invalid,try rebuild from block storage\n");
        // Rebuild ... 
        if (!RebuildContainer())
        {
            cntrBlock.Clear(); 
            WalleveLog("Failed to rebuild Block container,reconstruct all\n");
        } 
    }

    if (cntrBlock.IsEmpty())
    {
        CBlock block;
        pCoreProtocol->GetGenesisBlock(block);
        if (!InsertGenesisBlock(block))
        {
            WalleveLog("Failed to create genesis block\n");
            return false;
        }
    }    
        
    return true;
}

void CWorldLine::WalleveHandleHalt()
{
    cntrBlock.Deinitialize();
}

void CWorldLine::GetForkStatus(map<uint256,CForkStatus>& mapForkStatus)
{
    mapForkStatus.clear();

    multimap<int,CBlockIndex*> mapForkIndex;
    cntrBlock.ListForkIndex(mapForkIndex);
    for (multimap<int,CBlockIndex*>::iterator it = mapForkIndex.begin();it != mapForkIndex.end();++it)
    {
        CBlockIndex* pIndex = (*it).second;
        int nForkHeight = (*it).first;
        uint256 hashFork = pIndex->GetOriginHash();
        uint256 hashParent = pIndex->GetParentHash();

        map<uint256,CForkStatus>::iterator mi = mapForkStatus.insert(make_pair(hashFork,CForkStatus(hashFork,hashParent,nForkHeight))).first;
        if (hashParent != 0)
        {
            mapForkStatus[hashParent].mapSubline.insert(make_pair(nForkHeight,hashFork));
        }     
        CForkStatus& status = (*mi).second;
        status.hashLastBlock = pIndex->GetBlockHash();
        status.nLastBlockTime = pIndex->GetBlockTime();
        status.nLastBlockHeight = pIndex->GetBlockHeight();
        status.nMoneySupply = pIndex->GetMoneySupply();
    }
}

bool CWorldLine::GetBlockLocation(const uint256& hashBlock,uint256& hashFork,int& nHeight)
{
    CBlockIndex* pIndex = NULL;
    if (!cntrBlock.RetrieveIndex(hashBlock,&pIndex))
    {
        return false;
    }
    hashFork = pIndex->GetOriginHash();
    nHeight = pIndex->GetBlockHeight();
    return true;
}

bool CWorldLine::GetBlockHash(const uint256& hashFork,int nHeight,uint256& hashBlock)
{
    CBlockIndex* pIndex = NULL;
    if (!cntrBlock.RetrieveFork(hashFork,&pIndex) || pIndex->GetBlockHeight() < nHeight)
    {
        return false;
    }
    while (pIndex != NULL && pIndex->GetBlockHeight() > nHeight)
    {
        pIndex = pIndex->pPrev;
    }
    hashBlock = !pIndex ? 0 : pIndex->GetBlockHash();
    return (pIndex != NULL);
}

bool CWorldLine::GetLastBlock(const uint256& hashFork,uint256& hashBlock,int& nHeight,int64& nTime)
{
    CBlockIndex* pIndex = NULL;
    if (!cntrBlock.RetrieveFork(hashFork,&pIndex))
    {
        return false;
    }
    hashBlock = pIndex->GetBlockHash();
    nHeight = pIndex->GetBlockHeight();
    nTime = pIndex->GetBlockTime();
    return true;
}

bool CWorldLine::GetBlock(const uint256& hashBlock,CBlock& block)
{
    return cntrBlock.Retrieve(hashBlock,block);
}

bool CWorldLine::Exists(const uint256& hashBlock)
{
    return cntrBlock.Exists(hashBlock);
}

bool CWorldLine::GetTransaction(const uint256& txid,CTransaction& tx)
{
    return cntrBlock.RetrieveTx(txid,tx);
}

MvErr CWorldLine::AddNewBlock(CBlock& block,vector<CTransaction>& vtx,CWorldLineUpdate& update)
{
    uint256 hash = block.GetHash();
    MvErr err = MV_OK;

    if (cntrBlock.Exists(hash))
    {
        WalleveLog("AddNewBlock Already Exists : %s \n",hash.ToString().c_str());
        return MV_ERR_ALREADY_HAVE;
    }
    
    err = pCoreProtocol->ValidateBlock(block,vtx);
    if (err != MV_OK)
    {
        WalleveLog("AddNewBlock Validate Block Error(%s) : %s \n",MvErrString(err),hash.ToString().c_str());
        return err;
    }
   
    CBlockIndex* pIndexPrev;
    if (!cntrBlock.RetrieveIndex(block.hashPrev,&pIndexPrev))
    {
        WalleveLog("AddNewBlock Retrieve Prev Index Error: %s \n",block.hashPrev.ToString().c_str());
        return MV_ERR_SYS_STORAGE_ERROR;
    }

    storage::CBlockView view;
    if (!cntrBlock.GetBlockView(block.hashPrev,view,!block.IsOrigin()))
    {
        WalleveLog("AddNewBlock Get Block View Error: %s \n",block.hashPrev.ToString().c_str());
        return MV_ERR_SYS_STORAGE_ERROR;
    }
    
    CBlockTx txMint(block.txMint);
    err = GetBlockTx(txMint,view);
    if (err != MV_OK)
    {
        WalleveLog("AddNewBlock Get BlockTx Error(%s) : %s \n",MvErrString(err),block.txMint.GetHash().ToString().c_str());
        return err;
    } 
    
    int64 nTxFee = txMint.nTxFee;
    BOOST_FOREACH(const CTransaction& tx,vtx)
    {
        nTxFee += tx.nTxFee;
    }
    
    err = pCoreProtocol->VerifyBlock(block,txMint.destIn,txMint.nValueIn,nTxFee,pIndexPrev);
    if (err != MV_OK)
    {
        WalleveLog("AddNewBlock Verify Block Error(%s) : %s \n",MvErrString(err),hash.ToString().c_str());
        return err;
    } 
    
    vector<CBlockTx> vBlockTx;
    vBlockTx.reserve(vtx.size() + 1);
    vBlockTx.push_back(txMint);
    view.AddTx(block.txMint.GetHash(),txMint);

    int n = 0;
    BOOST_FOREACH(const CTransaction& tx,vtx)
    {
        const uint256& txid = block.vTxHash[n++];
        CBlockTx txBlock(tx);
        err = GetBlockTx(txBlock,view);
        if (err != MV_OK)
        {
            WalleveLog("AddNewBlock Get BlockTx Error(%s) : %s \n",MvErrString(err),txid.ToString().c_str());
            return err;
        }
        err = pCoreProtocol->VerifyBlockTx(txBlock,view);
        if (err != MV_OK)
        {
            WalleveLog("AddNewBlock Verify BlockTx Error(%s) : %s \n",MvErrString(err),txid.ToString().c_str());
            return err;
        }
        vBlockTx.push_back(txBlock);
        view.AddTx(txid,txBlock); 
    }

    CBlockIndex* pIndexNew;
    if (!cntrBlock.AddNew(hash,block,vBlockTx,&pIndexNew))
    {
        WalleveLog("AddNewBlock Storage AddNew Error : %s \n",hash.ToString().c_str());
        return MV_ERR_SYS_STORAGE_ERROR;
    }

    WalleveLog("AddNew Block : %s \n",hash.ToString().c_str());
    WalleveLog("    %s\n",pIndexNew->ToString().c_str());

    CBlockIndex* pIndexFork = NULL;
    if (cntrBlock.RetrieveFork(pIndexNew->GetOriginHash(),&pIndexFork)
        && pIndexFork->nChainTrust >= pIndexNew->nChainTrust )
    {
        return MV_OK;
    }

    if (!cntrBlock.CommitBlockView(view,pIndexNew))
    {
        WalleveLog("AddNewBlock Storage Commit BlockView Error : %s \n",hash.ToString().c_str());
        return MV_ERR_SYS_STORAGE_ERROR;
    }
   
    update = CWorldLineUpdate(pIndexNew);
    view.GetTxChanges(update.vTxAddNew,update.vTxRemove,update.setTxUpdate);
 
    return MV_OK;
}

bool CWorldLine::GetProofOfWorkTarget(const uint256& hashPrev,int nAlgo,int& nBits,int64& nReward)
{
    CBlockIndex* pIndexPrev;
    if (!cntrBlock.RetrieveIndex(hashPrev,&pIndexPrev))
    {
        WalleveLog("GetProofOfWorkTarget : Retrieve Prev Index Error: %s \n",hashPrev.ToString().c_str());
        return false;
    }
    if (!pIndexPrev->IsPrimary())
    {
        WalleveLog("GetProofOfWorkTarget : Previous is not primary: %s \n",hashPrev.ToString().c_str());
        return false;
    }
    if (!pCoreProtocol->GetProofOfWorkTarget(pIndexPrev,nAlgo,nBits,nReward))
    {
        WalleveLog("GetProofOfWorkTarget : Unknown proof-of-work algo: %s \n",hashPrev.ToString().c_str());
        return false;
    }
    return true;
}

bool CWorldLine::CheckContainer()
{
    if (cntrBlock.IsEmpty())
    {
        return true;
    }
    if (!cntrBlock.Exists(pCoreProtocol->GetGenesisBlockHash()))
    {
        return false;
    }
    return true;
}

bool CWorldLine::RebuildContainer()
{
    return false;
}

bool CWorldLine::InsertGenesisBlock(CBlock& block)
{
    uint256 hash = block.GetHash();
    uint256 txidMint = block.txMint.GetHash();
    CBlockTx txMint(block.txMint);
    vector<CBlockTx> vtx;
    vtx.push_back(txMint);
    CBlockIndex* pIndexNew = NULL;

    if (!cntrBlock.AddNew(block.GetHash(),block,vtx,&pIndexNew))
    {
        return false;
    }
    
    storage::CBlockView view;
    cntrBlock.GetBlockView(view);
    view.AddTx(txidMint,txMint);
    if (!cntrBlock.CommitBlockView(view,pIndexNew))
    {
        return false;
    }
    return true;
}

MvErr CWorldLine::GetBlockTx(CBlockTx& tx,storage::CBlockView& view)
{
    BOOST_FOREACH(const CTxIn& txin,tx.vInput)
    {
        CTxOutput output;
        if (!view.RetrieveUnspent(txin.prevout,output))
        {
            return MV_ERR_MISSING_PREV;
        }
        if (tx.destIn.IsNull())
        {
            tx.destIn = output.destTo;
        }
        else if (tx.destIn != output.destTo)
        {
            return MV_ERR_TRANSACTION_INVALID;
        }
        tx.nValueIn += output.nAmount;
        tx.vTxInput.push_back(CTxInput(txin.prevout,output.nAmount,output.nLockUntil));
    }
    return MV_OK;
}
