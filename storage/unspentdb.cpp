// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
    
#include "unspentdb.h"
#include "leveldbeng.h"

#include <boost/bind.hpp>   

using namespace std;
using namespace walleve;
using namespace multiverse::storage;

#define UNSPENT_FLUSH_INTERVAL                  (60)
    
//////////////////////////////
// CForkUnspentDB

CForkUnspentDB::CForkUnspentDB(const boost::filesystem::path& pathDB)
{
    CLevelDBArguments args;
    args.path = pathDB.string();
    args.syncwrite = false;
    CLevelDBEngine *engine = new CLevelDBEngine(args);

    if (!CKVDB::Open(engine))
    {
        delete engine;
    }
}   
    
CForkUnspentDB::~CForkUnspentDB()
{
    Close();
    dblCache.Clear();
}

bool CForkUnspentDB::RemoveAll()
{
    if (!CKVDB::RemoveAll())
    {
        return false;
    }
    dblCache.Clear();
    return true;
}

bool CForkUnspentDB::UpdateUnspent(const vector<CTxUnspent>& vAddNew,const vector<CTxOutPoint>& vRemove)
{
    walleve::CWalleveWriteLock wlock(rwUpper);

    MapType& mapUpper = dblCache.GetUpperMap();

    for(const CTxUnspent& unspent : vAddNew)
    {
        mapUpper[static_cast<const CTxOutPoint&>(unspent)] = unspent.output;
    }

    for(const CTxOutPoint& txout : vRemove)
    {
        mapUpper[txout].SetNull();
    }

    return true;
}

bool CForkUnspentDB::WriteUnspent(const CTxOutPoint& txout,const CTxOutput& output)
{
    return Write(txout,output);
}

bool CForkUnspentDB::ReadUnspent(const CTxOutPoint& txout,CTxOutput& output)
{
    {
        walleve::CWalleveReadLock rlock(rwUpper);

        MapType& mapUpper = dblCache.GetUpperMap();
        typename MapType::iterator it = mapUpper.find(txout);
        if (it != mapUpper.end())
        {
            if (!(*it).second.IsNull())
            {
                output = (*it).second;
                return true;
            }
            return false;
        }
    }

    if (rwLower.ReadTryLock())
    {
        MapType& mapLower = dblCache.GetLowerMap();
        typename MapType::iterator it = mapLower.find(txout);
        if (it != mapLower.end())
        {
            if (!(*it).second.IsNull())
            {
                output = (*it).second;
                rwLower.ReadUnlock();
                return true;
            }
            rwLower.ReadUnlock();
            return false;
        }
        rwLower.ReadUnlock();
    }

    return Read(txout,output);
}

bool CForkUnspentDB::Copy(CForkUnspentDB& dbUnspent)
{
    if (!dbUnspent.RemoveAll())
    {
        return false;
    }

    try
    {
        walleve::CWalleveReadLock rulock(rwUpper);
        walleve::CWalleveReadLock rdlock(rwLower);

        if (!WalkThrough(boost::bind(&CForkUnspentDB::CopyWalker,this,_1,_2,boost::ref(dbUnspent))))
        {
            return false;
        }

        dbUnspent.SetCache(dblCache); 
    }
    catch (exception& e)
    {
        StdError(__PRETTY_FUNCTION__, e.what());
        return false;
    }
    return true;
}

bool CForkUnspentDB::WalkThroughUnspent(CForkUnspentDBWalker& walker)
{
    try
    {
        walleve::CWalleveReadLock rulock(rwUpper);
        walleve::CWalleveReadLock rdlock(rwLower);

        MapType& mapUpper = dblCache.GetUpperMap();
        MapType& mapLower = dblCache.GetLowerMap();

        if (!WalkThrough(boost::bind(&CForkUnspentDB::LoadWalker,this,_1,_2,boost::ref(walker),
                                     boost::ref(mapUpper),boost::ref(mapLower))))
        {
            return false;
        }

        for (MapType::iterator it = mapLower.begin();it != mapLower.end();++it)
        {
            const CTxOutPoint& txout = (*it).first;
            const CTxOutput& output  = (*it).second;
            if (!mapUpper.count(txout) && !output.IsNull())
            {
                if (!walker.Walk(txout,output))
                {
                    return false;
                }
            }
        }
        for (MapType::iterator it = mapUpper.begin();it != mapUpper.end();++it)
        {
            const CTxOutPoint& txout = (*it).first;
            const CTxOutput& output  = (*it).second;
            if (!output.IsNull())
            {
                if (!walker.Walk(txout,output))
                {
                    return false;
                }
            }
        }
    }
    catch (exception& e)
    {
        StdError(__PRETTY_FUNCTION__, e.what());
        return false;
    }
    return true;
}

