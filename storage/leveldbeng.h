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

    bool Open() override;
    void Close() override;
    bool TxnBegin() override;
    bool TxnCommit() override;
    void TxnAbort() override;
    bool Get(walleve::CWalleveBufStream& ssKey,walleve::CWalleveBufStream& ssValue) override;
    bool Put(walleve::CWalleveBufStream& ssKey,walleve::CWalleveBufStream& ssValue, bool fOverwrite) override;
    bool Remove(walleve::CWalleveBufStream& ssKey) override;
    bool RemoveAll() override;
    bool MoveFirst() override;
    bool MoveTo(walleve::CWalleveBufStream& ssKey) override;
    bool MoveNext(walleve::CWalleveBufStream& ssKey,walleve::CWalleveBufStream& ssValue) override;
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

