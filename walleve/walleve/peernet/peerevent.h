// Copyright (c) 2016-2018 The LoMoCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  WALLEVE_PEER_EVENT_H
#define  WALLEVE_PEER_EVENT_H

#include "walleve/event/event.h"
#include "walleve/netio/nethost.h"
#include "walleve/peernet/peerinfo.h"
#include "walleve/peernet/epmngr.h"
#include <boost/ptr_container/ptr_vector.hpp>

namespace walleve
{

enum
{
    WALLEVE_EVENT_PEERNET_IDLE = WALLEVE_EVENT_PEER_BASE,
    WALLEVE_EVENT_PEERNET_GETIP,
    WALLEVE_EVENT_PEERNET_GETCOUNT,
    WALLEVE_EVENT_PEERNET_GETPEERS,
    WALLEVE_EVENT_PEERNET_ADDNODE,
    WALLEVE_EVENT_PEERNET_GETBANNED,
    WALLEVE_EVENT_PEERNET_SETBAN,
    WALLEVE_EVENT_PEERNET_CLRBANNED,
    WALLEVE_EVENT_PEERNET_REWARD,
    WALLEVE_EVENT_PEERNET_CLOSE,
};

class CWallevePeerEventListener;

#define TYPE_PEERNETEVENT(type,body,res) 	\
	CWalleveEventCategory<type,CWallevePeerEventListener,body,res>

typedef std::pair<std::vector<std::string>,int64> ADDRESSES_TO_BAN;

typedef TYPE_PEERNETEVENT(WALLEVE_EVENT_PEERNET_GETIP,int,std::string) CWalleveEventPeerNetGetIP;
typedef TYPE_PEERNETEVENT(WALLEVE_EVENT_PEERNET_GETCOUNT,int,std::size_t) CWalleveEventPeerNetGetCount;
typedef TYPE_PEERNETEVENT(WALLEVE_EVENT_PEERNET_GETPEERS,int,boost::ptr_vector<CPeerInfo>) CWalleveEventPeerNetGetPeers;
typedef TYPE_PEERNETEVENT(WALLEVE_EVENT_PEERNET_ADDNODE,CNetHost,bool) CWalleveEventPeerNetAddNode;
typedef TYPE_PEERNETEVENT(WALLEVE_EVENT_PEERNET_GETBANNED,int,std::vector<CAddressBanned>) CWalleveEventPeerNetGetBanned;
typedef TYPE_PEERNETEVENT(WALLEVE_EVENT_PEERNET_SETBAN,ADDRESSES_TO_BAN,std::size_t) CWalleveEventPeerNetSetBan;
typedef TYPE_PEERNETEVENT(WALLEVE_EVENT_PEERNET_CLRBANNED,std::vector<std::string>,std::size_t) CWalleveEventPeerNetClrBanned;
typedef TYPE_PEERNETEVENT(WALLEVE_EVENT_PEERNET_REWARD,CEndpointManager::Bonus,bool) CWalleveEventPeerNetReward;
typedef TYPE_PEERNETEVENT(WALLEVE_EVENT_PEERNET_CLOSE,CEndpointManager::CloseReason,bool) CWalleveEventPeerNetClose;


class CWallevePeerEventListener : virtual public CWalleveEventListener
{
public:
    virtual ~CWallevePeerEventListener() {}
    DECLARE_EVENTHANDLER(CWalleveEventPeerNetGetIP);
    DECLARE_EVENTHANDLER(CWalleveEventPeerNetGetCount);
    DECLARE_EVENTHANDLER(CWalleveEventPeerNetGetPeers);
    DECLARE_EVENTHANDLER(CWalleveEventPeerNetAddNode);
    DECLARE_EVENTHANDLER(CWalleveEventPeerNetGetBanned);
    DECLARE_EVENTHANDLER(CWalleveEventPeerNetSetBan);
    DECLARE_EVENTHANDLER(CWalleveEventPeerNetClrBanned);
    DECLARE_EVENTHANDLER(CWalleveEventPeerNetReward);
    DECLARE_EVENTHANDLER(CWalleveEventPeerNetClose);
};

} // namespace walleve

#endif //WALLEVE_PEER_EVENT_H

