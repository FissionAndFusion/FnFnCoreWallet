// Copyright (c) 2017-2018 The Multiverse developers xp@fnfn
#include "dnseedserver.h"


using namespace walleve;
using namespace multiverse;
using namespace multiverse::network;
using namespace multiverse::storage;

#define TIMING_FILTER_INTERVAL (24*60*60)
#define SEND_ADDRESS_LIMIT 50

bool DNSeedServer::init()
{
    //const CMvStorageConfig * config=dynamic_cast<const CMvStorageConfig *>(IWalleveBase::WalleveConfig());
    CMvDBConfig dbConfig("127.0.0.1",3306,"multiverse","multiverse","multiverse");
    if(!_db.init(dbConfig))
    {
        WalleveLog("Failed to initialize DNSeed database\n");
        return false;
    }
    if(_isDNSeedServerNode)
    {
        startDNSeedService();
    }
    _db.selectAllNode(this->_nodeList);

    return true;
}

void DNSeedServer::startDNSeedService()
{

}

bool DNSeedServer::add2list(boost::asio::ip::tcp::endpoint newep)
{
    //过滤局域网地址
    if(!IsRoutable(newep.address())) return false;
    if(this->hasAddress(newep)) return false;

    SeedNode sn(newep); 
    // 入库
    if(_db.insertNode(sn))
    {
        this->_nodeList.push_back(sn);
        return true;
    }else{
        WalleveLog("[DNSeed] add node fail;\n");
        return false;
    }
    
}

void DNSeedServer::getAddressList(std::vector<CAddress> & list,GetNodeWay gettype)
{
    std::vector<SeedNode> snlist;
    getAddressList(snlist,gettype);
    for(size_t i=0;i<snlist.size();i++)
    {
        list.push_back(CAddress(-1,snlist[i]._ep));
    }
}

void DNSeedServer::getAddressList(std::vector<SeedNode> & list,GetNodeWay gettype)
{
    if(gettype==GET_ALL)
    {
        list=_nodeList;
    }
   
    for(size_t i=0;i<_nodeList.size()&&list.size()<SEND_ADDRESS_LIMIT;i++)
    {
        //TODO choose role
        list.push_back(_nodeList[i]);
    }
    
}

bool DNSeedServer::hasAddress(boost::asio::ip::tcp::endpoint ep)
{
    for(size_t i=0;i<_nodeList.size();i++)
    {
        if(_nodeList[i]._ep.address()==ep.address()) return true;
    }
    return false;
}

void DNSeedServer::recvAddressList(std::vector<CAddress> epList)
{
    for(size_t i=0;i<epList.size();i++)
    {
        CAddress * sa=&epList[i];
        boost::asio::ip::tcp::endpoint ep;
        sa->ssEndpoint.GetEndpoint(ep);
        this->add2list(ep);
    }
}

void DNSeedServer::filterAddressList()
{
    //建立连接
    //检查高度
    //断开连接

}

bool DNSeedServer::updateScore(SeedNode node)
{
    this->_db.updateNodeScore(node);
}

int DNSeedServer::test()
{
    /* code */
    DNSeedServer dnseed;
    //DNSeedDB* db=dnseed.testGetDb();

    boost::asio::ip::tcp::endpoint ep(
        boost::asio::ip::address().from_string("172.137.61.150"),1111);
    
    bool rzt=dnseed.add2list(ep);

    
    return 0;
}