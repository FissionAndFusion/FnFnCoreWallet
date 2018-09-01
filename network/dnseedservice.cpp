// Copyright (c) 2017-2018 The Multiverse developers xp@fnfn
#include "dnseedservice.h"


using namespace walleve;
using namespace multiverse;
using namespace multiverse::network;
using namespace multiverse::storage;

#define TIMING_FILTER_INTERVAL (24*60*60)
#define SEND_ADDRESS_LIMIT 50

DNSeedService* DNSeedService::p_instance=NULL;

DNSeedService* DNSeedService::getInstance()
{
    if(p_instance == NULL)
    {
        p_instance=new DNSeedService();
    }
    return p_instance;
}


bool DNSeedService::init(CMvDBConfig & config)
{
    //const CMvStorageConfig * config=dynamic_cast<const CMvStorageConfig *>(IWalleveBase::WalleveConfig());
    //CMvDBConfig dbConfig("127.0.0.1",3306,"multiverse","multiverse","multiverse");
    if(!_db.init(config))
    {
        //WalleveLog("Failed to initialize DNSeed database\n");
        return false;
    }
    if(_isDNSeedServiceNode)
    {
        startDNSeedService();
    }
    _db.selectAllNode(this->_nodeList);

    return true;
}

void DNSeedService::startDNSeedService()
{

}

bool DNSeedService::add2list(boost::asio::ip::tcp::endpoint newep)
{
    //过滤局域网地址
    if(!IsRoutable(newep.address())) return false;
    if(this->hasAddress(newep))
    {
        return this->updateNode(newep);
    }
    else
    {
        SeedNode sn(newep); 
        // 入库
        if(_db.insertNode(sn))
        {
            this->_nodeList.push_back(sn);
            return true;
        }else{
            return false;
        }
    }

    
}

void DNSeedService::getAddressList(std::vector<CAddress> & list,GetNodeWay gettype)
{
    std::vector<SeedNode> snlist;
    getAddressList(snlist,gettype);
    for(size_t i=0;i<snlist.size();i++)
    {
        list.push_back(CAddress(-1,snlist[i]._ep));
    }
}

void DNSeedService::getAddressList(std::vector<SeedNode> & list,GetNodeWay gettype)
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

bool DNSeedService::hasAddress(boost::asio::ip::tcp::endpoint ep)
{
    for(size_t i=0;i<_nodeList.size();i++)
    {
        if(_nodeList[i]._ep.address()==ep.address()) return true;
    }
    return false;
}

void DNSeedService::recvAddressList(std::vector<CAddress> epList)
{
    for(size_t i=0;i<epList.size();i++)
    {
        CAddress * sa=&epList[i];
        boost::asio::ip::tcp::endpoint ep;
        sa->ssEndpoint.GetEndpoint(ep);
        this->add2list(ep);
    }
}

void DNSeedService::getConnectAddressList(std::vector<boost::asio::ip::tcp::endpoint> &epList,int limitCount)
{
    //TODO 排序,抽取评分高的节点

    //用于连接列表
    for(size_t i=0;i<this->_nodeList.size() && i<limitCount;i++)
    {
        SeedNode &sn= _nodeList[i];
        epList.push_back(sn._ep);
    }
}

void DNSeedService::filterAddressList()
{
    //建立连接
    //检查高度
    //断开连接

}

bool DNSeedService::updateNode(SeedNode node)
{
    if(this->_db.updateNode(node))
    {
        for(size_t i=0;i< this->_nodeList.size();i++)
        {
            if(this->_nodeList[i]._ep.address()== node._ep.address())
            {
                this->_nodeList[i]._ep=node._ep;
                return true;
            }
        }       
    }
    return false;
}

void DNSeedService::removeNode(boost::asio::ip::tcp::endpoint ep)
{

}