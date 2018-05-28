// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <stdio.h>
#include <string>
#include <algorithm>

using namespace std;

static std::string _(const char* psz)
{
    return psz;
}

void DisplayUsage()
{
    string strUsage = string() +
//          _("LoMoCoin version") + " " + FormatFullVersion() + "\n\n" +
          _("Usage:") + "\t\t\t\t\t\t\t\t\t\t\n" +
            "  multiverse [options]                   \t  " + "\n" +
            "  multiverse [options] <command> [params]\t  " + _("Send command to -server or multiverse") + "\n" +
            "  multiverse [options] help              \t\t  " + _("List commands") + "\n" +
            "  multiverse [options] help <command>    \t\t  " + _("Get help for a command") + "\n" +
          _("Options:") + "\n" +
            "  -conf=<file>     \t\t  " + _("Specify configuration file (default: lomocoin.conf)") + "\n" +
            "  -pid=<file>      \t\t  " + _("Specify pid file (default: multiverse.pid)") + "\n" +
            "  -gen             \t\t  " + _("Generate coins(POW)") + "\n" +
            "  -gen=0           \t\t  " + _("Don't generate coins(POW)") + "\n" +
            "  -stake           \t\t  " + _("Generate coins(POS)") + "\n" +
            "  -stake=0         \t\t  " + _("Don't generate coins(POS)") + "\n" +
            "  -min             \t\t  " + _("Start minimized") + "\n" +
            "  -splash          \t\t  " + _("Show splash screen on startup (default: 1)") + "\n" +
            "  -datadir=<dir>   \t\t  " + _("Specify data directory") + "\n" +
            "  -dbcache=<n>     \t\t  " + _("Set database cache size in megabytes (default: 25)") + "\n" +
            "  -dblogsize=<n>   \t\t  " + _("Set database disk log size in megabytes (default: 100)") + "\n" +
            "  -timeout=<n>     \t  "   + _("Specify connection timeout (in milliseconds)") + "\n" +
            "  -proxy=<ip:port> \t  "   + _("Connect through socks4 proxy") + "\n" +
            "  -dns             \t  "   + _("Allow DNS lookups for addnode and connect") + "\n" +
            "  -port=<port>     \t\t  " + _("Listen for connections on <port> (default: 6801 or testnet: 6803)") + "\n" +
            "  -maxconnections=<n>\t  " + _("Maintain at most <n> connections to peers (default: 125)") + "\n" +
            "  -addnode=<ip>    \t  "   + _("Add a node to connect to and attempt to keep the connection open") + "\n" +
            "  -connect=<ip>    \t\t  " + _("Connect only to the specified node") + "\n" +
            "  -listen          \t  "   + _("Accept connections from outside (default: 1)") + "\n" +
#ifdef QT_GUI
            "  -lang=<lang>     \t\t  " + _("Set language, for example \"de_DE\" (default: system locale)") + "\n" +
#endif
            "  -dnsseed         \t  "   + _("Find peers using DNS lookup (default: 1)") + "\n" +
            "  -banscore=<n>    \t  "   + _("Threshold for disconnecting misbehaving peers (default: 100)") + "\n" +
            "  -bantime=<n>     \t  "   + _("Number of seconds to keep misbehaving peers from reconnecting (default: 86400)") + "\n" +
            "  -maxreceivebuffer=<n>\t  " + _("Maximum per-connection receive buffer, <n>*1000 bytes (default: 10000)") + "\n" +
            "  -maxsendbuffer=<n>\t  "   + _("Maximum per-connection send buffer, <n>*1000 bytes (default: 10000)") + "\n" +
#ifdef USE_UPNP
#if USE_UPNP
            "  -upnp            \t  "   + _("Use Universal Plug and Play to map the listening port (default: 1)") + "\n" +
#else
            "  -upnp            \t  "   + _("Use Universal Plug and Play to map the listening port (default: 0)") + "\n" +
#endif
            "  -detachdb        \t  "   + _("Detach block and address databases. Increases shutdown time (default: 0)") + "\n" +
#endif
            "  -paytxfee=<amt>  \t  "   + _("Fee per KB to add to transactions you send") + "\n" +
#ifdef QT_GUI
            "  -server          \t\t  " + _("Accept command line and JSON-RPC commands") + "\n" +
#endif
#if !defined(WIN32) && !defined(QT_GUI)
            "  -daemon          \t\t  " + _("Run in the background as a daemon and accept commands") + "\n" +
#endif
            "  -testnet         \t\t  " + _("Use the test network") + "\n" +
            "  -debug           \t\t  " + _("Output extra debugging information") + "\n" +
            "  -logtimestamps   \t  "   + _("Prepend debug output with timestamp") + "\n" +
            "  -printtoconsole  \t  "   + _("Send trace/debug info to console instead of debug.log file") + "\n" +
#ifdef WIN32
            "  -printtodebugger \t  "   + _("Send trace/debug info to debugger") + "\n" +
#endif
            "  -rpcuser=<user>  \t  "   + _("Username for JSON-RPC connections") + "\n" +
            "  -rpcpassword=<pw>\t  "   + _("Password for JSON-RPC connections") + "\n" +
            "  -rpcport=<port>  \t\t  " + _("Listen for JSON-RPC connections on <port> (default: 6802)") + "\n" +
            "  -rpcallowip=<ip> \t\t  " + _("Allow JSON-RPC connections from specified IP address") + "\n" +
            "  -rpcconnect=<ip> \t  "   + _("Send commands to node running on <ip> (default: 127.0.0.1)") + "\n" +
            "  -blocknotify=<cmd> "     + _("Execute command when the best block changes (%s in cmd is replaced by block hash)") + "\n" +
            "  -walletnotify=<cmd> "    + _("Execute command when a wallet transaction changes (%s in cmd is replaced by TxID)") + "\n" +
            "  -upgradewallet   \t  "   + _("Upgrade wallet to latest format") + "\n" +
            "  -keypool=<n>     \t  "   + _("Set key pool size to <n> (default: 100)") + "\n" +
            "  -rescan          \t  "   + _("Rescan the block chain for missing wallet transactions") + "\n" +
            "  -checkblocks=<n> \t\t  " + _("How many blocks to check at startup (default: 2500, 0 = all)") + "\n" +
            "  -checklevel=<n>  \t\t  " + _("How thorough the block verification is (0-6, default: 1)") + "\n";

    strUsage += string() +
            _("\nSSL options: (see the Bitcoin Wiki for SSL setup instructions)") + "\n" +
            "  -rpcssl                                \t  " + _("Use OpenSSL (https) for JSON-RPC connections") + "\n" +
            "  -rpcsslcertificatechainfile=<file.cert>\t  " + _("Server certificate file (default: server.cert)") + "\n" +
            "  -rpcsslprivatekeyfile=<file.pem>       \t  " + _("Server private key (default: server.pem)") + "\n" +
            "  -rpcsslciphers=<ciphers>               \t  " + _("Acceptable ciphers (default: TLSv1+HIGH:!SSLv2:!aNULL:!eNULL:!AH:!3DES:@STRENGTH)") + "\n";

    strUsage += string() +
            "  -?               \t\t  " + _("This help message") + "\n";

    // Remove tabs
    strUsage.erase(std::remove(strUsage.begin(), strUsage.end(), '\t'), strUsage.end());
    fprintf(stderr, "%s", strUsage.c_str());
}


