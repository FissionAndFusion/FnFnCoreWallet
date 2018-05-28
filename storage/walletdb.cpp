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

bool CWalletDB::AddNewKey(const uint256& pubkey,const crypto::CCryptoCipher& cipher)
{
    ostringstream oss;
    oss << "INSERT INTO key(pubkey,encrypted,nonce) "
              "VALUES("
        <<            "\'" << dbConn.ToEscString(pubkey) << "\',"
        <<            "\'" << dbConn.ToEscString(cipher.encrypted,48) << "\',"
        <<            cipher.nonce << ")";
    return dbConn.Query(oss.str());
}

bool CWalletDB::WalkThroughKey(CWalletDBKeyWalker& walker)
{
    CMvDBRes res(dbConn,"SELECT pubkey,encrypted,nonce FROM key",true);
    while (res.GetRow())
    {
        uint256 pubkey;
        crypto::CCryptoCipher cipher;
        if (!res.GetField(0,pubkey) || !res.GetBinary(1,cipher.encrypted,48) || !res.GetField(2,cipher.nonce) 
            || !walker.Walk(pubkey,cipher))
        {
            return false;
        }
    }
    return true;
}

bool CWalletDB::CreateTable()
{
    return dbConn.Query("CREATE TABLE IF NOT EXISTS key("
                          "id INT NOT NULL AUTO_INCREMENT,"
                          "pubkey BINARY(32) NOT NULL UNIQUE KEY,"
                          "encrypted BINARY(48) NOT NULL,"
                          "nonce BIGINT UNSIGNED NOT NULL)"
                       );
}
