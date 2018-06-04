// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_CRYPTO_H
#define  MULTIVERSE_CRYPTO_H

#include "uint256.h"

#include <stdexcept>
#include <memory>
#include <string>
#include <vector>

namespace multiverse
{
namespace crypto
{

// Runtime except
class CCryptoError : public std::runtime_error
{
public:
    explicit CCryptoError(const std::string& str) : std::runtime_error(str) {}
};

// Secure memory
void* CryptoAlloc(const std::size_t size);
void CryptoFree(void* ptr);

template <typename T>
T* CryptoAlloc()
{
    return static_cast<T*>(CryptoAlloc(sizeof(T)));
}

template <typename T>
class CCryptoAllocator : public std::allocator<T>
{
public:
    CCryptoAllocator() throw() {}
    CCryptoAllocator(const CCryptoAllocator& a) throw() : std::allocator<T>(a) {}
    template <typename U>
    CCryptoAllocator(const CCryptoAllocator<U>& a) throw() : std::allocator<T>(a) {}
    ~CCryptoAllocator() throw() {}
    template<typename _Other> 
    struct rebind { typedef CCryptoAllocator<_Other> other; };

    T* allocate(std::size_t n, const void *hint = 0)
    {
        return static_cast<T*>(CryptoAlloc(sizeof(T) * n));
    }
    void deallocate(T* p, std::size_t n)
    {
        CryptoFree(p);
    }
};

typedef std::basic_string<char, std::char_traits<char>, CCryptoAllocator<char> > CCryptoString;
typedef std::vector<unsigned char, CCryptoAllocator<unsigned char> > CCryptoKeyData;

// Heap memory lock
void CryptoMLock(void* const addr, const std::size_t len);
void CryptoMUnlock(void* const addr, const std::size_t len);

// Rand
uint32 CryptoGetRand32();
uint64 CryptoGetRand64();
void CryptoGetRand256(uint256& u);

// Hash
uint256 CryptoHash(const void* msg,std::size_t len);

// Sign & verify
struct CCryptoKey
{
    uint256 secret;
    uint256 pubkey;
};

uint256 CryptoMakeNewKey(CCryptoKey& key);
uint256 CryptoImportKey(CCryptoKey& key,const uint256& secret);
void CryptoSign(CCryptoKey& key,const void* md,std::size_t len,std::vector<uint8>& vchSig);
bool CryptoVerify(const uint256& pubkey,const void* md,std::size_t len,const std::vector<uint8>& vchSig);

// Encrypt
struct CCryptoCipher
{
    uint8  encrypted[48]; 
    uint64 nonce;
};

void CryptoKeyFromPassphrase(int version,const CCryptoString& passphrase,const uint256& salt,CCryptoKeyData& key);
bool CryptoEncryptSecret(int version,const CCryptoString& passphrase,const CCryptoKey& key,CCryptoCipher& cipher);
bool CryptoDecryptSecret(int version,const CCryptoString& passphrase,const CCryptoCipher& cipher,CCryptoKey& key);

} // namespace crypto
} // namespace multiverse

#endif //MULTIVERSE_CRYPTO_H
