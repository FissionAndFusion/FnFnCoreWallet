// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "dbpool.h"

using namespace std;
using namespace multiverse::storage;

//////////////////////////////
// CMvDBPool

CMvDBPool::CMvDBPool()
{
    fAbort = false;
}

CMvDBPool::~CMvDBPool()
{
    Destroy();
}

bool CMvDBPool::Initialize(const CMvDBConfig& config,int nMaxInstance)
{
    {
        boost::unique_lock<boost::mutex> lock(mtxPool);

        if (!Create(config,nMaxInstance))
        {
            Destroy();
            return false;
        }
        fAbort = false;
    }
    condPool.notify_all();
    return true;
}

void CMvDBPool::Deinitialize()
{
    {
        boost::unique_lock<boost::mutex> lock(mtxPool);
        fAbort = true;
    }
    condPool.notify_all();
}

CMvDBConn* CMvDBPool::Alloc()
{
    CMvDBConn* pConn = NULL;
    boost::unique_lock<boost::mutex> lock(mtxPool);
    while (!fAbort && queFreeDBConn.empty())
    {
        condPool.wait(lock);
    }
    if (!fAbort && !queFreeDBConn.empty())
    {
        pConn = queFreeDBConn.front();
        queFreeDBConn.pop();
    }
    return pConn;
}

void CMvDBPool::Free(CMvDBConn* pConn)
{
    bool fNotify = false;
    if (pConn != NULL)
    {
        boost::unique_lock<boost::mutex> lock(mtxPool);
        if (!fAbort && setDBConn.count(pConn))
        {
            fNotify = queFreeDBConn.empty();
            pConn->Reset(); 
            queFreeDBConn.push(pConn);
        }
    }
    if (fNotify)
    {
        condPool.notify_one();
    }
}

bool CMvDBPool::Create(const CMvDBConfig& config,int nMaxInstance)
{
    Destroy();

    while (setDBConn.size() < nMaxInstance)
    {
        CMvDBConn* pConn = new CMvDBConn();
        if (pConn == NULL)
        {
            return false;
        }
        if (!pConn->Connect(config))
        {
            delete pConn;
            cerr << " Failed: MySQL connection";
            return false;
        }

        setDBConn.insert(pConn);
        queFreeDBConn.push(pConn);
    }
    return true;
}

void CMvDBPool::Destroy()
{
    for (set<CMvDBConn*>::iterator it = setDBConn.begin();it != setDBConn.end();++it)
    {
        delete (*it);
    }
    setDBConn.clear();

    while (!queFreeDBConn.empty())
    {
        queFreeDBConn.pop();
    }
}
