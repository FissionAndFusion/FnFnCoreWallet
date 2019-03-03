// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_EVENT_H
#define MULTIVERSE_EVENT_H

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
    MV_EVENT_BLOCKMAKER_AGREE,
    MV_EVENT_DBP_SOCKET_ADD_NEW_BLOCK,
    MV_EVENT_DBP_SOCKET_ADD_NEW_TX,

    MV_EVENT_DBP_REQ,
    MV_EVENT_DBP_RSP,
    MV_EVENT_DBP_CONNECT,
    MV_EVENT_DBP_CONNECTED,
    MV_EVENT_DBP_FAILED,
    MV_EVENT_DBP_SUB,
    MV_EVENT_DBP_UNSUB,
    MV_EVENT_DBP_NOSUB,
    MV_EVENT_DBP_READY,
    MV_EVENT_DBP_ADDED,
    MV_EVENT_DBP_METHOD,
    MV_EVENT_DBP_RESULT,

    /*super node*/
    MV_EVENT_DBP_VPEERNET,
    MV_EVENT_RPCROUTE_COMMON,
    MV_EVENT_RPCROUTE_ADDED,
    MV_EVENT_RPCROUTE_RESULT,
    MV_EVENT_RPCROUTE_DEL_COMPLTUNTIL,
    MV_EVENT_RPCROUTE_STOP,
    MV_EVENT_RPCROUTE_GET_FORK_COUNT,
    MV_EVENT_RPCROUTE_LIST_FORK,
    MV_EVENT_RPCROUTE_GET_BLOCK_LOCATION,
    MV_EVENT_RPCROUTE_GET_BLOCK_COUNT,
    MV_EVENT_RPCROUTE_GET_BLOCK_HASH,
    MV_EVENT_RPCROUTE_GET_BLOCK,
    MV_EVENT_RPCROUTE_GET_TXPOOL,
    MV_EVENT_RPCROUTE_GET_TRANSACTION,
    MV_EVENT_RPCROUTE_GET_FORK_HEIGHT,
    MV_EVENT_RPCROUTE_SEND_TRANSACTION,

    MV_EVENT_DBP_PING,
    MV_EVENT_DBP_PONG,

    MV_EVENT_DBP_BROKEN,
    MV_EVENT_DBP_REMOVE_SESSION
};

class CMvBlockMakerEventListener;
#define TYPE_BLOCKMAKEREVENT(type, body) \
    walleve::CWalleveEventCategory<type, CMvBlockMakerEventListener, body, CNil>

typedef TYPE_BLOCKMAKEREVENT(MV_EVENT_BLOCKMAKER_UPDATE, CBlockMakerUpdate) CMvEventBlockMakerUpdate;
typedef TYPE_BLOCKMAKEREVENT(MV_EVENT_BLOCKMAKER_AGREE, CBlockMakerAgreement) CMvEventBlockMakerAgree;

class CMvBlockMakerEventListener : virtual public walleve::CWalleveEventListener
{
public:
    virtual ~CMvBlockMakerEventListener() {}
    DECLARE_EVENTHANDLER(CMvEventBlockMakerUpdate);
    DECLARE_EVENTHANDLER(CMvEventBlockMakerAgree);
};

