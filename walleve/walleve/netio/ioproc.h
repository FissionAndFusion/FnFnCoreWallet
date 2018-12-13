// Copyright (c) 2016-2017 The LoMoCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  WALLEVE_IOPROC_H
#define  WALLEVE_IOPROC_H

#include "walleve/base/base.h"
#include "walleve/netio/netio.h"
#include "walleve/netio/nethost.h"
#include "walleve/netio/ioclient.h"
#include "walleve/netio/iocontainer.h"
#include <string>
#include <map>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp> 


namespace walleve
{

class CIOTimer
{
public:
    CIOTimer(uint32 nTimerIdIn,uint64 nNonceIn,int64 nExpiryAtIn);
public:
    uint32 nTimerId;
    uint64 nNonce;
    int64 nExpiryAt;
};

class CIOCompletion
{
public:
    CIOCompletion();
    bool WaitForComplete(bool& fResultRet);
    void Completed(bool fResultIn);
    void Abort();
    void Reset();
protected:
    boost::condition_variable cond;
    boost::mutex mutex;
    bool fResult;
    bool fCompleted;
    bool fAborted;
};

class CIOProc : public IIOProc
{
    friend class CIOOutBound;
    friend class CIOSSLOutBound;
    friend class CIOInBound;
public:
    CIOProc(const std::string& walleveOwnKeyIn);
    virtual ~CIOProc();
    boost::asio::io_service& GetIoService();
    boost::asio::io_service::strand& GetIoStrand();
    virtual bool DispatchEvent(CWalleveEvent* pEvent) override;
    virtual CIOClient* CreateIOClient(CIOContainer *pContainer);
protected:
    bool WalleveHandleInvoke() override;
    void WalleveHandleHalt() override;
   
    uint32 SetTimer(uint64 nNonce,int64 nElapse);
    void CancelTimer(uint32 nTimerId);
    void CancelClientTimers(uint64 nNonce);

    bool StartService(const boost::asio::ip::tcp::endpoint& epLocal,size_t nMaxConnections,
                      const std::vector<std::string>& vAllowMask = std::vector<std::string>());
    void StopService(const boost::asio::ip::tcp::endpoint& epLocal);

    bool Connect(const boost::asio::ip::tcp::endpoint& epRemote,int64 nTimeout);
    bool SSLConnect(const boost::asio::ip::tcp::endpoint& epRemote,int64 nTimeout,
                    const CIOSSLOption& optSSL = CIOSSLOption());
    std::size_t GetOutBoundIdleCount();
    void ResolveHost(const CNetHost& host);
    virtual void EnterLoop();
    virtual void LeaveLoop();
    virtual void HeartBeat();
    virtual void Timeout(uint64 nNonce,uint32 nTimerId);
    virtual std::size_t GetMaxOutBoundCount();
    virtual bool ClientAccepted(const boost::asio::ip::tcp::endpoint& epService,CIOClient *pClient);
    virtual bool ClientConnected(CIOClient *pClient);
    virtual void ClientFailToConnect(const boost::asio::ip::tcp::endpoint& epRemote);
    virtual void HostResolved(const CNetHost& host,const boost::asio::ip::tcp::endpoint& ep);
    virtual void HostFailToResolve(const CNetHost& host);
private:
    void IOThreadFunc();
    void IOProcHeartBeat(const boost::system::error_code& err);
    void IOProcPollTimer();
    void IOProcHandleEvent(CWalleveEvent * pEvent,CIOCompletion& compltHandle);
    void IOProcHandleResolved(const CNetHost& host,const boost::system::error_code& err,
                              boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
private:
    CWalleveThread thrIOProc;
    boost::asio::io_service ioService;
    boost::asio::io_service::strand ioStrand;
    boost::asio::ip::tcp::resolver resolverHost;
    std::map<boost::asio::ip::tcp::endpoint,CIOInBound*> mapService;
    CIOOutBound ioOutBound;
    CIOSSLOutBound ioSSLOutBound;

    boost::asio::deadline_timer timerHeartbeat;
    std::map<uint32,CIOTimer> mapTimerById;
    std::multimap<int64,uint32> mapTimerByExpiry;
};

} // namespace walleve

#endif //WALLEVE_IOPROC_H


