// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "blockdb.h"
#include "walleve/stream/datastream.h"

using namespace std;
using namespace multiverse::storage;

//////////////////////////////
// CBlockDB

CBlockDB::CBlockDB()
{
}

CBlockDB::~CBlockDB()
{
}

bool CBlockDB::DBPoolInitialize(const CMvDBConfig& config,int nMaxDBConn)
{

    if (!dbPool.Initialize(config,nMaxDBConn))
    {
        return false;
    }

    return true;
}

bool CBlockDB::Initialize(const boost::filesystem::path& pathData)
{
    if (!dbFork.Initialize(pathData))
    {
        return false;
    }

    if (!dbUnspent.Initialize(pathData))
    {
        return false;
    }

    if (!dbDelegate.Initialize(pathData))
    {
        return false;
    }
    
    if (!CreateTable())
    {
        return false;
    }

    return LoadFork();
}

void CBlockDB::Deinitialize()
{
    dbPool.Deinitialize();
    dbDelegate.Deinitialize();
    dbUnspent.Deinitialize();
    dbFork.Deinitialize();
}

bool CBlockDB::RemoveAll()
{
    CMvDBInst db(&dbPool);
    if (!db.Available())
    {
        return false;
    }

    {
        CMvDBTxn txn(*db);
        txn.Query("TRUNCATE TABLE block");
        txn.Query("TRUNCATE TABLE transaction");
        if (!txn.Commit())
        {
            return false;
        }
    }

    dbDelegate.Clear();
    dbUnspent.Clear();
    dbFork.Clear();

    return true;
}

bool CBlockDB::AddNewForkContext(const CForkContext& ctxt)
{
    return dbFork.AddNewForkContext(ctxt);
}

bool CBlockDB::RetrieveForkContext(const uint256& hash,CForkContext& ctxt)
{
    ctxt.SetNull();
    return dbFork.RetrieveForkContext(hash,ctxt);
}

bool CBlockDB::ListForkContext(vector<CForkContext>& vForkCtxt)
{
    vForkCtxt.clear();
    return dbFork.ListForkContext(vForkCtxt);
}

bool CBlockDB::AddNewFork(const uint256& hash)
{
    if (!dbFork.UpdateFork(hash))
    {
        return false;
    }

    if (!dbUnspent.AddNew(hash))
    {
        dbFork.RemoveFork(hash);
        return false;
    }

    return true;
}

bool CBlockDB::RemoveFork(const uint256& hash)
{
    if (!dbUnspent.Remove(hash))
    {
        return false;
    }
   
    return dbFork.RemoveFork(hash); 
}

bool CBlockDB::ListFork(vector<pair<uint256,uint256> >& vFork)
{
    vFork.clear();
    return dbFork.ListFork(vFork);
}

bool CBlockDB::UpdateFork(const uint256& hash,const uint256& hashRefBlock,const uint256& hashForkBased,
                          const vector<pair<uint256,CTxIndex> >& vTxNew,const vector<uint256>& vTxDel,
                          const vector<CTxUnspent>& vAddNew,const vector<CTxOutPoint>& vRemove)
{
    CMvDBInst db(&dbPool);
    if (!db.Available())
    {
        return false;
    }
   
    if (!dbUnspent.Exists(hash))
    {
        return false;
    }

    bool fIgnoreTxDel = false;
    if (hashForkBased != hash && hashForkBased != 0)
    {
        if (!dbUnspent.Copy(hashForkBased,hash))
        {
            return false;
        }
        fIgnoreTxDel = true;
    }

    {
        CMvDBTxn txn(*db);
        if (!fIgnoreTxDel)
        {
            for (int i = 0;i < vTxDel.size();i++)
            {
                ostringstream oss;
                oss << "DELETE FROM transaction WHERE txid = " 
                    <<            "\'" << db->ToEscString(vTxDel[i]) << "\'";
                txn.Query(oss.str());
            }
        }
        for (int i = 0;i < vTxNew.size();i++)
        {
            const CTxIndex& txIndex = vTxNew[i].second;
            string strEscTxid = db->ToEscString(vTxNew[i].first);
            ostringstream oss;
            oss << "INSERT INTO transaction(txid,version,type,lockuntil,anchor,sendto,amount,destin,valuein,height,file,offset) "
                      "VALUES("
                <<            "\'" << strEscTxid << "\',"
                <<            txIndex.nVersion << ","
                <<            txIndex.nType << ","
                <<            txIndex.nLockUntil << ","
                <<            "\'" << db->ToEscString(txIndex.hashAnchor) << "\',"
                <<            "\'" << db->ToEscString(txIndex.sendTo) << "\',"
                <<            txIndex.nAmount << ","
                <<            "\'" << db->ToEscString(txIndex.destIn) << "\',"
                <<            txIndex.nValueIn << ","
                <<            txIndex.nBlockHeight << ","
                <<            txIndex.nFile << ","
                <<            txIndex.nOffset << ")"
                << " ON DUPLICATE KEY UPDATE height = VALUES(height),file = VALUES(file),offset = VALUES(offset)";
            txn.Query(oss.str());
        }

        if (!dbFork.UpdateFork(hash,hashRefBlock))
        {
            txn.Abort();
            return false;
        }

        if (!dbUnspent.Update(hash,vAddNew,vRemove))
        {
            txn.Abort();
            return false;
        }

        if (!txn.Commit()) 
        {
            return false;
        }
    }
    return true;
}

