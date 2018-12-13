// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
    
#include "txpooldb.h"
#include "leveldbeng.h"

#include <boost/bind.hpp>
#include <boost/foreach.hpp>    

using namespace std;
using namespace walleve;
using namespace multiverse::storage;
    
//////////////////////////////
// CTxPoolDB

CTxPoolDB::CTxPoolDB()
: nSequence(0)
{   
}   
    
CTxPoolDB::~CTxPoolDB()
{   
}

bool CTxPoolDB::Initialize(const boost::filesystem::path& pathData)
{
    CLevelDBArguments args;
    args.path = (pathData / "txpool").string();
    args.syncwrite = false;
    CLevelDBEngine *engine = new CLevelDBEngine(args);

    if (!Open(engine))
    {
        delete engine;
        return false;
    }

    return true;
}

void CTxPoolDB::Deinitialize()
{
    Close();
}

bool CTxPoolDB::RemoveAll()
{
    if (!CKVDB::RemoveAll())
    {
        return false;
    }
    nSequence = 0;
    return true;
}

bool CTxPoolDB::UpdateTx(const uint256& hashFork,const vector<pair<uint256,CAssembledTx> >& vAddNew,
                                                 const vector<uint256>& vRemove)
{
    if (vAddNew.size() == 1 && vRemove.empty())
    {
        return Write(vAddNew[0].first,make_pair(nSequence++,make_pair(hashFork,vAddNew[0].second)));
    }
    
    if (!TxnBegin())
    {
        return false;
    }

    for (int i = 0;i < vAddNew.size();i++)
    {
        Write(vAddNew[i].first,make_pair(nSequence++,make_pair(hashFork,vAddNew[i].second)));
    }

    BOOST_FOREACH(const uint256& txid,vRemove)
    {
        Erase(txid);
    }

    return TxnCommit();
}

bool CTxPoolDB::WalkThroughTx(CTxPoolDBTxWalker& walker)
{
    map<uint64,pair<uint256,pair<uint256,CAssembledTx> > > mapTx;
    try
    {
        if (!WalkThrough(boost::bind(&CTxPoolDB::LoadWalker,this,_1,_2,boost::ref(mapTx))))
        {
            return false;
        }

        map<uint64,pair<uint256,pair<uint256,CAssembledTx> > >::iterator it = mapTx.begin();
        while (it != mapTx.end())
        {
            pair<uint256,CAssembledTx>& pairTx = (*it).second.second;
            if (!walker.Walk((*it).second.first,pairTx.first,pairTx.second))
            {
                return false;
            }
            ++it;
        }
        nSequence = mapTx.empty() ? 0 : (*mapTx.rbegin()).first + 1;
    }
    catch (exception& e)
    {
        StdError(__PRETTY_FUNCTION__, e.what());
        return false;
    }
    return true;
}

bool CTxPoolDB::LoadWalker(CWalleveBufStream& ssKey, CWalleveBufStream& ssValue,
                           map<uint64,pair<uint256,pair<uint256,CAssembledTx> > > & mapTx)
{
    uint256 txid;
    uint64 n;
    pair<uint256,CAssembledTx> pairTx;
    ssKey >> txid;
    ssValue >> n >> pairTx;
    
    mapTx.insert(make_pair(n,make_pair(txid,pairTx)));

    return true;
}
