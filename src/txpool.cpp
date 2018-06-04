// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "txpool.h"

using namespace std;
using namespace walleve;
using namespace multiverse;

//////////////////////////////
// CTxPool 

CTxPool::CTxPool()
{
    pCoreProtocol = NULL;
    pWorldLine = NULL;
}

CTxPool::~CTxPool()
{
}

bool CTxPool::WalleveHandleInitialize()
{
    if (!WalleveGetObject("coreprotocol",pCoreProtocol))
    {
        WalleveLog("Failed to request coreprotocol\n");
        return false;
    }

    if (!WalleveGetObject("worldline",pWorldLine))
    {
        WalleveLog("Failed to request worldline\n");
        return false;
    }

    return true;
}

void CTxPool::WalleveHandleDeinitialize()
{
    pCoreProtocol = NULL;
    pWorldLine = NULL;
}

bool CTxPool::WalleveHandleInvoke()
{
    return true;
}

void CTxPool::WalleveHandleHalt()
{
    Clear();
}

bool CTxPool::Exists(const uint256& txid)
{    
    boost::shared_lock<boost::shared_mutex> rlock(rwAccess);
    return (!!mapTx.count(txid));
}

void CTxPool::Clear()
{
    boost::unique_lock<boost::shared_mutex> wlock(rwAccess);
    mapTxCache.clear();
    mapTx.clear();
}

size_t CTxPool::Count(const uint256& fork) const
{
    boost::shared_lock<boost::shared_mutex> rlock(rwAccess);
    map<uint256,CTxCache>::const_iterator it = mapTxCache.find(fork);
    if (it != mapTxCache.end())
    {
        return ((*it).second.Size());
    }
    return 0;
}

bool CTxPool::Push(const uint256& hashFork,const uint256& txid,CTransaction& tx)
{
    return false;
}

bool CTxPool::Pop(const uint256& hashFork,const uint256& txid)
{
    return false;
}

MvErr CTxPool::AddNew(const CTransaction& tx)
{
    return MV_OK;
}

void CTxPool::Remove(const uint256& txid)
{
}

bool CTxPool::Get(const uint256& txid,CTransaction& tx) const
{
    return false;
}

bool CTxPool::Arrange(uint256& fork,vector<pair<uint256,CTransaction> >& vtx,size_t nMaxSize)
{
    return false;
}

bool CTxPool::FetchInputs(uint256& fork,const CTransaction& tx,vector<CTxOutput>& vUnspent)
{
    return false;
}

void CTxPool::ForkUpdate(uint256& fork,const vector<CBlockTx>& vAddNew,const vector<uint256>& vRemove)
{
}




