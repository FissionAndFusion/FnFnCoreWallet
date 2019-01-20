// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_WALLET_H
#define  MULTIVERSE_WALLET_H

#include "mvbase.h"
#include "walletdb.h"
#include "wallettx.h"
#include <boost/thread/thread.hpp>

namespace multiverse
{

class CWalletCoins
{
public:
    CWalletCoins() : nTotalValue(0) {}
    void Push(const CWalletTxOut& out)
    {
        if (!out.IsNull() && setCoins.insert(out).second)
        {
            nTotalValue += out.GetAmount();
            out.AddRef();
        }
    }
    void Pop(const CWalletTxOut& out)
    {
        if (!out.IsNull() && setCoins.erase(out))
        {
            nTotalValue -= out.GetAmount();
            out.Release();
        }
    }
public:
    int64 nTotalValue;
    std::set<CWalletTxOut> setCoins;
};

class CWalletUnspent
{
public:
    void Clear() { mapWalletCoins.clear(); }
    void Push(const uint256& hashFork,std::shared_ptr<CWalletTx>& spWalletTx,int n)
    {
        mapWalletCoins[hashFork].Push(CWalletTxOut(spWalletTx,n));
    }
    void Pop(const uint256& hashFork,std::shared_ptr<CWalletTx>& spWalletTx,int n)
    {
        mapWalletCoins[hashFork].Pop(CWalletTxOut(spWalletTx,n));
    }
    CWalletCoins& GetCoins(const uint256& hashFork)
    {
        return mapWalletCoins[hashFork];
    }
    void Dup(const uint256& hashFrom,const uint256& hashTo)
    {
        std::map<uint256,CWalletCoins>::iterator it = mapWalletCoins.find(hashFrom);
        if (it != mapWalletCoins.end())
        {
            CWalletCoins& coin = mapWalletCoins[hashTo];
            BOOST_FOREACH(const CWalletTxOut& out,(*it).second.setCoins)
            {
                coin.Push(out);
            }
        }
    }
public:
    std::map<uint256,CWalletCoins> mapWalletCoins;
};

class CWalletKeyStore
{
public:
    CWalletKeyStore() : nTimerId(0),nAutoLockTime(-1) {}
    CWalletKeyStore(const crypto::CKey& keyIn) : key(keyIn),nTimerId(0),nAutoLockTime(-1) {}
    virtual ~CWalletKeyStore() {}
public:
    crypto::CKey key;
    uint32 nTimerId;
    int64 nAutoLockTime;
};

class CWalletFork
{
public:
    CWalletFork(const uint256& hashParentIn=uint64(0),int nOriginHeightIn=-1,bool fIsolatedIn=true)
    : hashParent(hashParentIn),nOriginHeight(nOriginHeightIn),fIsolated(fIsolatedIn)
    {
    }
    void InsertSubline(int nHeight,const uint256& hashSubline)
    {
        mapSubline.insert(std::make_pair(nHeight,hashSubline));
    }
public:
    uint256 hashParent;
    int nOriginHeight;
    bool fIsolated;
    std::multimap<int,uint256> mapSubline;
};

class CWallet : public IWallet
{
public:
    CWallet();
    ~CWallet();
    bool IsMine(const CDestination& dest);
    /* Key store */
    bool AddKey(const crypto::CKey& key) override;
    bool LoadKey(const crypto::CKey& key);
    void GetPubKeys(std::set<crypto::CPubKey>& setPubKey) const override;
    bool Have(const crypto::CPubKey& pubkey) const override;
    bool Export(const crypto::CPubKey& pubkey,std::vector<unsigned char>& vchKey) const override;
    bool Import(const std::vector<unsigned char>& vchKey,crypto::CPubKey& pubkey) override;

