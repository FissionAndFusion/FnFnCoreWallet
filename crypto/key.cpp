// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "key.h"

using namespace multiverse::crypto;

//////////////////////////////
// CPubKey
CPubKey::CPubKey()
{
}

CPubKey::CPubKey(const uint256& pubkey)
: uint256(pubkey)
{
}

bool CPubKey::Verify(const uint256& hash,const std::vector<uint8>& vchSig)
{
    return CryptoVerify(*this,&hash,sizeof(hash),vchSig);
}

//////////////////////////////
// CKey

CKey::CKey()
{
    nVersion = 0;
    pCryptoKey = CryptoAlloc<CCryptoKey>();
    if (!pCryptoKey)
    {
        throw CCryptoError("CKey : Failed to alloc memory");
    }
    pCryptoKey->secret = 0;
    pCryptoKey->pubkey = 0;
}

CKey::CKey(const CKey& key)
{
    pCryptoKey = CryptoAlloc<CCryptoKey>();
    if (!pCryptoKey)
    {
        throw CCryptoError("CKey : Failed to alloc memory");
    }
    nVersion = key.nVersion;
    *pCryptoKey = *key.pCryptoKey;
    cipher = key.cipher;
}

CKey& CKey::operator=(const CKey& key)
{
    nVersion = key.nVersion;
    *pCryptoKey = *key.pCryptoKey;
    cipher = key.cipher;
}

CKey::~CKey()
{
    CryptoFree(pCryptoKey);
}

int CKey::GetVersion() const
{
    return nVersion;
}

bool CKey::IsNull() const
{
    return (pCryptoKey->pubkey == 0);
}

bool CKey::IsLocked() const
{
    return (pCryptoKey->secret == 0);
}

bool CKey::Renew()
{
    return (CryptoMakeNewKey(*pCryptoKey) != 0 && UpdateCipher());
}

bool CKey::Load(int nVersionIn,const CCryptoCipher& cipherIn,const CPubKey& pubkeyIn)
{
    nVersion = nVersionIn;
    cipher = cipherIn;
    pCryptoKey->secret = 0;
    pCryptoKey->pubkey = pubkeyIn;
}

bool CKey::SetSecret(const CCryptoKeyData& vchSecret)
{
    if (vchSecret.size() != 32)
    {
        return false;
    }
    return (CryptoImportKey(*pCryptoKey,*((uint256*)&vchSecret[0])) != 0 
            && UpdateCipher());
}

bool CKey::GetSecret(CCryptoKeyData& vchSecret) const
{
    if (!IsNull() && !IsLocked())
    {
        vchSecret.assign(pCryptoKey->secret.begin(),pCryptoKey->secret.end());
        return true;
    }
    return false;
}

CPubKey CKey::GetPubKey() const
{
    return CPubKey(pCryptoKey->pubkey);
}

bool CKey::Sign(const uint256& hash,std::vector<uint8>& vchSig) const
{
    if (!IsNull() && !IsLocked())
    {
        CryptoSign(*pCryptoKey,&hash,sizeof(hash),vchSig);
        return true;
    }
    return false;
}

const CCryptoCipher& CKey::GetCipher() const
{
    return cipher;
}

bool CKey::Encrypt(const CCryptoString& strPassphrase,
                   const CCryptoString& strCurrentPassphrase)
{
    if (!IsLocked())
    {
        Lock();
    }
    if (Unlock(strCurrentPassphrase))
    {
        return UpdateCipher(1,strPassphrase);
    } 
    return false;
}

void CKey::Lock()
{
    pCryptoKey->secret = 0;
}

bool CKey::Unlock(const CCryptoString& strPassphrase)
{
    try
    {
        return CryptoDecryptSecret(nVersion,strPassphrase,cipher,*pCryptoKey);
    }
    catch (...) {}
    return false;
}

bool CKey::UpdateCipher(int nVersionIn,const CCryptoString& strPassphrase)
{
    try
    {
        CCryptoCipher cipherNew;
        if (CryptoEncryptSecret(nVersionIn,strPassphrase,*pCryptoKey,cipherNew))
        {
            nVersion = nVersionIn;
            cipher = cipherNew;
            return true;
        }
    }
    catch (...) {}
    return false;
}

