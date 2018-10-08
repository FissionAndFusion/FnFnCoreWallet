// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_RPCCLIENT_H
#define MULTIVERSE_RPCCLIENT_H

#include <string>
#include <vector>

#include "mvbase.h"
#include "walleve/walleve.h"
#include "json/json_spirit_value.h"
#include "rpc/rpc.h"

namespace multiverse
{

class CRPCDispatch : public walleve::IIOModule, virtual public walleve::CWalleveHttpEventListener
{
  public:
    CRPCDispatch();
    CRPCDispatch(const std::vector<std::string> &vArgsIn);

    ~CRPCDispatch();

  protected:
    bool WalleveHandleInitialize() override;
    void WalleveHandleDeinitialize() override;
    bool WalleveHandleInvoke() override;
    void WalleveHandleHalt() override;
    const CMvRPCClientConfig *WalleveConfig();

    bool HandleEvent(walleve::CWalleveEventHttpGetRsp &event) override;
    bool GetResponse(uint64 nNonce, const std::string &content);
    bool CallRPC(rpc::CRPCParamPtr spParam, int nReqId);
    bool CallConsoleCommand(const std::vector<std::string> &vCommand);
    void LaunchConsole();
    void LaunchCommand();
    void CancelCommand();
    json_spirit::Value GetParamValue(const std::string &strParam);

  protected:
    walleve::IIOProc *pHttpGet;
    walleve::CWalleveThread thrDispatch;
    std::vector<std::string> vArgs;
    uint64 nLastNonce;
    walleve::CIOCompletion ioComplt;
};

} // namespace multiverse
#endif //MULTIVERSE_RPCCLIENT_H