    bool Encrypt(const crypto::CPubKey& pubkey,const crypto::CCryptoString& strPassphrase,
                                               const crypto::CCryptoString& strCurrentPassphrase) override;
    bool GetKeyStatus(const crypto::CPubKey& pubkey,int& nVersion,bool& fLocked,int64& nAutoLockTime) const override;
    bool IsLocked(const crypto::CPubKey& pubkey) const override;
    bool Lock(const crypto::CPubKey& pubkey) override;
    bool Unlock(const crypto::CPubKey& pubkey,const crypto::CCryptoString& strPassphrase,int64 nTimeout) override;
    void AutoLock(uint32 nTimerId,const crypto::CPubKey& pubkey);
    bool Sign(const crypto::CPubKey& pubkey,const uint256& hash,std::vector<uint8>& vchSig) const override;
    /* Template */
    bool LoadTemplate(CTemplatePtr ptr);
    void GetTemplateIds(std::set<CTemplateId>& setTemplateId) const override;
    bool Have(const CTemplateId& tid) const override;
    bool AddTemplate(CTemplatePtr& ptr) override;
    bool GetTemplate(const CTemplateId& tid,CTemplatePtr& ptr) override;
    /* Destination */
    void GetDestinations(std::set<CDestination>& setDest);
    /* Wallet Tx */
    std::size_t GetTxCount() override;
    bool ListTx(int nOffset,int nCount,std::vector<CWalletTx>& vWalletTx) override;
    bool GetBalance(const CDestination& dest,const uint256& hashFork,int nForkHeight,CWalletBalance& balance) override;
    bool SignTransaction(const CDestination& destIn,CTransaction& tx,bool& fCompleted) const override;
    bool ArrangeInputs(const CDestination& destIn,const uint256& hashFork,int nForkHeight,CTransaction& tx) override;
    /* Update */
    bool SynchronizeTxSet(const CTxSetChange& change) override;
    bool AddNewTx(const uint256& hashFork,const CAssembledTx& tx) override;
    bool UpdateTx(const uint256& hashFork,const CAssembledTx& tx);
    bool LoadTx(const CWalletTx& wtx);
    bool AddNewFork(const uint256& hashFork,const uint256& hashParent,int nOriginHeight) override;
    /* Resync */
    bool SynchronizeWalletTx(const CDestination& destNew) override;
    bool ResynchronizeWalletTx() override;
protected:
    bool WalleveHandleInitialize() override;
    void WalleveHandleDeinitialize() override;
    bool WalleveHandleInvoke() override;
    void WalleveHandleHalt() override;
    bool LoadDB();
    void Clear();
    bool ClearTx();
    bool InsertKey(const crypto::CKey& key);
    int64 SelectCoins(const CDestination& dest,const uint256& hashFork,int nForkHeight,
                      int64 nTargetValue,std::size_t nMaxInput,std::vector<CTxOutPoint>& vCoins);

