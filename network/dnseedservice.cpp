// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include "dnseedservice.h"


using namespace walleve;
using namespace multiverse;
using namespace multiverse::network;
using namespace multiverse::storage;


#define SEND_ADDRESS_LIMIT 20

DNSeedService::DNSeedService()
{
       
} 

bool DNSeedService::init(CMvDBConfig & config)
{
    if(!_db.init(config))
    {
        return false;
    }
    _db.selectAllNode(this->_activeNodeList);
    return true;
}


bool DNSeedService::add2list(boost::asio::ip::tcp::endpoint newep,bool forceAdd)
{
    //过滤局域网地址
    if(!IsRoutable(newep.address())) return false;
    if(this->hasAddress(newep)) return false;

    SeedNode sn(newep); 
    if(!forceAdd)
    {
        this->_newNodeList.push_back(sn);
        return true;
    }
    else
    {
        if(_db.insertNode(sn))
        {
            this->_activeListLocker.lock();
            this->_activeNodeList.push_back(sn);
            this->_activeListLocker.unlock();
            return true;
        }else{
            return false;
        }
    }
 
}

void DNSeedService::getSendAddressList(std::vector<CAddress> & list)
{
    std::vector<SeedNode> snlist;
    size_t vSize=_activeNodeList.size();
    if(vSize <= SEND_ADDRESS_LIMIT)
    {
        snlist=_activeNodeList;
    }
    else
    {
        std::sort(_activeNodeList.begin(),_activeNodeList.end(),[=](SeedNode s1,SeedNode s2){
            return s1._score>s2._score;
        });
        //To avoid all nodes getting exactly the same list
        size_t randomSize;
        randomSize=vSize>=SEND_ADDRESS_LIMIT *2?SEND_ADDRESS_LIMIT *2:vSize;
        this->initRandomTool(randomSize);
        for(size_t i=0;i<this->_activeNodeList.size()&& (snlist.size()<SEND_ADDRESS_LIMIT);i++)
        {  
            snlist.push_back(this->_activeNodeList[this->getRandomIndex()]);
        }
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
        if(this->_activeNodeList[i]._ep==ep ) return true;
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

bool DNSeedService::updateNode(SeedNode node)
{
    this->_activeListLocker.lock();
    bool rzt=false;
    if(this->_db.updateNode(node))
    {
        for(size_t i=0;i< this->_activeNodeList.size();i++)
        {
            if(this->_activeNodeList[i]._ep== node._ep)
            {
                this->_activeNodeList[i]._ep=node._ep;
                this->_activeNodeList[i]._score=node._score;
                rzt= true;
                break;
            }
        }       
    }
    this->_activeListLocker.unlock();
    return rzt;
}

void DNSeedService::removeNode(const boost::asio::ip::tcp::endpoint &ep)
{
    //if(!this->hasAddress(ep)) return;
    this->_activeListLocker.lock();
    SeedNode sn(ep);
    bool rzt=this->_db.deleteNode(sn);
    for(auto it=this->_activeNodeList.begin();it!=this->_activeNodeList.end();it++)
    {
        if(it->_ep==ep)
        {
            this->_activeNodeList.erase(it);
            break;
        }
    }
    this->_activeListLocker.unlock();
}

void DNSeedService::getAllNodeList4Filter(std::vector<boost::asio::ip::tcp::endpoint> &epList)
{   
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

storage::SeedNode * DNSeedService::findSeedNode(const boost::asio::ip::tcp::endpoint& ep)
{
    for(size_t i=0;i<this->_activeNodeList.size();i++)
    {
        if(this->_activeNodeList[i]._ep==ep) return &this->_activeNodeList[i];
    }
    return NULL;
}

storage::SeedNode * DNSeedService::addNode(boost::asio::ip::tcp::endpoint& ep,bool forceAdd)
{

    if(this->add2list(ep,forceAdd))
    {
        return findSeedNode(ep);
    }
    return NULL;
}

storage::SeedNode * DNSeedService::addNewNode(boost::asio::ip::tcp::endpoint& ep)
{
    if(_activeNodeList.size()<SEND_ADDRESS_LIMIT)
    {
        return addNode(ep,true);
    }
    else
    {
        return addNode(ep,false);
    }
}

void DNSeedService::goodNode(storage::SeedNode* node,CanTrust canTrust)
{
    node->_reconnTimes=0;
    if(canTrust==CanTrust::dontKown)
    {
        return;
    }
    else if(canTrust==CanTrust::yes)
    {
        node->_score+=10;
    }
    else if(canTrust==CanTrust::no)
    {
        node->_score-=10;
    }

    if(node->_score>100)
    {
        node->_score=100;
        return;
    } 
    if(node->_score<=-100)
    {
        this->removeNode(node->_ep);
        return;
    }
    this->updateNode(*node);
}

bool DNSeedService::badNode(storage::SeedNode* node)
{
    node->_reconnTimes--;
    if(node->_reconnTimes<= -this->_maxConnectFailTimes)
    {
        this->removeNode(node->_ep);
        return true;
    }
    this->updateNode(*node);
    return false;
}