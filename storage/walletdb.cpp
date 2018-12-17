// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
    
#include "walletdb.h"
#include <boost/foreach.hpp>    

using namespace std;
using namespace walleve;
using namespace multiverse::storage;
    
//////////////////////////////
// CWalletDB

CWalletDB::CWalletDB()
{   
}   
    
CWalletDB::~CWalletDB()
{   
    Deinitialize();
}

bool CWalletDB::Initialize(const CMvDBConfig& config)
{
    if (!dbConn.Connect(config))
    {
        return false;
    }
    return CreateTable();
}

void CWalletDB::Deinitialize()
{
    dbConn.Disconnect();
}

bool CWalletDB::AddNewKey(const uint256& pubkey,int version,const crypto::CCryptoCipher& cipher)
{
    ostringstream oss;
    oss << "INSERT INTO walletkey(pubkey,version,encrypted,nonce) "
              "VALUES("
        <<            "\'" << dbConn.ToEscString(pubkey) << "\',"
        <<            version << ","
        <<            "\'" << dbConn.ToEscString(cipher.encrypted,48) << "\',"
        <<            cipher.nonce << ")";
    return dbConn.Query(oss.str());
}

bool CWalletDB::UpdateKey(const uint256& pubkey,int version,const crypto::CCryptoCipher& cipher)
{
    ostringstream oss;
    oss << "UPDATE walletkey SET version = " << version << ","  
                << "encrypted = \'" << dbConn.ToEscString(cipher.encrypted,48) << "\',"
                << "nonce = " << cipher.nonce 
                << " WHERE pubkey = " << "\'" << dbConn.ToEscString(pubkey) << "\'";

    return dbConn.Query(oss.str());
}

bool CWalletDB::WalkThroughKey(CWalletDBKeyWalker& walker)
{
    CMvDBRes res(dbConn,"SELECT pubkey,version,encrypted,nonce FROM walletkey",true);
    while (res.GetRow())
    {
        int version;
        uint256 pubkey;
        crypto::CCryptoCipher cipher;
        if (!res.GetField(0,pubkey) || !res.GetField(1,version)
            || !res.GetBinary(2,cipher.encrypted,48) || !res.GetField(3,cipher.nonce) 
            || !walker.Walk(pubkey,version,cipher))
        {
            return false;
        }
    }
    return true;
}

bool CWalletDB::AddNewTemplate(const uint256& tid,uint16 nType,const vector<unsigned char>& vchData)
{
    ostringstream oss;
    oss << "INSERT INTO wallettemplate(tid,type,data) "
              "VALUES("
        <<            "\'" << dbConn.ToEscString(tid) << "\',"
        <<            nType << ","
        <<            "\'" << dbConn.ToEscString(vchData) << "\')";
    return dbConn.Query(oss.str());
}

bool CWalletDB::WalkThroughTemplate(CWalletDBTemplateWalker& walker)
{
    CMvDBRes res(dbConn,"SELECT tid,type,data FROM wallettemplate",true);
    while (res.GetRow())
    {
        uint256 tid;
        uint16 type;
        vector<unsigned char> vchData;
        if (!res.GetField(0,tid) || !res.GetField(1,type) || !res.GetField(2,vchData) 
            || !walker.Walk(tid,type,vchData))
        {
            return false;
        }
    }
    return true;
}

bool CWalletDB::AddNewTx(const CWalletTx& wtx)
{
    walleve::CWalleveBufStream ss;
    ss << wtx.vInput;
    string strEscIn = dbConn.ToEscString(ss.GetData(),ss.GetSize());

    ostringstream oss;
    oss << "INSERT INTO wallettx(txid,version,type,lockuntil,sendto,amount,txfee,destin,valuein,height,flags,fork,txin) "
              "VALUES("
        <<            "\'" << dbConn.ToEscString(wtx.txid) << "\',"
        <<            wtx.nVersion << ","
        <<            wtx.nType << ","
        <<            wtx.nLockUntil << ","
        <<            "\'" << dbConn.ToEscString(wtx.sendTo) << "\',"
        <<            wtx.nAmount << ","
        <<            wtx.nTxFee << ","
        <<            "\'" << dbConn.ToEscString(wtx.destIn) << "\',"
        <<            wtx.nValueIn << ","
        <<            wtx.nBlockHeight << ","
        <<            wtx.nFlags << ","
        <<            "\'" << dbConn.ToEscString(wtx.hashFork) << "\',"
        <<            "\'" << strEscIn << "\')";
    return dbConn.Query(oss.str());
}

