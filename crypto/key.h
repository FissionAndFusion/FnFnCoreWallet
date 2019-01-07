// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_KEY_H
#define  MULTIVERSE_KEY_H

#include "crypto.h"

namespace multiverse
{
namespace crypto
{

class CPubKey : public uint256
{
public:
    CPubKey();
    CPubKey(const uint256& pubkey);
    bool Verify(const uint256& hash,const std::vector<uint8>& vchSig);
};

class CKey
{
public:
    CKey();
    CKey(const CKey& key);
    CKey& operator=(const CKey& key);
    ~CKey();
    uint32 GetVersion() const;
    bool IsNull() const;
    bool IsLocked() const;
    bool Renew();
    void Load(const CPubKey& pubkeyIn,const uint32 nVersionIn,const CCryptoCipher& cipherIn);
    bool Load(const std::vector<unsigned char>& vchKey);
    void Save(CPubKey& pubkeyRet,uint32& nVersionRet,CCryptoCipher& cipherRet) const;
    void Save(std::vector<unsigned char>& vchKey) const;
    bool SetSecret(const CCryptoKeyData& vchSecret);
    bool GetSecret(CCryptoKeyData& vchSecret) const;
    const CPubKey GetPubKey() const;
    const CCryptoCipher& GetCipher() const;
    bool Sign(const uint256& hash,std::vector<uint8>& vchSig) const;
    bool Encrypt(const CCryptoString& strPassphrase,
                 const CCryptoString& strCurrentPassphrase = "");
    void Lock();
    bool Unlock(const CCryptoString& strPassphrase = "");
protected:
    bool UpdateCipher(uint32 nVersionIn = 0,const CCryptoString& strPassphrase = "");

protected:
    uint32 nVersion;
    CCryptoKey* pCryptoKey;
    CCryptoCipher cipher;
};

} // namespace crypto
} // namespace multiverse

#endif //MULTIVERSE_KEY_H
