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


class DNSeedServer:public walleve::IWalleveBase 
{
public: 
    DNSeedServer(){
        _isDNSeedServerNode=true;//todo config
        _initTime=walleve::GetTime();
        _runTime=0;
        init();
    } 
    ~DNSeedServer(){}
public:
    enum GetNodeWay{
        GET_ALL,
        GET_A_LOT
    };
    void getAddressList(std::vector<CAddress> & list,GetNodeWay gettype=GET_A_LOT);
    bool add2list(boost::asio::ip::tcp::endpoint newep);
    void recvAddressList(std::vector<CAddress> epList);
    bool updateScore(storage::SeedNode node);
    static int test();
    storage::DNSeedDB * testGetDb(){return &_db;}
    bool isDNseedServer(){return _isDNSeedServerNode;}
private:
    bool init();
    void startDNSeedService();
    void filterAddressList();
    bool hasAddress(boost::asio::ip::tcp::endpoint ep);   
    void getAddressList(std::vector<storage::SeedNode> & list,GetNodeWay gettype=GET_A_LOT);
    
private:
    
    std::vector<storage::SeedNode> _nodeList;
    bool _isDNSeedServerNode;
    multiverse::storage::DNSeedDB _db;
    //timer
    int64 _initTime;
    int64 _runTime;

//advanced
    //定时任务

};
}
}
#endif