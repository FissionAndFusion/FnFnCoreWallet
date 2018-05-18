// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_DESTINATION_H
#define  MULTIVERSE_DESTINATION_H

#include "uint256.h"
#include "key.h"
#include <walleve/stream/stream.h>

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
    };
    CDestination()
    {
        SetNull();
    }
    CDestination(const multiverse::crypto::CPubKey& pubkey)
    {
        SetPubKey(pubkey);
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
    void SetTemplateHash(const uint256& hash)
    {
        prefix = PREFIX_TEMPLATE;
        data = hash;
    }
    bool IsTemplate() const
    {
        return (prefix == PREFIX_TEMPLATE);
    }
    
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
protected:
    template <typename O>
    void WalleveSerialize(walleve::CWalleveStream& s,O& opt)
    {
        s.Serialize(prefix,opt);
        s.Serialize(data,opt);
    }
};

#endif //MULTIVERSE_DESTINATION_H

