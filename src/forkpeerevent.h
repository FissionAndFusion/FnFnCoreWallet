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

class ForkState
{
public:
    uint256 forkHash;
    int nHeight;
    uint256 lastBlockHash;
};

class TxNotice
{
public:
    uint256 forkHash;
    uint256 txHash;
};

class BlockNotice
{
public:
    uint256 forkHash;
    int nHeight;
    uint256 blockHash;
};



enum class ecForkEventType : int
{
    //FORK NODE PEER NET EVENT
    FK_EVENT_NODE_MESSAGE, //messages received from Dbp service
    FK_EVENT_NODE_ACTIVE,
    FK_EVENT_NODE_DEACTIVE,
    FK_EVENT_NODE_SUBSCRIBE,
    FK_EVENT_NODE_UNSUBSCRIBE,
    FK_EVENT_NODE_GETBLOCKS,
    FK_EVENT_NODE_INV,
    FK_EVENT_NODE_GETDATA,
    FK_EVENT_NODE_BLOCK_ARRIVE,
    FK_EVENT_NODE_TX_ARRIVE,
    FK_EVENT_NODE_BLOCK_REQUEST,
    FK_EVENT_NODE_TX_REQUEST,
    FK_EVENT_NODE_BLOCK,
    FK_EVENT_NODE_TX,

    FK_EVENT_NODE_UPDATE_FORK_STATE,
    FK_EVENT_NODE_SEND_TX_NOTICE,
    FK_EVENT_NODE_SEND_BLOCK_NOTICE,
    FK_EVENT_NODE_SEND_TX,
    FK_EVENT_NODE_SEND_BLOCK,

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
        s.Serialize(hashBlock, opt);
    }
public:
    uint256 hashFork;   //fork id
    uint256 hashBlock;  //hash of requested block
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
    uint256 hashTx;     //Tx hash
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
    uint256 hashBlock;  //hash of new block mint by fork node
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

template <int type, typename L, typename D>
class CFkEventSendBlock : public walleve::CWalleveEvent
{   //used when a block filtered arrives from p2p network
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
class CFkEventNodeData : public walleve::CWalleveEvent
{
    friend class walleve::CWalleveStream;
public:
    CFkEventNodeData(uint64 nNonceIn, const uint256& hashForkIn)
            : CWalleveEvent(nNonceIn, type), hashFork(hashForkIn) {}
    virtual ~CFkEventNodeData() noexcept {}
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
    uint256 hashFork;
    int nBlockHeight;
    D data;
};

typedef boost::variant<network::CAddress, std::vector<uint256>, CBlockLocator,
        std::vector<network::CInv>, CBlock, CTransaction> ForkMsgData_type;

template <int type, typename L, typename D>
class CFkEventMessageData : public walleve::CWalleveEvent
{
    friend class walleve::CWalleveStream;
public:
    CFkEventMessageData(uint64 nNonceIn, const ecForkEventType& nMsgTypeIn)
    : CWalleveEvent(nNonceIn, type), fkMsgType(nMsgTypeIn) {}
    virtual ~CFkEventMessageData() noexcept {}
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
        s.Serialize(fkMsgType, opt);
        s.Serialize(fkMsgData, opt);
    }
public:
    ecForkEventType fkMsgType;
    D fkMsgData; //ForkMsgData_type
};

class CFkNodeEventListener;

#define TYPE_FORKNODEEVENT(type, body)       \
        CFkEventNodeData<static_cast<int>(type), CFkNodeEventListener, body>

#define TYPE_FORKNODEMSGEVENT(type, body)       \
        CFkEventMessageData<static_cast<int>(type), CFkNodeEventListener, body>

typedef TYPE_FORKNODEMSGEVENT(ecForkEventType::FK_EVENT_NODE_MESSAGE, ForkMsgData_type) CFkEventNodeMessage;

typedef TYPE_FORKNODEEVENT(ecForkEventType::FK_EVENT_NODE_ACTIVE, network::CAddress) CFkEventNodeActive;
typedef TYPE_FORKNODEEVENT(ecForkEventType::FK_EVENT_NODE_DEACTIVE, network::CAddress) CFkEventNodeDeactive;
typedef TYPE_FORKNODEEVENT(ecForkEventType::FK_EVENT_NODE_SUBSCRIBE, std::vector<uint256>) CFkEventNodeSubscribe;
typedef TYPE_FORKNODEEVENT(ecForkEventType::FK_EVENT_NODE_UNSUBSCRIBE, std::vector<uint256>) CFkEventNodeUnsubscribe;
typedef TYPE_FORKNODEEVENT(ecForkEventType::FK_EVENT_NODE_GETBLOCKS, CBlockLocator) CFkEventNodeGetBlocks;
typedef TYPE_FORKNODEEVENT(ecForkEventType::FK_EVENT_NODE_INV, std::vector<network::CInv>) CFkEventNodeInv;
typedef TYPE_FORKNODEEVENT(ecForkEventType::FK_EVENT_NODE_GETDATA, std::vector<network::CInv>) CFkEventNodeGetData;
typedef TYPE_FORKNODEEVENT(ecForkEventType::FK_EVENT_NODE_BLOCK, CBlock) CFkEventNodeBlock;
typedef TYPE_FORKNODEEVENT(ecForkEventType::FK_EVENT_NODE_TX, CTransaction) CFkEventNodeTx;


