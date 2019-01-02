// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_WALLETDB_H
#define  MULTIVERSE_WALLETDB_H

#include "dbconn.h"
#include "key.h"
#include "wallettx.h"

namespace multiverse
{
namespace storage
{

class CWalletDBKeyWalker
{
public:
    virtual bool Walk(const uint256& pubkey,int version,const crypto::CCryptoCipher& cipher) = 0;
};

class CWalletDBTemplateWalker
{
public:
    virtual bool Walk(const uint256& tid,uint16 nType,const std::vector<unsigned char>& vchData) = 0;
};

class CWalletDBTxWalker
{
public:
    virtual bool Walk(const CWalletTx& wtx) = 0;
};

class CWalletDB
{
public:
    CWalletDB();
    ~CWalletDB();
    bool Initialize(const CMvDBConfig& config);
    void Deinitialize();
    bool AddNewKey(const uint256& pubkey,int version,const crypto::CCryptoCipher& cipher);
    bool UpdateKey(const uint256& pubkey,int version,const crypto::CCryptoCipher& cipher);
    bool WalkThroughKey(CWalletDBKeyWalker& walker); 

    bool AddNewTemplate(const uint256& tid,uint16 nType,const std::vector<unsigned char>& vchData);
    bool WalkThroughTemplate(CWalletDBTemplateWalker& walker);

    bool AddNewTx(const CWalletTx& wtx);
    bool UpdateTx(const std::vector<CWalletTx>& vWalletTx,const std::vector<uint256>& vRemove=std::vector<uint256>());
    bool RetrieveTx(const uint256& txid,CWalletTx& wtx);
    bool ExistsTx(const uint256& txid);
    std::size_t GetTxCount();
    bool ListTx(int nOffset,int nCount,std::vector<CWalletTx>& vWalletTx);
    bool ListForkTx(const uint256& hashFork,int nMinHeight,std::vector<uint256>& vForkTx);
    bool WalkThroughTx(CWalletDBTxWalker& walker);
    bool ClearTx();
protected:
    bool ParseTxIn(const std::vector<unsigned char>& vchTxIn,CWalletTx& wtx);
    bool CreateTable();
protected:
    CMvDBConn dbConn;
};

} // namespace storage
} // namespace multiverse

#endif //MULTIVERSE_WALLETDB_H