    std::shared_ptr<CWalletTx> LoadWalletTx(const uint256& txid);
    std::shared_ptr<CWalletTx> InsertWalletTx(const uint256& txid,const CAssembledTx &tx,const uint256& hashFork,bool fIsMine,bool fFromMe);
    bool SignPubKey(const crypto::CPubKey& pubkey,const uint256& hash,std::vector<uint8>& vchSig) const;
    bool SignDestination(const CDestination& destIn,const uint256& hash,std::vector<uint8>& vchSig,bool& fCompleted) const;
    bool UpdateFork();
    void GetWalletTxFork(const uint256& hashFork,int nHeight,std::vector<uint256>& vFork);
    void AddNewWalletTx(std::shared_ptr<CWalletTx>& spWalletTx,std::vector<uint256>& vFork);
    void RemoveWalletTx(std::shared_ptr<CWalletTx>& spWalletTx,const uint256& hashFork);
    bool SyncWalletTx(CTxFilter& txFilter);
protected:
    storage::CWalletDB dbWallet;
    ICoreProtocol* pCoreProtocol;
    IWorldLine* pWorldLine;
    ITxPool* pTxPool;
    mutable boost::shared_mutex rwKeyStore;
    mutable boost::shared_mutex rwWalletTx;
    std::map<crypto::CPubKey,CWalletKeyStore> mapKeyStore;
    std::map<CTemplateId,CTemplatePtr> mapTemplatePtr;
    std::map<uint256,std::shared_ptr<CWalletTx> > mapWalletTx;
    std::map<CDestination,CWalletUnspent> mapWalletUnspent;
    std::map<uint256,CWalletFork> mapFork;
};

// dummy wallet for on wallet server
class CDummyWallet : public IWallet
{
public:
    CDummyWallet() {}
    ~CDummyWallet() {}
    /* Key store */
    virtual bool AddKey(const crypto::CKey& key) override
    {
        return false;
    }
    virtual void GetPubKeys(std::set<crypto::CPubKey>& setPubKey) const override {}
    virtual bool Have(const crypto::CPubKey& pubkey) const override
    {
        return false;
    }
    virtual bool Export(const crypto::CPubKey& pubkey,
                        std::vector<unsigned char>& vchKey) const override
    {
        return false;
    }
    virtual bool Import(const std::vector<unsigned char>& vchKey,
                        crypto::CPubKey& pubkey) override
    {
        return false;
    }
    virtual bool Encrypt(
      const crypto::CPubKey& pubkey, const crypto::CCryptoString& strPassphrase,
      const crypto::CCryptoString& strCurrentPassphrase) override
    {
        return false;
    }
    virtual bool GetKeyStatus(const crypto::CPubKey& pubkey, int& nVersion,
                              bool& fLocked,
                              int64& nAutoLockTime) const override
    {
        return false;
    }
    virtual bool IsLocked(const crypto::CPubKey& pubkey) const override
    {
        return false;
    }
    virtual bool Lock(const crypto::CPubKey& pubkey) override
    {
        return false;
    }
    virtual bool Unlock(const crypto::CPubKey& pubkey,
                        const crypto::CCryptoString& strPassphrase,
                        int64 nTimeout) override
    {
        return false;
    }
    virtual bool Sign(const crypto::CPubKey& pubkey, const uint256& hash,
                      std::vector<uint8>& vchSig) const override
    {
        return false;
    }
    /* Template */
    virtual void GetTemplateIds(std::set<CTemplateId>& setTemplateId) const override {}
    virtual bool Have(const CTemplateId& tid) const override
    {
        return false;
    }
    virtual bool AddTemplate(CTemplatePtr& ptr) override
    {
        return false;
    }
    virtual bool GetTemplate(const CTemplateId& tid, CTemplatePtr& ptr) override
    {
        return false;
    }
    /* Wallet Tx */
    virtual std::size_t GetTxCount() override
    {
        return 0;
    }
    virtual bool ListTx(int nOffset, int nCount, std::vector<CWalletTx>& vWalletTx) override
    {
        return true;
    }
    virtual bool GetBalance(const CDestination& dest, const uint256& hashFork,
                            int nForkHeight, CWalletBalance& balance) override
    {
        return false;
    }
    virtual bool SignTransaction(const CDestination& destIn, CTransaction& tx,
                                 bool& fCompleted) const override
    {
        return false;
    }
    virtual bool ArrangeInputs(const CDestination& destIn,
                               const uint256& hashFork, int nForkHeight,
                               CTransaction& tx) override
    {
        return false;
    }
    /* Update */
    virtual bool SynchronizeTxSet(const CTxSetChange& change) override
    {
        return true;
    }
    virtual bool AddNewTx(const uint256& hashFork,const CAssembledTx& tx) override
    {
        return true;
    }
    virtual bool AddNewFork(const uint256& hashFork, const uint256& hashParent,
                            int nOriginHeight) override
    {
        return true;
    }

    virtual bool SynchronizeWalletTx(const CDestination& destNew) override
    {
        return true;
    }

    virtual bool ResynchronizeWalletTx() override
    {
        return true;
    }
};

} // namespace multiverse

#endif //MULTIVERSE_WALLET_H

