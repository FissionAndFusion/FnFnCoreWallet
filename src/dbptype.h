// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_DBP_TYPE_H
#define MULTIVERSE_DBP_TYPE_H

#include <boost/any.hpp>
#include "profile.h"

namespace multiverse
{

static const std::string ALL_BLOCK_TOPIC("all-block");
static const std::string ALL_TX_TOPIC("all-tx");
static const std::string SYS_CMD_TOPIC("sys-cmd");
static const std::string BLOCK_CMD_TOPIC("block-cmd");
static const std::string TX_CMD_TOPIC("tx-cmd");
static const std::string CHANGED_TOPIC("changed");
static const std::string REMOVED_TOPIC("removed");
static const std::string RPC_CMD_TOPIC("rpc-cmd");

class CMvDbpContent
{
public:
};

class CMvDbpRequest : public CMvDbpContent
{
public:
};

class CMvDbpRespond : public CMvDbpContent
{
public:
};

class CMvDbpConnect : public CMvDbpRequest
{
public:
    bool isReconnect;
    std::string session;
    int32 version;
    std::string client;
    std::string forks; // supre node child node fork ids
};

class CMvDbpSub : public CMvDbpRequest
{
public:
    std::string id;
    std::string name;
};

class CMvDbpUnSub : public CMvDbpRequest
{
public:
    std::string id;
};

class CMvDbpNoSub : public CMvDbpRespond
{
public:
    std::string id;
    std::string error;
};

class CMvDbpReady : public CMvDbpRespond
{
public:
    std::string id;
};

class CMvDbpVirtualPeerNetEvent
{
public:
    enum EventType : int
    {
        DBP_EVENT_PEER_ACTIVE = 0x00,
        DBP_EVENT_PEER_DEACTIVE = 0x01,
        DBP_EVENT_PEER_SUBSCRIBE = 0x02,
        DBP_EVENT_PEER_UNSUBSCRIBE = 0x03,
        DBP_EVENT_PEER_INV = 0x04,
        DBP_EVENT_PEER_GETDATA = 0x05,
        DBP_EVENT_PEER_GETBLOCKS= 0x06,
        DBP_EVENT_PEER_TX = 0x07,
        DBP_EVENT_PEER_BLOCK = 0x08,
        DBP_EVENT_PEER_REWARD = 0x09,
        DBP_EVENT_PEER_CLOSE = 0x0A
    };
public:
    uint64 nNonce; 
    int type;
    uint256 hashFork;
    std::vector<uint8> data;
};

//rpc route

class CMvRPCRouteAdded : public CMvDbpRespond
{
public:
    std::string name;
    std::string id;
    int type;
    std::vector<uint8> vData;
};

class CMvRPCRoute
{
    friend class walleve::CWalleveStream;
public:
    enum
    {
        DBP_RPCROUTE_STOP = 0,
        DBP_RPCROUTE_GET_FORK_COUNT = 1,
        DBP_RPCROUTE_LIST_FORK = 2,
        DBP_RPCROUTE_GET_BLOCK_LOCATION = 3,
        DBP_RPCROUTE_GET_BLOCK_COUNT = 4,
        DBP_RPCROUTE_GET_BLOCK_HASH = 5,
        DBP_RPCROUTE_GET_BLOCK = 6,
        DBP_RPCROUTE_GET_TXPOOL = 7,
        DBP_RPCROUTE_GET_TRANSACTION = 8,
        DBP_RPCROUTE_GET_FORK_HEIGHT = 9,
        DBP_RPCROUTE_SEND_TRANSACTION = 10
    };

