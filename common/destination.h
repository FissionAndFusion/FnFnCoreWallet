// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_DESTINATION_H
#define  MULTIVERSE_DESTINATION_H

#include "uint256.h"
#include "key.h"
#include <walleve/stream/stream.h>
#include <walleve/util.h>

class CTemplateId : public uint256
{
public:
    CTemplateId() {}
    CTemplateId(const uint256& data) : uint256(data) {}
    CTemplateId(const uint16 type,const uint256& hash) : uint256((hash << 16) | (const uint64)(type)) {}
   
    uint16 GetType() const { return (Get32() & 0xFFFF); } 
    CTemplateId& operator=(uint64 b) { *((uint256*)this)=b; return *this; }
};

class CDestination
{
    friend class walleve::CWalleveStream;
public:
    uint8 prefix;
    uint256 data;
    enum 
    {
        PREFIX_NULL     = 0x00,
        PREFIX_PUBKEY   = 0x01,
        PREFIX_TEMPLATE = 0x02,
        PREFIX_MAX
    };
    enum {DESTINATION_SIZE = 33};
    CDestination()
    {
        SetNull();
    }
    CDestination(const multiverse::crypto::CPubKey& pubkey)
    {
        SetPubKey(pubkey);
    }
    CDestination(const CTemplateId& tid)
    {
        SetTemplateId(tid);
    }
    void SetNull()
    {
        prefix = PREFIX_NULL;
        data = 0;
    }
    bool IsNull() const
    {
        return (prefix == PREFIX_NULL);
    }
    bool SetHex(const std::string& strHex)
    {
        if (strHex.size() != DESTINATION_SIZE*2 || strHex[0] != '0' || strHex[1] < '0' || strHex[1] > '2')
        {
            return false;
        }
        prefix = strHex[1] - '0';
        if (walleve::ParseHexString(strHex.c_str() + 2,data.begin(),sizeof(uint256)) != sizeof(uint256))
        {
            return false;
        }
        return true;
    }
    void SetPubKey(const multiverse::crypto::CPubKey& pubkey)
    {
        prefix = PREFIX_PUBKEY;
        data = pubkey;
    }
    bool IsPubKey() const
    {
        return (prefix == PREFIX_PUBKEY);
    }
    bool GetPubKey(multiverse::crypto::CPubKey& pubkey) const
    {
        if (prefix == PREFIX_PUBKEY)
        {
            pubkey = multiverse::crypto::CPubKey(data);
            return true;
        }
        return false;
    }
    void SetTemplateId(const CTemplateId& tid)
    {
        prefix = PREFIX_TEMPLATE;
        data = tid;
    }
    bool IsTemplate() const
    {
        return (prefix == PREFIX_TEMPLATE);
    }
    bool GetTemplateId(CTemplateId& tid) const
    {
        if (prefix == PREFIX_TEMPLATE)
        {
            tid = CTemplateId(data);
            return true;
        }
        return false;
    } 
    bool VerifySignature(const uint256& hash,const std::vector<unsigned char>& vchSig) const;
    friend bool operator==(const CDestination& a, const CDestination& b)
    {
        return (a.prefix == b.prefix && a.data == b.data );
    }
    friend bool operator!=(const CDestination& a, const CDestination& b)
    {
        return !(a == b);
    }
    friend bool operator<(const CDestination& a, const CDestination& b)
    {
        return (a.prefix < b.prefix || (a.prefix == b.prefix && a.data < b.data));
    }
    std::string GetHex() const
    {
        return (std::string(1,(char)(prefix + '0')) + walleve::ToHexString(data.begin(),sizeof(uint256)));
    }
protected:
    template <typename O>
    void WalleveSerialize(walleve::CWalleveStream& s,O& opt)
    {
        s.Serialize(prefix,opt);
        s.Serialize(data,opt);
    }
};

#endif //MULTIVERSE_DESTINATION_H