bool CForkUnspentDB::CopyWalker(CWalleveBufStream& ssKey, CWalleveBufStream& ssValue,
                                CForkUnspentDB& dbUnspent)
{
    CTxOutPoint txout;
    CTxOutput output;
    ssKey >> txout;
    ssValue >> output;
    
    return dbUnspent.WriteUnspent(txout,output);
}

bool CForkUnspentDB::LoadWalker(CWalleveBufStream& ssKey, CWalleveBufStream& ssValue,
                                CForkUnspentDBWalker& walker,const MapType& mapUpper,const MapType& mapLower)
{
    CTxOutPoint txout;
    CTxOutput output;
    ssKey >> txout;

    if (mapUpper.count(txout) || mapLower.count(txout))
    {
        return true;
    }

    ssValue >> output;
     
    return walker.Walk(txout,output);
}

bool CForkUnspentDB::Flush()
{
    walleve::CWalleveUpgradeLock ulock(rwLower);

    vector<pair<CTxOutPoint,CTxOutput> > vAddNew;
    vector<CTxOutPoint> vRemove;

    MapType& mapLower = dblCache.GetLowerMap();
    for (typename MapType::iterator it = mapLower.begin(); it != mapLower.end(); ++it)
    {
        CTxOutput& output = (*it).second;
        if (!output.IsNull())
        {
            vAddNew.push_back(*it);
        }
        else
        {
            vRemove.push_back((*it).first);
        }
    }

    if (!TxnBegin())
    {
        return false;
    }

    for (int i = 0;i < vAddNew.size();i++)
    {
        Write(vAddNew[i].first,vAddNew[i].second);
    }

    for (int i = 0;i < vRemove.size();i++)
    {
        Erase(vRemove[i]);
    }

    if (!TxnCommit())
    {
        return false;
    }

    ulock.Upgrade();

    {
        walleve::CWalleveWriteLock wlock(rwUpper);
        dblCache.Flip();
    }

    return true;
}

//////////////////////////////
// CUnspentDB

CUnspentDB::CUnspentDB()
{
    pThreadFlush = NULL;
    fStopFlush = true;
}

bool CUnspentDB::Initialize(const boost::filesystem::path& pathData)
{
    pathUnspent = pathData / "unspent";

    if (!boost::filesystem::exists(pathUnspent))
    {
        boost::filesystem::create_directories(pathUnspent);
    }

    if (!boost::filesystem::is_directory(pathUnspent))
    {
        return false;
    }

    fStopFlush = false;
    pThreadFlush = new boost::thread(boost::bind(&CUnspentDB::FlushProc,this));
    if (pThreadFlush == NULL)
    {
        fStopFlush = true;
        return false;
    }

    return true;
}

void CUnspentDB::Deinitialize()
{
    if (pThreadFlush)
    {
        {
            boost::unique_lock<boost::mutex> lock(mtxFlush);
            fStopFlush = true;
        }
        condFlush.notify_all();
        pThreadFlush->join();
        delete pThreadFlush;
        pThreadFlush = NULL;
    }
    
    {
        CWalleveWriteLock wlock(rwAccess);

        for (map<uint256,std::shared_ptr<CForkUnspentDB> >::iterator it = mapUnspentDB.begin();
             it != mapUnspentDB.end();++it)
        {
            std::shared_ptr<CForkUnspentDB> spUnspent = (*it).second;

            spUnspent->Flush();
            spUnspent->Flush();
        }
        mapUnspentDB.clear();
    }
}

