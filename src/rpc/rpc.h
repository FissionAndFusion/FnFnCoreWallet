// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_RPC_RPC_H
#define MULTIVERSE_RPC_RPC_H

#include <vector>

#include "rpc/rpc_error.h"
#include "rpc/rpc_req.h"
#include "rpc/rpc_resp.h"

#include "rpc/auto_rpc.h"

namespace multiverse
{

typedef rpc::CRPCError CRPCError;
typedef rpc::CRPCException CRPCException;

typedef rpc::CRPCParam CRPCParam;
typedef rpc::CRPCParamPtr CRPCParamPtr;

typedef rpc::CRPCReq CRPCReq;
typedef rpc::CRPCReqPtr CRPCReqPtr;
typedef rpc::CRPCReqVec CRPCReqVec;
typedef rpc::CRPCReqMap CRPCReqMap;

typedef rpc::CRPCResult CRPCResult;
typedef rpc::CRPCResultPtr CRPCResultPtr;

typedef rpc::CRPCResp CRPCResp;
typedef rpc::CRPCRespPtr CRPCRespPtr;
typedef rpc::CRPCRespVec CRPCRespVec;

}  // namespace multiverse

#endif  // MULTIVERSE_RPC_RPC_H