    walleve::CIOCompletion *ioComplt;
    walleve::CIOCompletionUntil *pIoCompltUntil;
    int type;
    uint64 nNonce;

protected:
    template <typename O>
    void WalleveSerialize(walleve::CWalleveStream& s,O& opt)
    {
        s.Serialize(type,opt);
        s.Serialize(nNonce, opt);
    }    
};

class CMvRPCRouteStop : public CMvRPCRoute
{
    friend class walleve::CWalleveStream;
public:

protected:
    template <typename O>
    void WalleveSerialize(walleve::CWalleveStream& s,O& opt)
    {
        CMvRPCRoute::WalleveSerialize(s, opt);
    } 
};

class CMvRPCRouteGetForkCount : public CMvRPCRoute
{
    friend class walleve::CWalleveStream;
public:

protected:
    template <typename O>
    void WalleveSerialize(walleve::CWalleveStream& s,O& opt)
    {
        CMvRPCRoute::WalleveSerialize(s, opt);
    } 
};

class CMvRPCRouteListFork : public CMvRPCRoute
{
    friend class walleve::CWalleveStream;
public:
    bool fAll;

protected:
    template<typename O>
    void WalleveSerialize(walleve::CWalleveStream& s, O& opt)
    {
        CMvRPCRoute::WalleveSerialize(s, opt);
        s.Serialize(fAll, opt);
    }
};

class CMvRPCRouteGetBlockLocation : public CMvRPCRoute
{
    friend class walleve::CWalleveStream;
public:
    std::string strBlock;

protected:
    template<typename O>
    void WalleveSerialize(walleve::CWalleveStream& s, O& opt)
    {
        CMvRPCRoute::WalleveSerialize(s, opt);
        s.Serialize(strBlock, opt);
    }
};

class CMvRPCRouteGetBlockCount: public CMvRPCRoute
{
    friend class walleve::CWalleveStream;
public:
    std::string strFork;

protected:
    template<typename O>
    void WalleveSerialize(walleve::CWalleveStream& s, O& opt)
    {
        CMvRPCRoute::WalleveSerialize(s, opt);
        s.Serialize(strFork, opt);
    }
};

class CMvRPCRouteGetBlockHash : public CMvRPCRoute
{
    friend class walleve::CWalleveStream;
public:
    int height;
    std::string strFork;

protected:
    template<typename O>
    void WalleveSerialize(walleve::CWalleveStream& s, O& opt)
    {
        CMvRPCRoute::WalleveSerialize(s, opt);
        s.Serialize(height, opt);
        s.Serialize(strFork, opt);
    }
};

class CMvRPCRouteGetBlock : public CMvRPCRoute
{
    friend class walleve::CWalleveStream;
public:
    std::string hash;

protected:
    template<typename O>
    void WalleveSerialize(walleve::CWalleveStream& s, O& opt)
    {
        CMvRPCRoute::WalleveSerialize(s, opt);
        s.Serialize(hash, opt);
    }
};

class CMvRPCRouteGetTxPool: public CMvRPCRoute
{
    friend class walleve::CWalleveStream;
public:
    std::string strFork;
    bool fDetail;

protected:
    template<typename O>
    void WalleveSerialize(walleve::CWalleveStream& s, O& opt)
    {
        CMvRPCRoute::WalleveSerialize(s, opt);
        s.Serialize(strFork, opt);
        s.Serialize(fDetail, opt);
    }
};

class CMvRPCRouteGetTransaction: public CMvRPCRoute
{
    friend class walleve::CWalleveStream;
public:
    std::string hash;

protected:
    template<typename O>
    void WalleveSerialize(walleve::CWalleveStream& s, O& opt)
    {
        CMvRPCRoute::WalleveSerialize(s, opt);
        s.Serialize(hash, opt);
    }
};

class CMvRPCRouteGetForkHeight : public CMvRPCRoute
{
    friend class walleve::CWalleveStream;
public:
    std::string hash;

protected:
    template<typename O>
    void WalleveSerialize(walleve::CWalleveStream& s, O& opt)
    {
        CMvRPCRoute::WalleveSerialize(s, opt);
        s.Serialize(hash, opt);
    }
};

class CMvRPCRouteSendTransaction: public CMvRPCRoute
{
    friend class walleve::CWalleveStream;
public:
    std::string hash;

protected:
    template<typename O>
    void WalleveSerialize(walleve::CWalleveStream& s, O& opt)
    {
        CMvRPCRoute::WalleveSerialize(s, opt);
        s.Serialize(hash, opt);
    }
};

class CMvRPCRouteResult
{
public:
    int type;
    std::vector<uint8> vData;
    std::vector<uint8> vRawData;
};

class CMvRPCRouteRet
{
    friend class walleve::CWalleveStream;
public:
    int type;
    uint64 nNonce;

protected:
    template<typename O>
    void WalleveSerialize(walleve::CWalleveStream& s, O& opt)
    {
        s.Serialize(type, opt);
        s.Serialize(nNonce, opt);
    }
};

class CMvRPCRouteStopRet : public CMvRPCRouteRet
{
    friend class walleve::CWalleveStream;
public:
protected:
    template<typename O>
    void WalleveSerialize(walleve::CWalleveStream& s, O& opt)
    {
        CMvRPCRouteRet::WalleveSerialize(s, opt);
    }
};

class CMvRPCRouteGetForkCountRet : public CMvRPCRouteRet
{
    friend class walleve::CWalleveStream;
public:
    int count;

protected:
    template<typename O>
    void WalleveSerialize(walleve::CWalleveStream& s, O& opt)
    {
        CMvRPCRouteRet::WalleveSerialize(s, opt);
        s.Serialize(count, opt);
    }
};

class CMvRPCProfile
{
    friend class walleve::CWalleveStream;
public:
    std::string strHex;
    std::string strName;
    std::string strSymbol;
    bool fIsolated;
    bool fPrivate;
    bool fEnclosed;
    std::string address;
protected:
    template <typename O>
    void WalleveSerialize(walleve::CWalleveStream& s, O& opt)
    {
        s.Serialize(strHex, opt);
        s.Serialize(strName, opt);
        s.Serialize(strSymbol, opt);
        s.Serialize(fIsolated, opt);
        s.Serialize(fPrivate, opt);
        s.Serialize(fEnclosed, opt);
        s.Serialize(address, opt);
    }
};

class CMvRPCRouteListForkRet: public CMvRPCRouteRet
{
    friend class walleve::CWalleveStream;
public:
    std::vector<CMvRPCProfile> vFork;

protected:
    template<typename O>
    void WalleveSerialize(walleve::CWalleveStream& s, O& opt)
    {
        CMvRPCRouteRet::WalleveSerialize(s, opt);
        s.Serialize(vFork, opt);
    }
};

class CMvRPCRouteGetBlockLocationRet : public CMvRPCRouteRet
{
    friend class walleve::CWalleveStream;
public:
    std::string strFork;
    int height;

protected:
    template<typename O>
    void WalleveSerialize(walleve::CWalleveStream& s, O& opt)
    {
        CMvRPCRouteRet::WalleveSerialize(s, opt);
        s.Serialize(strFork, opt);
        s.Serialize(height, opt);
    }
};

class CMvRPCRouteGetBlockCountRet : public CMvRPCRouteRet
{
    friend class walleve::CWalleveStream;
public:
    std::string strFork;
    int height;
    int exception; // 0-nomal, 1-invalid fork, 2-unknow fork

protected:
    template<typename O>
    void WalleveSerialize(walleve::CWalleveStream& s, O& opt)
    {
        CMvRPCRouteRet::WalleveSerialize(s, opt);
        s.Serialize(strFork, opt);
        s.Serialize(height, opt);
        s.Serialize(exception, opt);
    }
};

class CMvRPCRouteGetBlockHashRet : public CMvRPCRouteRet
{
    friend class walleve::CWalleveStream;
public:
    std::vector<std::string> vHash;
    int exception; // 0-nomal, 1-invalid fork, 2-unknow fork, 3-out of range

protected:
    template<typename O>
    void WalleveSerialize(walleve::CWalleveStream& s, O& opt)
    {
        CMvRPCRouteRet::WalleveSerialize(s, opt);
        s.Serialize(vHash, opt);
        s.Serialize(exception, opt);
    }
};

class CMvRPCRouteGetBlockRet : public CMvRPCRouteRet
{
    friend class walleve::CWalleveStream;
public:
    CBlock block;
    int height;
    std::string strFork;
    int exception; // 0-nomal, 1-unknown block

protected:
    template<typename O>
    void WalleveSerialize(walleve::CWalleveStream& s, O& opt)
    {
        CMvRPCRouteRet::WalleveSerialize(s, opt);
        s.Serialize(block, opt);
        s.Serialize(height, opt);
        s.Serialize(strFork, opt);
        s.Serialize(exception, opt);
    }
};

class CMvRPCRouteGetTxPoolRet: public CMvRPCRouteRet
{
    friend class walleve::CWalleveStream;
public:
    std::vector<std::pair<std::string, size_t>> vTxPool;
    int exception; // 0-nomal, 1-invalid block, 2-unknown fork

protected:
    template<typename O>
    void WalleveSerialize(walleve::CWalleveStream& s, O& opt)
    {
        CMvRPCRouteRet::WalleveSerialize(s, opt);
        s.Serialize(vTxPool, opt);
        s.Serialize(exception, opt);
    }
};

class CMvRPCRouteGetTransactionRet: public CMvRPCRouteRet
{
    friend class walleve::CWalleveStream;
public:
    CBlock block;
    int height;
    std::string strFork;
    int exception; // 0-nomal, 1-unknown block

protected:
    template<typename O>
    void WalleveSerialize(walleve::CWalleveStream& s, O& opt)
    {
        CMvRPCRouteRet::WalleveSerialize(s, opt);
        s.Serialize(block, opt);
        s.Serialize(height, opt);
        s.Serialize(strFork, opt);
        s.Serialize(exception, opt);
    }
};

class CMvRPCRouteGetForkHeightRet: public CMvRPCRouteRet
{
    friend class walleve::CWalleveStream;
public:
    CBlock block;
    int height;
    std::string strFork;
    int exception; // 0-nomal, 1-unknown block

protected:
    template<typename O>
    void WalleveSerialize(walleve::CWalleveStream& s, O& opt)
    {
        CMvRPCRouteRet::WalleveSerialize(s, opt);
        s.Serialize(block, opt);
        s.Serialize(height, opt);
        s.Serialize(strFork, opt);
        s.Serialize(exception, opt);
    }
};

class CMvRPCRouteSendTransactionRet : public CMvRPCRouteRet
{
    friend class walleve::CWalleveStream;
public:
    CBlock block;
    int height;
    std::string strFork;
    int exception; // 0-nomal, 1-unknown block

protected:
    template<typename O>
    void WalleveSerialize(walleve::CWalleveStream& s, O& opt)
    {
        CMvRPCRouteRet::WalleveSerialize(s, opt);
        s.Serialize(block, opt);
        s.Serialize(height, opt);
        s.Serialize(strFork, opt);
        s.Serialize(exception, opt);
    }
};

//

class CMvDbpTxIn
{
public:
    std::vector<uint8> hash;
    uint32 n;
};

class CMvDbpDestination
{
public:
    enum PREFIX
    {
        PREFIX_NULL = 0,
        PREFIX_PUBKEY = 1,
        PREFIX_TEMPLATE = 2,
        PREFIX_MAX = 3
    };

public:
    uint32 prefix;
    std::vector<uint8> data;
    uint32 size; //设置为33
};

class CMvDbpTransaction
{
public:
    uint32 nVersion;               //版本号,目前交易版本为 0x0001
    uint32 nType;                  //类型, 区分公钥地址交易、模板地址交易、即时业务交易和跨分支交易
    uint32 nLockUntil;             //交易冻结至高度为 nLockUntil 区块
    std::vector<uint8> hashAnchor; //交易有效起始区块 HASH
    std::vector<CMvDbpTxIn> vInput;
    CMvDbpDestination cDestination; // 输出地址
    int64 nAmount;                  //输出金额
    int64 nTxFee;                   //网络交易费
    int64 nChange;                  //余额
    std::vector<uint8> vchData;     //输出参数(模板地址参数、跨分支交易共轭交易)
    std::vector<uint8> vchSig;      //交易签名
    std::vector<uint8> hash;        // 当前交易的 hash
    std::vector<uint8> fork;        // 当前交易的 forkid
};

class CMvDbpBlock
{
public:
    uint32 nVersion;
    uint32 nType;                       // 类型,区分创世纪块、主链区块、业务区块和业务子区块
    uint32 nTimeStamp;                  //时间戳，采用UTC秒单位
    std::vector<uint8> hashPrev;        //前一区块的hash
    std::vector<uint8> hashMerkle;      //Merkle tree的根
    std::vector<uint8> vchProof;        //用于校验共识合法性数据
    CMvDbpTransaction txMint;           // 出块奖励交易
    std::vector<CMvDbpTransaction> vtx; //区块打包的所有交易
    std::vector<uint8> vchSig;          //区块签名
    uint32 nHeight;                     // 当前区块高度
    std::vector<uint8> hash;            //当前区块的hash
    std::vector<uint8> fork;            // 当前区块的forkid
};

class CMvDbpAdded : public CMvDbpRespond
{
public:
    std::string name;
    std::string id;
    std::string forkid;
    boost::any anyAddedObj; // busniess object (block,tx...)
};

class CMvDbpMethod : public CMvDbpRequest
{
public: 