bool CWalletDB::UpdateTx(const vector<CWalletTx>& vWalletTx,const vector<uint256>& vRemove)
{
    CMvDBTxn txn(dbConn);
    BOOST_FOREACH(const CWalletTx& wtx,vWalletTx)
    {
        walleve::CWalleveBufStream ss;
        ss << wtx.vInput;
        string strEscIn = dbConn.ToEscString(ss.GetData(),ss.GetSize());

        ostringstream oss;
        oss << "INSERT INTO wallettx(txid,version,type,lockuntil,sendto,amount,txfee,destin,valuein,height,flags,fork,txin) "
                  "VALUES("
            <<            "\'" << dbConn.ToEscString(wtx.txid) << "\',"
            <<            wtx.nVersion << ","
            <<            wtx.nType << ","
            <<            wtx.nLockUntil << ","
            <<            "\'" << dbConn.ToEscString(wtx.sendTo) << "\',"
            <<            wtx.nAmount << ","
            <<            wtx.nTxFee << ","
            <<            "\'" << dbConn.ToEscString(wtx.destIn) << "\',"
            <<            wtx.nValueIn << ","
            <<            wtx.nBlockHeight << ","
            <<            wtx.nFlags << ","
            <<            "\'" << dbConn.ToEscString(wtx.hashFork) << "\',"
            <<            "\'" << strEscIn << "\')"
            <<  " ON DUPLICATE KEY UPDATE "
                          "height = VALUES(height),flags = VALUES(flags)";
        txn.Query(oss.str());
    }
    BOOST_FOREACH(const uint256& txid,vRemove)
    {
        ostringstream oss;
        oss << "DELETE FROM wallettx WHERE txid = \'" << dbConn.ToEscString(txid) << "\'";
        txn.Query(oss.str());
    }
    return txn.Commit();
}

bool CWalletDB::RetrieveTx(const uint256& txid,CWalletTx& wtx)
{
    wtx.txid = txid;
    wtx.nRefCount = 0;
    ostringstream oss;
    oss << "SELECT version,type,lockuntil,sendto,amount,txfee,destin,valuein,height,flags,fork,txin FROM wallettx WHERE txid = "
        <<            "\'" << dbConn.ToEscString(txid) << "\'";

    vector<unsigned char> vchTxIn;
    CMvDBRes res(dbConn,oss.str());

    return (res.GetRow()
            && res.GetField(0,wtx.nVersion) && res.GetField(1,wtx.nType)
            && res.GetField(2,wtx.nLockUntil) && res.GetField(3,wtx.sendTo)
            && res.GetField(4,wtx.nAmount) && res.GetField(5,wtx.nTxFee)
            && res.GetField(6,wtx.destIn) && res.GetField(7,wtx.nValueIn)
            && res.GetField(8,wtx.nBlockHeight) && res.GetField(9,wtx.nFlags)
            && res.GetField(10,wtx.hashFork) && res.GetField(11,vchTxIn) && ParseTxIn(vchTxIn,wtx));
}

bool CWalletDB::ExistsTx(const uint256& txid)
{
    size_t count = 0;
    ostringstream oss;
    oss << "SELECT COUNT(*) FROM wallettx WHERE txid = \'" << dbConn.ToEscString(txid) << "\'";
    CMvDBRes res(dbConn,oss.str());
    return (res.GetRow() && res.GetField(0,count) && count != 0);
}

std::size_t CWalletDB::GetTxCount()
{
    size_t count = 0;
    CMvDBRes res(dbConn,"SELECT COUNT(*) FROM wallettx");
    if (res.GetRow())
    {
        res.GetField(0,count);
    }
    return count;
}

