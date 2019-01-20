// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_DBPOOL_H
#define  MULTIVERSE_DBPOOL_H

#include <set>
#include <queue>
#include <boost/thread/thread.hpp>
#include "dbconn.h"

namespace multiverse
{
namespace storage
{

class CMvDBPool
{
public:
    CMvDBPool();
    ~CMvDBPool();
    bool Initialize(const CMvDBConfig& config,int nMaxInstance);
    void Deinitialize();
    CMvDBConn* Alloc();
    void Free(CMvDBConn* pConn);
protected:
    bool Create(const CMvDBConfig& config,int nMaxInstance);
    void Destroy();
protected:
    boost::mutex mtxPool;
    boost::condition_variable condPool;
    std::set<CMvDBConn*> setDBConn;
    std::queue<CMvDBConn*> queFreeDBConn;
    bool fAbort;
};

class CMvDBInst
{
public:
    CMvDBInst(CMvDBPool* pDBPoolIn) : pDBPool(pDBPoolIn)
    {
        pDBConn = pDBPool->Alloc();
    }
    ~CMvDBInst()
    {
        pDBPool->Free(pDBConn);
    }
    bool Available() const { return (pDBConn != NULL); }
    CMvDBConn& operator *() { return *pDBConn; }
    CMvDBConn* operator ->() { return pDBConn; }
protected:
    CMvDBConn* pDBConn;
    CMvDBPool* pDBPool;
};

} // namespace storage
} // namespace multiverse

#endif //MULTIVERSE_DBPOOL_H

