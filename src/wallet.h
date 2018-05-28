// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_WALLET_H
#define  MULTIVERSE_WALLET_H

#include "mvbase.h"
#include "walletdb.h"
#include "crypto.h"
#include "keystore.h"
namespace multiverse
{

class CWallet : public IWallet
{
public:
    CWallet();
    ~CWallet();
    bool AddKey(crypto::CKey& key);
    
protected:
    bool WalleveHandleInitialize();
    void WalleveHandleDeinitialize();
    bool WalleveHandleInvoke();
    void WalleveHandleHalt();
protected:
    boost::shared_mutex rwAccess;
    crypto::CKeyStore keyStore;
    storage::CWalletDB dbWallet;
    ICoreProtocol* pCoreProtocol;
    IWorldLine* pWorldLine;
};

} // namespace multiverse

#endif //MULTIVERSE_WALLET_H

