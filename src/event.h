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
    MV_EVENT_BLOCKMAKER_PUBLISH,
    MV_EVENT_BLOCKMAKER_AGREE,
    MV_EVENT_DBP_SOCKET_ADD_NEW_BLOCK,
    MV_EVENT_DBP_SOCKET_ADD_NEW_TX
};

class CMvBlockMakerEventListener;
#define TYPE_BLOCKMAKEREVENT(type,body)       \
        walleve::CWalleveEventCategory<type,CMvBlockMakerEventListener,body,CNil>

typedef TYPE_BLOCKMAKEREVENT(MV_EVENT_BLOCKMAKER_UPDATE,CBlockMakerUpdate) CMvEventBlockMakerUpdate;
typedef TYPE_BLOCKMAKEREVENT(MV_EVENT_BLOCKMAKER_AGREE,CBlockMakerAgreement) CMvEventBlockMakerAgree;

class CMvBlockMakerEventListener : virtual public walleve::CWalleveEventListener
{
public:
    virtual ~CMvBlockMakerEventListener() {}
    DECLARE_EVENTHANDLER(CMvEventBlockMakerUpdate);
    DECLARE_EVENTHANDLER(CMvEventBlockMakerAgree);
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

class CMvDBPEventListener;
#define TYPE_DBPEVENT(type,body)       \
        CMvEventDbpSocketData<type,CMvDBPEventListener,body>

typedef TYPE_DBPEVENT(MV_EVENT_DBP_SOCKET_ADD_NEW_BLOCK,uint256) CMvEventDbpUpdateNewBlock;
typedef TYPE_DBPEVENT(MV_EVENT_DBP_SOCKET_ADD_NEW_TX,CTransaction) CMvEventDbpUpdateNewTx;

class CMvDBPEventListener : virtual public walleve::CWalleveEventListener
{
public:
    virtual ~CMvDBPEventListener() {}
    DECLARE_EVENTHANDLER(CMvEventDbpUpdateNewBlock);
    DECLARE_EVENTHANDLER(CMvEventDbpUpdateNewTx);
};

} // namespace multiverse

#endif //MULTIVERSE_EVENT_H