typedef TYPE_FORKNODEEVENT(ecForkEventType::FK_EVENT_NODE_UPDATE_FORK_STATE, ForkState) CFkEventUpdateForkState;
typedef TYPE_FORKNODEEVENT(ecForkEventType::FK_EVENT_NODE_SEND_TX_NOTICE, TxNotice) CFkEventSendTxNotice;
typedef TYPE_FORKNODEEVENT(ecForkEventType::FK_EVENT_NODE_SEND_BLOCK_NOTICE, BlockNotice) CFkEventSendBlockNotice;
typedef TYPE_FORKNODEEVENT(ecForkEventType::FK_EVENT_NODE_SEND_TX, CTransaction) CFkEventSendTx;
typedef TYPE_FORKNODEEVENT(ecForkEventType::FK_EVENT_NODE_SEND_BLOCK, CBlockEx) CFkEventSendBlock;





#define TYPE_FORK_NODE_BLOCK_ARRIVE_EVENT(type, body)       \
        CFkEventBlockArrive<static_cast<int>(type), CFkNodeEventListener, body>

#define TYPE_FORK_NODE_TX_ARRIVE_EVENT(type, body)       \
        CFkEventTxArrive<static_cast<int>(type), CFkNodeEventListener, body>

#define TYPE_FORK_NODE_BLOCK_REQUEST_EVENT(type)       \
        CFkEventBlockRequest<static_cast<int>(type), CFkNodeEventListener>

#define TYPE_FORK_NODE_TX_REQUEST_EVENT(type)       \
        CFkEventTxRequest<static_cast<int>(type), CFkNodeEventListener>

typedef TYPE_FORK_NODE_BLOCK_ARRIVE_EVENT(ecForkEventType::FK_EVENT_NODE_BLOCK_ARRIVE, CBlockEx) CFkEventNodeBlockArrive;
typedef TYPE_FORK_NODE_TX_ARRIVE_EVENT(ecForkEventType::FK_EVENT_NODE_TX_ARRIVE, CTransaction) CFkEventNodeTxArrive;
typedef TYPE_FORK_NODE_BLOCK_REQUEST_EVENT(ecForkEventType::FK_EVENT_NODE_BLOCK_REQUEST) CFkEventNodeBlockRequest;
typedef TYPE_FORK_NODE_TX_REQUEST_EVENT(ecForkEventType::FK_EVENT_NODE_TX_REQUEST) CFkEventNodeTxRequest;


class CFkNodeEventListener : virtual public walleve::CWalleveEventListener
{
public:
    virtual ~CFkNodeEventListener() {}
    DECLARE_EVENTHANDLER(CFkEventNodeMessage);
    DECLARE_EVENTHANDLER(CFkEventNodeActive);
    DECLARE_EVENTHANDLER(CFkEventNodeDeactive);
    DECLARE_EVENTHANDLER(CFkEventNodeSubscribe);
    DECLARE_EVENTHANDLER(CFkEventNodeUnsubscribe);
    DECLARE_EVENTHANDLER(CFkEventNodeGetBlocks);
    DECLARE_EVENTHANDLER(CFkEventNodeInv);
    DECLARE_EVENTHANDLER(CFkEventNodeGetData);
    DECLARE_EVENTHANDLER(CFkEventNodeBlock);
    DECLARE_EVENTHANDLER(CFkEventNodeTx);

    //subscribe/publish(p2p to fork node)
    DECLARE_EVENTHANDLER(CFkEventNodeBlockArrive);
    DECLARE_EVENTHANDLER(CFkEventNodeTxArrive);
    DECLARE_EVENTHANDLER(CFkEventNodeBlockRequest);
    DECLARE_EVENTHANDLER(CFkEventNodeTxRequest);
    //RPC(fork node to p2p)
    DECLARE_EVENTHANDLER(CFkEventUpdateForkState);
    DECLARE_EVENTHANDLER(CFkEventSendBlockNotice);
    DECLARE_EVENTHANDLER(CFkEventSendTxNotice);
    DECLARE_EVENTHANDLER(CFkEventSendTx);
    DECLARE_EVENTHANDLER(CFkEventSendBlock);

};

}
#endif //MULTIVERSE_FORKPEEREVENT_H
