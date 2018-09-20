// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_MODE_MODULE_TYPE_H
#define MULTIVERSE_MODE_MODULE_TYPE_H

namespace multiverse
{
// module type
enum class EModuleType
{
    LOCK,          // lock file
    BLOCKMAKER,    // CBlockMaker
    COREPROTOCOL,  // CMvCoreProtocol
    DISPATCHER,    // CDispatcher
    HTTPGET,       // CHttpGet
    HTTPSERVER,    // CHttpServer
    MINER,         // CMiner
    NETCHANNEL,    // CNetChannel
    NETWORK,       // CNetwork
    RPCDISPATCH,   // CRPCDispatch
    RPCMODE,       // CRPCMod
    SERVICE,       // CService
    TXPOOL,        // CTxPool
    WALLET,        // CWallet
    WORLDLINE,     // CWorldLine
    CONSENSUS,     // CConsensus
    DBPSOCKET,     // CDummyDbpSocket
};

}  // namespace multiverse

#endif  // MULTIVERSE_MODE_MODULE_TYPE_H