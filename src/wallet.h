// Copyright (c) 2017-2018 The Multiverse developers
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
        }
    }
    void Pop(const CWalletTxOut& out)
    {
        if (!out.IsNull() && setCoins.erase(out))
        {
            nTotalValue -= out.GetAmount();
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
    void Push(CWalletTx* pWalletTx,int n)
    {
        if (!pWalletTx->IsNull())
        {
            mapWalletCoins[pWalletTx->hashFork].Push(CWalletTxOut(pWalletTx,n));
        }
    }
    void Pop(CWalletTx* pWalletTx,int n)
    {
        if (!pWalletTx->IsNull())
        {
            mapWalletCoins[pWalletTx->hashFork].Pop(CWalletTxOut(pWalletTx,n));
        }
    }
    CWalletCoins& GetCoins(const uint256& hashFork)
    {
        return mapWalletCoins[hashFork];
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

class CWallet : public IWallet
{
public:
    CWallet();
    ~CWallet();
    bool IsMine(const CDestination& dest);
    /* Key store */
    bool AddKey(const crypto::CKey& key);
    bool LoadKey(const crypto::CKey& key);
    void GetPubKeys(std::set<crypto::CPubKey>& setPubKey) const;
    bool Have(const crypto::CPubKey& pubkey) const;
    bool Export(const crypto::CPubKey& pubkey,std::vector<unsigned char>& vchKey) const;
    bool Import(const std::vector<unsigned char>& vchKey,crypto::CPubKey& pubkey);

    bool Encrypt(const crypto::CPubKey& pubkey,const crypto::CCryptoString& strPassphrase,
                                               const crypto::CCryptoString& strCurrentPassphrase);
    bool GetKeyStatus(const crypto::CPubKey& pubkey,int& nVersion,bool& fLocked,int64& nAutoLockTime) const;
    bool IsLocked(const crypto::CPubKey& pubkey) const;
    bool Lock(const crypto::CPubKey& pubkey);
    bool Unlock(const crypto::CPubKey& pubkey,const crypto::CCryptoString& strPassphrase,int64 nTimeout);
    void AutoLock(uint32 nTimerId,const crypto::CPubKey& pubkey);
    bool Sign(const crypto::CPubKey& pubkey,const uint256& hash,std::vector<uint8>& vchSig) const;
    /* Template */
    bool LoadTemplate(CTemplatePtr ptr);
    void GetTemplateIds(std::set<CTemplateId>& setTemplateId) const;
    bool Have(const CTemplateId& tid) const;
    bool AddTemplate(CTemplatePtr& ptr);
    bool GetTemplate(const CTemplateId& tid,CTemplatePtr& ptr);
    /* Wallet Tx */
    std::size_t GetTxCount();
    bool ListTx(int nOffset,int nCount,std::vector<CWalletTx>& vWalletTx);
    bool GetBalance(const CDestination& dest,const uint256& hashFork,int nForkHeight,CWalletBalance& balance);
    bool SignTransaction(const CDestination& destIn,CTransaction& tx,bool& fCompleted) const;
    bool ArrangeInputs(const CDestination& destIn,const uint256& hashFork,int nForkHeight,CTransaction& tx);
    /* Update */
    bool SynchronizeTxSet(CTxSetChange& change);
    bool UpdateTx(const uint256& hashFork,const CAssembledTx& tx);
    bool ClearTx();
    bool LoadTx(const CWalletTx& wtx);
protected:
    bool WalleveHandleInitialize();
    void WalleveHandleDeinitialize();
    bool WalleveHandleInvoke();
    void WalleveHandleHalt();
    bool LoadDB();
    void Clear();
    bool InsertKey(const crypto::CKey& key);
    int64 SelectCoins(const CDestination& dest,const uint256& hashFork,int nForkHeight,
                      int64 nTargetValue,std::size_t nMaxInput,std::vector<CTxOutPoint>& vCoins);

    CWalletTx* LoadWalletTx(const uint256& txid);
    CWalletTx* InsertWalletTx(const uint256& txid,const CAssembledTx &tx,const uint256& hashFork,bool fIsMine,bool fFromMe);
    bool SignPubKey(const crypto::CPubKey& pubkey,const uint256& hash,std::vector<uint8>& vchSig) const;
    bool SignDestination(const CDestination& destIn,const uint256& hash,std::vector<uint8>& vchSig,bool& fCompleted) const;
protected:
    storage::CWalletDB dbWallet;
    ICoreProtocol* pCoreProtocol;
    IWorldLine* pWorldLine;
    mutable boost::shared_mutex rwKeyStore;
    mutable boost::shared_mutex rwWalletTx;
    std::map<crypto::CPubKey,CWalletKeyStore> mapKeyStore;
    std::map<CTemplateId,CTemplatePtr> mapTemplatePtr;
    std::map<uint256,CWalletTx> mapWalletTx;
    std::map<CDestination,CWalletUnspent> mapWalletUnspent;
};

} // namespace multiverse

#endif //MULTIVERSE_WALLET_H

