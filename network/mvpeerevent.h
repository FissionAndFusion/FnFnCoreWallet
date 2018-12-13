// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_PEER_EVENT_H
#define  MULTIVERSE_PEER_EVENT_H

#include "mvproto.h"
#include "block.h"
#include "transaction.h"
#include "walleve/walleve.h"

using namespace walleve;

namespace multiverse
{
namespace network
{

enum
{
    MV_EVENT_PEER_BASE = walleve::WALLEVE_EVENT_USER_BASE,
    //PEER
    MV_EVENT_PEER_ACTIVE,
    MV_EVENT_PEER_DEACTIVE,
    MV_EVENT_PEER_SUBSCRIBE,
    MV_EVENT_PEER_UNSUBSCRIBE,
    MV_EVENT_PEER_INV,
    MV_EVENT_PEER_GETDATA,
    MV_EVENT_PEER_GETBLOCKS,
    MV_EVENT_PEER_TX,
    MV_EVENT_PEER_BLOCK,
    MV_EVENT_PEER_BULLETIN,
    MV_EVENT_PEER_GETDELEGATED,
    MV_EVENT_PEER_DISTRIBUTE,
    MV_EVENT_PEER_PUBLISH,
    MV_EVENT_PEER_MAX,
};


template <int type,typename L,typename D>
class CMvEventPeerData : public walleve::CWalleveEvent
{
    friend class walleve::CWalleveStream;
public:
    CMvEventPeerData(uint64 nNonceIn,const uint256& hashForkIn) 
    : CWalleveEvent(nNonceIn,type), hashFork(hashForkIn) {}
    virtual ~CMvEventPeerData() {}
    virtual bool Handle(walleve::CWalleveEventListener& listener)
    {
        try
        {
            return (dynamic_cast<L&>(listener)).HandleEvent(*this);
        }
        catch (std::bad_cast&)
        {
            return listener.HandleEvent(*this);
        }
        catch (std::exception& e)
        {
            StdError(__PRETTY_FUNCTION__, e.what());
        }
        return false;
    }
protected:
    template <typename O>
    void WalleveSerialize(walleve::CWalleveStream& s,O& opt)
    {
        s.Serialize(hashFork,opt);
        s.Serialize(data,opt);
    }
public:
    uint256 hashFork;
    D data;
};

template <int type,typename L,typename D>
class CMvEventPeerDelegated : public walleve::CWalleveEvent
{
    friend class walleve::CWalleveStream;
public:
    CMvEventPeerDelegated(uint64 nNonceIn,const uint256& hashAnchorIn) 
    : CWalleveEvent(nNonceIn,type), hashAnchor(hashAnchorIn) {}
    virtual ~CMvEventPeerDelegated() {}
    virtual bool Handle(walleve::CWalleveEventListener& listener)
    {
        try
        {
            return (dynamic_cast<L&>(listener)).HandleEvent(*this);
        }
        catch (std::bad_cast&)
        {
            return listener.HandleEvent(*this);
        }
        catch (std::exception& e)
        {
            StdError(__PRETTY_FUNCTION__, e.what());
        }
        return false;
    }
protected:
    template <typename O>
    void WalleveSerialize(walleve::CWalleveStream& s,O& opt)
    {
        s.Serialize(hashAnchor,opt);
        s.Serialize(data,opt);
    }
public:
    uint256 hashAnchor;
    D data;
};

class CMvEventPeerDelegatedBulletin
{
    friend class walleve::CWalleveStream;
public:
    class CDelegatedBitmap
    {
    public:
        CDelegatedBitmap(const uint256& hashAnchorIn = uint64(0), uint64 bitmapIn = 0)
        : hashAnchor(hashAnchorIn),bitmap(bitmapIn)
        {
        }
        template <typename O>
        void WalleveSerialize(walleve::CWalleveStream& s,O& opt)
        {   
            s.Serialize(hashAnchor,opt);
            s.Serialize(bitmap,opt);
        }
    public:
        uint256 hashAnchor;
        uint64 bitmap;
    };
public:
    void AddBitmap(const uint256& hash,uint64 bitmap)
    {
        vBitmap.push_back(CDelegatedBitmap(hash,bitmap));
    }
protected:
    template <typename O>
    void WalleveSerialize(walleve::CWalleveStream& s,O& opt)
    {
        s.Serialize(bmDistribute,opt);
        s.Serialize(bmPublish,opt);
        s.Serialize(vBitmap,opt);
    }
public:
    uint64 bmDistribute;
    uint64 bmPublish;
    std::vector<CDelegatedBitmap> vBitmap;
};

class CMvEventPeerDelegatedGetData
{
    friend class walleve::CWalleveStream;
protected:
    template <typename O>
    void WalleveSerialize(walleve::CWalleveStream& s,O& opt)
    {
        s.Serialize(nInvType,opt);
        s.Serialize(destDelegate,opt);
    }
public:
    uint32 nInvType;
    CDestination destDelegate;
};

class CMvEventPeerDelegatedData
{
    friend class walleve::CWalleveStream;
protected:
    template <typename O>
    void WalleveSerialize(walleve::CWalleveStream& s,O& opt)
    {
        s.Serialize(destDelegate,opt);
        s.Serialize(vchData,opt);
    }
public:
    CDestination destDelegate;
    std::vector<unsigned char> vchData;
};

class CMvPeerEventListener;

#define TYPE_PEEREVENT(type,body)       \
        walleve::CWalleveEventCategory<type,CMvPeerEventListener,body,bool>

#define TYPE_PEERDATAEVENT(type,body)       \
        CMvEventPeerData<type,CMvPeerEventListener,body>

#define TYPE_PEERDELEGATEDEVENT(type,body)       \
        CMvEventPeerDelegated<type,CMvPeerEventListener,body>

typedef TYPE_PEEREVENT(MV_EVENT_PEER_ACTIVE,CAddress) CMvEventPeerActive;
typedef TYPE_PEEREVENT(MV_EVENT_PEER_DEACTIVE,CAddress) CMvEventPeerDeactive;
typedef TYPE_PEERDATAEVENT(MV_EVENT_PEER_SUBSCRIBE,std::vector<uint256>) CMvEventPeerSubscribe;
typedef TYPE_PEERDATAEVENT(MV_EVENT_PEER_UNSUBSCRIBE,std::vector<uint256>) CMvEventPeerUnsubscribe;
typedef TYPE_PEERDATAEVENT(MV_EVENT_PEER_INV,std::vector<CInv>) CMvEventPeerInv;
typedef TYPE_PEERDATAEVENT(MV_EVENT_PEER_GETDATA,std::vector<CInv>) CMvEventPeerGetData;
typedef TYPE_PEERDATAEVENT(MV_EVENT_PEER_GETBLOCKS,CBlockLocator) CMvEventPeerGetBlocks;
typedef TYPE_PEERDATAEVENT(MV_EVENT_PEER_TX,CTransaction) CMvEventPeerTx;
typedef TYPE_PEERDATAEVENT(MV_EVENT_PEER_BLOCK,CBlock) CMvEventPeerBlock;

typedef TYPE_PEERDELEGATEDEVENT(MV_EVENT_PEER_BULLETIN,CMvEventPeerDelegatedBulletin) CMvEventPeerBulletin;
typedef TYPE_PEERDELEGATEDEVENT(MV_EVENT_PEER_GETDELEGATED,CMvEventPeerDelegatedGetData) CMvEventPeerGetDelegated;
typedef TYPE_PEERDELEGATEDEVENT(MV_EVENT_PEER_DISTRIBUTE,CMvEventPeerDelegatedData) CMvEventPeerDistribute;
typedef TYPE_PEERDELEGATEDEVENT(MV_EVENT_PEER_PUBLISH,CMvEventPeerDelegatedData) CMvEventPeerPublish;

class CMvPeerEventListener : virtual public walleve::CWalleveEventListener
{
public:
    virtual ~CMvPeerEventListener() {}
    DECLARE_EVENTHANDLER(CMvEventPeerActive);
    DECLARE_EVENTHANDLER(CMvEventPeerDeactive);
    DECLARE_EVENTHANDLER(CMvEventPeerSubscribe);
    DECLARE_EVENTHANDLER(CMvEventPeerUnsubscribe);
    DECLARE_EVENTHANDLER(CMvEventPeerInv);
    DECLARE_EVENTHANDLER(CMvEventPeerGetData);
    DECLARE_EVENTHANDLER(CMvEventPeerGetBlocks);
    DECLARE_EVENTHANDLER(CMvEventPeerTx);
    DECLARE_EVENTHANDLER(CMvEventPeerBlock);
    DECLARE_EVENTHANDLER(CMvEventPeerBulletin);
    DECLARE_EVENTHANDLER(CMvEventPeerGetDelegated);
    DECLARE_EVENTHANDLER(CMvEventPeerDistribute);
    DECLARE_EVENTHANDLER(CMvEventPeerPublish);
};

} // namespace network
} // namespace multiverse

#endif //MULTIVERSE_PEER_EVENT_H