bool CWalletDB::ListTx(int nOffset,int nCount,std::vector<CWalletTx>& vWalletTx)
{
    ostringstream oss;
    oss << "SELECT txid,version,type,lockuntil,sendto,amount,txfee,destin,valuein,height,flags,fork,txin FROM wallettx ORDER BY id LIMIT " << nOffset << "," << nCount;
    vector<unsigned char> vchTxIn;
    CMvDBRes res(dbConn,oss.str(),true);
    while (res.GetRow())
    {
        CWalletTx wtx;
        if (res.GetField(0,wtx.txid) 
            && res.GetField(1,wtx.nVersion) && res.GetField(2,wtx.nType)
            && res.GetField(3,wtx.nLockUntil) && res.GetField(4,wtx.sendTo)
            && res.GetField(5,wtx.nAmount) && res.GetField(6,wtx.nTxFee)
            && res.GetField(7,wtx.destIn) && res.GetField(8,wtx.nValueIn)
            && res.GetField(9,wtx.nBlockHeight) && res.GetField(10,wtx.nFlags)
            && res.GetField(11,wtx.hashFork) && res.GetField(12,vchTxIn)
            && ParseTxIn(vchTxIn,wtx))
        {
            vWalletTx.push_back(wtx);
        }
        else
        {
            return false;
        }
    }
    return true;
}

bool CWalletDB::ListForkTx(const uint256& hashFork,int nMinHeight,vector<uint256>& vForkTx)
{
    ostringstream oss;
    oss << "SELECT txid FROM wallettx"
                 " WHERE fork=" << "\'" << dbConn.ToEscString(hashFork) << "\'"
                       " AND (height < 0 OR height >=" << nMinHeight << ")"           
                 " ORDER BY id";
    
    CMvDBRes res(dbConn,oss.str(),true);
    while (res.GetRow())
    {
        uint256 txid;
        if (res.GetField(0,txid))
        {
            vForkTx.push_back(txid);
        }
        else
        {
            return false;
        } 
    }
    return true;
}

bool CWalletDB::WalkThroughTx(CWalletDBTxWalker& walker)
{
    string strSelectUnspent = string("SELECT txid,version,type,lockuntil,sendto,amount,txfee,destin,valuein,height,flags,fork,txin FROM wallettx ORDER BY id");
    vector<unsigned char> vchTxIn;
    CMvDBRes res(dbConn,strSelectUnspent,true);
    while (res.GetRow())
    {
        CWalletTx wtx;
        if (!res.GetField(0,wtx.txid) 
            || !res.GetField(1,wtx.nVersion)     || !res.GetField(2,wtx.nType)
            || !res.GetField(3,wtx.nLockUntil)   || !res.GetField(4,wtx.sendTo)
            || !res.GetField(5,wtx.nAmount)      || !res.GetField(6,wtx.nTxFee)
            || !res.GetField(7,wtx.destIn)       || !res.GetField(8,wtx.nValueIn)
            || !res.GetField(9,wtx.nBlockHeight) || !res.GetField(10,wtx.nFlags)
            || !res.GetField(11,wtx.hashFork)    || !res.GetField(12,vchTxIn)
            || !ParseTxIn(vchTxIn,wtx)           || !walker.Walk(wtx))
        {
            return false;
        }
    }
    return true;
}

bool CWalletDB::ClearTx()
{
    return dbConn.Query("TRUNCATE TABLE wallettx");
}

