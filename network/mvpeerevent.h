// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_PEER_EVENT_H
#define  MULTIVERSE_PEER_EVENT_H

#include "mvproto.h"
#include "block.h"
#include "transaction.h"
#include "walleve/walleve.h"

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
    MV_EVENT_PEER_INV,
    MV_EVENT_PEER_GETDATA,
    MV_EVENT_PEER_GETBLOCKS,
    MV_EVENT_PEER_TX,
    MV_EVENT_PEER_BLOCK,
    MV_EVENT_PEER_MAX
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
        catch (...) {}
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

class CMvPeerEventListener;

#define TYPE_PEEREVENT(type,body)       \
        walleve::CWalleveEventCategory<type,CMvPeerEventListener,body,bool>

#define TYPE_PEERDATAEVENT(type,body)       \
        CMvEventPeerData<type,CMvPeerEventListener,body>

typedef TYPE_PEEREVENT(MV_EVENT_PEER_ACTIVE,CAddress) CMvEventPeerActive;
typedef TYPE_PEEREVENT(MV_EVENT_PEER_DEACTIVE,CAddress) CMvEventPeerDeactive;
typedef TYPE_PEERDATAEVENT(MV_EVENT_PEER_INV,std::vector<CInv>) CMvEventPeerInv;
typedef TYPE_PEERDATAEVENT(MV_EVENT_PEER_GETDATA,std::vector<CInv>) CMvEventPeerGetData;
typedef TYPE_PEERDATAEVENT(MV_EVENT_PEER_GETBLOCKS,CBlockLocator) CMvEventPeerGetBlocks;
typedef TYPE_PEERDATAEVENT(MV_EVENT_PEER_TX,CTransaction) CMvEventPeerTx;
typedef TYPE_PEERDATAEVENT(MV_EVENT_PEER_BLOCK,CBlock) CMvEventPeerBlock;
/*
typedef TYPE_PEEREVENT(MV_EVENT_PEER_INV,std::vector<CInv>) CMvEventPeerInv;
typedef TYPE_PEEREVENT(MV_EVENT_PEER_GETDATA,std::vector<CInv>) CMvEventPeerGetData;
typedef TYPE_PEEREVENT(MV_EVENT_PEER_GETBLOCKS,CBlockLocator) CMvEventPeerGetBlocks;
typedef TYPE_PEEREVENT(MV_EVENT_PEER_TX,CTransaction) CMvEventPeerTx;
typedef TYPE_PEEREVENT(MV_EVENT_PEER_BLOCK,CBlock) CMvEventPeerBlock;
*/
class CMvPeerEventListener : virtual public walleve::CWalleveEventListener
{
public:
    virtual ~CMvPeerEventListener() {}
    DECLARE_EVENTHANDLER(CMvEventPeerActive);
    DECLARE_EVENTHANDLER(CMvEventPeerDeactive);
    DECLARE_EVENTHANDLER(CMvEventPeerInv);
    DECLARE_EVENTHANDLER(CMvEventPeerGetData);
    DECLARE_EVENTHANDLER(CMvEventPeerGetBlocks);
    DECLARE_EVENTHANDLER(CMvEventPeerTx);
    DECLARE_EVENTHANDLER(CMvEventPeerBlock);
};

} // namespace network
} // namespace multiverse

#endif //MULTIVERSE_PEER_EVENT_H
