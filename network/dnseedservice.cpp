// Copyright (c) 2017-2018 The Multiverse developers xp@fnfn
#include "dnseedservice.h"


using namespace walleve;
using namespace multiverse;
using namespace multiverse::network;
using namespace multiverse::storage;


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

DNSeedService::DNSeedService()
{
    _isDNSeedServiceNode=false;     
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
    _db.selectAllNode(this->_activeNodeList);

    return true;
}

void DNSeedService::enableDNSeedServer()
{
    _isDNSeedServiceNode=true;
}

bool DNSeedService::add2list(boost::asio::ip::tcp::endpoint newep,bool forceAdd)
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
        if(isDNSeedService()&&!forceAdd)
        {
            this->_newNodeList.push_back(sn);
            return true;
        }
        else
        {
            if(_db.insertNode(sn))
            {
                this->_activeNodeList.push_back(sn);
                return true;
            }else{
                return false;
            }
        }
       
    }

    
}

void DNSeedService::getSendAddressList(std::vector<CAddress> & list,GetNodeWay gettype)
{
    std::vector<SeedNode> snlist;

    if(gettype==GET_ALL)
    {
        snlist=this->_activeNodeList;
    }
    //TODO 
    for(size_t i=0;i<this->_activeNodeList.size()&&snlist.size()<SEND_ADDRESS_LIMIT;i++)
    {
        //TODO choose role
        snlist.push_back(this->_activeNodeList[i]);
    }

    for(size_t i=0;i<snlist.size();i++)
    {
        list.push_back(CAddress(-1,snlist[i]._ep));
    }
}

bool DNSeedService::hasAddress(boost::asio::ip::tcp::endpoint ep)
{
    for(size_t i=0;i<this->_activeNodeList.size();i++)
    {
        if(this->_activeNodeList[i]._ep.address()==ep.address()) return true;
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

void DNSeedService::getLocalConnectAddressList(std::vector<boost::asio::ip::tcp::endpoint> &epList,int limitCount)
{
    //TODO 排序,抽取评分高的节点

    //用于连接列表
    for(size_t i=0;i<this->_activeNodeList.size() && i<limitCount;i++)
    {
        SeedNode &sn= this->_activeNodeList[i];
        epList.push_back(sn._ep);
    }
}

bool DNSeedService::updateNode(SeedNode node)
{
    if(this->_db.updateNode(node))
    {
        for(size_t i=0;i< this->_activeNodeList.size();i++)
        {
            if(this->_activeNodeList[i]._ep.address()== node._ep.address())
            {
                this->_activeNodeList[i]._ep=node._ep;
                return true;
            }
        }       
    }
    return false;
}

void DNSeedService::removeNode(const boost::asio::ip::tcp::endpoint &ep)
{
    //if(!this->hasAddress(ep)) return;
    SeedNode sn(ep);
    bool rzt=this->_db.deleteNode(sn);
    std::cout<<"delete:"<<rzt<<std::endl;
    for(auto it=this->_activeNodeList.begin();it!=this->_activeNodeList.end();it++)
    {
        if(it->_ep==ep)
        {
            this->_activeNodeList.erase(it);
            break;
        }
    }

}

void DNSeedService::getAllNodeList4Filter(std::vector<boost::asio::ip::tcp::endpoint> &epList)
{
    if(!this->isDNSeedService())return;
    
    for(SeedNode & sn :this->_activeNodeList)
    {
        epList.push_back(sn._ep);
    }
    for(SeedNode & sn :this->_newNodeList)
    {
        epList.push_back(sn._ep);
    }
}

void DNSeedService::resetNewNodeList()
{
    this->_newNodeList.clear();
}