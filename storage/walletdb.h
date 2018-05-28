// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_WALLETDB_H
#define  MULTIVERSE_WALLETDB_H

#include "dbconn.h"
#include "key.h"

namespace multiverse
{
namespace storage
{

class CWalletDBKeyWalker
{
public:
    virtual bool Walk(const uint256& pubkey,const crypto::CCryptoCipher& cipher) = 0;
};

class CWalletDB
{
public:
    CWalletDB();
    ~CWalletDB();
    bool Initialize(const CMvDBConfig& config);
    void Deinitialize();
    bool AddNewKey(const uint256& pubkey,const crypto::CCryptoCipher& cipher);
    bool WalkThroughKey(CWalletDBKeyWalker& walker); 
protected:
    bool CreateTable();
protected:
    CMvDBConn dbConn;
};

} // namespace storage
} // namespace multiverse

#endif //MULTIVERSE_WALLETDB_H

