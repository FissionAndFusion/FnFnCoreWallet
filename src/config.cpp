// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "config.h"
#include "address.h"

using namespace std;
using namespace boost::filesystem;
using boost::asio::ip::tcp;
using namespace walleve;
using namespace multiverse;

#define MAINNET_MAGICNUM                0xd5f97d23
#define TESTNET_MAGICNUM                0xef93a7b2
#define DEFAULT_DB_CONNECTION		8

#define DEFAULT_P2PPORT                 6811
#define DEFAULT_TESTNET_P2PPORT         6813
#define DEFAULT_RPCPORT                 6812
#define DEFAULT_TESTNET_RPCPORT         6814

#define DEFAULT_DNSEED_PORT             6816
#define DEFAULT_MAX_INBOUNDS            125
#define DEFAULT_MAX_OUTBOUNDS           10
#define DEFAULT_CONNECT_TIMEOUT         5
#define DNSEED__DEFAULT_MAX_TIMES_CONNECT_FAIL 10

#define DEFAULT_RPC_MAX_CONNECTIONS     5
#define DEFAULT_RPC_CONNECT_TIMEOUT     120

namespace po = boost::program_options;

#define OPTBOOL(name,var,def)           (name,po::value<bool>(&(var))->default_value(def))
#define OPTUINT(name,var,def)           (name,po::value<unsigned int>(&(var))->default_value(def))
#define OPTINT(name,var,def)            (name,po::value<int>(&(var))->default_value(def))
#define OPTSTR(name,var,def)            (name,po::value<string>(&(var))->default_value(def))
#define OPTVEC(name,var)                (name,po::value<vector<string> >(&(var)))

//////////////////////////////
// CMvBasicConfig

CMvBasicConfig::CMvBasicConfig()
{
    po::options_description desc("MvBasic");

    desc.add_options()

    OPTBOOL("testnet",fTestNet,false);

    AddOptions(desc);
}

CMvBasicConfig::~CMvBasicConfig()
{
}

bool CMvBasicConfig::PostLoad()
{
    if (fTestNet)
    {   
        pathData /= "testnet";
    }
    nMagicNum = fTestNet ? TESTNET_MAGICNUM : MAINNET_MAGICNUM;
    
    return true;
}

string CMvBasicConfig::ListConfig()
{   
    ostringstream oss;
    oss << "TestNet : " << (fTestNet ? "Y" : "N") << "\n";
    return oss.str();
}

//////////////////////////////
// CMvStorageConfig

CMvStorageConfig::CMvStorageConfig()
{
    po::options_description desc("MvStorage");

    desc.add_options()

    OPTSTR("dbhost",strDBHost,"localhost")
    OPTSTR("dbname",strDBName,"multiverse")
    OPTSTR("dbuser",strDBUser,"multiverse")
    OPTSTR("dbpass",strDBPass,"multiverse")
    OPTINT("dbport",nDBPort,0)
    OPTINT("dbconn",nDBConn,DEFAULT_DB_CONNECTION);

    AddOptions(desc);
}

CMvStorageConfig::~CMvStorageConfig()
{
}

bool CMvStorageConfig::PostLoad()
{
    if (fTestNet)
    {
        strDBName += "-testnet";
    }
    if (nDBPort < 0 || nDBPort > 0xFFFF)
    {
        nDBPort = 0;
    }
    return true;
}

std::string CMvStorageConfig::ListConfig()
{
    return "";
}

//////////////////////////////
// CMvNetworkConfig

CMvNetworkConfig::CMvNetworkConfig()
{
    po::options_description desc("LoMoNetwork");

    desc.add_options()

    OPTBOOL("listen",fListen,false)
    OPTBOOL("bloom",fBloom,true)
    OPTINT("port",nPortInt,0)
    OPTINT("dnseedport",nDNSeedPort,DEFAULT_DNSEED_PORT)
    OPTINT("maxconnections",nMaxConnection,DEFAULT_MAX_OUTBOUNDS + DEFAULT_MAX_INBOUNDS)
    OPTUINT("timeout",nConnectTimeout,DEFAULT_CONNECT_TIMEOUT)
    OPTVEC("addnode",vNode)
    OPTVEC("connect",vConnectTo);
    OPTUINT("dnseedmaxtimes",nMaxTimes2ConnectFail,DNSEED__DEFAULT_MAX_TIMES_CONNECT_FAIL);

    AddOptions(desc);
}
CMvNetworkConfig::~CMvNetworkConfig()
{
}

bool CMvNetworkConfig::PostLoad()
{
    if (nPortInt <= 0 || nPortInt > 0xFFFF)
    {
        nPort = (fTestNet ? DEFAULT_TESTNET_P2PPORT : DEFAULT_P2PPORT);
    }
    else
    {
        nPort = (unsigned short)nPortInt;
    }
    nMaxOutBounds = DEFAULT_MAX_OUTBOUNDS;
    if (nMaxConnection <= DEFAULT_MAX_OUTBOUNDS)
    {
        nMaxInBounds = fListen ? 1 : 0;
    }
    else
    {
        nMaxInBounds = nMaxConnection - DEFAULT_MAX_OUTBOUNDS;
    }

    if (nConnectTimeout == 0)
    {
        nConnectTimeout = 1;
    }

    if (!fTestNet)
    {
        vDNSeed.push_back("174.137.61.150");
//        vDNSeed.push_back("123.57.22.233");
    }
    return true;
}

