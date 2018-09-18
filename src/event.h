// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_EVENT_H
#define  MULTIVERSE_EVENT_H

#include "mvtype.h"
#include "dbptype.h"
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
    MV_EVENT_BLOCKMAKER_PUBLISH,
    MV_EVENT_DBP_SOCKET_ADD_NEW_BLOCK,
    MV_EVENT_DBP_SOCKET_ADD_NEW_TX
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

template <int type,typename L,typename D>
class CMvEventDbpSocketData : public walleve::CWalleveEvent
{
    friend class walleve::CWalleveStream;
public:
    CMvEventDbpSocketData(uint64 nNonceIn,const uint256& hashForkIn)
            : CWalleveEvent(nNonceIn,type), hashFork(hashForkIn) {}
    virtual ~CMvEventDbpSocketData() {}
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


enum {
    WALLEVE_EVENT_DBP_REQ,
    WALLEVE_EVENT_DBP_RSP,
    WALLEVE_EVENT_DBP_CONNECT,
    WALLEVE_EVENT_DBP_CONNECTED,
    WALLEVE_EVENT_DBP_FAILED,
    WALLEVE_EVENT_DBP_SUB,
    WALLEVE_EVENT_DBP_UNSUB,
    WALLEVE_EVENT_DBP_NOSUB,
    WALLEVE_EVENT_DBP_READY,
    WALLEVE_EVENT_DBP_ADDED,
    WALLEVE_EVENT_DBP_METHOD,
    WALLEVE_EVENT_DBP_RESULT,

    WALLEVE_EVENT_DBP_PING,
    WALLEVE_EVENT_DBP_PONG,

    WALLEVE_EVENT_DBP_BROKEN
};

class CMvDBPEventListener;
class CDBPEventListener;
#define TYPE_DBPEVENT(type,body)       \
        CMvEventDbpSocketData<type,CMvDBPEventListener,body>

#define TYPE_DBP_EVENT(type,body) 	\
	walleve::CWalleveEventCategory<type,CDBPEventListener,body,bool>

typedef TYPE_DBPEVENT(MV_EVENT_DBP_SOCKET_ADD_NEW_BLOCK,uint256) CMvEventDbpUpdateNewBlock;
typedef TYPE_DBPEVENT(MV_EVENT_DBP_SOCKET_ADD_NEW_TX,CTransaction) CMvEventDbpUpdateNewTx;

typedef TYPE_DBP_EVENT(WALLEVE_EVENT_DBP_REQ,CWalleveDbpRequest) CWalleveEventDbpRequest;
typedef TYPE_DBP_EVENT(WALLEVE_EVENT_DBP_RSP,CWalleveDbpRespond) CWalleveEventDbpRespond;
typedef TYPE_DBP_EVENT(WALLEVE_EVENT_DBP_CONNECT,CWalleveDbpConnect) CWalleveEventDbpConnect;
typedef TYPE_DBP_EVENT(WALLEVE_EVENT_DBP_CONNECTED,CWalleveDbpConnected) CWalleveEventDbpConnected;
typedef TYPE_DBP_EVENT(WALLEVE_EVENT_DBP_FAILED,CWalleveDbpFailed) CWalleveEventDbpFailed;
typedef TYPE_DBP_EVENT(WALLEVE_EVENT_DBP_SUB,CWalleveDbpSub) CWalleveEventDbpSub;
typedef TYPE_DBP_EVENT(WALLEVE_EVENT_DBP_UNSUB,CWalleveDbpUnSub) CWalleveEventDbpUnSub;
typedef TYPE_DBP_EVENT(WALLEVE_EVENT_DBP_NOSUB,CWalleveDbpNoSub) CWalleveEventDbpNoSub;
typedef TYPE_DBP_EVENT(WALLEVE_EVENT_DBP_READY,CWalleveDbpReady) CWalleveEventDbpReady;
typedef TYPE_DBP_EVENT(WALLEVE_EVENT_DBP_ADDED,CWalleveDbpAdded) CWalleveEventDbpAdded;
typedef TYPE_DBP_EVENT(WALLEVE_EVENT_DBP_METHOD,CWalleveDbpMethod) CWalleveEventDbpMethod;
typedef TYPE_DBP_EVENT(WALLEVE_EVENT_DBP_RESULT,CWalleveDbpMethodResult) CWalleveEventDbpMethodResult;
typedef TYPE_DBP_EVENT(WALLEVE_EVENT_DBP_BROKEN,CWalleveDbpBroken) CWalleveEventDbpBroken;

// HeartBeats
typedef TYPE_DBP_EVENT(WALLEVE_EVENT_DBP_PING,CWalleveDbpPing) CWalleveEventDbpPing;
typedef TYPE_DBP_EVENT(WALLEVE_EVENT_DBP_PONG,CWalleveDbpPong) CWalleveEventDbpPong;

class CMvDBPEventListener : virtual public walleve::CWalleveEventListener
{
public:
    virtual ~CMvDBPEventListener() {}
    DECLARE_EVENTHANDLER(CMvEventDbpUpdateNewBlock);
    DECLARE_EVENTHANDLER(CMvEventDbpUpdateNewTx);
};

class CDBPEventListener : virtual public walleve::CWalleveEventListener
{
public:
    virtual ~CDBPEventListener() {}
    DECLARE_EVENTHANDLER(CWalleveEventDbpRequest);
    DECLARE_EVENTHANDLER(CWalleveEventDbpRespond);
    DECLARE_EVENTHANDLER(CWalleveEventDbpConnect);
    DECLARE_EVENTHANDLER(CWalleveEventDbpConnected);
    DECLARE_EVENTHANDLER(CWalleveEventDbpFailed);
    DECLARE_EVENTHANDLER(CWalleveEventDbpSub);
    DECLARE_EVENTHANDLER(CWalleveEventDbpUnSub);
    DECLARE_EVENTHANDLER(CWalleveEventDbpNoSub);
    DECLARE_EVENTHANDLER(CWalleveEventDbpReady);
    DECLARE_EVENTHANDLER(CWalleveEventDbpAdded);
    DECLARE_EVENTHANDLER(CWalleveEventDbpMethod);
    DECLARE_EVENTHANDLER(CWalleveEventDbpMethodResult);
    DECLARE_EVENTHANDLER(CWalleveEventDbpBroken);

    DECLARE_EVENTHANDLER(CWalleveEventDbpPing);
    DECLARE_EVENTHANDLER(CWalleveEventDbpPong);
};

} // namespace multiverse

#endif //MULTIVERSE_EVENT_H
