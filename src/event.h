// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_EVENT_H
#define  MULTIVERSE_EVENT_H

#include "mvtype.h"
#include "block.h"
#include "transaction.h"
#include "mvpeerevent.h"
#include "walleve/walleve.h"
#include <vector>
#include <map>
#include <set>

namespace multiverse
{

enum
{
    MV_EVENT_BASE = network::MV_EVENT_PEER_MAX,
    MV_EVENT_BLOCKMAKER_UPDATE,
    MV_EVENT_BLOCKMAKER_ENROLL,
    MV_EVENT_BLOCKMAKER_DISTRIBUTE,
    MV_EVENT_BLOCKMAKER_PUBLISH
};

class CMvBlockMakerEventListener;
#define TYPE_BLOCKMAKEREVENT(type,body)       \
        walleve::CWalleveEventCategory<type,CMvBlockMakerEventListener,body,CNil>
typedef std::pair<uint256,int64> PairHashTime;

typedef TYPE_BLOCKMAKEREVENT(MV_EVENT_BLOCKMAKER_UPDATE,PairHashTime) CMvEventBlockMakerUpdate;

class CMvBlockMakerEventListener : virtual public walleve::CWalleveEventListener
{
public:
    virtual ~CMvBlockMakerEventListener() {}
    DECLARE_EVENTHANDLER(CMvEventBlockMakerUpdate);
};

enum
{
    MV_EVENT_DBP_SOCKET_STUB = MV_EVENT_BLOCKMAKER_PUBLISH,
	MV_EVENT_DBP_SOCKET_ADD_NEW_BLOCK,
	MV_EVENT_DBP_SOCKET_ADD_NEW_TX
};

class CMvDBPEventListener;
#define TYPE_DBPEVENT(type,body)       \
		walleve::CWalleveEventCategory<type,CMvDBPEventListener,body,CNil>

typedef TYPE_DBPEVENT(MV_EVENT_DBP_SOCKET_ADD_NEW_BLOCK,uint256) CMvEventDbpUpdateNewBlock;
typedef TYPE_DBPEVENT(MV_EVENT_DBP_SOCKET_ADD_NEW_TX,uint256) CMvEventDbpUpdateNewTx;

class CMvDBPEventListener : virtual public walleve::CWalleveEventListener
{
public:
	virtual ~CMvDBPEventListener() {}
	DECLARE_EVENTHANDLER(CMvEventDbpUpdateNewBlock);
	DECLARE_EVENTHANDLER(CMvEventDbpUpdateNewTx);
};

} // namespace multiverse

#endif //MULTIVERSE_EVENT_H
