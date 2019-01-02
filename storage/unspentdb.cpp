// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
    
#include "unspentdb.h"
#include "leveldbeng.h"

#include <boost/bind.hpp>
#include <boost/foreach.hpp>    

using namespace std;
using namespace walleve;
using namespace multiverse::storage;
    
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
}

bool CForkUnspentDB::RemoveAll()
{
    if (!CKVDB::RemoveAll())
    {
        return false;
    }
    return true;
}

bool CForkUnspentDB::UpdateUnspent(const vector<CTxUnspent>& vAddNew,const vector<CTxOutPoint>& vRemove)
{
    if (!TxnBegin())
    {
        return false;
    }

    BOOST_FOREACH(const CTxUnspent& unspent,vAddNew)
    {
        Write(static_cast<const CTxOutPoint&>(unspent),unspent.output);
    }

    BOOST_FOREACH(const CTxOutPoint& txout,vRemove)
    {
        Erase(txout);
    }

    return TxnCommit();
}

bool CForkUnspentDB::WriteUnspent(const CTxOutPoint& txout,const CTxOutput& output)
{
    return Write(txout,output);
}

bool CForkUnspentDB::ReadUnspent(const CTxOutPoint& txout,CTxOutput& output)
{
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
        if (!WalkThrough(boost::bind(&CForkUnspentDB::CopyWalker,this,_1,_2,boost::ref(dbUnspent))))
        {
            return false;
        }
        
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
        if (!WalkThrough(boost::bind(&CForkUnspentDB::LoadWalker,this,_1,_2,boost::ref(walker))))
        {
            return false;
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
                                CForkUnspentDBWalker& walker)
{
    CTxOutPoint txout;
    CTxOutput output;
    ssKey >> txout;
    ssValue >> output;
    
    return walker.Walk(txout,output);
}

//////////////////////////////
// CUnspentDB

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

    return true;
}

void CUnspentDB::Deinitialize()
{
    mapUnspenDB.clear();
}

bool CUnspentDB::AddNew(const uint256& hashFork)
{
    map<uint256,std::shared_ptr<CForkUnspentDB> >::iterator it = mapUnspenDB.find(hashFork);
    if (it != mapUnspenDB.end())
    {
        return true;
    }

    std::shared_ptr<CForkUnspentDB> spUnspent(new CForkUnspentDB(pathUnspent / hashFork.GetHex()));
    if (spUnspent == NULL || !spUnspent->IsValid())
    {
        return false;
    }
    mapUnspenDB.insert(make_pair(hashFork,spUnspent));
    return true;
}

bool CUnspentDB::Remove(const uint256& hashFork)
{
    map<uint256,std::shared_ptr<CForkUnspentDB> >::iterator it = mapUnspenDB.find(hashFork);
    if (it != mapUnspenDB.end())
    {
        (*it).second->RemoveAll();
        mapUnspenDB.erase(it);
        return true;
    }
    return false;
}

bool CUnspentDB::Update(const uint256& hashFork,
                        const vector<CTxUnspent>& vAddNew,const vector<CTxOutPoint>& vRemove)
{
    map<uint256,std::shared_ptr<CForkUnspentDB> >::iterator it = mapUnspenDB.find(hashFork);
    if (it != mapUnspenDB.end())
    {
        return (*it).second->UpdateUnspent(vAddNew,vRemove);
    }
    return false;
}

bool CUnspentDB::Retrieve(const uint256& hashFork,const CTxOutPoint& txout,CTxOutput& output)
{
    map<uint256,std::shared_ptr<CForkUnspentDB> >::iterator it = mapUnspenDB.find(hashFork);
    if (it != mapUnspenDB.end())
    {
        return (*it).second->ReadUnspent(txout,output);
    }
    return false;
}

bool CUnspentDB::Copy(const uint256& srcFork,const uint256& destFork)
{
    map<uint256,std::shared_ptr<CForkUnspentDB> >::iterator itSrc = mapUnspenDB.find(srcFork);
    if (itSrc == mapUnspenDB.end())
    {
        return false;
    }

    map<uint256,std::shared_ptr<CForkUnspentDB> >::iterator itDest = mapUnspenDB.find(destFork);
    if (itDest == mapUnspenDB.end())
    {
        return false;
    }

    return ((*itSrc).second->Copy(*(*itDest).second));
}

bool CUnspentDB::WalkThrough(const uint256& hashFork,CForkUnspentDBWalker& walker)
{
    map<uint256,std::shared_ptr<CForkUnspentDB> >::iterator it = mapUnspenDB.find(hashFork);
    if (it != mapUnspenDB.end())
    {
        return (*it).second->WalkThroughUnspent(walker);
    }
    return false;
}
