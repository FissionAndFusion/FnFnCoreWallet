// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_PROTO_H
#define  MULTIVERSE_PROTO_H

#include "uint256.h"
#include "crc24q.h"
#include "walleve/walleve.h"

namespace multiverse 
{

namespace network 
{

enum
{
    NODE_NETWORK           = (1 << 0)
};

enum
{
    MVPROTO_CHN_NETWORK     = 0,
    MVPROTO_CHN_DELEGATE    = 1,
    MVPROTO_CHN_DATA        = 2,
    MVPROTO_CHN_USER        = 3
};

enum
{
    MVPROTO_CMD_HELLO       = 1,
    MVPROTO_CMD_HELLO_ACK   = 2,
    MVPROTO_CMD_GETADDRESS  = 3,
    MVPROTO_CMD_ADDRESS     = 4,
    MVPROTO_CMD_PING        = 5,
    MVPROTO_CMD_PONG        = 6,
};

enum
{
    MVPROTO_CMD_SUBSCRIBE   = 1,
    MVPROTO_CMD_UNSUBSCRIBE = 2,
    MVPROTO_CMD_GETBLOCKS   = 3,
    MVPROTO_CMD_GETDATA     = 4,
    MVPROTO_CMD_INV         = 5,
    MVPROTO_CMD_TX          = 6,
    MVPROTO_CMD_BLOCK       = 7
};

enum
{
    MVPROTO_CMD_DISTRIBUTE  = 1,
    MVPROTO_CMD_PUBLISH     = 2
};

#define MESSAGE_HEADER_SIZE		16
#define MESSAGE_PAYLOAD_MAX_SIZE        0x400000

class CMvPeerMessageHeader
{
    friend class walleve::CWalleveStream;
public:
    uint32 nMagic;
    uint8  nType;
    uint32 nPayloadSize;
    uint32 nPayloadChecksum;
    uint32 nHeaderChecksum;
public:
    int GetChannel() const { return (nType >> 6); }
    int GetCommand() const { return (nType & 0x3F); }
    uint32 GetHeaderChecksum() const
    {
        unsigned char buf[MESSAGE_HEADER_SIZE];
        *(uint32*)&buf[0]  = nMagic;
        *(uint8* )&buf[4]  = nType;
        *(uint32*)&buf[5]  = nPayloadSize;
        *(uint32*)&buf[9]  = nPayloadChecksum;
        return multiverse::crypto::crc24q(buf,13);
    }
    bool Verify() const
    {
        return (nPayloadSize <= MESSAGE_PAYLOAD_MAX_SIZE && nHeaderChecksum == GetHeaderChecksum());
    }
    static uint8 GetMessageType(int nChannel,int nCommand)
    {
        return ((nChannel << 6) | (nCommand & 0x3F));
    }
protected:
    void WalleveSerialize(walleve::CWalleveStream& s,walleve::SaveType&)
    {
        char buf[MESSAGE_HEADER_SIZE + 1];
        *(uint32*)&buf[0]  = nMagic;
        *(uint8* )&buf[4]  = nType;
        *(uint32*)&buf[5]  = nPayloadSize;
        *(uint32*)&buf[9]  = nPayloadChecksum;
        *(uint32*)&buf[13] = nHeaderChecksum;
        s.Write(buf,MESSAGE_HEADER_SIZE);
    }
    void WalleveSerialize(walleve::CWalleveStream& s,walleve::LoadType&)
    {
        char buf[MESSAGE_HEADER_SIZE + 1];
        s.Read(buf,MESSAGE_HEADER_SIZE);
        nMagic           = *(uint32*)&buf[0];
        nType            = *(uint8* )&buf[4];
        nPayloadSize     = *(uint32*)&buf[5];
        nPayloadChecksum = *(uint32*)&buf[9];
        nHeaderChecksum  = *(uint32*)&buf[13] & 0xFFFFFF;
    }
    void WalleveSerialize(walleve::CWalleveStream& s,std::size_t& serSize)
    {
        (void)s;
        serSize += MESSAGE_HEADER_SIZE;
    }
};

class CInv
{
    friend class walleve::CWalleveStream;
public:
    enum {MSG_ERROR=0,MSG_TX,MSG_BLOCK,MSG_FILTERED_BLOCK,MSG_CMPCT_BLOCK};
    enum {MAX_INV_COUNT = 1024 * 8};
    CInv() {}
    CInv(uint32 nTypeIn,const uint256& nHashIn)
        : nType(nTypeIn),nHash(nHashIn) {}
    friend bool operator<(const CInv& a, const CInv& b)
    {
        return (a.nType < b.nType || (a.nType == b.nType && a.nHash < b.nHash));
    }
    friend bool operator==(const CInv& a, const CInv& b)
    {
        return (a.nType == b.nType && a.nHash == b.nHash);
    }
protected:
    template <typename O>
    void WalleveSerialize(walleve::CWalleveStream& s,O& opt)
    {
        s.Serialize(nType,opt);
        s.Serialize(nHash,opt);
    }
public:
    uint32 nType;
    uint256 nHash;
};

class CEndpoint : public walleve::CBinary
{
public:
    enum {BINSIZE=18};
    CEndpoint();
    CEndpoint(const boost::asio::ip::tcp::endpoint& ep);
    CEndpoint(const CEndpoint& other);
    void GetEndpoint(boost::asio::ip::tcp::endpoint& ep);
    void CopyTo(unsigned char *ssTo) const;
    bool IsRoutable();
    const CEndpoint& operator=(const CEndpoint& other)
    {
        other.CopyTo(ss);
        return (*this);
    } 
protected: 
    unsigned char ss[BINSIZE];
};

class CAddress
{
    friend class walleve::CWalleveStream;
public:
    CAddress() {}
    CAddress(uint64 nServiceIn,const boost::asio::ip::tcp::endpoint& ep)
        : nService(nServiceIn),ssEndpoint(ep) {}
    CAddress(uint64 nServiceIn,const CEndpoint& ssEndpointIn)
        : nService(nServiceIn),ssEndpoint(ssEndpointIn) {}
protected:
    template <typename O>
    void WalleveSerialize(walleve::CWalleveStream& s,O& opt)
    {
        s.Serialize(nService,opt);
        s.Serialize(ssEndpoint,opt);
    }
public:
    uint64 nService;
    CEndpoint ssEndpoint;
};

} // namespace network
} // namespace multiverse

#endif //MULTIVERSE_PROTO_H

