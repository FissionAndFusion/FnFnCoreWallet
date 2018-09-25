// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
    
#include "walletdb.h"
#include <boost/foreach.hpp>    
using namespace std;
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
    ostringstream oss;
    oss << "INSERT INTO wallettx(txid,version,type,lockuntil,sendto,amount,txfee,destin,valuein,height,flags,fork,txspent,txchange) "
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
        <<            "\'" << dbConn.ToEscString(wtx.txidSpent) << "\',"
        <<            "\'" << dbConn.ToEscString(wtx.txidChange) << "\')";
    return dbConn.Query(oss.str());
}

bool CWalletDB::UpdateTx(const vector<CWalletTx>& vWalletTx,const vector<uint256>& vRemove)
{
    CMvDBTxn txn(dbConn);
    BOOST_FOREACH(const CWalletTx& wtx,vWalletTx)
    {
        ostringstream oss;
        oss << "INSERT INTO wallettx(txid,version,type,lockuntil,sendto,amount,txfee,destin,valuein,height,flags,fork,txspent,txchange) "
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
            <<            "\'" << dbConn.ToEscString(wtx.txidSpent) << "\',"
            <<            "\'" << dbConn.ToEscString(wtx.txidChange) << "\')"
            <<  " ON DUPLICATE KEY UPDATE "
                          "height = VALUES(height),flags = VALUES(flags),"
                          "txspent = VALUES(txspent),txchange = VALUES(txchange)";
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
    ostringstream oss;
    oss << "SELECT version,type,lockuntil,sendto,amount,txfee,destin,valuein,height,flags,fork,txspent,txchange FROM wallettx WHERE txid = "
        <<            "\'" << dbConn.ToEscString(txid) << "\'";
    CMvDBRes res(dbConn,oss.str());
    return (res.GetRow()
            && res.GetField(0,wtx.nVersion) && res.GetField(1,wtx.nType)
            && res.GetField(2,wtx.nLockUntil) && res.GetField(3,wtx.sendTo)
            && res.GetField(4,wtx.nAmount) && res.GetField(5,wtx.nTxFee)
            && res.GetField(6,wtx.destIn) && res.GetField(7,wtx.nValueIn)
            && res.GetField(8,wtx.nBlockHeight) && res.GetField(9,wtx.nFlags)
            && res.GetField(10,wtx.hashFork) && res.GetField(11,wtx.txidSpent)
            && res.GetField(12,wtx.txidChange));
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
    oss << "SELECT txid,version,type,lockuntil,sendto,amount,txfee,destin,valuein,height,flags,fork,txspent,txchange FROM wallettx ORDER BY id LIMIT " << nOffset << "," << nCount;
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
            && res.GetField(11,wtx.hashFork) && res.GetField(12,wtx.txidSpent)
            && res.GetField(13,wtx.txidChange))
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

bool CWalletDB::WalkThroughUnspent(CWalletDBTxWalker& walker)
{
    string zero = dbConn.ToEscString(uint256(0));
    string strSelectUnspent = string("SELECT txid,version,type,lockuntil,sendto,amount,txfee,destin,valuein,height,flags,fork,txspent,txchange FROM wallettx WHERE txspent = \'") 
                              + zero 
                              + string("\' OR txchange = \'")
                              + zero
                              + "\'";

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
            || !res.GetField(11,wtx.hashFork)    || !res.GetField(12,wtx.txidSpent)
            || !res.GetField(13,wtx.txidChange)  || !walker.Walk(wtx))
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

bool CWalletDB::CreateTable()
{
    return dbConn.Query("CREATE TABLE IF NOT EXISTS walletkey("
                          "id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,"
                          "pubkey BINARY(32) NOT NULL UNIQUE KEY,"
                          "version INT NOT NULL,"
                          "encrypted BINARY(48) NOT NULL,"
                          "nonce BIGINT UNSIGNED NOT NULL)"
                          "ENGINE=InnoDB"
                       )
             &&
           dbConn.Query("CREATE TABLE IF NOT EXISTS wallettemplate("
                          "id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,"
                          "tid BINARY(32) NOT NULL UNIQUE KEY,"
                          "type SMALLINT NOT NULL,"
                          "data VARBINARY(640) NOT NULL)"
                          "ENGINE=InnoDB"
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
                          "txspent BINARY(32) NOT NULL,"
                          "txchange BINARY(32) NOT NULL)"
                          "ENGINE=InnoDB"
                       );
}
