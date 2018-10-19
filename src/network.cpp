// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "network.h"
#include "version.h"
#include <boost/bind.hpp>
#include <boost/any.hpp>

using namespace std;
using namespace walleve;
using namespace multiverse;
using boost::asio::ip::tcp;

//////////////////////////////
// CNetwork 

CNetwork::CNetwork()
{
}

CNetwork::~CNetwork()
{
}

bool CNetwork::WalleveHandleInitialize()
{
    Configure(NetworkConfig()->nMagicNum,PROTO_VERSION,network::NODE_NETWORK | network::NODE_DELEGATED,
              FormatSubVersion(),!NetworkConfig()->vConnectTo.empty());

    CPeerNetConfig config;
    if (NetworkConfig()->fListen)
    {
        config.vecService.push_back(CPeerService(tcp::endpoint(tcp::v4(), NetworkConfig()->nPort),
                                                 NetworkConfig()->nMaxInBounds));
    }
    config.nMaxOutBounds = NetworkConfig()->nMaxOutBounds;
    config.nPortDefault = NetworkConfig()->nPort;
    BOOST_FOREACH(const string& conn,NetworkConfig()->vConnectTo)
    {
        config.vecNode.push_back(CNetHost(conn,config.nPortDefault,conn,
                                          boost::any(uint64(network::NODE_NETWORK))));
    }
    if (config.vecNode.empty())
    {
        BOOST_FOREACH(const string& seed,NetworkConfig()->vDNSeed)
        {
            // HACK: dnseed port is different from peer port
            //       dnseed port should be hardcode rather than in configuration
            config.vecNode.push_back(CNetHost(seed,config.nPortDefault,"dnseed",
                                              boost::any(uint64(network::NODE_NETWORK))));
        }
        BOOST_FOREACH(const string& node,NetworkConfig()->vNode)
        {
            config.vecNode.push_back(CNetHost(node,config.nPortDefault,node,
                                              boost::any(uint64(network::NODE_NETWORK))));
        }
    }
    ConfigNetwork(config);

    return network::CMvPeerNet::WalleveHandleInitialize();
}

void CNetwork::WalleveHandleDeinitialize()
{
    network::CMvPeerNet::WalleveHandleDeinitialize();
}

bool CNetwork::CheckPeerVersion(uint32 nVersionIn,uint64 nServiceIn,const string& subVersionIn)
{
    (void)subVersionIn;
    if (nVersionIn < MIN_PROTO_VERSION || (nServiceIn & network::NODE_NETWORK) == 0)
    {
        return false;
    }
    return true;
}
