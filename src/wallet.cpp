// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "wallet.h"

using namespace std;
using namespace walleve;
using namespace multiverse;

#define MAX_TXIN_SELECTIONS        128
#define MAX_SIGNATURE_SIZE         2048

//////////////////////////////
// CDBKeyWalker
 
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
// CDBTemplateWalker
 
class CDBTemplateWalker : public storage::CWalletDBTemplateWalker
{
public:
    CDBTemplateWalker(CWallet* pWalletIn) : pWallet(pWalletIn) {}
    bool Walk(const uint256& tid,uint16 nType,const std::vector<unsigned char>& vchData)
    {
        CTemplatePtr ptr = CTemplateGeneric::CreateTemplatePtr(nType,vchData);
        if (ptr != NULL && tid == ptr->GetTemplateId())
        {
            return  pWallet->LoadTemplate(ptr);
        }
        return false;
    }
protected:
    CWallet* pWallet;
};

//////////////////////////////
// CDBTxWalker
 
class CDBTxWalker : public storage::CWalletDBTxWalker
{
public:
    CDBTxWalker(CWallet* pWalletIn) : pWallet(pWalletIn) {}
    bool Walk(const CWalletTx& wtx)
    {
        return pWallet->LoadTx(wtx);
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
    
    return true;
}

void CWallet::WalleveHandleHalt()
{
    dbWallet.Deinitialize();
    Clear();    
}

bool CWallet::IsMine(const CDestination& dest)
{
    crypto::CPubKey pubkey;
    CTemplateId nTemplateId;
    if (dest.GetPubKey(pubkey))
    {
        return (!!mapKeyStore.count(pubkey));
    }
    else if (dest.GetTemplateId(nTemplateId))
    {
        return (!!mapTemplatePtr.count(nTemplateId));
    }
    return false;
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
    return SignPubKey(pubkey,hash,vchSig);
}

bool CWallet::LoadTemplate(CTemplatePtr ptr)
{
    if (ptr != NULL)
    {
        return mapTemplatePtr.insert(make_pair(ptr->GetTemplateId(),ptr)).second;
    }
    return false;
}

void CWallet::GetTemplateIds(set<CTemplateId>& setTemplateId) const
{
    boost::shared_lock<boost::shared_mutex> rlock(rwKeyStore);
    for (map<CTemplateId,CTemplatePtr>::const_iterator it = mapTemplatePtr.begin();
         it != mapTemplatePtr.end(); ++it)
    {
        setTemplateId.insert((*it).first);
    }
}

bool CWallet::Have(const CTemplateId& tid) const
{
    boost::shared_lock<boost::shared_mutex> rlock(rwKeyStore);
    return (!!mapTemplatePtr.count(tid));
}

bool CWallet::AddTemplate(CTemplatePtr& ptr)
{
    boost::unique_lock<boost::shared_mutex> wlock(rwKeyStore);
    if (ptr != NULL && !ptr->IsNull())
    {
        CTemplateId tid = ptr->GetTemplateId();
        if (mapTemplatePtr.insert(make_pair(tid,ptr)).second)
        {
            vector<unsigned char> vchData;
            ptr->GetTemplateData(vchData);
            return dbWallet.AddNewTemplate(tid,ptr->GetTemplateType(),vchData);
        }
    }
    return false;
}

bool CWallet::GetTemplate(const CTemplateId& tid,CTemplatePtr& ptr)
{
    boost::shared_lock<boost::shared_mutex> rlock(rwKeyStore);
    map<CTemplateId,CTemplatePtr>::const_iterator it = mapTemplatePtr.find(tid);
    if (it !=  mapTemplatePtr.end())
    {
        ptr = (*it).second;
        return true;
    }
    return false;
}

size_t CWallet::GetTxCount()
{
    boost::shared_lock<boost::shared_mutex> rlock(rwWalletTx);
    return dbWallet.GetTxCount();
}

bool CWallet::ListTx(int nOffset,int nCount,vector<CWalletTx>& vWalletTx)
{
    boost::shared_lock<boost::shared_mutex> rlock(rwWalletTx);
    return dbWallet.ListTx(nOffset,nCount,vWalletTx);
}

bool CWallet::GetBalance(const CDestination& dest,const uint256& hashFork,int nForkHeight,CWalletBalance& balance)
{
    boost::shared_lock<boost::shared_mutex> rlock(rwWalletTx);
    map<CDestination,CWalletUnspent>::iterator it = mapWalletUnspent.find(dest);
    if (it == mapWalletUnspent.end())
    {
        return false;
    }
    CWalletCoins& coins = (*it).second.GetCoins(hashFork);
    balance.SetNull();
    BOOST_FOREACH(const CWalletTxOut& txout,coins.setCoins)
    {
        if (txout.IsLocked(nForkHeight))
        {
            balance.nLocked += txout.GetAmount();
        }
        else
        {
            if (txout.GetDepth(nForkHeight) == 0)
            {
                balance.nUnconfirmed += txout.GetAmount();
            }
        }
    }
    balance.nAvailable = coins.nTotalValue - balance.nLocked;
    return true;
}

bool CWallet::SignTransaction(const CDestination& destIn,CTransaction& tx,bool& fCompleted) const
{
    boost::shared_lock<boost::shared_mutex> rlock(rwKeyStore);
    return SignDestination(destIn,tx.GetSignatureHash(),tx.vchSig,fCompleted);
}

bool CWallet::ArrangeInputs(const CDestination& destIn,const uint256& hashFork,int nForkHeight,CTransaction& tx)
{
    tx.vInput.clear();
    size_t nNoInputSize = GetSerializeSize(tx);
    int nMaxInput = (MAX_TX_SIZE - MAX_SIGNATURE_SIZE - 4) / 33;
    int64 nTargeValue = tx.nAmount + tx.nTxFee;
    vector<CTxOutPoint> vCoins;
    {
        boost::shared_lock<boost::shared_mutex> rlock(rwWalletTx);
        int64 nValueIn = SelectCoins(destIn,hashFork,nForkHeight,nTargeValue,nMaxInput,vCoins);
        if (nValueIn < nTargeValue)
        {
            return false;
        }
    }
    tx.vInput.reserve(vCoins.size());
    BOOST_FOREACH(const CTxOutPoint& out,vCoins)
    {
        tx.vInput.push_back(CTxIn(out));
    }
    return true;
}

bool CWallet::LoadTx(const CWalletTx& wtx)
{
    CWalletTx& wtxNew = (*mapWalletTx.insert(make_pair(wtx.txid,wtx)).first).second;

    vector<uint256> vFork;
    GetWalletTxFork(wtxNew.hashFork,wtxNew.nBlockHeight,vFork);

    AddNewWalletTx(wtxNew,vFork);

    return true; 
}

bool CWallet::AddNewFork(const uint256& hashFork,const uint256& hashParent,int nOriginHeight)
{
    boost::unique_lock<boost::shared_mutex> wlock(rwWalletTx);
    
    CProfile profile;
    if (!pWorldLine->GetForkProfile(hashFork,profile))
    {
        return false;
    }
    if (mapFork.insert(make_pair(hashFork,CWalletFork(hashParent,nOriginHeight,profile.IsIsolated()))).second)
    {
        if (hashParent != 0)
        {
            mapFork[hashParent].InsertSubline(nOriginHeight,hashFork);
        }
        if (!profile.IsIsolated())
        {
            for (map<CDestination,CWalletUnspent>::iterator it = mapWalletUnspent.begin();
                 it != mapWalletUnspent.end(); ++it)
            {
                CWalletUnspent& unspent = (*it).second;
                unspent.Dup(hashParent,hashFork);
            }
            vector<uint256> vForkTx;
            if (!dbWallet.ListForkTx(hashParent,nOriginHeight + 1,vForkTx))
            {
                return false;
            }
            for (int i = vForkTx.size() - 1;i >= 0;--i)
            {
                CWalletTx* pWalletTx = LoadWalletTx(vForkTx[i]);
                if (pWalletTx != NULL)
                {
                    RemoveWalletTx(*pWalletTx,hashFork);
                    if (!pWalletTx->GetRefCount())
                    {
                        mapWalletTx.erase(vForkTx[i]);
                    }
                }
                else
                {
                    return false;
                }
            }
        }
    }
    return true;
}

bool CWallet::LoadDB()
{
    {
        boost::unique_lock<boost::shared_mutex> wlock(rwKeyStore);
        {
            CDBKeyWalker walker(this);
            if (!dbWallet.WalkThroughKey(walker))
            {
                return false;
            }
        }
        {
            CDBTemplateWalker walker(this);
            if (!dbWallet.WalkThroughTemplate(walker))
            {
                return false;
            }
        }
    }

    {
        boost::unique_lock<boost::shared_mutex> wlock(rwWalletTx);
        if (!UpdateFork())
        {
            return false;
        }

        CDBTxWalker walker(this);
        if (!dbWallet.WalkThroughTx(walker))
        {
            return false;
        } 
    }
    return true;
}

void CWallet::Clear()
{
    {
        boost::unique_lock<boost::shared_mutex> wlock(rwKeyStore);
        mapKeyStore.clear();
        mapTemplatePtr.clear();
    }
    {
        boost::unique_lock<boost::shared_mutex> wlock(rwWalletTx);
        mapWalletUnspent.clear();
        mapWalletTx.clear();
    }
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

bool CWallet::SynchronizeTxSet(CTxSetChange& change)
{
    boost::unique_lock<boost::shared_mutex> wlock(rwWalletTx);

    vector<CWalletTx> vWalletTx;
    vector<uint256> vRemove;
    
    for (std::size_t i = 0;i < change.vTxRemove.size();i++)
    {
        uint256& txid = change.vTxRemove[i].first;
        CWalletTx* pWalletTx = LoadWalletTx(txid);
        if (pWalletTx != NULL)
        {
            RemoveWalletTx(*pWalletTx,change.hashFork);
            mapWalletTx.erase(txid); 
            vRemove.push_back(txid); 
        }
    }
    
    for (map<uint256,int>::iterator it = change.mapTxUpdate.begin();it != change.mapTxUpdate.end();++it)
    {
        const uint256& txid = (*it).first;
        CWalletTx* pWalletTx = LoadWalletTx(txid);
        if (pWalletTx != NULL)
        {
            pWalletTx->nBlockHeight = (*it).second;
            vWalletTx.push_back(*pWalletTx);
        }
    }

    map<int,vector<uint256> > mapPreFork;
    BOOST_FOREACH(CAssembledTx& tx,change.vTxAddNew)
    {
        bool fIsMine = IsMine(tx.sendTo);
        bool fFromMe = IsMine(tx.destIn);
        if (fFromMe || fIsMine)
        {
            uint256 txid = tx.GetHash();
            CWalletTx* pWalletTx = InsertWalletTx(txid,tx,change.hashFork,fIsMine,fFromMe);
            if (pWalletTx != NULL)
            {
                vector<uint256>& vFork = mapPreFork[pWalletTx->nBlockHeight];
                if (vFork.empty())
                {
                    GetWalletTxFork(change.hashFork,pWalletTx->nBlockHeight,vFork);
                }
                AddNewWalletTx(*pWalletTx,vFork);
                vWalletTx.push_back(*pWalletTx);
            }
        }
    }

    BOOST_FOREACH(const CWalletTx& wtx,vWalletTx)
    {
        map<uint256,CWalletTx>::iterator it = mapWalletTx.find(wtx.txid);
        if (it != mapWalletTx.end() && !(*it).second.GetRefCount())
        {
            mapWalletTx.erase(it);
        }
    }
    
    if (!vWalletTx.empty() || !vRemove.empty())
    {
        return dbWallet.UpdateTx(vWalletTx,vRemove);
    }

    return true;
}

bool CWallet::UpdateTx(const uint256& hashFork,const CAssembledTx& tx)
{
    vector<CWalletTx> vWalletTx;
    bool fIsMine = IsMine(tx.sendTo);
    bool fFromMe = IsMine(tx.destIn);
    if (fFromMe || fIsMine)
    {
        uint256 txid = tx.GetHash();
        CWalletTx* pWalletTx = InsertWalletTx(txid,tx,hashFork,fIsMine,fFromMe);
        if (pWalletTx != NULL)
        {
            vector<uint256> vFork;
            GetWalletTxFork(hashFork,tx.nBlockHeight,vFork);
            AddNewWalletTx(*pWalletTx,vFork);
            vWalletTx.push_back(*pWalletTx);
            if (!pWalletTx->GetRefCount())
            {
                mapWalletTx.erase(txid);
            }
        }
    }
    if (!vWalletTx.empty())
    {
        return dbWallet.UpdateTx(vWalletTx);
    }
    return true;
}

bool CWallet::ClearTx()
{
    boost::unique_lock<boost::shared_mutex> wlock(rwWalletTx);
    mapWalletUnspent.clear();
    mapWalletTx.clear();
    return dbWallet.ClearTx();
}

CWalletTx* CWallet::LoadWalletTx(const uint256& txid)
{
    map<uint256,CWalletTx>::iterator it = mapWalletTx.find(txid);
    if (it == mapWalletTx.end())
    {
        CWalletTx wtx;
        if (!dbWallet.RetrieveTx(txid,wtx))
        {
            return NULL;
        }
        it = mapWalletTx.insert(make_pair(txid,wtx)).first;
    }
    CWalletTx& wtx = (*it).second;
    return (!wtx.IsNull() ? &wtx : NULL);
}

CWalletTx* CWallet::InsertWalletTx(const uint256& txid,const CAssembledTx &tx,const uint256& hashFork,bool fIsMine,bool fFromMe)
{
    CWalletTx& wtx = mapWalletTx[txid];
    wtx = CWalletTx(txid,tx,hashFork,fIsMine,fFromMe);
    return &wtx;
}

int64 CWallet::SelectCoins(const CDestination& dest,const uint256& hashFork,int nForkHeight,
                           int64 nTargetValue,size_t nMaxInput,vector<CTxOutPoint>& vCoins)
{
    vCoins.clear();

    CWalletCoins& walletCoins = mapWalletUnspent[dest].GetCoins(hashFork);
    if (walletCoins.nTotalValue < nTargetValue)
    {
        return 0;
    }

    pair<int64,CWalletTxOut> coinLowestLarger;
    coinLowestLarger.first = std::numeric_limits<int64>::max();
    int64 nTotalLower = 0;

    multimap<int64,CWalletTxOut> mapValue;

    BOOST_FOREACH(const CWalletTxOut& out,walletCoins.setCoins)
    {
        if (out.IsLocked(nForkHeight))
        {
            continue;
        }
        int64 nValue = out.GetAmount();
        pair<int64,CWalletTxOut> coin = make_pair(nValue,out);

        if (nValue == nTargetValue)
        {
            vCoins.push_back(out.GetTxOutPoint());
            return nValue;
        }
        else if (nValue < nTargetValue)
        {
            mapValue.insert(coin);
            nTotalLower += nValue;
            while (mapValue.size() > nMaxInput)
            {
                multimap<int64,CWalletTxOut>::iterator mi = mapValue.begin();
                nTotalLower -= (*mi).first;
                mapValue.erase(mi);
            }
            if (nTotalLower >= nTargetValue)
            {
                break;
            }
        }
        else if (nValue < coinLowestLarger.first)
        {
            coinLowestLarger = coin;
        }
    } 
    
    int64 nValueRet = 0;
    if (nTotalLower >= nTargetValue)
    {
        while (nValueRet < nTargetValue)
        {
            int64 nShortage = nTargetValue - nValueRet;
            multimap<int64,CWalletTxOut>::iterator it = mapValue.lower_bound(nShortage);
            if (it == mapValue.end())
            {   
                --it;
            }
            vCoins.push_back((*it).second.GetTxOutPoint());
            nValueRet += (*it).first;
            mapValue.erase(it);
        }
    }
    else if (!coinLowestLarger.second.IsNull())
    {
        vCoins.push_back(coinLowestLarger.second.GetTxOutPoint());
        nValueRet += coinLowestLarger.first;
        multimap<int64,CWalletTxOut>::iterator it = mapValue.begin();
        for (int i = 0;i < 3 && it != mapValue.end();i++,++it)
        {
            vCoins.push_back((*it).second.GetTxOutPoint());
            nValueRet += (*it).first;
        }
    }
    return nValueRet;
}

bool CWallet::SignPubKey(const crypto::CPubKey& pubkey,const uint256& hash,vector<uint8>& vchSig) const
{
    map<crypto::CPubKey,CWalletKeyStore>::const_iterator it = mapKeyStore.find(pubkey);
    if (it != mapKeyStore.end())
    {
        return (*it).second.key.Sign(hash,vchSig);
    }
    return false;
}

bool CWallet::SignDestination(const CDestination& destIn,const uint256& hash,vector<uint8>& vchSig,bool& fCompleted) const
{
    crypto::CPubKey pubkey;
    if (destIn.GetPubKey(pubkey))
    {
        fCompleted = SignPubKey(pubkey,hash,vchSig);
        return fCompleted;
    }
    CTemplateId tid;
    if (!destIn.GetTemplateId(tid))
    {
        return false;
    }
    
    map<CTemplateId,CTemplatePtr>::const_iterator it = mapTemplatePtr.find(tid);
    if (it == mapTemplatePtr.end())
    {
        return false;
    }
    CTemplatePtr ptr = (*it).second;
    
    uint16 nType = ptr->GetTemplateType();
    if (nType == TEMPLATE_WEIGHTED || nType == TEMPLATE_MULTISIG)
    {
        bool fSigned = false;
        CTemplateWeighted* p = (CTemplateWeighted*)ptr.get();
        for (map<crypto::CPubKey,unsigned char>::iterator it = p->mapPubKeyWeight.begin();
             it != p->mapPubKeyWeight.end(); ++it)
        {
            const crypto::CPubKey& pk = (*it).first;
            vector<uint8> vchKeySig;
            if (SignPubKey(pk,hash,vchSig))
            {
                if (!p->BuildTxSignature(hash,vchKeySig,vchSig,vchSig,fCompleted))
                {
                    return false;
                }
                fSigned = true;
            }
            if (fCompleted)
            {
                break;
            }
        }

        return fSigned;
    }

    else if (nType == TEMPLATE_FORK)
    {
        CTemplateFork* p = (CTemplateFork*)ptr.get();
        vector<uint8> vchRedeemSig;
        if (!vchSig.empty())
        {
            vchRedeemSig.assign(vchSig.begin() + p->vchData.size(),vchSig.end());
        }
        if (!SignDestination(p->destRedeem,hash,vchRedeemSig,fCompleted))
        {
            return false;
        }
        return p->BuildTxSignature(hash,vchRedeemSig,vchSig);
    }
    else if (nType == TEMPLATE_MINT)
    {
        CTemplateMint* p = (CTemplateMint*)ptr.get();
        vector<uint8> vchSpendSig;
        if (!vchSig.empty())
        {
            vchSpendSig.assign(vchSig.begin() + p->vchData.size(),vchSig.end());
        }
        if (!SignDestination(p->destSpend,hash,vchSpendSig,fCompleted))
        {
            return false;
        }
        return p->BuildTxSignature(hash,vchSpendSig,vchSig);
    }
    else if (nType == TEMPLATE_DELEGATE)
    {
        CTemplateDelegate* p = (CTemplateDelegate*)ptr.get();
        vector<uint8> vchSpendSig;
        if (!vchSig.empty())
        {
            vchSpendSig.assign(vchSig.begin() + p->vchData.size() + 33,vchSig.end());
        }
        if (!SignDestination(p->destOwner,hash,vchSpendSig,fCompleted))
        {
            return false;
        }
        return p->BuildTxSignature(p->destOwner,hash,vchSpendSig,vchSig);
    }
    return false;
}

bool CWallet::UpdateFork()
{
    map<uint256,CForkStatus> mapForkStatus;
    multimap<uint256,pair<int,uint256> > mapSubline;

    pWorldLine->GetForkStatus(mapForkStatus);

    for (map<uint256,CForkStatus>::iterator it = mapForkStatus.begin();it != mapForkStatus.end();++it)
    {
        const uint256 hashFork = (*it).first;
        const CForkStatus& status = (*it).second;
        if (!mapFork.count(hashFork))
        {
            CProfile profile;
            if (!pWorldLine->GetForkProfile(hashFork,profile))
            {
                return false;
            }
            mapFork.insert(make_pair(hashFork,CWalletFork(status.hashParent,status.nOriginHeight,profile.IsIsolated())));
            if (status.hashParent != 0)
            {
                mapSubline.insert(make_pair(status.hashParent,make_pair(status.nOriginHeight,hashFork)));
            }
        }
    }

    for (multimap<uint256,pair<int,uint256> >::iterator it = mapSubline.begin();it != mapSubline.end();++it)
    {
        mapFork[(*it).first].InsertSubline((*it).second.first,(*it).second.second);
    }
    return true;
}

void CWallet::GetWalletTxFork(const uint256& hashFork,int nHeight,vector<uint256>& vFork)
{
    vector<pair<uint256,CWalletFork*> >vForkPtr;
    {
        map<uint256,CWalletFork>::iterator it = mapFork.find(hashFork);
        if (it != mapFork.end())
        {
            vForkPtr.push_back(make_pair(hashFork,&(*it).second));
        }
    }
    if (nHeight >= 0)
    {
        for (size_t i = 0;i < vForkPtr.size();i++)
        {
            CWalletFork* pFork = vForkPtr[i].second;
            for (multimap<int,uint256>::iterator mi = pFork->mapSubline.lower_bound(nHeight);mi != pFork->mapSubline.end();++mi)
            {
                map<uint256,CWalletFork>::iterator it = mapFork.find((*mi).second);
                if (it != mapFork.end() && !(*it).second.fIsolated)
                {
                    vForkPtr.push_back(make_pair((*it).first,&(*it).second));
                }
            } 
        }
    }
    vFork.reserve(vForkPtr.size());
    for (size_t i = 0;i < vForkPtr.size();i++)
    {
        vFork.push_back(vForkPtr[i].first);
    }
}

void CWallet::AddNewWalletTx(CWalletTx& wtx,vector<uint256>& vFork)
{
    if (wtx.IsFromMe())
    {
        BOOST_FOREACH(const CTxIn& txin,wtx.vInput)
        {
            map<uint256,CWalletTx>::iterator it = mapWalletTx.find(txin.prevout.hash);
            if (it != mapWalletTx.end())
            {
                CWalletTx& wtxPrev = (*it).second;
                BOOST_FOREACH(const uint256& hashFork,vFork)
                {
                    mapWalletUnspent[wtx.destIn].Pop(hashFork,&wtxPrev,txin.prevout.n);
                }
                if (!wtxPrev.GetRefCount())
                {
                     mapWalletTx.erase(it);
                }
            }
        }
        BOOST_FOREACH(const uint256& hashFork,vFork)
        {
            mapWalletUnspent[wtx.destIn].Push(hashFork,&wtx,1);
        }
    }
    if (wtx.IsMine())
    {
        BOOST_FOREACH(const uint256& hashFork,vFork)
        {
            mapWalletUnspent[wtx.sendTo].Push(hashFork,&wtx,0);
        }
    }
}

void CWallet::RemoveWalletTx(CWalletTx& wtx,const uint256& hashFork)
{
    if (wtx.IsFromMe())
    {
        BOOST_FOREACH(const CTxIn& txin,wtx.vInput)
        {
            CWalletTx* pWalletPrevTx = LoadWalletTx(txin.prevout.hash);
            if (pWalletPrevTx != NULL)
            {
                mapWalletUnspent[wtx.destIn].Push(hashFork,pWalletPrevTx,txin.prevout.n);
            }
        }
        mapWalletUnspent[wtx.destIn].Pop(hashFork,&wtx,1);
    }
    if (wtx.IsMine())
    {
        mapWalletUnspent[wtx.sendTo].Pop(hashFork,&wtx,0);
    }
}