template <int type, typename L, typename D>
class CMvEventDbpSocketData : public walleve::CWalleveEvent
{
    friend class walleve::CWalleveStream;

public:
    CMvEventDbpSocketData(uint64 nNonceIn, const uint256& hashForkIn, int64 nChangeIn)
        : CWalleveEvent(nNonceIn, type), hashFork(hashForkIn), nChange(nChangeIn) {}
    virtual ~CMvEventDbpSocketData() {}
    virtual bool Handle(walleve::CWalleveEventListener& listener)
    {
        try
        {
            return (dynamic_cast<L &>(listener)).HandleEvent(*this);
        }
        catch (std::bad_cast& )
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
    void WalleveSerialize(walleve::CWalleveStream& s, O& opt)
    {
        s.Serialize(hashFork, opt);
        s.Serialize(data, opt);
    }

public:
    uint256 hashFork;
    int64 nChange;
    D data;
};

class CMvDBPEventListener;
class CDBPEventListener;
#define TYPE_DBPEVENT(type, body) \
    CMvEventDbpSocketData<type, CMvDBPEventListener, body>

#define TYPE_DBP_EVENT(type, body) \
    walleve::CWalleveEventCategory<type, CDBPEventListener, body, bool>

#define TYPE_RPCROUTE_EVENT(type, body, result) \
    walleve::CWalleveEventCategory<type, CDBPEventListener, body, result>

typedef TYPE_DBPEVENT(MV_EVENT_DBP_SOCKET_ADD_NEW_BLOCK, CBlockEx) CMvEventDbpUpdateNewBlock;
typedef TYPE_DBPEVENT(MV_EVENT_DBP_SOCKET_ADD_NEW_TX, CTransaction) CMvEventDbpUpdateNewTx;

typedef TYPE_DBP_EVENT(MV_EVENT_DBP_REQ, CMvDbpRequest) CMvEventDbpRequest;
typedef TYPE_DBP_EVENT(MV_EVENT_DBP_RSP, CMvDbpRespond) CMvEventDbpRespond;
typedef TYPE_DBP_EVENT(MV_EVENT_DBP_CONNECT, CMvDbpConnect) CMvEventDbpConnect;
typedef TYPE_DBP_EVENT(MV_EVENT_DBP_CONNECTED, CMvDbpConnected) CMvEventDbpConnected;
typedef TYPE_DBP_EVENT(MV_EVENT_DBP_FAILED, CMvDbpFailed) CMvEventDbpFailed;
typedef TYPE_DBP_EVENT(MV_EVENT_DBP_SUB, CMvDbpSub) CMvEventDbpSub;
typedef TYPE_DBP_EVENT(MV_EVENT_DBP_UNSUB, CMvDbpUnSub) CMvEventDbpUnSub;
typedef TYPE_DBP_EVENT(MV_EVENT_DBP_NOSUB, CMvDbpNoSub) CMvEventDbpNoSub;
typedef TYPE_DBP_EVENT(MV_EVENT_DBP_READY, CMvDbpReady) CMvEventDbpReady;
typedef TYPE_DBP_EVENT(MV_EVENT_DBP_ADDED, CMvDbpAdded) CMvEventDbpAdded;
typedef TYPE_DBP_EVENT(MV_EVENT_DBP_METHOD, CMvDbpMethod) CMvEventDbpMethod;
typedef TYPE_DBP_EVENT(MV_EVENT_DBP_RESULT, CMvDbpMethodResult) CMvEventDbpMethodResult;
typedef TYPE_DBP_EVENT(MV_EVENT_DBP_BROKEN, CMvDbpBroken) CMvEventDbpBroken;
typedef TYPE_DBP_EVENT(MV_EVENT_DBP_REMOVE_SESSION, CMvDbpRemoveSession) CMvEventDbpRemoveSession;

/*supernode*/
typedef TYPE_DBP_EVENT(MV_EVENT_DBP_VPEERNET, CMvDbpVirtualPeerNetEvent) CMvEventDbpVirtualPeerNet;
typedef TYPE_RPCROUTE_EVENT(MV_EVENT_RPCROUTE_STOP, CMvRPCRouteStop, bool) CMvEventRPCRouteStop;
typedef TYPE_RPCROUTE_EVENT(MV_EVENT_RPCROUTE_GET_FORK_COUNT, CMvRPCRouteGetForkCount, bool) CMvEventRPCRouteGetForkCount;
typedef TYPE_RPCROUTE_EVENT(MV_EVENT_RPCROUTE_LIST_FORK, CMvRPCRouteListFork, bool) CMvEventRPCRouteListFork;
typedef TYPE_RPCROUTE_EVENT(MV_EVENT_RPCROUTE_GET_BLOCK_LOCATION, CMvRPCRouteGetBlockLocation, bool) CMvEventRPCRouteGetBlockLocation;
typedef TYPE_RPCROUTE_EVENT(MV_EVENT_RPCROUTE_GET_BLOCK_COUNT, CMvRPCRouteGetBlockCount, bool) CMvEventRPCRouteGetBlockCount;
typedef TYPE_RPCROUTE_EVENT(MV_EVENT_RPCROUTE_GET_BLOCK_HASH, CMvRPCRouteGetBlockHash, bool) CMvEventRPCRouteGetBlockHash;
typedef TYPE_RPCROUTE_EVENT(MV_EVENT_RPCROUTE_GET_BLOCK, CMvRPCRouteGetBlock, bool) CMvEventRPCRouteGetBlock;
typedef TYPE_RPCROUTE_EVENT(MV_EVENT_RPCROUTE_GET_TXPOOL, CMvRPCRouteGetTxPool, bool) CMvEventRPCRouteGetTxPool;
typedef TYPE_RPCROUTE_EVENT(MV_EVENT_RPCROUTE_GET_TRANSACTION, CMvRPCRouteGetTransaction, bool) CMvEventRPCRouteGetTransaction;
typedef TYPE_RPCROUTE_EVENT(MV_EVENT_RPCROUTE_GET_FORK_HEIGHT, CMvRPCRouteGetForkHeight, bool) CMvEventRPCRouteGetForkHeight;
typedef TYPE_RPCROUTE_EVENT(MV_EVENT_RPCROUTE_SEND_TRANSACTION, CMvRPCRouteSendTransaction, bool) CMvEventRPCRouteSendTransaction;
typedef TYPE_RPCROUTE_EVENT(MV_EVENT_RPCROUTE_ADDED, CMvRPCRouteAdded, bool) CMvEventRPCRouteAdded;
typedef TYPE_RPCROUTE_EVENT(MV_EVENT_RPCROUTE_RESULT, CMvRPCRouteResult, bool) CMvEventRPCRouteResult;
typedef TYPE_RPCROUTE_EVENT(MV_EVENT_RPCROUTE_DEL_COMPLTUNTIL, CMvRPCRouteDelCompltUntil, bool) CMvEventRPCRouteDelCompltUntil;

// HeartBeats
typedef TYPE_DBP_EVENT(MV_EVENT_DBP_PING, CMvDbpPing) CMvEventDbpPing;
typedef TYPE_DBP_EVENT(MV_EVENT_DBP_PONG, CMvDbpPong) CMvEventDbpPong;

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
    DECLARE_EVENTHANDLER(CMvEventDbpRequest);
    DECLARE_EVENTHANDLER(CMvEventDbpRespond);
    DECLARE_EVENTHANDLER(CMvEventDbpConnect);
    DECLARE_EVENTHANDLER(CMvEventDbpConnected);
    DECLARE_EVENTHANDLER(CMvEventDbpFailed);
    DECLARE_EVENTHANDLER(CMvEventDbpSub);
    DECLARE_EVENTHANDLER(CMvEventDbpUnSub);
    DECLARE_EVENTHANDLER(CMvEventDbpNoSub);
    DECLARE_EVENTHANDLER(CMvEventDbpReady);
    DECLARE_EVENTHANDLER(CMvEventDbpAdded);
    DECLARE_EVENTHANDLER(CMvEventDbpMethod);
    DECLARE_EVENTHANDLER(CMvEventDbpMethodResult);
    DECLARE_EVENTHANDLER(CMvEventDbpBroken);
    DECLARE_EVENTHANDLER(CMvEventDbpRemoveSession);

