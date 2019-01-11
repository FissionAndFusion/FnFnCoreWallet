// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "leveldbeng.h"
#include "leveldb/filter_policy.h"
#include "leveldb/cache.h"
#include <boost/filesystem/path.hpp>
using namespace walleve;
using namespace multiverse::storage;

CLevelDBArguments::CLevelDBArguments()
{
    cache = 32 << 20;
    syncwrite = false;
    files = 256;
}

CLevelDBArguments::~CLevelDBArguments()
{
}

CLevelDBEngine::CLevelDBEngine(CLevelDBArguments& arguments)
{
    path = arguments.path;
    options.block_cache = leveldb::NewLRUCache(arguments.cache / 2);    
    options.write_buffer_size = arguments.cache / 4;
    options.filter_policy = leveldb::NewBloomFilterPolicy(10);
    options.create_if_missing = true;
    options.compression = leveldb::kNoCompression;
    options.max_open_files = arguments.files;
        
    pdb = NULL;
    piter = NULL;
    pbatch = NULL;

    readoptions.verify_checksums = true;

    writeoptions.sync = arguments.syncwrite;
    batchoptions.sync = true;
}

CLevelDBEngine::~CLevelDBEngine()
{
    delete pbatch;
    pbatch = NULL;
    delete piter;
    piter = NULL;
    delete pdb;
    pdb = NULL;
    delete options.filter_policy;
    options.filter_policy = NULL;
    delete options.block_cache;
    options.block_cache = NULL;
}

bool CLevelDBEngine::Open()
{
    leveldb::Status status = leveldb::DB::Open(options, path, &pdb);
    if (!status.ok())
    {
        return false;
    }

    return true;
}

void CLevelDBEngine::Close()
{
    delete pbatch;
    pbatch = NULL;
    delete piter;
    piter = NULL;
    delete pdb;
    pdb = NULL;
}

bool CLevelDBEngine::TxnBegin()
{
    if (pbatch != NULL)
    {
        return false;
    }
    return ((pbatch = new leveldb::WriteBatch()) != NULL); 
}

bool CLevelDBEngine::TxnCommit()
{
    if (pbatch != NULL)
    {
        leveldb::WriteOptions batchoption;
        
        leveldb::Status status = pdb->Write(batchoptions,pbatch);
        delete pbatch;
        pbatch = NULL;
        return status.ok();
    }
    return false;
}

void CLevelDBEngine::TxnAbort()
{
    delete pbatch;
    pbatch = NULL;
}

bool CLevelDBEngine::Get(CWalleveBufStream& ssKey,CWalleveBufStream& ssValue)
{
    leveldb::Slice slKey(ssKey.GetData(), ssKey.GetSize());
    std::string strValue;       
    leveldb::Status status = pdb->Get(readoptions, slKey, &strValue);
    if (status.ok())
    {
        ssValue.Write(strValue.data(), strValue.size());
        return true;
    }
    return false;
}

bool CLevelDBEngine::Put(CWalleveBufStream& ssKey,CWalleveBufStream& ssValue, bool fOverwrite)
{
    leveldb::Slice slKey(ssKey.GetData(), ssKey.GetSize());
    if (!fOverwrite)
    { 
        std::string strValue;
        leveldb::Status status = pdb->Get(readoptions, slKey, &strValue);
        if (status.ok() || !status.IsNotFound())
        {
            return false;
        }
    }

    leveldb::Slice slValue(ssValue.GetData(), ssValue.GetSize());
    if (pbatch != NULL)
    {
        pbatch->Put(slKey, slValue);
        return true;
    }

    leveldb::Status status = pdb->Put(writeoptions, slKey, slValue);
    
    return status.ok();
}

bool CLevelDBEngine::Remove(CWalleveBufStream& ssKey)
{
    leveldb::Slice slKey(ssKey.GetData(), ssKey.GetSize());
    
    if (pbatch != NULL)
    {
        pbatch->Delete(slKey);
        return true;
    }

    leveldb::Status status = pdb->Delete(writeoptions, slKey);

    return status.ok();
}

bool CLevelDBEngine::RemoveAll()
{
    Close();

    leveldb::Status status = leveldb::DestroyDB(path,options);
    if (!status.ok())
    {
        return false;
    }

    return Open();
}

bool CLevelDBEngine::MoveFirst()
{
    delete piter;

    if ((piter = pdb->NewIterator(readoptions)) == NULL)
    {
        return false;
    }
    
    piter->SeekToFirst();

    return true;
}

bool CLevelDBEngine::MoveNext(CWalleveBufStream& ssKey,CWalleveBufStream& ssValue)
{
    if (piter == NULL || !piter->Valid())
        return false;

    leveldb::Slice slKey = piter->key();
    leveldb::Slice slValue = piter->value();

    ssKey.Write(slKey.data(), slKey.size());
    ssValue.Write(slValue.data(), slValue.size());

    piter->Next();

    return true;
}