bool CBlockDB::AddNewBlock(const CBlockOutline& outline)
{
    CMvDBInst db(&dbPool);
    if (!db.Available())
    {
        return false;
    }
    
    ostringstream oss;
    oss << "INSERT INTO block(hash,prev,txid,minttype,version,type,time,height,beacon,trust,supply,algo,bits,file,offset) "
              "VALUES("
        <<            "\'" << db->ToEscString(outline.hashBlock) << "\',"
        <<            "\'" << db->ToEscString(outline.hashPrev) << "\',"
        <<            "\'" << db->ToEscString(outline.txidMint) << "\',"
        <<            outline.nMintType << ","
        <<            outline.nVersion << ","
        <<            outline.nType << ","
        <<            outline.nTimeStamp << ","
        <<            outline.nHeight << ","
        <<            outline.nRandBeacon << ","
        <<            outline.nChainTrust << ","
        <<            outline.nMoneySupply << ","
        <<            (int)outline.nProofAlgo << ","
        <<            (int)outline.nProofBits << ","
        <<            outline.nFile << ","
        <<            outline.nOffset << ")";
    return db->Query(oss.str());
}

bool CBlockDB::RemoveBlock(const uint256& hash)
{
    CMvDBInst db(&dbPool);
    if (!db.Available())
    {
        return false;
    }

    ostringstream oss;
    oss << "DELETE FROM block" << " WHERE hash = \'" << db->ToEscString(hash) << "\'";
    return db->Query(oss.str());
}

bool CBlockDB::UpdateDelegateContext(const uint256& hash,const CDelegateContext& ctxtDelegate)
{
    return dbDelegate.AddNew(hash,ctxtDelegate);
}

bool CBlockDB::WalkThroughBlock(CBlockDBWalker& walker)
{
    CMvDBInst db(&dbPool);
    if (!db.Available())
    {
        return false;
    }
    {
        CMvDBRes res(*db,"SELECT hash,prev,txid,minttype,version,type,time,height,beacon,trust,supply,algo,bits,file,offset FROM block ORDER BY id",true);
        while (res.GetRow())
        {
            CBlockOutline outline;
            if (   !res.GetField(0,outline.hashBlock)     || !res.GetField(1,outline.hashPrev)
                || !res.GetField(2,outline.txidMint)      || !res.GetField(3,outline.nMintType)
                || !res.GetField(4,outline.nVersion)      || !res.GetField(5,outline.nType)
                || !res.GetField(6,outline.nTimeStamp)    || !res.GetField(7,outline.nHeight)      
                || !res.GetField(8,outline.nRandBeacon)   || !res.GetField(9,outline.nChainTrust)  
                || !res.GetField(10,outline.nMoneySupply) || !res.GetField(11,outline.nProofAlgo)
                || !res.GetField(12,outline.nProofBits)   || !res.GetField(13,outline.nFile)       
                || !res.GetField(14,outline.nOffset)      || !walker.Walk(outline))
            {
                return false;
            }
        }
    }
    return true;
}

bool CBlockDB::ExistsTx(const uint256& txid)
{
    CMvDBInst db(&dbPool);
    if (!db.Available())
    {
        return false;
    }
    {
        size_t count = 0;
        ostringstream oss;
        oss << "SELECT COUNT(*) FROM transaction WHERE txid = "
            <<            "\'" << db->ToEscString(txid) << "\'";

        CMvDBRes res(*db,oss.str());
        return (res.GetRow() && res.GetField(0,count) && count != 0);
    }
}

bool CBlockDB::RetrieveTxIndex(const uint256& txid,CTxIndex& txIndex)
{
    txIndex.SetNull();

    CMvDBInst db(&dbPool);
    if (db.Available())
    {
        ostringstream oss;
        oss << "SELECT version,type,lockuntil,anchor,sendto,amount,destin,valuein,height,file,offset FROM transaction WHERE txid = "
            <<            "\'" << db->ToEscString(txid) << "\'";
        CMvDBRes res(*db,oss.str());
        if (res.GetRow()
            && res.GetField(0,txIndex.nVersion) && res.GetField(1,txIndex.nType) 
            && res.GetField(2,txIndex.nLockUntil) && res.GetField(3,txIndex.hashAnchor) 
            && res.GetField(4,txIndex.sendTo) && res.GetField(5,txIndex.nAmount)
            && res.GetField(6,txIndex.destIn) && res.GetField(7,txIndex.nValueIn)
            && res.GetField(8,txIndex.nBlockHeight)
            && res.GetField(9,txIndex.nFile) && res.GetField(10,txIndex.nOffset))
        {
            return true;
        }
    }
    return false;
}

