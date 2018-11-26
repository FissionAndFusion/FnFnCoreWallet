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

bool CBlockDB::DBPoolInitialize(const CMvDBConfig& config,int nMaxDBConn)
{

    if (!dbPool.Initialize(config,nMaxDBConn))
    {
        return false;
    }

    return true;
}

bool CBlockDB::Initialize()
{

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
        txn.Query("TRUNCATE TABLE forkcontext");
        txn.Query("TRUNCATE TABLE fork");
        txn.Query("TRUNCATE TABLE block");
        txn.Query("TRUNCATE TABLE transaction");
        txn.Query("TRUNCATE TABLE delegate");
        txn.Query("TRUNCATE TABLE enroll");
        if (!txn.Commit())
        {
            return false;
        }
    }
    mapForkIndex.clear();
    return true;
}

bool CBlockDB::AddNewForkContext(const CForkContext& ctxt)
{
    CMvDBInst db(&dbPool);
    if (!db.Available())
    {
        return false;
    }
    ostringstream oss;
    oss << "INSERT INTO forkcontext (name,symbol,hash,parent,joint,txid,version,flag,mintreward,mintxfee,owner,jointheight) "
        <<    "VALUE("
        <<           "\'" << db->ToEscString(ctxt.strName) << "\',"
        <<           "\'" << db->ToEscString(ctxt.strSymbol) << "\',"
        <<           "\'" << db->ToEscString(ctxt.hashFork) << "\',"
        <<           "\'" << db->ToEscString(ctxt.hashParent) << "\',"
        <<           "\'" << db->ToEscString(ctxt.hashJoint) << "\',"
        <<           "\'" << db->ToEscString(ctxt.txidEmbedded) << "\',"
        <<           ctxt.nVersion << ","
        <<           ctxt.nFlag << ","
        <<           ctxt.nMintReward << ","
        <<           ctxt.nMinTxFee << ","
        <<           "\'" << db->ToEscString(ctxt.destOwner) << "\',"
        <<           ctxt.nJointHeight << ")";

    return db->Query(oss.str());
}

bool CBlockDB::RetrieveForkContext(const uint256& hash,CForkContext& ctxt)
{
    ctxt.SetNull();

    CMvDBInst db(&dbPool);
    if (db.Available())
    {
        ctxt.hashFork = hash;

        ostringstream oss;
        oss << "SELECT name,symbol,parent,joint,txid,version,flag,mintreward,mintxfee,owner,jointheight FROM forkcontext WHERE hash = "
            <<            "\'" << db->ToEscString(hash) << "\'";
        CMvDBRes res(*db,oss.str());
        return (res.GetRow()
                && res.GetField(0,ctxt.strName) && res.GetField(1,ctxt.strSymbol) 
                && res.GetField(2,ctxt.hashParent) && res.GetField(3,ctxt.hashJoint) 
                && res.GetField(4,ctxt.txidEmbedded) && res.GetField(5,ctxt.nVersion) 
                && res.GetField(6,ctxt.nFlag) && res.GetField(7,ctxt.nMintReward) 
                && res.GetField(8,ctxt.nMinTxFee) && res.GetField(9,ctxt.destOwner)
                && res.GetField(10,ctxt.nJointHeight));
    }

    return false;
}

