// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "crypto.h"

#include <iostream>
#include <sodium.h>

namespace multiverse
{
namespace crypto
{

//////////////////////////////
// CCryptoSodiumInitializer()
class CCryptoSodiumInitializer
{
public:
    CCryptoSodiumInitializer()
    {
        if (sodium_init() < 0)
        {
            throw CCryptoError("CCryptoSodiumInitializer : Failed to initialize libsodium");
        }
    }
    ~CCryptoSodiumInitializer()
    {
    }
};

static CCryptoSodiumInitializer _CCryptoSodiumInitializer;

//////////////////////////////
// Secure memory
void* CryptoAlloc(const std::size_t size)
{
    return sodium_malloc(size);
}

void CryptoFree(void* ptr)
{
    sodium_free(ptr);
}

//////////////////////////////
// Heap memory lock

void CryptoMLock(void* const addr, const std::size_t len)
{
    if (sodium_mlock(addr,len) < 0)
    {
        throw CCryptoError("CryptoMLock : Failed to mlock");
    }
}

void CryptoMUnlock(void* const addr, const std::size_t len)
{
    if (sodium_munlock(addr,len) < 0)
    {
        throw CCryptoError("CryptoMUnlock : Failed to munlock");
    }
}

//////////////////////////////
// Rand

uint32 CryptoGetRand32()
{
    return randombytes_random();
}

uint64 CryptoGetRand64()
{
    return (randombytes_random() | (((uint64)randombytes_random()) << 32));
}

void CryptoGetRand256(uint256& u)
{
    randombytes_buf(&u,32);
}

//////////////////////////////
// Hash
 
uint256 CryptoHash(const void* msg,std::size_t len)
{
    uint256 hash;
    crypto_generichash_blake2b((uint8*)&hash,sizeof(hash),(const uint8*)msg,len,NULL,0);
    return hash;
}

uint256 CryptoHash(const uint256& h1,const uint256& h2)
{
    uint256 hash;
    crypto_generichash_blake2b_state state;
    crypto_generichash_blake2b_init(&state,NULL,0,sizeof(hash));
    crypto_generichash_blake2b_update(&state,h1.begin(),sizeof(h1));
    crypto_generichash_blake2b_update(&state,h2.begin(),sizeof(h2));
    crypto_generichash_blake2b_final(&state,hash.begin(),sizeof(hash));
    return hash;
}

//////////////////////////////
// Sign & verify

uint256 CryptoMakeNewKey(CCryptoKey& key)
{
    uint256 pk;
    crypto_sign_ed25519_keypair((uint8*)&pk,(uint8*)&key);
    return pk;
}

uint256 CryptoImportKey(CCryptoKey& key,const uint256& secret)
{
    uint256 pk;
    crypto_sign_ed25519_seed_keypair((uint8*)&pk,(uint8*)&key,(uint8*)&secret);
    return pk;
}

void CryptoSign(CCryptoKey& key,const void* md,std::size_t len,std::vector<uint8>& vchSig)
{
    vchSig.resize(64);
    crypto_sign_ed25519_detached(&vchSig[0],NULL,(const uint8*)md,len,(uint8*)&key);
}

bool CryptoVerify(const uint256& pubkey,const void* md,std::size_t len,const std::vector<uint8>& vchSig)
{
    return (vchSig.size() == 64 
            && !crypto_sign_ed25519_verify_detached(&vchSig[0],(const uint8*)md,len,(uint8*)&pubkey));
}

// Encrypt
void CryptoKeyFromPassphrase(int version,const CCryptoString& passphrase,const uint256& salt,CCryptoKeyData& key)
{
    key.resize(32);
    if (version == 0)
    {
        key.assign(salt.begin(),salt.end());
    }
    else if (version == 1)
    {
        if (crypto_pwhash_scryptsalsa208sha256(&key[0],32,passphrase.c_str(),passphrase.size(),(const uint8*)&salt,
                                               crypto_pwhash_scryptsalsa208sha256_OPSLIMIT_INTERACTIVE,
                                               crypto_pwhash_scryptsalsa208sha256_MEMLIMIT_INTERACTIVE) != 0)
        {
            throw CCryptoError("CryptoKeyFromPassphrase : Failed to create key, key version = 1");
        }
    }
    else 
    {
        throw CCryptoError("CryptoKeyFromPassphrase : Failed to create key, unknown key version");
    }
}

bool CryptoEncryptSecret(int version,const CCryptoString& passphrase,const CCryptoKey& key,CCryptoCipher& cipher)
{
    CCryptoKeyData ek;
    CryptoKeyFromPassphrase(version,passphrase,key.pubkey,ek);
    randombytes_buf(&cipher.nonce, sizeof(cipher.nonce));
    
    return (crypto_aead_chacha20poly1305_encrypt(cipher.encrypted,NULL,
                                                 (const uint8*)&key.secret,32,
                                                 (const uint8*)&key.pubkey,32,
                                                 NULL,(uint8*)&cipher.nonce,&ek[0]) == 0);
    
}

bool CryptoDecryptSecret(int version,const CCryptoString& passphrase,const CCryptoCipher& cipher,CCryptoKey& key)
{
    CCryptoKeyData ek;
    CryptoKeyFromPassphrase(version,passphrase,key.pubkey,ek);
    return (crypto_aead_chacha20poly1305_decrypt((uint8*)&key.secret,NULL,NULL,
                                                 cipher.encrypted,48,
                                                 (const uint8*)&key.pubkey,32,
                                                 (const uint8*)&cipher.nonce,&ek[0]) == 0);
}

} // namespace crypto
} // namespace multiverse



