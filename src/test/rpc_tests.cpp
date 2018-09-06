// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "rpcmod.h"
#include "test_fnfn.h"

#include <boost/test/unit_test.hpp>
using namespace boost;

struct RPCSetup
{
    multiverse::CRPCMod rpcmdl;
    RPCSetup()
    {
        rpcmdl.WalleveInitialize();
    }
    ~RPCSetup()
    {
        rpcmdl.WalleveDeinitialize();
    }
    void CallRPCAPI(const std::string& params)
    {
        walleve::CWalleveEventHttpReq eventHttpReq;
        try
        {
            bool ret = rpcmdl.HandleEvent(eventHttpReq);
        }
        catch(...)
        {
            throw std::runtime_error("error occured!");
        }
    }
};

BOOST_FIXTURE_TEST_SUITE(rpc_tests, RPCSetup)

BOOST_AUTO_TEST_CASE(rpc_listkey)
{
    BOOST_CHECK_THROW(CallRPCAPI("listkey xxx"), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(rpc_getblock)
{
    BOOST_CHECK_THROW(CallRPCAPI("getblock"), std::runtime_error);
}

BOOST_AUTO_TEST_SUITE_END()