bool CUnspentDB::AddNewFork(const uint256& hashFork)
{
    CWalleveWriteLock wlock(rwAccess);
    
    map<uint256,std::shared_ptr<CForkUnspentDB> >::iterator it = mapUnspentDB.find(hashFork);
    if (it != mapUnspentDB.end())
    {
        return true;
    }

    std::shared_ptr<CForkUnspentDB> spUnspent(new CForkUnspentDB(pathUnspent / hashFork.GetHex()));
    if (spUnspent == NULL || !spUnspent->IsValid())
    {
        return false;
    }
    mapUnspentDB.insert(make_pair(hashFork,spUnspent));
    return true;
}

bool CUnspentDB::RemoveFork(const uint256& hashFork)
{
    CWalleveWriteLock wlock(rwAccess);

    map<uint256,std::shared_ptr<CForkUnspentDB> >::iterator it = mapUnspentDB.find(hashFork);
    if (it != mapUnspentDB.end())
    {
        (*it).second->RemoveAll();
        mapUnspentDB.erase(it);
        return true;
    }
    return false;
}

void CUnspentDB::Clear()
{
    CWalleveWriteLock wlock(rwAccess);

    map<uint256,std::shared_ptr<CForkUnspentDB> >::iterator it = mapUnspentDB.begin();
    while (it != mapUnspentDB.end())
    {
        (*it).second->RemoveAll();
        mapUnspentDB.erase(it++);
    }
}

bool CUnspentDB::Update(const uint256& hashFork,
                        const vector<CTxUnspent>& vAddNew,const vector<CTxOutPoint>& vRemove)
{
    CWalleveReadLock rlock(rwAccess);

    map<uint256,std::shared_ptr<CForkUnspentDB> >::iterator it = mapUnspentDB.find(hashFork);
    if (it != mapUnspentDB.end())
    {
        return (*it).second->UpdateUnspent(vAddNew,vRemove);
    }
    return false;
}

bool CUnspentDB::Retrieve(const uint256& hashFork,const CTxOutPoint& txout,CTxOutput& output)
{
    CWalleveReadLock rlock(rwAccess);

    map<uint256,std::shared_ptr<CForkUnspentDB> >::iterator it = mapUnspentDB.find(hashFork);
    if (it != mapUnspentDB.end())
    {
        return (*it).second->ReadUnspent(txout,output);
    }
    return false;
}

bool CUnspentDB::Copy(const uint256& srcFork,const uint256& destFork)
{
    CWalleveReadLock rlock(rwAccess);

    map<uint256,std::shared_ptr<CForkUnspentDB> >::iterator itSrc = mapUnspentDB.find(srcFork);
    if (itSrc == mapUnspentDB.end())
    {
        return false;
    }

    map<uint256,std::shared_ptr<CForkUnspentDB> >::iterator itDest = mapUnspentDB.find(destFork);
    if (itDest == mapUnspentDB.end())
    {
        return false;
    }

    return ((*itSrc).second->Copy(*(*itDest).second));
}

bool CUnspentDB::WalkThrough(const uint256& hashFork,CForkUnspentDBWalker& walker)
{
    CWalleveReadLock rlock(rwAccess);

    map<uint256,std::shared_ptr<CForkUnspentDB> >::iterator it = mapUnspentDB.find(hashFork);
    if (it != mapUnspentDB.end())
    {
        return (*it).second->WalkThroughUnspent(walker);
    }
    return false;
}

void CUnspentDB::FlushProc()
{
    boost::system_time timeout = boost::get_system_time();

    boost::unique_lock<boost::mutex> lock(mtxFlush);
    while (!fStopFlush)
    {
        timeout += boost::posix_time::seconds(UNSPENT_FLUSH_INTERVAL);

        while (!fStopFlush)
        {
            if (!condFlush.timed_wait(lock,timeout))
            {
                break;
            }
        }

        if (!fStopFlush)
        {
            vector<std::shared_ptr<CForkUnspentDB> > vUnspentDB;
            vUnspentDB.reserve(mapUnspentDB.size());
            {
                CWalleveReadLock rlock(rwAccess);

                for (map<uint256,std::shared_ptr<CForkUnspentDB> >::iterator it = mapUnspentDB.begin();
                     it != mapUnspentDB.end();++it)
                {
                    vUnspentDB.push_back((*it).second);
                }
            }
            for (int i = 0;i < vUnspentDB.size();i++)
            {
                vUnspentDB[i]->Flush();
            }
        }
    }
}
