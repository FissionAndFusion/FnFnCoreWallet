// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "wallet.h"

using namespace std;
using namespace walleve;
using namespace multiverse;

//////////////////////////////
// CWalletKeyWalker
 
class CDBKeyWalker : public storage::CWalletDBKeyWalker
{
public:
    CDBKeyWalker(CWallet* pWalletIn) : pWallet(pWalletIn) {}
    bool Walk(const uint256& pubkey,int version,const crypto::CCryptoCipher& cipher)
    {
        crypto::CKey key;
        key.Load(pubkey,version,cipher);
        return pWallet->LoadKey(key);
    }
protected:
    CWallet* pWallet;
};

//////////////////////////////
// CWallet 

CWallet::CWallet()
{
    pCoreProtocol = NULL;
    pWorldLine = NULL;
}

CWallet::~CWallet()
{
}

bool CWallet::WalleveHandleInitialize()
{
    if (!WalleveGetObject("coreprotocol",pCoreProtocol))
    {
        WalleveLog("Failed to request coreprotocol\n");
        return false;
    }

    if (!WalleveGetObject("worldline",pWorldLine))
    {
        WalleveLog("Failed to request worldline\n");
        return false;
    }

    return true;
}

void CWallet::WalleveHandleDeinitialize()
{
    pCoreProtocol = NULL;
    pWorldLine = NULL;
}

bool CWallet::WalleveHandleInvoke()
{
    storage::CMvDBConfig dbConfig(StorageConfig()->strDBHost,StorageConfig()->nDBPort,
                                  StorageConfig()->strDBName,StorageConfig()->strDBUser,StorageConfig()->strDBPass);
    if (!dbWallet.Initialize(dbConfig))
    {
        WalleveLog("Failed to initialize wallet database\n");
        return false;
    }

    if (!LoadDB())
    {
        WalleveLog("Failed to load wallet database\n");
        return false;
    }
    
    if (mapKeyStore.empty())
    {
        crypto::CKey key;
        if (!key.Renew() || !AddKey(key))
        {
            WalleveLog("Failed to add initial key\n");
            return false;
        }
    }
    return true;
}

void CWallet::WalleveHandleHalt()
{
    dbWallet.Deinitialize();
    Clear();    
}

bool CWallet::AddKey(const crypto::CKey& key)
{
    boost::unique_lock<boost::shared_mutex> wlock(rwKeyStore);
    if (!InsertKey(key))
    {
        WalleveLog("AddKey : invalid or duplicated key\n");
        return false;
    }

    if (!dbWallet.AddNewKey(key.GetPubKey(),key.GetVersion(),key.GetCipher()))
    {
        mapKeyStore.erase(key.GetPubKey());
        WalleveLog("AddKey : failed to save key\n");
        return false;
    }
    return true;
}

bool CWallet::LoadKey(const crypto::CKey& key)
{
    if (!InsertKey(key))
    {
        WalleveLog("LoadKey : invalid or duplicated key\n");
        return false;
    }
    return true;
}

void CWallet::GetPubKeys(set<crypto::CPubKey>& setPubKey) const
{
    boost::shared_lock<boost::shared_mutex> rlock(rwKeyStore);
    
    for (map<crypto::CPubKey,CWalletKeyStore>::const_iterator it = mapKeyStore.begin();
         it != mapKeyStore.end();++it)
    {
        setPubKey.insert((*it).first);
    }
}

bool CWallet::Have(const crypto::CPubKey& pubkey) const
{
    boost::shared_lock<boost::shared_mutex> rlock(rwKeyStore);
    return (!!mapKeyStore.count(pubkey));
}

bool CWallet::Export(const crypto::CPubKey& pubkey,vector<unsigned char>& vchKey) const
{
    boost::shared_lock<boost::shared_mutex> rlock(rwKeyStore);
    map<crypto::CPubKey,CWalletKeyStore>::const_iterator it = mapKeyStore.find(pubkey);
    if (it != mapKeyStore.end())
    {
        (*it).second.key.Save(vchKey);
        return true;
    }
    return false;
}

bool CWallet::Import(const vector<unsigned char>& vchKey,crypto::CPubKey& pubkey)
{
    crypto::CKey key;
    if (!key.Load(vchKey))
    {
        return false;
    }
    pubkey = key.GetPubKey();
    return AddKey(key);
}

bool CWallet::Encrypt(const crypto::CPubKey& pubkey,const crypto::CCryptoString& strPassphrase,
                                                    const crypto::CCryptoString& strCurrentPassphrase)
{
    boost::unique_lock<boost::shared_mutex> wlock(rwKeyStore);
    map<crypto::CPubKey,CWalletKeyStore>::iterator it = mapKeyStore.find(pubkey);
    if (it != mapKeyStore.end())
    {
        crypto::CKey& key = (*it).second.key;
        crypto::CKey keyTemp(key);
        if (!keyTemp.Encrypt(strPassphrase,strCurrentPassphrase))
        {
            return false;
        }
        if (!dbWallet.UpdateKey(key.GetPubKey(),keyTemp.GetVersion(),keyTemp.GetCipher()))
        {
            WalleveLog("AddKey : failed to update key\n");
            return false;
        }
        key.Encrypt(strPassphrase,strCurrentPassphrase);
        key.Lock();
        return true;
    }
    return false;
}

