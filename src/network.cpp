// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "network.h"
#include "version.h"
#include <boost/bind.hpp>
#include <boost/any.hpp>
#include "dnseedservice.h"

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
    Configure(NetworkConfig()->nMagicNum,PROTO_VERSION,network::NODE_NETWORK,
              FormatSubVersion(),!NetworkConfig()->vConnectTo.empty());

    CPeerNetConfig config;
    if (NetworkConfig()->fListen)
    {
        config.vecService.push_back(CPeerService(tcp::endpoint(tcp::v4(), NetworkConfig()->nPort),
                                                 NetworkConfig()->nMaxInBounds));
    }
    config.nMaxOutBounds = NetworkConfig()->nMaxOutBounds;
    config.nPortDefault = NetworkConfig()->nPort;

    storage::CMvDBConfig dbConfig(StorageConfig()->strDBHost,StorageConfig()->nDBPort
                        ,StorageConfig()->strDBName,StorageConfig()->strDBUser,StorageConfig()->strDBPass);
    network::DNSeedService* dns=network::DNSeedService::getInstance();
    dns->init(dbConfig);
    dns->_maxConnectFailTimes=NetworkConfig()->nMaxTimes2ConnectFail;

    BOOST_FOREACH(const string& conn,NetworkConfig()->vConnectTo)
    {
        config.vecNode.push_back(CNetHost(conn,config.nPortDefault,conn,
                                          boost::any(uint64(network::NODE_NETWORK))));
    }
    if (config.vecNode.empty())
    {
        BOOST_FOREACH(const string& node,NetworkConfig()->vNode)
        {
            config.vecNode.push_back(CNetHost(node,config.nPortDefault,node,
                                              boost::any(uint64(network::NODE_NETWORK))));
        }
    }
    //加载数据库中的节点列表
    std::vector<tcp::endpoint> eplist;
    dns->getLocalConnectAddressList(eplist,NetworkConfig()->nMaxOutBounds);
    BOOST_FOREACH(tcp::endpoint ep,eplist)
    {
        std::string ip=ep.address().to_string();
        config.vecNode.push_back(CNetHost(ip,ep.port(),ip,
                                          boost::any(uint64(network::NODE_NETWORK))));
    }
    if(config.vecNode.empty())
    {
        BOOST_FOREACH(const string& seed,NetworkConfig()->vDNSeed)
        {
            config.vecNode.push_back(CNetHost(seed,NetworkConfig()->nPort,"dnseed",
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

void CNetwork::ClientFailToConnect(const tcp::endpoint& epRemote)
{
    CPeerNet::ClientFailToConnect(epRemote);
    WalleveLog("ConnectFailTo>>>%s %d\n",epRemote.address().to_string().c_str(),epRemote.port());
    /*In order to make the disconnected nodes return to the network 
    as soon as possible, some optimization is made here*/
    network::DNSeedService * dns=network::DNSeedService::getInstance();
    // storage::SeedNode * sn=dns->findSeedNode(epRemote);
    // if(sn)
    // {
    //     if(dns->badNode(sn))
    //     {
            this->RemoveNode(epRemote);
            dns->removeNode(epRemote);
    //     } 
    // }

    //Check to see if there are peer and nodes to connect, and if they are empty, connect DNseed
    CWalleveEventPeerNetGetPeers eventGetPeers(0);
    this->DispatchEvent(&eventGetPeers); 
    if(eventGetPeers.result.size() == 0 && this->GetCandidateNodeCount()<=0)
    {
        WalleveLog("Connect 2 DnSeed server");
        BOOST_FOREACH(const string& seed,NetworkConfig()->vDNSeed)
        {
            this->AddNewNode(CNetHost(seed,NetworkConfig()->nPort,"dnseed",
                                              boost::any(uint64(network::NODE_NETWORK))));
            
        }
    }

}
