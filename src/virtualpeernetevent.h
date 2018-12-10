// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_FORKPEEREVENT_H
#define MULTIVERSE_FORKPEEREVENT_H

#include <boost/variant.hpp>

#include "walleve/walleve.h"
#include "mvproto.h"
#include "block.h"
#include "mvpeerevent.h"

using namespace multiverse::network;

namespace multiverse
{


enum class ecForkEventType : int
{
    //FORK NODE PEER NET EVENT
    FK_EVENT_NODE_PEER_ACTIVE,
    FK_EVENT_NODE_SUBSCRIBE,
    FK_EVENT_NODE_INV,

    FK_EVENT_NODE_MAX
};

template <int type,typename L,typename D>
class CFkEventPeerData : public walleve::CWalleveEvent
{
    friend class walleve::CWalleveStream;
public:
    CFkEventPeerData(uint64 nNonceIn,const uint256& hashForkIn)
            : CWalleveEvent(nNonceIn,type), hashFork(hashForkIn) {}
    virtual ~CFkEventPeerData() {}
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

class CFkNodeEventListener;

#define TYPE_FORK_NODE_PEER_ACTIVE_EVENT(type, body)       \
        walleve::CWalleveEventCategory<static_cast<int>(type), CFkNodeEventListener, body, bool>

#define TYPE_FORK_NODE_PEER_DATA_EVENT(type, body)       \
        CFkEventPeerData<static_cast<int>(type), CFkNodeEventListener, body>

typedef TYPE_FORK_NODE_PEER_ACTIVE_EVENT(ecForkEventType::FK_EVENT_NODE_PEER_ACTIVE, network::CAddress) CFkEventNodePeerActive;
typedef TYPE_FORK_NODE_PEER_DATA_EVENT(ecForkEventType::FK_EVENT_NODE_SUBSCRIBE, std::vector<uint256>) CFkEventNodeSubscribe;
typedef TYPE_FORK_NODE_PEER_DATA_EVENT(ecForkEventType::FK_EVENT_NODE_INV, std::vector<network::CInv>) CFkEventNodeInv;

class CFkNodeEventListener : virtual public walleve::CWalleveEventListener
{
public:
    virtual ~CFkNodeEventListener() {}
    DECLARE_EVENTHANDLER(CMvEventPeerActive);
    DECLARE_EVENTHANDLER(CMvEventPeerDeactive);
    DECLARE_EVENTHANDLER(CFkEventNodeInv);
};

}
#endif //MULTIVERSE_FORKPEEREVENT_H
