// Copyright (c) 2017-2018 The Multiverse developers xp
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef __DNSEED_SERVER__
#define __DNSEED_SERVER__

#include "walleve/walleve.h"
#include "dnseeddb.h"
#include "mvproto.h"



namespace multiverse 
{

namespace network
{


class DNSeedService 
{
public: 
    static DNSeedService* getInstance();
    ~DNSeedService(){}
public:
    enum GetNodeWay{
        GET_ALL,
        GET_A_LOT
    };
    bool init(storage::CMvDBConfig & config);
    bool isDNSeedService(){return _isDNSeedServiceNode;}
    void enableDNSeedServer();

    void getSendAddressList(std::vector<CAddress> & list,GetNodeWay gettype=GET_A_LOT);
    void getLocalConnectAddressList(std::vector<boost::asio::ip::tcp::endpoint> &epList ,int limitCount=10);
    bool add2list(boost::asio::ip::tcp::endpoint newep);
    void recvAddressList(std::vector<CAddress> epList);
    bool updateNode(storage::SeedNode node);
    void removeNode(boost::asio::ip::tcp::endpoint& ep);
    void getAllNodeList4Filter(std::vector<boost::asio::ip::tcp::endpoint> &epList);
protected:
    DNSeedService();

    bool hasAddress(boost::asio::ip::tcp::endpoint ep);   
    
protected:
    static DNSeedService* p_instance;
    std::vector<storage::SeedNode> _activeNodeList;
    std::vector<storage::SeedNode> _newNodeList;
    bool _isDNSeedServiceNode;
    multiverse::storage::DNSeedDB _db;
    

//advanced
    //定时任务

};
}
}
#endif