// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_CONFIG_H
#define  MULTIVERSE_CONFIG_H

#include <walleve/walleve.h>

namespace multiverse
{

class CMvBasicConfig : virtual public walleve::CWalleveConfig
{
public:
    CMvBasicConfig();
    virtual ~CMvBasicConfig();
    virtual bool PostLoad();
    virtual std::string ListConfig();
public:
    unsigned int nMagicNum;
    bool fTestNet;
    
};

class CMvStorageConfig : virtual public CMvBasicConfig
{
public:
    CMvStorageConfig();
    virtual ~CMvStorageConfig();
    virtual bool PostLoad();
    virtual std::string ListConfig();
public:
    std::string strDBHost;
    std::string strDBName;
    std::string strDBUser;
    std::string strDBPass;
    int nDBPort;
    int nDBConn;
};

class CMvNetworkConfig : virtual public CMvBasicConfig
{
public:
    CMvNetworkConfig();
    virtual ~CMvNetworkConfig();
    virtual bool PostLoad();
    virtual std::string ListConfig();
public:
    bool fListen;
    bool fBloom;    
    unsigned short nPort;
    unsigned int nMaxInBounds;
    unsigned int nMaxOutBounds;
    unsigned int nConnectTimeout;
    std::vector<std::string> vNode;
    std::vector<std::string> vConnectTo;
    std::vector<std::string> vDNSeed;
protected:
    int nPortInt;
    int nMaxConnection;
};

class CMvRPCConfig : virtual public CMvBasicConfig
{
public:
    CMvRPCConfig();
    virtual ~CMvRPCConfig();
    virtual bool PostLoad();
    virtual std::string ListConfig();
public:
    boost::asio::ip::tcp::endpoint epRPC;
    std::string strRPCConnect;
    unsigned int nRPCPort;
    unsigned int nRPCConnectTimeout;
    unsigned int nRPCMaxConnections;
    std::vector<std::string> vRPCAllowIP;
    std::string strRPCWallet;
    std::string strRPCUser;
    std::string strRPCPass;
    bool fRPCSSLEnable;
    bool fRPCSSLVerify;
    std::string strRPCCAFile;
    std::string strRPCCertFile;
    std::string strRPCPKFile;
    std::string strRPCCiphers;
protected:
    int nRPCPortInt;
};

class CMvConfig : virtual public CMvStorageConfig,
                  virtual public CMvNetworkConfig,
                  virtual public CMvRPCConfig
{
public:
    CMvConfig();
    virtual ~CMvConfig();
    virtual bool PostLoad();
    virtual std::string ListConfig();
};

} // namespace multiverse

#endif //MULTIVERSE_CONFIG_H
