// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_WALLETDB_H
#define  MULTIVERSE_WALLETDB_H

#include "dbconn.h"
#include "key.h"
#include "wallettx.h"

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>

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

class CWalletTxCache
{
typedef boost::multi_index_container<
  CWalletTx,
  boost::multi_index::indexed_by<
    boost::multi_index::sequenced<>,
    boost::multi_index::ordered_unique<boost::multi_index::member<CWalletTx,uint256,&CWalletTx::txid> >,
    boost::multi_index::ordered_non_unique<boost::multi_index::member<CWalletTx,uint256,&CWalletTx::hashFork> >
  >
> CWalletTxList;
typedef CWalletTxList::nth_index<1>::type CWalletTxListByTxid;
typedef CWalletTxList::nth_index<2>::type CWalletTxListByFork;
public:
    bool Exists(const uint256& txid)
    {
        return (!!listWalletTx.get<1>().count(txid));
    }
    std::size_t Count()
    {
        return listWalletTx.size();
    }
    void Clear()
    {
        listWalletTx.clear();
    }
    void AddNew(const CWalletTx& wtx)
    {
        listWalletTx.push_back(wtx);
    }
    void Remove(const uint256& txid)
    {
        listWalletTx.get<1>().erase(txid);
    }
    void Update(const CWalletTx& wtx)
    {
        CWalletTxListByTxid& idxByTxid = listWalletTx.get<1>();
        CWalletTxListByTxid::iterator it = idxByTxid.find(wtx.txid);
        if (it != idxByTxid.end())
        {
            (*it).nFlags = wtx.nFlags;
        }
        else
        {
            listWalletTx.push_back(wtx);
        }
    } 
    bool Get(const uint256& txid,CWalletTx& wtx)
    {
        CWalletTxListByTxid& idxByTxid = listWalletTx.get<1>();
        CWalletTxListByTxid::iterator it = idxByTxid.find(txid);
        if (it != idxByTxid.end())
        {
            wtx = (*it);
            wtx.nRefCount = 0;
            return true;
        }
        return false;
    }
    void ListTx(int nOffset,int nCount,std::vector<CWalletTx>& vWalletTx)
    {
        CWalletTxList::iterator it = listWalletTx.begin();
        while (it != listWalletTx.end() && nOffset--)
        {
            ++it;
        }
        for (;it != listWalletTx.end() && nCount--;++it)
        {
            vWalletTx.push_back((*it));
        }
    }
    void ListForkTx(const uint256& hashFork,std::vector<uint256>& vForkTx)
    {
        CWalletTxListByFork& idxByFork = listWalletTx.get<2>();
        for (CWalletTxListByFork::iterator it = idxByFork.lower_bound(hashFork);
             it != idxByFork.upper_bound(hashFork);++it)
        {
            vForkTx.push_back((*it).txid);
        }
    }
protected:
    CWalletTxList listWalletTx;
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
    bool ListRollBackTx(const uint256& hashFork,int nMinHeight,std::vector<uint256>& vForkTx);
    bool WalkThroughTx(CWalletDBTxWalker& walker);
    bool ClearTx();
    bool CheckWalletTx();
protected:
    bool ListDBTx(int nOffset,int nCount,std::vector<CWalletTx>& vWalletTx);
    bool ParseTxIn(const std::vector<unsigned char>& vchTxIn,CWalletTx& wtx);
    bool CreateTable();
protected:
    CMvDBConn dbConn;
    CWalletTxCache txCache;
};

} // namespace storage
} // namespace multiverse

#endif //MULTIVERSE_WALLETDB_H

