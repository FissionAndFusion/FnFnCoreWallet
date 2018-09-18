// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef DBP_TYPE_H
#define DBP_TYPE_H

#include <boost/any.hpp>

namespace multiverse{

class CWalleveDbpContent
{
public:
};

class CWalleveDbpRequest: public CWalleveDbpContent
{
public:

};

class CWalleveDbpRespond: public CWalleveDbpContent
{
public:

};

class CWalleveDbpConnect: public CWalleveDbpRequest
{
public:
    bool isReconnect;
    std::string session;
    int32 version;
    std::string client;
};

class CWalleveDbpSub: public CWalleveDbpRequest
{
public:
    std::string id;
    std::string name;
};

class CWalleveDbpUnSub: public CWalleveDbpRequest
{
public:
    std::string id;
};

class CWalleveDbpNoSub: public CWalleveDbpRespond
{
public:
    std::string id;
    std::string error;
};

class CWalleveDbpReady: public CWalleveDbpRespond
{
public:
    std::string id;    
};

class  CWalleveDbpTxIn
{
public:
    std::vector<uint8> hash;
    uint32 n;
};

class  CWalleveDbpDestination
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

class CWalleveDbpTransaction
{
public:
    uint32 nVersion; //版本号,目前交易版本为 0x0001
    uint32 nType; //类型, 区分公钥地址交易、模板地址交易、即时业务交易和跨分支交易
    uint32 nLockUntil; //交易冻结至高度为 nLockUntil 区块
    std::vector<uint8> hashAnchor; //交易有效起始区块 HASH
    std::vector<CWalleveDbpTxIn> vInput;
    CWalleveDbpDestination cDestination; // 输出地址
    int64 nAmount; //输出金额
    int64 nTxFee; //网络交易费 
    std::vector<uint8> vchData; //输出参数(模板地址参数、跨分支交易共轭交易)
    std::vector<uint8> vchSig; //交易签名
    std::vector<uint8> hash;
};

class CWalleveDbpBlock
{
public:
    uint32 nVersion;
    uint32 nType; // 类型,区分创世纪块、主链区块、业务区块和业务子区块
    uint32 nTimeStamp; //时间戳，采用UTC秒单位
    std::vector<uint8> hashPrev; //前一区块的hash
    std::vector<uint8> hashMerkle; //Merkle tree的根
    std::vector<uint8> vchProof;  //用于校验共识合法性数据
    CWalleveDbpTransaction txMint; // 出块奖励交易
    std::vector<CWalleveDbpTransaction> vtx; //区块打包的所有交易
    std::vector<uint8> vchSig; //区块签名
    uint32 nHeight; // 当前区块高度
    std::vector<uint8> hash; //当前区块的hash
};

class CWalleveDbpAdded: public CWalleveDbpRespond
{
public:
    std::string name;
    std::string id;
    boost::any anyAddedObj; // busniess object (block,tx...)
};

class CWalleveDbpMethod: public CWalleveDbpRequest
{
public:
    enum Method{
        GET_BLOCKS,
        GET_TX,
        SEND_TX
    };
    
    // param name => param value
    typedef std::map<std::string,std::string> ParamMap;
public:
    Method method;
    std::string id;
    ParamMap params;
};

class CWalleveDbpSendTxRet
{
public:
    std::string hash;
    std::string result;
    std::string reason;
};

class CWalleveDbpMethodResult : public CWalleveDbpRespond
{
public:
    std::string id;
    std::string error;
    std::vector<boost::any> anyResultObjs; // blocks,tx,send_tx_ret
};

class CWalleveDbpError: public CWalleveDbpRespond
{
public:

};

class CWalleveDbpConnected: public CWalleveDbpRespond
{
public:
    std::string session;
};

class CWalleveDbpPing : public CWalleveDbpRequest, public CWalleveDbpRespond
{
public:
    std::string id;
};

class CWalleveDbpPong : public CWalleveDbpRequest, public CWalleveDbpRespond
{
public:
    std::string id;
};

class CWalleveDbpFailed: public CWalleveDbpRespond
{
public:
    std::string reason; // failed reason
    std::string session; // for delete session map
    std::vector<int32> versions;
};

class CWalleveDbpBroken
{
public:
    bool fEventStream;
};
} // namespace multiverse
#endif //DBP_TYPE_H