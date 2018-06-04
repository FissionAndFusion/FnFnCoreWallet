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

bool CWalletDB::CreateTable()
{
    return dbConn.Query("CREATE TABLE IF NOT EXISTS walletkey("
                          "id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,"
                          "pubkey BINARY(32) NOT NULL UNIQUE KEY,"
                          "version INT NOT NULL,"
                          "encrypted BINARY(48) NOT NULL,"
                          "nonce BIGINT UNSIGNED NOT NULL)"
                       );
}
