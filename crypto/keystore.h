// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_KEYSTORE_H
#define  MULTIVERSE_KEYSTORE_H

#include "key.h"
#include <map>
#include <boost/thread/thread.hpp>

namespace multiverse
{
namespace crypto
{

class CKeyStore
{
public:
    CKeyStore();
    virtual ~CKeyStore();
    void Clear();
    bool IsEmpty() const { return mapKey.empty(); }
    bool AddKey(const CKey& key);
    void RemoveKey(const CPubKey& pubkey);
    bool HaveKey(const CPubKey& pubkey) const;
    bool GetSecret(const CPubKey& pubkey,CCryptoKeyData& vchSecret) const;
    void GetKeys(std::set<CPubKey>& setPubKey) const;
    bool EncryptKey(const CPubKey& pubkey,const CCryptoString& strPassphrase,
                                          const CCryptoString& strCurrentPassphrase); 
    bool IsLocked(const CPubKey& pubkey) const;
    bool LockKey(const CPubKey& pubkey);
    bool UnlockKey(const CPubKey& pubkey,const CCryptoString& strPassphrase = "");
    bool Sign(const CPubKey& pubkey,const uint256& hash,std::vector<uint8>& vchSig) const;
protected:
    mutable boost::shared_mutex rwAccess;
    std::map<CPubKey,CKey> mapKey;
};

} // namespace crypto
} // namespace multiverse

#endif //MULTIVERSE_KEYSTORE_H
