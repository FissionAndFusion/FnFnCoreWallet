// Copyright (c) 2017-2018 The Multiverse developers
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

bool CBlockDB::Initialize(const CMvDBConfig& config,int nMaxDBConn)
{
    if (!dbPool.Initialize(config,nMaxDBConn))
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
    mapForkIndex.clear();
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
        for (map<uint256,int>::iterator it = mapForkIndex.begin();it != mapForkIndex.end();++it)
        {
            ostringstream oss;
            oss << "DROP TABLE IF EXISTS unspent" << (*it).second; 
            txn.Query(oss.str());
        }
        txn.Query("TRUNCATE TABLE fork");
        txn.Query("TRUNCATE TABLE block");
        txn.Query("TRUNCATE TABLE transaction");
        txn.Query("TRUNCATE TABLE txloc");
        if (!txn.Commit())
        {
            return false;
        }
    }
    mapForkIndex.clear();
    return true;
}

bool CBlockDB::AddNewFork(const uint256& hash)
{
    CMvDBInst db(&dbPool);
    if (!db.Available())
    {
        return false;
    }
  
    int nIndex = 0; 
    string strEscHash = db->ToEscString(hash);
    {
        ostringstream oss;
        oss << "INSERT INTO fork(hash,refblock) "
            << "VALUE("
            << "\'" << strEscHash << "\',"
            << "\'" << db->ToEscString(uint256(0)) << "\')";
        if (!db->Query(oss.str()))
        {
            return false;
        }
    }

    {
        ostringstream oss;
        oss << "SELECT id FROM fork WHERE hash = \'" << strEscHash << "\'";
        CMvDBRes res(*db,oss.str());
        if (!res.GetRow() || !res.GetField(0,nIndex))
        {
            return false;
        }
    }
    {
        CMvDBTxn txn(*db);
        {
            ostringstream oss;
            oss << "DROP TABLE IF EXISTS unspent" << nIndex; 
            txn.Query(oss.str());
        }
        {
            ostringstream oss;
            oss << "CREATE TABLE unspent" << nIndex
                << "(id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,"
                <<   "txid BINARY(32) NOT NULL,"
                <<   "nout TINYINT UNSIGNED NOT NULL,"
                <<   "dest BINARY(33) NOT NULL,"
                <<   "amount BIGINT NOT NULL,"
                <<   "lockuntil INT UNSIGNED NOT NULL,"
                <<   "UNIQUE KEY txid (txid,nout))";
            txn.Query(oss.str());
        } 
        if (!txn.Commit()) 
        {
            ostringstream del;
            del << "DELETE FROM fork WHERE id = " << nIndex;
            db->Query(del.str());
            return false;
        }
    }
    mapForkIndex.insert(make_pair(hash,nIndex));
    return true;
}

bool CBlockDB::RemoveFork(const uint256& hash)
{
    CMvDBInst db(&dbPool);
    if (!db.Available())
    {
        return false;
    }

    int nIndex = GetForkIndex(hash);
    if (nIndex < 0)
    {
        return false;
    }
    
    {
        CMvDBTxn txn(*db);
        {
            ostringstream oss;
            oss << "DELETE FROM fork WHERE id = " << nIndex;
            txn.Query(oss.str());
        }
        {
            ostringstream oss;
            oss << "DROP TABLE IF EXISTS unspent" << nIndex; 
            txn.Query(oss.str());
        }
        
        if (!txn.Commit()) 
        {
            return false;
        }
    }
    mapForkIndex.erase(hash);
    return true;
}

bool CBlockDB::RetrieveFork(vector<uint256>& vFork)
{
    vFork.clear();

    CMvDBInst db(&dbPool);
    if (!db.Available())
    {
        return false;
    }

    {
        CMvDBRes res(*db,"SELECT refblock FROM fork",true);
        while (res.GetRow())
        {
            uint256 ref;
            if (!res.GetField(0,ref))
            {
                return false;
            }
            vFork.push_back(ref);
        }
    }
    return true;
}

