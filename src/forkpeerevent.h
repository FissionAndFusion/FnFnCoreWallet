// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_FORKPEEREVENT_H
#define MULTIVERSE_FORKPEEREVENT_H

#include <boost/variant.hpp>

#include "walleve/walleve.h"
#include "mvproto.h"
#include "block.h"

namespace multiverse
{


enum class ecForkEventType : int
{
    //FORK NODE PEER NET EVENT
    FK_EVENT_NODE_BLOCK_ARRIVE,
    FK_EVENT_NODE_TX_ARRIVE,
    FK_EVENT_NODE_BLOCK_REQUEST,
    FK_EVENT_NODE_TX_REQUEST,
    FK_EVENT_NODE_UPDATE_FORK_STATE,
    FK_EVENT_NODE_SEND_TX_NOTICE,
    FK_EVENT_NODE_SEND_BLOCK_NOTICE,
    FK_EVENT_NODE_SEND_TX,
    FK_EVENT_NODE_SEND_BLOCK,

    FK_EVENT_NODE_IS_FORKNODE,

    FK_EVENT_NODE_MAIN_BLOCK_REQUEST,

    FK_EVENT_NODE_ADD_NEW_FORK_NODE,

    FK_EVENT_NODE_MAX
};

template <int type, typename L, typename D>
class CFkEventBlockArrive : public walleve::CWalleveEvent
{   //used when a block filtered arrives from p2p network
    friend class walleve::CWalleveStream;
public:
    CFkEventBlockArrive(uint64 nNonceIn, const uint256& hashForkIn)
            : CWalleveEvent(nNonceIn, type), hashFork(hashForkIn) {}
    virtual ~CFkEventBlockArrive() {}
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
    void WalleveSerialize(walleve::CWalleveStream& s, O& opt)
    {
        s.Serialize(hashFork, opt);
        s.Serialize(height, opt);
        s.Serialize(data, opt);
    }
public:
    uint256 hashFork;   //fork id
    int height;         //last height plus one
    D data;             //object of CBlockEx
};

template <int type, typename L, typename D>
class CFkEventTxArrive : public walleve::CWalleveEvent
{   //used when a transaction filtered arrives from p2p network
    friend class walleve::CWalleveStream;
public:
    CFkEventTxArrive(uint64 nNonceIn, const uint256& hashForkIn)
            : CWalleveEvent(nNonceIn, type), hashFork(hashForkIn) {}
    virtual ~CFkEventTxArrive() {}
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
    void WalleveSerialize(walleve::CWalleveStream& s, O& opt)
    {
        s.Serialize(hashFork, opt);
        s.Serialize(data, opt);
    }
public:
    uint256 hashFork;   //fork id
    D data;             //object of CTransaction
};

template <int type, typename L>
class CFkEventBlockRequest : public walleve::CWalleveEvent
{   //used when a request for a block filtered arrives from p2p network
    friend class walleve::CWalleveStream;
public:
    CFkEventBlockRequest(uint64 nNonceIn, const uint256& hashForkIn)
            : CWalleveEvent(nNonceIn, type), hashFork(hashForkIn) {}
    virtual ~CFkEventBlockRequest() {}
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
    void WalleveSerialize(walleve::CWalleveStream& s, O& opt)
    {
        s.Serialize(hashFork, opt);
        s.Serialize(hashBlock, opt);
    }
public:
    uint256 hashFork;   //fork id
    uint256 hashBlock;  //hash of requested block
};

template <int type, typename L>
class CFkEventTxRequest : public walleve::CWalleveEvent
{   //used when a request for a transaction filtered arrives from p2p network
    friend class walleve::CWalleveStream;
public:
    CFkEventTxRequest(uint64 nNonceIn, const uint256& hashForkIn)
            : CWalleveEvent(nNonceIn, type), hashFork(hashForkIn) {}
    virtual ~CFkEventTxRequest() {}
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
    void WalleveSerialize(walleve::CWalleveStream& s, O& opt)
    {
        s.Serialize(hashFork, opt);
        s.Serialize(hashTx, opt);
    }
public:
    uint256 hashFork;   //fork id
    uint256 hashTx;  //hash of requested block
};

template <int type, typename L>
class CFkEventUpdateForkState : public walleve::CWalleveEvent
{
    friend class walleve::CWalleveStream;
public:
    CFkEventUpdateForkState(uint64 nNonceIn, const uint256& hashForkIn)
            : CWalleveEvent(nNonceIn, type), hashFork(hashForkIn) {}
    virtual ~CFkEventUpdateForkState() {}
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
    void WalleveSerialize(walleve::CWalleveStream& s, O& opt)
    {
        s.Serialize(hashFork, opt);
        s.Serialize(height, opt);
        s.Serialize(hashBlock, opt);
    }
public:
    uint256 hashFork;   //fork id
    int     height;     //last height plus one
    uint256 hashBlock;  //last hash of CBlockEx
};

template <int type, typename L>
class CFkEventSendBlockNotice : public walleve::CWalleveEvent
{
    friend class walleve::CWalleveStream;
public:
    CFkEventSendBlockNotice(uint64 nNonceIn, const uint256& hashForkIn)
            : CWalleveEvent(nNonceIn, type), hashFork(hashForkIn) {}
    virtual ~CFkEventSendBlockNotice() {}
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
    void WalleveSerialize(walleve::CWalleveStream& s, O& opt)
    {
        s.Serialize(hashFork, opt);
        s.Serialize(hashBlock, opt);
    }
public:
    uint256 hashFork;   //fork id
    uint256 hashBlock;  //hash of new block mint by fork node to notify over p2p network
};

template <int type, typename L>
class CFkEventSendTxNotice : public walleve::CWalleveEvent
{
    friend class walleve::CWalleveStream;
public:
    CFkEventSendTxNotice(uint64 nNonceIn, const uint256& hashForkIn)
            : CWalleveEvent(nNonceIn, type), hashFork(hashForkIn) {}
    virtual ~CFkEventSendTxNotice() {}
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
    void WalleveSerialize(walleve::CWalleveStream& s, O& opt)
    {
        s.Serialize(hashFork, opt);
        s.Serialize(hashTx, opt);
    }
public:
    uint256 hashFork;   //fork id
    uint256 hashTx;     //Tx hash to notify over p2p network
};

template <int type, typename L, typename D>
class CFkEventSendBlock : public walleve::CWalleveEvent
{
    friend class walleve::CWalleveStream;
public:
    CFkEventSendBlock(uint64 nNonceIn, const uint256& hashForkIn)
            : CWalleveEvent(nNonceIn, type), hashFork(hashForkIn) {}
    virtual ~CFkEventSendBlock() {}
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
    void WalleveSerialize(walleve::CWalleveStream& s, O& opt)
    {
        s.Serialize(hashFork, opt);
        s.Serialize(data, opt);
    }
public:
    uint256 hashFork;   //fork id
    D data;             //object of CBlockEx
};

template <int type, typename L, typename D>
class CFkEventSendTx : public walleve::CWalleveEvent
{
    friend class walleve::CWalleveStream;
public:
    CFkEventSendTx(uint64 nNonceIn, const uint256& hashForkIn)
            : CWalleveEvent(nNonceIn, type), hashFork(hashForkIn) {}
    virtual ~CFkEventSendTx() {}
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
    void WalleveSerialize(walleve::CWalleveStream& s, O& opt)
    {
        s.Serialize(hashFork, opt);
        s.Serialize(data, opt);
    }
public:
    uint256 hashFork;   //fork id
    D data;             //object of CTransaction
};

template <int type, typename L>
class CFkEventIsForkNode : public walleve::CWalleveEvent
{
    friend class walleve::CWalleveStream;
public:
    CFkEventIsForkNode(uint64 nNonceIn)
            : CWalleveEvent(nNonceIn, type) {}
    virtual ~CFkEventIsForkNode() {}
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
    void WalleveSerialize(walleve::CWalleveStream& s, O& opt)
    {
        s.Serialize(fIsForkNode, opt);
    }
public:
    bool fIsForkNode;           
};

template <int type, typename L>
class CFkEventMainBlockRequest : public walleve::CWalleveEvent
{   //used when a request for a block filtered arrives from p2p network
    friend class walleve::CWalleveStream;
public:
    CFkEventMainBlockRequest(uint64 nNonceIn)
            : CWalleveEvent(nNonceIn, type) {}
    virtual ~CFkEventMainBlockRequest() {}
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
    void WalleveSerialize(walleve::CWalleveStream& s, O& opt)
    {
        s.Serialize(height, opt);
    }
public:
    int height;         //last height of main chain block
};

template <int type, typename L, typename D>
class CFkEventNewForkNodeConnected : public walleve::CWalleveEvent
{   //used when a block filtered arrives from p2p network
    friend class walleve::CWalleveStream;
public:
    CFkEventNewForkNodeConnected(uint64 nNonceIn)
            : CWalleveEvent(nNonceIn, type) {}
    virtual ~CFkEventNewForkNodeConnected() {}
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
    void WalleveSerialize(walleve::CWalleveStream& s, O& opt)
    {
        s.Serialize(data, opt);
    }
public:
    D data;             //set of nonce
};

class CFkNodeEventListener;

#define TYPE_FORK_NODE_BLOCK_ARRIVE_EVENT(type, body)       \
        CFkEventBlockArrive<static_cast<int>(type), CFkNodeEventListener, body>
#define TYPE_FORK_NODE_TX_ARRIVE_EVENT(type, body)       \
        CFkEventTxArrive<static_cast<int>(type), CFkNodeEventListener, body>
#define TYPE_FORK_NODE_BLOCK_REQUEST_EVENT(type)       \
        CFkEventBlockRequest<static_cast<int>(type), CFkNodeEventListener>
#define TYPE_FORK_NODE_TX_REQUEST_EVENT(type)       \
        CFkEventTxRequest<static_cast<int>(type), CFkNodeEventListener>

#define TYPE_FORK_NODE_UPDATE_FORK_STATE_EVENT(type)       \
        CFkEventUpdateForkState<static_cast<int>(type), CFkNodeEventListener>
#define TYPE_FORK_NODE_SEND_BLOCK_NOTICE_ARRIVE_EVENT(type)       \
        CFkEventSendBlockNotice<static_cast<int>(type), CFkNodeEventListener>
#define TYPE_FORK_NODE_SEND_TX_NOTICE_ARRIVE_EVENT(type)       \
        CFkEventSendTxNotice<static_cast<int>(type), CFkNodeEventListener>
#define TYPE_FORK_NODE_SEND_BLOCK_EVENT(type, body)       \
        CFkEventSendBlock<static_cast<int>(type), CFkNodeEventListener, body>
#define TYPE_FORK_NODE_SEND_TX_EVENT(type, body)       \
        CFkEventSendTx<static_cast<int>(type), CFkNodeEventListener, body>

#define TYPE_FORK_NODE_IS_FORK_NODE_EVENT(type)       \
        CFkEventIsForkNode<static_cast<int>(type), CFkNodeEventListener>

#define TYPE_FORK_NODE_MAIN_BLOCK_REQUEST_EVENT(type)       \
        CFkEventMainBlockRequest<static_cast<int>(type), CFkNodeEventListener>

#define TYPE_FORK_NODE_NEW_FORK_NODE_CONNECTED_EVENT(type, body)       \
        CFkEventNewForkNodeConnected<static_cast<int>(type), CFkNodeEventListener, body>

typedef TYPE_FORK_NODE_BLOCK_ARRIVE_EVENT(ecForkEventType::FK_EVENT_NODE_BLOCK_ARRIVE, CBlockEx) CFkEventNodeBlockArrive;
typedef TYPE_FORK_NODE_TX_ARRIVE_EVENT(ecForkEventType::FK_EVENT_NODE_TX_ARRIVE, CTransaction) CFkEventNodeTxArrive;
typedef TYPE_FORK_NODE_BLOCK_REQUEST_EVENT(ecForkEventType::FK_EVENT_NODE_BLOCK_REQUEST) CFkEventNodeBlockRequest;
typedef TYPE_FORK_NODE_TX_REQUEST_EVENT(ecForkEventType::FK_EVENT_NODE_TX_REQUEST) CFkEventNodeTxRequest;

typedef TYPE_FORK_NODE_UPDATE_FORK_STATE_EVENT(ecForkEventType::FK_EVENT_NODE_UPDATE_FORK_STATE) CFkEventNodeUpdateForkState;
typedef TYPE_FORK_NODE_SEND_BLOCK_NOTICE_ARRIVE_EVENT(ecForkEventType::FK_EVENT_NODE_SEND_BLOCK_NOTICE) CFkEventNodeSendBlockNotice;
typedef TYPE_FORK_NODE_SEND_TX_NOTICE_ARRIVE_EVENT(ecForkEventType::FK_EVENT_NODE_SEND_TX_NOTICE) CFkEventNodeSendTxNotice;
typedef TYPE_FORK_NODE_SEND_BLOCK_EVENT(ecForkEventType::FK_EVENT_NODE_SEND_BLOCK, CBlockEx) CFkEventNodeSendBlock;
typedef TYPE_FORK_NODE_SEND_TX_EVENT(ecForkEventType::FK_EVENT_NODE_SEND_TX, CTransaction) CFkEventNodeSendTx;
typedef TYPE_FORK_NODE_IS_FORK_NODE_EVENT(ecForkEventType::FK_EVENT_NODE_IS_FORKNODE) CFkEventNodeIsForkNode;

typedef TYPE_FORK_NODE_MAIN_BLOCK_REQUEST_EVENT(ecForkEventType::FK_EVENT_NODE_MAIN_BLOCK_REQUEST) CFkEventNodeMainBlockRequest;

typedef TYPE_FORK_NODE_NEW_FORK_NODE_CONNECTED_EVENT(ecForkEventType::FK_EVENT_NODE_ADD_NEW_FORK_NODE, std::set<uint64>) CFkEventNodeNewForkNodeConnected;

class CFkNodeEventListener : virtual public walleve::CWalleveEventListener
{
public:
    virtual ~CFkNodeEventListener() {}
    //subscribe/publish(p2p to fork node)
    DECLARE_EVENTHANDLER(CFkEventNodeBlockArrive);
    DECLARE_EVENTHANDLER(CFkEventNodeTxArrive);
    DECLARE_EVENTHANDLER(CFkEventNodeBlockRequest);
    DECLARE_EVENTHANDLER(CFkEventNodeTxRequest);
    //RPC(fork node to p2p)
    DECLARE_EVENTHANDLER(CFkEventNodeUpdateForkState);
    DECLARE_EVENTHANDLER(CFkEventNodeSendBlockNotice);
    DECLARE_EVENTHANDLER(CFkEventNodeSendTxNotice);
    DECLARE_EVENTHANDLER(CFkEventNodeSendBlock);
    DECLARE_EVENTHANDLER(CFkEventNodeSendTx);
    DECLARE_EVENTHANDLER(CFkEventNodeIsForkNode);

    DECLARE_EVENTHANDLER(CFkEventNodeMainBlockRequest);

    DECLARE_EVENTHANDLER(CFkEventNodeNewForkNodeConnected);
};

}
#endif //MULTIVERSE_FORKPEEREVENT_H
