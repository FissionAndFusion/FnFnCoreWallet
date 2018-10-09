// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_RPCCLIENT_H
#define MULTIVERSE_RPCCLIENT_H

#include <string>
#include <vector>
#include <boost/asio.hpp>

#include "mvbase.h"
#include "walleve/walleve.h"
#include "json/json_spirit_value.h"
#include "rpc/rpc.h"

namespace multiverse
{

class CRPCClient : public walleve::IIOModule, virtual public walleve::CWalleveHttpEventListener
{
public:
    CRPCClient(bool fConsole = true);
    ~CRPCClient();
    void DispatchLine(const std::string& strLine);

protected:
    bool WalleveHandleInitialize() override;
    void WalleveHandleDeinitialize() override;
    bool WalleveHandleInvoke() override;
    void WalleveHandleHalt() override;
    const CMvRPCClientConfig * WalleveConfig();

    bool HandleEvent(walleve::CWalleveEventHttpGetRsp& event) override;
    bool GetResponse(uint64 nNonce, const std::string& content);
    bool CallRPC(rpc::CRPCParamPtr spParam, int nReqId);
    bool CallConsoleCommand(const std::vector<std::string>& vCommand);
    void LaunchConsole();
    void LaunchCommand();
    void CancelCommand();

    void WaitForChars();
#if BOOST_VERSION < 106600
    void HandleRead(const boost::system::error_code& err, size_t nTransferred);
#else
    void HandleRead(const boost::system::error_code& err);
#endif
    void EnterLoop();
    void LeaveLoop();
    void ConsoleHandleLine(const std::string& strLine);;

protected:
    walleve::IIOProc *pHttpGet;
    walleve::CWalleveThread thrDispatch;
    std::vector<std::string> vArgs;
    uint64 nLastNonce;
    walleve::CIOCompletion ioComplt;
    boost::asio::io_service ioService;
    boost::asio::io_service::strand ioStrand;
    stream_desc inStream;
#if BOOST_VERSION < 106600
    boost::asio::null_buffers bufRead;
#endif
};

} // namespace multiverse
#endif //MULTIVERSE_RPCCLIENT_H