bool CBlockDB::UpdateFork(const uint256& hash,const uint256& ref,const uint256& anchor,
                              const vector<CTxUnspent>& vAddNew,const vector<CTxOutPoint>& vRemove)
{
    CMvDBInst db(&dbPool);
    if (!db.Available())
    {
        return false;
    }
    
    bool fTrunc = false; 
    int nIndex = GetForkIndex(hash);
    if (nIndex < 0)
    {
        return false;
    }
    int nIndexAnchor = -1;
    if (anchor != hash && anchor != 0)
    {
        if ((nIndexAnchor = GetForkIndex(anchor)) < 0);
        {
            return false;
        }

        size_t count = 0;
        ostringstream oss;
        oss << "SELECT COUNT(*) FROM unspent" << nIndex;
        CMvDBRes res(*db,oss.str());
        if (!res.GetRow() || !res.GetField(0,count))
        {
            return false;
        }
        fTrunc = (count != 0);
    }
    {
        CMvDBTxn txn(*db);
        if (fTrunc)
        {
            ostringstream oss;
            oss << "TRUNCATE TABLE unspent" << nIndex;
            txn.Query(oss.str());
        }
        if (nIndexAnchor > 0)
        {
            ostringstream oss;
            oss << "INSERT unspent" << nIndex << " SELECT * FROM unspent" << nIndexAnchor; 
            txn.Query(oss.str());
        }
        for (int i = 0;i < vAddNew.size();i++)
        {
            const CTxUnspent& unspent = vAddNew[i];
            ostringstream oss;
            oss << "INSERT INTO unspent" << nIndex << "(txid,nout,dest,amount,lockuntil) "
                      "VALUES("
                <<            "\'" << db->ToEscString(unspent.hash) << "\',"
                <<            ((int)unspent.n) << ","
                <<            "\'" << db->ToEscString(unspent.output.destTo) << "\',"
                <<            unspent.output.nAmount << ","
                <<            unspent.output.nLockUntil << ")";
            txn.Query(oss.str());
        } 
        for (int i = 0;i < vRemove.size();i++)
        {
            const CTxOutPoint& out = vRemove[i];
            ostringstream oss;
            oss << "DELETE FROM unspent" << nIndex
                << " WHERE txid = \'" << db->ToEscString(out.hash) << "\' AND "
                <<        "nout = " << ((int)out.n);
            txn.Query(oss.str());
        }
        {
            ostringstream oss;
            oss << "UPDATE fork SET refblock = \'" << db->ToEscString(ref) << "\' "
                                "WHERE hash = \'" << db->ToEscString(hash) << "\'"; 
            txn.Query(oss.str());
        }
        if (!txn.Commit()) 
        {
            return false;
        }
    }
    return true;
}

bool CBlockDB::AddNewBlock(const CDiskBlockIndex& diskIndex,
                               const vector<pair<uint256,CTxIndex> >& vTxIndex)
{
    CMvDBInst db(&dbPool);
    if (!db.Available())
    {
        return false;
    }
    string strEscHash = db->ToEscString(diskIndex.hashBlock);
    {
        CMvDBTxn txn(*db);
        {
            ostringstream oss;
            oss << "INSERT INTO block(hash,prev,txid,minttype,version,type,time,height,trust,supply,beacon,file,offset,txs) "
                      "VALUES("
                <<            "\'" << strEscHash << "\',"
                <<            "\'" << db->ToEscString(diskIndex.hashPrev) << "\',"
                <<            "\'" << db->ToEscString(diskIndex.txidMint) << "\',"
                <<            diskIndex.nMintType << ","
                <<            diskIndex.nVersion << ","
                <<            diskIndex.nType << ","
                <<            diskIndex.nTimeStamp << ","
                <<            diskIndex.nHeight << ","
                <<            diskIndex.nChainTrust << ","
                <<            diskIndex.nMoneySupply << ","
                <<            diskIndex.nRandBeacon << ","
                <<            diskIndex.nFile << ","
                <<            diskIndex.nOffset << ","
                <<            diskIndex.nTxs << ")";
            txn.Query(oss.str());
        }
        {
            for (size_t i = 0;i < vTxIndex.size();i++)
            {
                const CTxIndex& txIndex = vTxIndex[i].second;
                vector<uint8> vchInput;
                walleve::CWalleveODataStream ss(vchInput);
                ss << txIndex.vTxInput;
                
                string strEscTxid = db->ToEscString(vTxIndex[i].first);
                if (!txIndex.IsNull())
                {
                    ostringstream oss;
                    oss << "INSERT INTO transaction(txid,version,type,lockuntil,anchor,destin,valuein,file,offset,vin) "
                              "VALUES("
                        <<            "\'" << strEscTxid << "\',"
                        <<            txIndex.nVersion << ","
                        <<            txIndex.nType << ","
                        <<            txIndex.nLockUntil << ","
                        <<            "\'" << db->ToEscString(txIndex.hashAnchor) << "\',"
                        <<            "\'" << db->ToEscString(txIndex.destIn) << "\',"
                        <<            txIndex.nValueIn << ","
                        <<            txIndex.nFile << ","
                        <<            txIndex.nOffset << ","
                        <<            "\'" << db->ToEscString(vchInput) << "\')"
                        <<    " ON DUPLICATE KEY UPDATE id = id";
                    txn.Query(oss.str());
                }
                {
                    ostringstream oss;
                    oss << "INSERT INTO txloc(txid,block) "
                              "VALUES("
                        <<            "\'" << strEscTxid << "\',"
                        <<            "\'" << strEscHash << "\')";
                    txn.Query(oss.str());
                }
            }
        }
        return txn.Commit();
    }    
}