    /*supernode*/
    DECLARE_EVENTHANDLER(CMvEventDbpVirtualPeerNet);
    DECLARE_EVENTHANDLER(CMvEventRPCRouteAdded);
    DECLARE_EVENTHANDLER(CMvEventRPCRouteResult);
    DECLARE_EVENTHANDLER(CMvEventRPCRouteStop);
    DECLARE_EVENTHANDLER(CMvEventRPCRouteGetForkCount);
    DECLARE_EVENTHANDLER(CMvEventRPCRouteListFork);
    DECLARE_EVENTHANDLER(CMvEventRPCRouteGetBlockLocation);
    DECLARE_EVENTHANDLER(CMvEventRPCRouteGetBlockCount);
    DECLARE_EVENTHANDLER(CMvEventRPCRouteGetBlockHash);
    DECLARE_EVENTHANDLER(CMvEventRPCRouteGetBlock);
    DECLARE_EVENTHANDLER(CMvEventRPCRouteGetTxPool);
    DECLARE_EVENTHANDLER(CMvEventRPCRouteGetTransaction);
    DECLARE_EVENTHANDLER(CMvEventRPCRouteGetForkHeight);
    DECLARE_EVENTHANDLER(CMvEventRPCRouteSendTransaction);
    DECLARE_EVENTHANDLER(CMvEventRPCRouteDelCompltUntil);
    DECLARE_EVENTHANDLER(CMvEventDbpPing);
    DECLARE_EVENTHANDLER(CMvEventDbpPong);

};

} // namespace multiverse

#endif //MULTIVERSE_EVENT_H
