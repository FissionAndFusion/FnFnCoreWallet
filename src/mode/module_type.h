// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_MODULE_TYPE_H
#define MULTIVERSE_MODULE_TYPE_H

namespace multiverse
{
// module type
enum class EModuleType
{
    LOCK,                // lock file
    BLOCKMAKER,          // CBlockMaker
    COREPROTOCOL,        // CMvCoreProtocol
    DISPATCHER,          // CDispatcher
    HTTPGET,             // CHttpGet
    HTTPSERVER,          // CHttpServer
    MINER,               // CMiner
    NETCHANNEL,          // CNetChannel
    DELEGATEDCHANNEL,    // CDelegatedChannel
    NETWORK,             // CNetwork
    RPCCLIENT,           // CRPCClient
    RPCMODE,             // CRPCMod
    SERVICE,             // CService
    TXPOOL,              // CTxPool
    WALLET,              // CWallet
    WORLDLINE,           // CWorldLine
    CONSENSUS,           // CConsensus
    FORKMANAGER,         // CForkManager
    DBPCLIENT,           // CMvDbpClient
    DBPSERVER,           // CDbpServer
    DBPSERVICE,          // CDbpService
    DNSEED,              // CDNSeed
};

} // namespace multiverse

#endif // MULTIVERSE_MODULE_TYPE_H