bool CBlockDB::RemoveBlock(const uint256& hash)
{
    CMvDBInst db(&dbPool);
    if (!db.Available())
    {
        return false;
    }
    {
        CMvDBTxn txn(*db);
        string strEscHash = db->ToEscString(hash);
        {
            ostringstream oss;
            oss << "DELETE FROM block WHERE hash = " 
                <<            "\'" << strEscHash << "\'";
            txn.Query(oss.str());
        }
        {
            ostringstream oss;
            oss << "DELETE FROM txloc WHERE block = " 
                <<            "\'" << strEscHash << "\'";
            txn.Query(oss.str());
        }
        return txn.Commit();
    }
}

bool CBlockDB::WalkThroughBlock(CBlockDBWalker& walker)
{
    CMvDBInst db(&dbPool);
    if (!db.Available())
    {
        return false;
    }
    {
        CMvDBRes res(*db,"SELECT hash,prev,txid,minttype,version,type,time,height,trust,supply,beacon,file,offset,txs FROM block ORDER BY id",true);
        while (res.GetRow())
        {
            CDiskBlockIndex diskIndex;
            if (   !res.GetField(0,diskIndex.hashBlock)    || !res.GetField(1,diskIndex.hashPrev)
                || !res.GetField(2,diskIndex.txidMint)     || !res.GetField(3,diskIndex.nMintType)
                || !res.GetField(4,diskIndex.nVersion)     || !res.GetField(5,diskIndex.nType)
                || !res.GetField(6,diskIndex.nTimeStamp)   || !res.GetField(7,diskIndex.nHeight)      
                || !res.GetField(8,diskIndex.nChainTrust)  || !res.GetField(9,diskIndex.nMoneySupply) 
                || !res.GetField(10,diskIndex.nRandBeacon) || !res.GetField(11,diskIndex.nFile)       
                || !res.GetField(12,diskIndex.nOffset)     || !res.GetField(13,diskIndex.nTxs)
                || !walker.Walk(diskIndex))
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
        vector<uint8> vchInput;
        ostringstream oss;
        oss << "SELECT version,type,lockuntil,anchor,destin,valuein,file,offset,vin FROM transaction WHERE txid = "
            <<            "\'" << db->ToEscString(txid) << "\'";
        CMvDBRes res(*db,oss.str());
        if (res.GetRow()
            && res.GetField(0,txIndex.nVersion) && res.GetField(1,txIndex.nType) 
            && res.GetField(2,txIndex.nLockUntil) && res.GetField(3,txIndex.hashAnchor) 
            && res.GetField(4,txIndex.destIn) && res.GetField(5,txIndex.nValueIn)
            && res.GetField(6,txIndex.nFile) && res.GetField(7,txIndex.nOffset) 
            && res.GetField(8,vchInput))
        {
            walleve::CWalleveIDataStream ss(vchInput);
            ss >> txIndex.vTxInput;
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

bool CBlockDB::RetrieveTxLocation(const uint256& txid,vector<uint256>& vBlockHash)
{
    vBlockHash.clear();

    CMvDBInst db(&dbPool);
    if (!db.Available())
    {
        return false;
    }

    {
        ostringstream oss;
        oss << "SELECT block FROM txloc WHERE txid = \'" << db->ToEscString(txid) << "\'";
        CMvDBRes res(*db,oss.str(),true);
        while (res.GetRow())
        {
            uint256 hash;
            if (!res.GetField(0,hash))
            {
                return false;
            }
            vBlockHash.push_back(hash);
        }
    }
    return true;
}

bool CBlockDB::RetrieveTxUnspent(const uint256& fork,const CTxOutPoint& out,CTxOutput& unspent)
{
    CMvDBInst db(&dbPool);
    if (!db.Available())
    {
        return false;
    }

    map<uint256,int>::iterator it = mapForkIndex.find(fork);
    if (it == mapForkIndex.end())
    {
        return false;
    }
    int nIndex = (*it).second;
    
    {
        ostringstream oss;
        oss << "SELECT dest,amount,lockuntil FROM unspent" << nIndex
            << " WHERE txid = \'" << db->ToEscString(out.hash) << "\' AND "
            <<        "nout = " << ((int)out.n);
        CMvDBRes res(*db,oss.str());
        return (res.GetRow() && res.GetField(0,unspent.destTo)
                             && res.GetField(1,unspent.nAmount)
                             && res.GetField(2,unspent.nLockUntil));
    } 
}

bool CBlockDB::CreateTable()
{
    CMvDBInst db(&dbPool);
    if (!db.Available())
    {
        return false;
    }

    return (
        db->Query("CREATE TABLE IF NOT EXISTS fork ("
                    "id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,"
                    "hash BINARY(32) NOT NULL UNIQUE KEY,"
                    "refblock BINARY(32) NOT NULL)")
           &&
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
                    "trust BIGINT UNSIGNED NOT NULL,"
                    "supply BIGINT NOT NULL,"
                    "beacon BIGINT UNSIGNED NOT NULL,"
                    "file INT UNSIGNED NOT NULL,"
                    "offset INT UNSIGNED NOT NULL,"
                    "txs INT UNSIGNED NOT NULL,"
                    "INDEX(id))"
                    "PARTITION BY KEY(hash) PARTITIONS 16")
           &&
        db->Query("CREATE TABLE IF NOT EXISTS transaction("
                    "id INT NOT NULL AUTO_INCREMENT,"
                    "txid BINARY(32) NOT NULL UNIQUE KEY,"
                    "version SMALLINT UNSIGNED NOT NULL,"
                    "type SMALLINT UNSIGNED NOT NULL,"
                    "lockuntil INT UNSIGNED NOT NULL,"
                    "anchor BINARY(32) NOT NULL,"
                    "destin BINARY(33) NOT NULL,"
                    "valuein BIGINT UNSIGNED NOT NULL,"
                    "file INT UNSIGNED NOT NULL,"
                    "offset INT UNSIGNED NOT NULL,"
                    "vin BLOB NOT NULL,"
                    "INDEX(id))"
                    "PARTITION BY KEY(txid) PARTITIONS 256")
           &&
        db->Query("CREATE TABLE IF NOT EXISTS txloc("
                    "id INT NOT NULL AUTO_INCREMENT,"
                    "txid BINARY(32) NOT NULL,"
                    "block BINARY(32) NOT NULL,"
                    "INDEX(id),"
                    "INDEX(txid))"
                    "PARTITION BY KEY(txid) PARTITIONS 256")
            );
}

bool CBlockDB::LoadFork()
{
    CMvDBInst db(&dbPool);
    if (!db.Available())
    {
        return false;
    }
    {
        CMvDBRes res(*db,"SELECT id,hash FROM fork",true);
        while (res.GetRow())
        {
            int id;
            uint256 hash;
            if (!res.GetField(0,id) || !res.GetField(1,hash))
            {
                return false;
            }
            mapForkIndex.insert(make_pair(hash,id));
        }
    }
    return true;
}