bool CBlockDB::RetrieveTxPos(const uint256& txid,uint32& nFile,uint32& nOffset)
{
    CMvDBInst db(&dbPool);
    if (!db.Available())
    {
        return false;
    }
    {
        ostringstream oss;
        oss << "SELECT file,offset FROM transaction WHERE txid = "
            <<            "\'" << db->ToEscString(txid) << "\'";
        CMvDBRes res(*db,oss.str());
        return (res.GetRow() && res.GetField(0,nFile) && res.GetField(1,nOffset));
    }
}

bool CBlockDB::RetrieveTxLocation(const uint256& txid,uint256& hashAnchor,int& nBlockHeight)
{
    CMvDBInst db(&dbPool);
    if (!db.Available())
    {
        return false;
    }
    {
        ostringstream oss;
        oss << "SELECT anchor,height FROM transaction WHERE txid = "
            <<            "\'" << db->ToEscString(txid) << "\'";
        CMvDBRes res(*db,oss.str());
        return (res.GetRow() && res.GetField(0,hashAnchor) && res.GetField(1,nBlockHeight));
    }
}

bool CBlockDB::RetrieveTxUnspent(const uint256& fork,const CTxOutPoint& out,CTxOutput& unspent)
{
    return dbUnspent.Retrieve(fork,out,unspent);
}

bool CBlockDB::RetrieveDelegate(const uint256& hash,map<CDestination,int64>& mapDelegate)
{
    return dbDelegate.RetrieveDelegatedVote(hash,mapDelegate);
}

bool CBlockDB::RetrieveEnroll(const uint256& hashAnchor,const vector<uint256>& vBlockRange, 
                                                        map<CDestination,CDiskPos>& mapEnrollTxPos)
{
    return dbDelegate.RetrieveEnrollTx(hashAnchor,vBlockRange,mapEnrollTxPos);
}

bool CBlockDB::CreateTable()
{
    CMvDBInst db(&dbPool);
    if (!db.Available())
    {
        return false;
    }

    return (
        db->Query("CREATE TABLE IF NOT EXISTS block("
                    "id INT NOT NULL AUTO_INCREMENT,"
                    "hash BINARY(32) NOT NULL UNIQUE KEY,"
                    "prev BINARY(32) NOT NULL,"
                    "txid BINARY(32) NOT NULL,"
                    "minttype SMALLINT UNSIGNED NOT NULL,"
                    "version SMALLINT UNSIGNED NOT NULL,"
                    "type SMALLINT UNSIGNED NOT NULL,"
                    "time INT UNSIGNED NOT NULL,"
                    "height INT UNSIGNED NOT NULL,"
                    "beacon BIGINT UNSIGNED NOT NULL,"
                    "trust BIGINT UNSIGNED NOT NULL,"
                    "supply BIGINT NOT NULL,"
                    "algo TINYINT UNSIGNED NOT NULL,"
                    "bits TINYINT UNSIGNED NOT NULL,"
                    "file INT UNSIGNED NOT NULL,"
                    "offset INT UNSIGNED NOT NULL,"
                    "INDEX(id))"
                    "ENGINE=InnoDB PARTITION BY KEY(hash) PARTITIONS 16")
           &&
        db->Query("CREATE TABLE IF NOT EXISTS transaction("
                    "id INT NOT NULL AUTO_INCREMENT,"
                    "txid BINARY(32) NOT NULL UNIQUE KEY,"
                    "version SMALLINT UNSIGNED NOT NULL,"
                    "type SMALLINT UNSIGNED NOT NULL,"
                    "lockuntil INT UNSIGNED NOT NULL,"
                    "anchor BINARY(32) NOT NULL,"
                    "sendto BINARY(33) NOT NULL,"
                    "amount BIGINT UNSIGNED NOT NULL,"
                    "destin BINARY(33) NOT NULL,"
                    "valuein BIGINT UNSIGNED NOT NULL,"
                    "height INT NOT NULL,"
                    "file INT UNSIGNED NOT NULL,"
                    "offset INT UNSIGNED NOT NULL,"
                    "INDEX(txid),INDEX(id))"
                    "ENGINE=InnoDB PARTITION BY KEY(txid) PARTITIONS 256")
            );
}

bool CBlockDB::LoadFork()
{
    vector<pair<uint256,uint256> > vFork;
    if (!dbFork.ListFork(vFork))
    {
        return false;
    }

    for (int i = 0;i < vFork.size();i++)
    {
        if (!dbUnspent.AddNew(vFork[i].first))
        {
            return false;
        }
    }
    return true;
}


bool CBlockDB::InnoDB()
{
    CMvDBInst db(&dbPool);
    if (!db.Available())
    {
        return false;
    }

    vector<unsigned char> support;

    CMvDBRes res(*db,"SHOW ENGINES",true);

    while (res.GetRow())
    {
        res.GetField(0,support);

        string engine(support.begin(),support.end());

        if (engine == "InnoDB")
        {
            return true;
        }
    }

    return false;
}