bool CBlockDB::FilterForkContext(CForkContextFilter& filter)
{
    CMvDBInst db(&dbPool);
    if (!db.Available())
    {
        return false;
    }

    string strQuery = "SELECT name,symbol,hash,parent,joint,txid,version,flag,mintreward,mintxfee,owner,jointheight FROM forkcontext";
    if (filter.hashParent != 0)
    {
        strQuery += string(" WHERE parent = \'") + db->ToEscString(filter.hashParent) + "\'";
        if (!filter.strSymbol.empty())
        {
            strQuery += string(" AND symbol = \'") + db->ToEscString(filter.strSymbol) + "\'";
        }
    }
    else if (!filter.strSymbol.empty())
    {
        strQuery += string(" WHERE symbol = \'") + db->ToEscString(filter.strSymbol) + "\'";
    }

    strQuery += " ORDER BY id";

    {
        CMvDBRes res(*db,strQuery,true);
        while (res.GetRow())
        {
            CForkContext ctxt;
            if (   !res.GetField(0,ctxt.strName)     || !res.GetField(1,ctxt.strSymbol)
                || !res.GetField(2,ctxt.hashFork)    || !res.GetField(3,ctxt.hashParent) 
                || !res.GetField(4,ctxt.hashJoint)   || !res.GetField(5,ctxt.txidEmbedded) 
                || !res.GetField(6,ctxt.nVersion)    || !res.GetField(7,ctxt.nFlag) 
                || !res.GetField(8,ctxt.nMintReward) || !res.GetField(9,ctxt.nMinTxFee) 
                || !res.GetField(10,ctxt.destOwner)  || !res.GetField(11,ctxt.nJointHeight)
                || !filter.FoundForkContext(ctxt) )
            {
                return false;
            }
        }
    }
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
            <<   "VALUE("
            <<          "\'" << strEscHash << "\',"
            <<          "\'" << db->ToEscString(uint64(0)) << "\')";
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
                <<   "UNIQUE KEY txid (txid,nout))"
                <<   "ENGINE=InnoDB";
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

bool CBlockDB::UpdateFork(const uint256& hash,const uint256& hashRefBlock,const uint256& hashForkBased,
                          const vector<pair<uint256,CTxIndex> >& vTxNew,const vector<uint256>& vTxDel,
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
    int nIndexBased = -1;
    if (hashForkBased != hash && hashForkBased != 0)
    {
        if ((nIndexBased = GetForkIndex(hashForkBased)) < 0)
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
        if (nIndexBased < 0)
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
        if (fTrunc)
        {
            ostringstream oss;
            oss << "TRUNCATE TABLE unspent" << nIndex;
            txn.Query(oss.str());
        }
        if (nIndexBased > 0)
        {
            ostringstream oss;
            oss << "INSERT unspent" << nIndex << " SELECT * FROM unspent" << nIndexBased; 
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
            oss << "UPDATE fork SET refblock = \'" << db->ToEscString(hashRefBlock) << "\' "
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

bool CBlockDB::UpdateDelegate(const uint256& hash,const map<CDestination,int64>& mapDelegate)
{
    CMvDBInst db(&dbPool);
    if (!db.Available())
    {
        return false;
    }

    {
        CMvDBTxn txn(*db);
        
        for (map<CDestination,int64>::const_iterator it = mapDelegate.begin();it != mapDelegate.end();++it)
        {
            if ((*it).second != 0)
            {
                ostringstream oss;
                oss << "INSERT INTO delegate(block,dest,amount) " 
                          "VALUES("
                    <<            "\'" << db->ToEscString(hash) << "\',"
                    <<            "\'" << db->ToEscString((*it).first) << "\',"
                    <<            (*it).second << ")";
                txn.Query(oss.str());
            }
        }

        if (!txn.Commit()) 
        {
            return false;
        }
    }
    return true;
}

bool CBlockDB::UpdateEnroll(vector<pair<CTxIndex,uint256> >& vEnroll)
{
    CMvDBInst db(&dbPool);
    if (!db.Available())
    {
        return false;
    }

    {
        CMvDBTxn txn(*db);
        for (int i = 0;i < vEnroll.size();i++)
        {
            const CTxIndex& txIndex = vEnroll[i].first;
            const uint256& hash = vEnroll[i].second;
            ostringstream oss;
            oss << "INSERT INTO enroll(anchor,dest,block,file,offset) "
                      "VALUES("
                <<            "\'" << db->ToEscString(txIndex.hashAnchor) << "\',"
                <<            "\'" << db->ToEscString(txIndex.destIn) << "\',"
                <<            "\'" << db->ToEscString(hash) << "\',"
                <<            txIndex.nFile << ","
                <<            txIndex.nOffset << ")"
                <<  " ON DUPLICATE KEY UPDATE "
                              "block = VALUES(block),file = VALUES(file),offset = VALUES(offset)";
                txn.Query(oss.str());
        }
        if (!txn.Commit()) 
        {
            return false;
        }
    }
    return true;
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

bool CBlockDB::FilterTx(CBlockDBTxFilter& filter)
{
    CMvDBInst db(&dbPool);
    if (!db.Available())
    {
        return false;
    }
    ostringstream oss;
    oss << "SELECT destin,valuein,height,file,offset FROM transaction";
    if (filter.destIn.IsNull())
    {
        if (!filter.sendTo.IsNull())
        {
            oss << " WHERE sendto = \'" << db->ToEscString(filter.sendTo) << "\'";
        }
    }
    else
    {
        oss << " WHERE destin = \'" << db->ToEscString(filter.destIn) << "\'";
        if (!filter.sendTo.IsNull())
        {
            oss << " OR sendto = \'" << db->ToEscString(filter.sendTo) << "\'";
        }
    }
    oss << " ORDER BY id";

    CMvDBRes res(*db,oss.str(),true);
    while (res.GetRow())
    {
        CDestination destIn;
        int64 nValueIn;
        int nHeight;
        uint32 nFile,nOffset;
        if (   !res.GetField(0,destIn) || !res.GetField(1,nValueIn) || !res.GetField(2,nHeight) 
            || !res.GetField(3,nFile)  || !res.GetField(4,nOffset)
            || !filter.FoundTxIndex(destIn,nValueIn,nHeight,nFile,nOffset))
        {
            return false;
        }
    }
    return true;
}

bool CBlockDB::RetrieveDelegate(const uint256& hash,int64 nMinAmount,map<CDestination,int64>& mapDelegate)
{
    CMvDBInst db(&dbPool);
    if (!db.Available())
    {
        return false;
    }
    ostringstream oss;
    oss << "SELECT dest,amount FROM delegate" 
           " WHERE block = \'" << db->ToEscString(hash) << "\'"
           " AND amount >= " << nMinAmount; 
    
    CMvDBRes res(*db,oss.str(),true);
    while (res.GetRow())
    {
        CDestination dest;
        int64 amount;
        if (!res.GetField(0,dest) || !res.GetField(1,amount))
        {
            return false;
        }
        mapDelegate.insert(make_pair(dest,amount));
    }
    return true;
}

bool CBlockDB::RetrieveEnroll(const uint256& hashAnchor,const set<uint256>& setBlockRange, 
                                                        map<CDestination,pair<uint32,uint32> >& mapEnrollTxPos)
{
    CMvDBInst db(&dbPool);
    if (!db.Available())
    {
        return false;
    }
    ostringstream oss;
    oss << "SELECT dest,block,file,offset FROM enroll" 
           " WHERE anchor = \'" << db->ToEscString(hashAnchor) << "\'";
    
    CMvDBRes res(*db,oss.str(),true);
    while (res.GetRow())
    {
        CDestination dest;
        uint256 hash;
        pair<uint32,uint32> pos;
        uint32 nFile,nOffset;
        if (!res.GetField(0,dest)  || !res.GetField(1,hash)
            || !res.GetField(2,pos.first) || !res.GetField(3,pos.second))
        {
            return false;
        }
        if (setBlockRange.count(hash))
        {
            mapEnrollTxPos.insert(make_pair(dest,pos));
        }
    }
    return true;
}

bool CBlockDB::CreateTable()
{
    CMvDBInst db(&dbPool);
    if (!db.Available())
    {
        return false;
    }

    return (
        db->Query("CREATE TABLE IF NOT EXISTS forkcontext ("
                    "id INT NOT NULL AUTO_INCREMENT,"
                    "name VARCHAR(128) NOT NULL UNIQUE KEY,"
                    "symbol VARCHAR(16) NOT NULL,"
                    "hash BINARY(32) NOT NULL UNIQUE KEY,"
                    "parent BINARY(32) NOT NULL,"
                    "joint BINARY(32) NOT NULL,"
                    "txid BINARY(32) NOT NULL,"
                    "version SMALLINT UNSIGNED NOT NULL,"
                    "flag SMALLINT UNSIGNED NOT NULL,"
                    "mintreward BIGINT UNSIGNED NOT NULL,"
                    "mintxfee BIGINT UNSIGNED NOT NULL,"
                    "owner BINARY(33) NOT NULL,"
                    "jointheight INT NOT NULL,"
                    "INDEX(hash),INDEX(id))"
                    "ENGINE=InnoDB")
           &&
        db->Query("CREATE TABLE IF NOT EXISTS fork ("
                    "id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,"
                    "hash BINARY(32) NOT NULL UNIQUE KEY,"
                    "refblock BINARY(32) NOT NULL)"
                    "ENGINE=InnoDB")
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
           &&
        db->Query("CREATE TABLE IF NOT EXISTS delegate("
                    "id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,"
                    "block BINARY(32) NOT NULL,"
                    "dest BINARY(33) NOT NULL,"
                    "amount BIGINT UNSIGNED NOT NULL,"
                    "INDEX(block,amount))"
                    "ENGINE=InnoDB")
           &&
        db->Query("CREATE TABLE IF NOT EXISTS enroll("
                    "id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,"
                    "anchor BINARY(32) NOT NULL,"
                    "dest BINARY(33) NOT NULL,"
                    "block BINARY(32) NOT NULL,"
                    "file INT UNSIGNED NOT NULL,"
                    "offset INT UNSIGNED NOT NULL,"
                    "UNIQUE KEY enroll (anchor,dest),"
                    "INDEX(anchor))"
                    "ENGINE=InnoDB")
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

