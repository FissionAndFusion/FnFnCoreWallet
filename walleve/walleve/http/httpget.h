// Copyright (c) 2016-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  WALLEVE_HTTPGET_H
#define  WALLEVE_HTTPGET_H

#include "walleve/netio/ioproc.h"
#include "walleve/http/httpevent.h"

namespace walleve
{

enum
{
    HTTPGET_OK                =  0,
    HTTPGET_CONNECT_FAILED    = -1,
    HTTPGET_INVALID_NONCE     = -2,
    HTTPGET_ACTIVATE_FAILED   = -3,
    HTTPGET_INTERRUPTED       = -4,
    HTTPGET_RESP_TIMEOUT      = -5,
    HTTPGET_RESOLVE_FAILED    = -6,
    HTTPGET_INTERNAL_FAILURE  = -7,
    HTTPGET_ABORTED           = -8
};

class CHttpGet;

class CHttpGetClient
{
public:
    CHttpGetClient(const std::string& strIOModuleIn,const uint64 nNonceIn,
                   CHttpGet* pHttpGetIn,CIOClient* pClientIn);
    ~CHttpGetClient();
    const std::string& GetIOModule();
    uint64 GetNonce();
    uint32 GetTimerId();
    CNetHost GetHost();
    bool IsIdle();
    void GetResponse(CWalleveHttpRsp& rsp);
    void Activate(CWalleveHttpGet& httpGet,uint32 nTimerIdIn);
protected:
    void HandleWritenRequest(std::size_t nTransferred);
    void HandleReadHeader(std::size_t nTransferred);
    void HandleReadPayload(std::size_t nTransferred);
    void HandleReadChunked(std::size_t nTransferred);
    void HandleReadCompleted(); 
protected:
    const std::string strIOModule;
    const uint64 nNonce;
    CHttpGet* pHttpGet;
    CIOClient *pClient;
    uint32 nTimerId;
    bool fIdle;
    CWalleveBufStream ssRecv;
    CWalleveBufStream ssSend;
    MAPIKeyValue mapHeader;
    MAPCookie mapCookie;
    std::string strChunked;
};

class CHttpGet : public CIOProc, virtual public CWalleveHttpEventListener
{
public:
    CHttpGet();
    virtual ~CHttpGet();
    void HandleClientCompleted(CHttpGetClient *pGetClient);
    void HandleClientError(CHttpGetClient *pGetClient);    
protected:
    void LeaveLoop() override;
    void HostResolved(const CNetHost& host,const boost::asio::ip::tcp::endpoint& ep) override;
    void HostFailToResolve(const CNetHost& host) override;
    bool ClientConnected(CIOClient* pClient) override;
    void ClientFailToConnect(const boost::asio::ip::tcp::endpoint& epRemote) override;
    void Timeout(uint64 nNonce,uint32 nTimerId) override;
    int  ActivateConn(CIOClient *pClient,CWalleveEventHttpGet& eventGet);
    bool PostResponse(const std::string& strIOModule,CWalleveEventHttpGetRsp *pEventResp);
    void PostError(const CWalleveEventHttpGet& eventGet,int nErrCode);
    void PostError(const std::string& strIOModule,uint64 nNonce,int nErrCode);
    bool HandleEvent(CWalleveEventHttpGet& eventGet) override;
    bool HandleEvent(CWalleveEventHttpAbort& eventAbort) override;
    void CloseConn(CHttpGetClient *pGetClient,int nErrCode = HTTPGET_OK);
    CIOSSLOption GetSSLOption(CWalleveHttpGet& httpGet,const std::string& strHost);
protected:
    std::multimap<CNetHost,CWalleveEventHttpGet> mapRequest;
    std::multimap<uint64,CHttpGetClient*> mapGetClient;
};

} // namespace walleve
#endif //WALLEVE_HTTPGET_H