bool CWallet::GetKeyStatus(const crypto::CPubKey& pubkey,int& nVersion,bool& fLocked,int64& nAutoLockTime) const
{
    boost::shared_lock<boost::shared_mutex> rlock(rwKeyStore);
    map<crypto::CPubKey,CWalletKeyStore>::const_iterator it = mapKeyStore.find(pubkey);
    if (it != mapKeyStore.end())
    {
        const CWalletKeyStore& keystore = (*it).second;
        nVersion = keystore.key.GetVersion();
        fLocked = keystore.key.IsLocked();
        nAutoLockTime = (!fLocked && keystore.nAutoLockTime > 0) ? keystore.nAutoLockTime : 0;
        return true;
    }
    return false;
}

bool CWallet::IsLocked(const crypto::CPubKey& pubkey) const
{
    boost::shared_lock<boost::shared_mutex> rlock(rwKeyStore);
    map<crypto::CPubKey,CWalletKeyStore>::const_iterator it = mapKeyStore.find(pubkey);
    if (it != mapKeyStore.end())
    {
        return (*it).second.key.IsLocked();
    }
    return false;
}

bool CWallet::Lock(const crypto::CPubKey& pubkey)
{
    boost::unique_lock<boost::shared_mutex> wlock(rwKeyStore);
    map<crypto::CPubKey,CWalletKeyStore>::iterator it = mapKeyStore.find(pubkey);
    if (it != mapKeyStore.end())
    {
        CWalletKeyStore& keystore = (*it).second;
        keystore.key.Lock();
        if (keystore.nTimerId)
        {
            WalleveCancelTimer(keystore.nTimerId); 
            keystore.nTimerId = 0;
            keystore.nAutoLockTime = -1;
        }
        return true;
    }
    return false;
}

bool CWallet::Unlock(const crypto::CPubKey& pubkey,const crypto::CCryptoString& strPassphrase,int64 nTimeout)
{
    boost::unique_lock<boost::shared_mutex> wlock(rwKeyStore);
    map<crypto::CPubKey,CWalletKeyStore>::iterator it = mapKeyStore.find(pubkey);
    if (it != mapKeyStore.end())
    {
        CWalletKeyStore& keystore = (*it).second;
        if (!keystore.key.IsLocked() || !keystore.key.Unlock(strPassphrase))
        {
            return false;
        }

        if (nTimeout > 0)
        {
            keystore.nAutoLockTime = GetTime() + nTimeout;
            keystore.nTimerId = WalleveSetTimer(nTimeout * 1000,boost::bind(&CWallet::AutoLock,this,_1,(*it).first));
        }
        return true;
    }
    return false;
}

void CWallet::AutoLock(uint32 nTimerId,const crypto::CPubKey& pubkey)
{
    boost::unique_lock<boost::shared_mutex> wlock(rwKeyStore);
    map<crypto::CPubKey,CWalletKeyStore>::iterator it = mapKeyStore.find(pubkey);
    if (it != mapKeyStore.end())
    {
        CWalletKeyStore& keystore = (*it).second;
        if (keystore.nTimerId == nTimerId)
        {
            keystore.key.Lock();
            keystore.nTimerId = 0;
            keystore.nAutoLockTime = -1;
        }
    }
}

bool CWallet::Sign(const crypto::CPubKey& pubkey,const uint256& hash,vector<uint8>& vchSig) const
{
    boost::shared_lock<boost::shared_mutex> rlock(rwKeyStore);
    map<crypto::CPubKey,CWalletKeyStore>::const_iterator it = mapKeyStore.find(pubkey);
    if (it != mapKeyStore.end())
    {
        return (*it).second.key.Sign(hash,vchSig);
    }
    return false;
}

bool CWallet::LoadDB()
{
    boost::unique_lock<boost::shared_mutex> wlock(rwKeyStore);
    CDBKeyWalker walker(this);
    return dbWallet.WalkThroughKey(walker);
}

void CWallet::Clear()
{
    boost::unique_lock<boost::shared_mutex> wlock(rwKeyStore);
    mapKeyStore.clear();
}

bool CWallet::InsertKey(const crypto::CKey& key)
{
    if (!key.IsNull())
    {
        pair<map<crypto::CPubKey,CWalletKeyStore>::iterator,bool> ret;
        ret = mapKeyStore.insert(make_pair(key.GetPubKey(),CWalletKeyStore(key)));
        if (ret.second)
        {
            (*(ret.first)).second.key.Lock();
            return true;
        }
    }
    return false;
}
