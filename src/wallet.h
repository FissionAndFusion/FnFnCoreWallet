// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_WALLET_H
#define  MULTIVERSE_WALLET_H

#include "mvbase.h"
#include "walletdb.h"
#include <boost/thread/thread.hpp>

namespace multiverse
{

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
protected:
    bool WalleveHandleInitialize();
    void WalleveHandleDeinitialize();
    bool WalleveHandleInvoke();
    void WalleveHandleHalt();
    bool LoadDB();
    void Clear();
    bool InsertKey(const crypto::CKey& key);
protected:
    storage::CWalletDB dbWallet;
    ICoreProtocol* pCoreProtocol;
    IWorldLine* pWorldLine;
    mutable boost::shared_mutex rwKeyStore;
    std::map<crypto::CPubKey,CWalletKeyStore> mapKeyStore;
};

} // namespace multiverse

#endif //MULTIVERSE_WALLET_H