string CMvNetworkConfig::ListConfig()
{
    return "";
}

//////////////////////////////
// CMvRPCConfig

CMvRPCConfig::CMvRPCConfig()
{
    po::options_description desc("LoMoRPC");

    desc.add_options()

    OPTSTR("rpcconnect",strRPCConnect,"127.0.0.1")
    OPTINT("rpcport",nRPCPortInt,0)
    OPTUINT("rpctimeout",nRPCConnectTimeout,DEFAULT_RPC_CONNECT_TIMEOUT)
    OPTUINT("rpcmaxconnections",nRPCMaxConnections,DEFAULT_RPC_MAX_CONNECTIONS)
    OPTVEC("rpcallowip",vRPCAllowIP)

    OPTSTR("rpcwallet",strRPCWallet,"")
    OPTSTR("rpcuser",strRPCUser,"")
    OPTSTR("rpcpassword",strRPCPass,"")

    OPTBOOL("rpcssl",fRPCSSLEnable,false)
    OPTBOOL("rpcsslverify",fRPCSSLVerify,true)
    OPTSTR("rpcsslcafile",strRPCCAFile,"ca.crt")
    OPTSTR("rpcsslcertificatechainfile",strRPCCertFile,"server.crt")
    OPTSTR("rpcsslprivatekeyfile",strRPCPKFile,"server.key")
    OPTSTR("rpcsslciphers",strRPCCiphers,"TLSv1+HIGH:!SSLv2:!aNULL:!eNULL:!AH:!3DES:@STRENGTH");

    AddOptions(desc);
}

CMvRPCConfig::~CMvRPCConfig()
{
}

bool CMvRPCConfig::PostLoad()
{
    if (nRPCPortInt <= 0 || nRPCPortInt > 0xFFFF)
    {
        nRPCPort = (fTestNet ? DEFAULT_TESTNET_RPCPORT : DEFAULT_RPCPORT);
    }
    else
    {
        nRPCPort = (unsigned short)nRPCPortInt;
    }

    if (nRPCConnectTimeout == 0)
    {
        nRPCConnectTimeout = 1;
    }

    epRPC = tcp::endpoint(!vRPCAllowIP.empty()
                              ? boost::asio::ip::address_v4::any()
                                : boost::asio::ip::address_v4::loopback(),
                          nRPCPort);

    if (!path(strRPCCAFile).is_complete())
    {
        strRPCCAFile = (pathRoot / strRPCCAFile).string();
    }

    if (!path(strRPCCertFile).is_complete())
    {
        strRPCCertFile = (pathRoot / strRPCCertFile).string();
    }

    if (!path(strRPCPKFile).is_complete())
    {
        strRPCPKFile = (pathRoot / strRPCPKFile).string();
    }

    return true;
}

string CMvRPCConfig::ListConfig()
{
    return "";
}

//////////////////////////////
// CMvMintConfig

CMvMintConfig::CMvMintConfig()
{
    po::options_description desc("LoMoMint");

    desc.add_options()

    OPTSTR("mpvssaddress",strAddressMPVss,"")
    OPTSTR("mpvsskey",strKeyMPVss,"")
    OPTSTR("blake512address",strAddressBlake512,"")
    OPTSTR("blake512key",strKeyBlake512,"");

    AddOptions(desc);
}

CMvMintConfig::~CMvMintConfig()
{
}

bool CMvMintConfig::PostLoad()
{
    ExtractMintParamPair(strAddressMPVss,strKeyMPVss,destMPVss,keyMPVss);
    ExtractMintParamPair(strAddressBlake512,strKeyBlake512,destBlake512,keyBlake512);
    return true;
}

string CMvMintConfig::ListConfig()
{
    return "";
}

void CMvMintConfig::ExtractMintParamPair(const std::string& strAddress,const std::string& strKey,
                                         CDestination& dest,uint256& privkey)
{
    CMvAddress address;
    if (address.ParseString(strAddress) && !address.IsNull() && strKey.size() == 64)
    {
        dest = address;
        privkey.SetHex(strKey);
    }
}

//////////////////////////////
// CMvConfig

CMvConfig::CMvConfig()
{
}

CMvConfig::~CMvConfig()
{
}

bool CMvConfig::PostLoad()
{
    return (CWalleveConfig::PostLoad()
            && CMvBasicConfig::PostLoad()
            && CMvNetworkConfig::PostLoad()
            && CMvRPCConfig::PostLoad()
            && CMvMintConfig::PostLoad());
}

string CMvConfig::ListConfig()
{
    return (CWalleveConfig::ListConfig()
            + CMvBasicConfig::ListConfig()
            + CMvNetworkConfig::ListConfig()
            + CMvRPCConfig::ListConfig()
            + CMvMintConfig::ListConfig());
}
