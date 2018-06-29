// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
    
#include "walletdb.h"
#include "walleve/stream/datastream.h"
    
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
}

bool CWalletDB::UpdateTxSpent(const uint256& txid,int n,const uint256& hashSpent)
{
}

bool CWalletDB::RetrieveTx(const uint256& txid,CWalletTx& wtx)
{
}

bool CWalletDB::ExistsTx(const uint256& txid)
{
}

bool CWalletDB::WalkThroughUnspent(CWalletDBTxWalker& walker)
{
}

bool CWalletDB::ListTx(const uint256& txidPrev,int nCount,std::vector<CWalletTx>& vWalletTx)
{
}

bool CWalletDB::CreateTable()
{
    return dbConn.Query("CREATE TABLE IF NOT EXISTS walletkey("
                          "id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,"
                          "pubkey BINARY(32) NOT NULL UNIQUE KEY,"
                          "version INT NOT NULL,"
                          "encrypted BINARY(48) NOT NULL,"
                          "nonce BIGINT UNSIGNED NOT NULL)"
                       )
             &&
           dbConn.Query("CREATE TABLE IF NOT EXISTS wallettemplate("
                          "id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,"
                          "tid BINARY(32) NOT NULL UNIQUE KEY,"
                          "type SMALLINT NOT NULL,"
                          "data VARBINARY(640) NOT NULL)"
                       );
}
