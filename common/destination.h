// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_DESTINATION_H
#define MULTIVERSE_DESTINATION_H

#include <vector>

#include "walleve/stream/datastream.h"
#include "walleve/stream/stream.h"

#include "key.h"
#include "template/templateid.h"
#include "uint256.h"

class CTransaction;
class CBlock;

class CDestination
{
    friend class walleve::CWalleveStream;

public:
    uint8 prefix;
    uint256 data;

    enum
    {
        PREFIX_NULL = 0x00,
        PREFIX_PUBKEY = 0x01,
        PREFIX_TEMPLATE = 0x02,
        PREFIX_MAX
    };
    enum
    {
        DESTINATION_SIZE = 33
    };

public:
    CDestination();
    CDestination(const multiverse::crypto::CPubKey& pubkey);
    CDestination(const CTemplateId& tid);

    // null
    void SetNull();
    bool IsNull() const;

    // public key
    bool IsPubKey() const;
    bool SetPubKey(const std::string& str);
    CDestination& SetPubKey(const multiverse::crypto::CPubKey& pubkey);
    bool GetPubKey(multiverse::crypto::CPubKey& pubkey) const;
    const multiverse::crypto::CPubKey GetPubKey() const;

    // template id
    bool IsTemplate() const;
    bool SetTemplateId(const std::string& str);
    CDestination& SetTemplateId(const CTemplateId& tid);
    bool GetTemplateId(CTemplateId& tid) const;
    const CTemplateId GetTemplateId() const;

    bool VerifyTxSignature(const uint256& hash, const uint256& hashAnchor, const CDestination& destTo,
                           const std::vector<uint8>& vchSig, bool& fCompleted) const;
    bool VerifyTxSignature(const uint256& hash, const uint256& hashAnchor, const CDestination& destTo,
                           const std::vector<uint8>& vchSig) const;
    bool VerifyBlockSignature(const uint256& hash, const std::vector<uint8>& vchSig) const;

    // format
    virtual bool ParseString(const std::string& str);
    virtual std::string ToString() const;

    void ToDataStream(walleve::CWalleveODataStream& os) const;
    void FromDataStream(walleve::CWalleveIDataStream& is);

protected:
    template <typename O>
    void WalleveSerialize(walleve::CWalleveStream& s, O& opt)
    {
        s.Serialize(prefix, opt);
        s.Serialize(data, opt);
    }
};

inline bool operator==(const CDestination& a, const CDestination& b)
{
    return (a.prefix == b.prefix && a.data == b.data);
}

inline bool operator!=(const CDestination& a, const CDestination& b)
{
    return !(a == b);
}
inline bool operator<(const CDestination& a, const CDestination& b)
{
    return (a.prefix < b.prefix || (a.prefix == b.prefix && a.data < b.data));
}

#endif // MULTIVERSE_DESTINATION_H
