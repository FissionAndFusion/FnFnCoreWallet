// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_LEVELDBENG_H
#define  MULTIVERSE_LEVELDBENG_H
#include "walleve/walleve.h"
#include <leveldb/db.h>
#include <leveldb/write_batch.h>
#include <boost/filesystem/path.hpp>

namespace multiverse
{
namespace storage
{

class CLevelDBArguments
{
public:
    CLevelDBArguments();
    ~CLevelDBArguments();
public:
    std::string path;
    size_t cache;
    bool syncwrite;
    int files;    
};

class CLevelDBEngine : public walleve::CKVDBEngine
{
public:
    CLevelDBEngine(CLevelDBArguments& arguments);
    ~CLevelDBEngine();

    bool Open();
    void Close();
    bool TxnBegin();
    bool TxnCommit();
    void TxnAbort();
    bool Get(walleve::CWalleveBufStream& ssKey,walleve::CWalleveBufStream& ssValue);
    bool Put(walleve::CWalleveBufStream& ssKey,walleve::CWalleveBufStream& ssValue, bool fOverwrite);
    bool Remove(walleve::CWalleveBufStream& ssKey);
    bool RemoveAll();
    bool MoveFirst();
    bool MoveTo(walleve::CWalleveBufStream& ssKey);
    bool MoveNext(walleve::CWalleveBufStream& ssKey,walleve::CWalleveBufStream& ssValue);
protected:
    std::string path;
    leveldb::DB* pdb;
    leveldb::Iterator* piter;
    leveldb::WriteBatch *pbatch;
    leveldb::Options options;
    leveldb::ReadOptions readoptions;
    leveldb::WriteOptions writeoptions;
    leveldb::WriteOptions batchoptions;
};

} // namespace storage
} // namespace multiverse

#endif //MULTIVERSE_LEVELDBENG_H