    enum SnMethod : uint32_t
    {
        SEND_EVENT = 0x03,
        RPC_ROUTE = 0x04
    };
    
    enum  LwsMethod : uint32_t
    {
        GET_BLOCKS = 0x00,
        GET_TRANSACTION = 0x01,
        SEND_TRANSACTION = 0x02
    };

    // param name => param value
    typedef std::map<std::string, boost::any> ParamMap;

public:
    uint32_t method;
    std::string id;
    ParamMap params;
};

class CMvDbpSendTransactionRet
{
public:
    std::string hash;
    std::string result;
    std::string reason;
};

class CMvDbpMethodResult : public CMvDbpRespond
{
public:
    std::string id;
    std::string error;
    std::vector<boost::any> anyResultObjs; // blocks,tx,send_tx_ret
};

class CMvDbpError : public CMvDbpRespond
{
public:
};

class CMvDbpConnected : public CMvDbpRespond
{
public:
    std::string session;
};

class CMvDbpPing : public CMvDbpRequest, public CMvDbpRespond
{
public:
    std::string id;
};

class CMvDbpPong : public CMvDbpRequest, public CMvDbpRespond
{
public:
    std::string id;
};

class CMvDbpFailed : public CMvDbpRespond
{
public:
    std::string reason;  // failed reason
    std::string session; // for delete session map
    std::vector<int32> versions;
};

class CMvDbpBroken
{
public:
    std::string session;
};

class CMvDbpRemoveSession
{
public:
    std::string session;
};
} // namespace multiverse
#endif //MULTIVERSE_DBP_TYPE_H