bool CWalletDB::CheckWalletTx()
{
    //check if wallettx table only contains Tx's
    //which occurred on addresses owned by the wallet
    int rows = -1;
    string sql = R"(
                    SELECT
                        COUNT(DISTINCT txid)
                    FROM
                        wallettx
                    WHERE
                        RIGHT(sendto, 32) NOT IN
                            (SELECT
                                pubkey
                            FROM
                                walletkey
                            UNION
                            SELECT
                                tid
                            FROM
                                wallettemplate)
                        AND
                        RIGHT(destin, 32) NOT IN
                            (SELECT
                                pubkey
                            FROM
                                walletkey
                            UNION
                            SELECT
                                tid
                            FROM
                                wallettemplate)
                    )";
    {
        CMvDBRes res(dbConn, sql, true);
        if(!res.GetRow() || !res.GetField<int>(0, rows) || rows != 0)
        {
            return false;
        }
    }

    //check if wallet tx set is equal to transaction one
    //which belong to the wallet
    rows = -1;
    sql = R"(
            SELECT
                COUNT(txid)
            FROM
                (SELECT DISTINCT
                    transaction.txid AS txid
                FROM
                    transaction
                WHERE
                    RIGHT(sendto, 32) IN
                        (SELECT
                            pubkey
                        FROM
                            walletkey
                        UNION
                        SELECT
                            tid
                        FROM
                            wallettemplate)
                    OR
                    RIGHT(destin, 32) IN
                        (SELECT
                            pubkey
                        FROM
                            walletkey
                        UNION
                        SELECT
                            tid
                        FROM
                            wallettemplate)
                UNION ALL
                SELECT DISTINCT
                    wallettx.txid AS txid
                FROM
                    wallettx
                WHERE
                    RIGHT(sendto, 32) IN
                        (SELECT
                            pubkey
                        FROM
                            walletkey
                        UNION
                        SELECT
                            tid
                        FROM
                            wallettemplate)
                    OR
                    RIGHT(destin, 32) IN
                        (SELECT
                            pubkey
                        FROM
                            walletkey UNION SELECT
                            tid
                        FROM
                            wallettemplate)) diffset
            GROUP BY txid
            HAVING COUNT(txid) = 1
            )";
    {
        CMvDBRes res(dbConn, sql, true);
        if(!res.GetRow() || !res.GetField<int>(0, rows) || rows != 0)
        {
            return false;
        }
    }

    //at this point, both of transaction sets for wallet
    //are equal, then we should validate detailed fields
    //between tables of wallettx and transaction row by row
    rows = -1;
    sql = R"(
            SELECT
                COUNT(*)
            FROM
                transaction t
                    INNER JOIN
                wallettx w ON t.txid = w.txid
            WHERE
                t.version <> w.version
                OR t.type <> w.type
                OR t.lockuntil <> w.lockuntil
                OR t.sendto <> w.sendto
                OR t.amount <> w.amount
                OR t.destin <> w.destin
                OR t.valuein <> w.valuein
                OR t.height <> w.height
            )";
    {
        CMvDBRes res(dbConn, sql, true);
        if(!res.GetRow() || !res.GetField<int>(0, rows) || rows != 0)
        {
            return false;
        }
    }
    return true;
}

bool CWalletDB::ParseTxIn(const std::vector<unsigned char>& vchTxIn,CWalletTx& wtx)
{
    walleve::CWalleveBufStream ss;
    ss.Write((char*)&vchTxIn[0],vchTxIn.size());
    try
    {
        ss >> wtx.vInput;
    }
    catch (exception& e)
    {
        StdError(__PRETTY_FUNCTION__, e.what());
        return false; 
    }
    
    return true;
}

bool CWalletDB::CreateTable()
{
    return dbConn.Query("CREATE TABLE IF NOT EXISTS walletkey("
                          "id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,"
                          "pubkey BINARY(32) NOT NULL UNIQUE KEY,"
                          "version INT NOT NULL,"
                          "encrypted BINARY(48) NOT NULL,"
                          "nonce BIGINT UNSIGNED NOT NULL)"
                        " ENGINE=InnoDB"
                       )
             &&
           dbConn.Query("CREATE TABLE IF NOT EXISTS wallettemplate("
                          "id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,"
                          "tid BINARY(32) NOT NULL UNIQUE KEY,"
                          "type SMALLINT NOT NULL,"
                          "data VARBINARY(640) NOT NULL)"
                        " ENGINE=InnoDB"
                       )
             &&
           dbConn.Query("CREATE TABLE IF NOT EXISTS wallettx("
                          "id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,"
                          "txid BINARY(32) NOT NULL UNIQUE KEY,"
                          "version SMALLINT UNSIGNED NOT NULL,"
                          "type SMALLINT UNSIGNED NOT NULL,"
                          "lockuntil INT UNSIGNED NOT NULL,"
                          "sendto BINARY(33) NOT NULL,"
                          "amount BIGINT NOT NULL,"
                          "txfee BIGINT NOT NULL,"
                          "destin BINARY(33) NOT NULL,"
                          "valuein BIGINT NOT NULL,"
                          "height INT NOT NULL,"
                          "flags INT NOT NULL,"
                          "fork BINARY(32) NOT NULL,"
                          "txin BLOB NOT NULL)"
                        " ENGINE=InnoDB"
                       );
}
