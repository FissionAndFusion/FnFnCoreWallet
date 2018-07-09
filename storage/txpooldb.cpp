// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
    
#include "txpooldb.h"
#include "walleve/stream/stream.h"
#include <boost/foreach.hpp>    
using namespace std;
using namespace multiverse::storage;
    
//////////////////////////////
// CTxPoolDB

CTxPoolDB::CTxPoolDB()
{   
}   
    
CTxPoolDB::~CTxPoolDB()
{   
    Deinitialize();
}

bool CTxPoolDB::Initialize(const CMvDBConfig& config)
{
    if (!dbConn.Connect(config))
    {
        return false;
    }
    return CreateTable();
}

void CTxPoolDB::Deinitialize()
{
    dbConn.Disconnect();
}

bool CTxPoolDB::UpdateTx(const uint256& hashFork,const vector<pair<uint256,CAssembledTx> >& vAddNew,
                                                 const vector<uint256>& vRemove)
{
    string strEscHashFork = dbConn.ToEscString(hashFork);

    CMvDBTxn txn(dbConn);
    for (int i = 0;i < vAddNew.size();i++)
    {
        const CAssembledTx& tx = vAddNew[i].second;
        walleve::CWalleveBufStream ss;
        ss << tx;
        string strEscTx = dbConn.ToEscString(ss.GetData(),ss.GetSize());

        ostringstream oss;
        oss << "INSERT INTO txpool(txid,fork,tx) "
                  "VALUES("
            <<            "\'" << dbConn.ToEscString(vAddNew[i].first) << "\',"
            <<            "\'" << strEscHashFork << "\',"
            <<            "\'" << strEscTx << "\')";
        txn.Query(oss.str());
    }
    BOOST_FOREACH(const uint256& txid,vRemove)
    {
        ostringstream oss;
        oss << "DELETE FROM txpool WHERE txid = \'" << dbConn.ToEscString(txid) << "\'";
        txn.Query(oss.str());
    }
    return txn.Commit();
}

bool CTxPoolDB::WalkThroughTx(CTxPoolDBTxWalker& walker)
{
    CMvDBRes res(dbConn,"SELECT txid,fork,tx FROM txpool",true);
    while (res.GetRow())
    {
        uint256 txid,fork;
        vector<unsigned char> vchTx;
        if (!res.GetField(0,txid) || !res.GetField(1,fork) || !res.GetField(2,vchTx) || vchTx.empty())
        {
            return false;
        }
        walleve::CWalleveBufStream ss;
        ss.Write((char*)&vchTx[0],vchTx.size());
        CAssembledTx tx;
        try
        {
            ss >> tx;
            if (!walker.Walk(txid,fork,tx))
            {
                return false;
            }
        }
        catch (...)
        {
            return false;
        }
    }
    return true;
}

bool CTxPoolDB::CreateTable()
{
    return dbConn.Query("CREATE TABLE IF NOT EXISTS txpool("
                          "id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,"
                          "txid BINARY(32) NOT NULL UNIQUE KEY,"
                          "fork BINARY(32) NOT NULL,"
                          "tx BLOB NOT NULL)"
                       );
}
