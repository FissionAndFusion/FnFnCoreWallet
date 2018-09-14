// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef __DNSEED_SERVER__
#define __DNSEED_SERVER__

#include "walleve/walleve.h"
#include "dnseeddb.h"
#include "mvproto.h"
#include <mutex>


namespace multiverse 
{

namespace network
{


class DNSeedService 
{
public: 
    static DNSeedService* getInstance();
    static void Release();
    ~DNSeedService(){}
public:
    bool init(storage::CMvDBConfig & config);
    bool isDNSeedService(){return _isDNSeedServiceNode;}
    void enableDNSeedServer();

    void getSendAddressList(std::vector<CAddress> & list);
    void getLocalConnectAddressList(std::vector<boost::asio::ip::tcp::endpoint> &epList ,int limitCount=10);
    void recvAddressList(std::vector<CAddress> epList);
    bool updateNode(storage::SeedNode node);
    void removeNode(const boost::asio::ip::tcp::endpoint& ep);
    void getAllNodeList4Filter(std::vector<boost::asio::ip::tcp::endpoint> &epList);
    void resetNewNodeList();
    storage::SeedNode * findSeedNode(const boost::asio::ip::tcp::endpoint& ep);
    void addNode(boost::asio::ip::tcp::endpoint& ep,bool forceAdd=false);
    void goodNode(storage::SeedNode* node);
    //Return result: whether to remove from the list 
    bool badNode(storage::SeedNode* node);
protected:
    DNSeedService();

    bool hasAddress(boost::asio::ip::tcp::endpoint ep);   
    bool add2list(boost::asio::ip::tcp::endpoint newep,bool forceAdd=false);
    
protected:
    std::mutex _activeListLocker;
    static DNSeedService* p_instance;
    std::vector<storage::SeedNode> _activeNodeList;
    std::vector<storage::SeedNode> _newNodeList;
    bool _isDNSeedServiceNode;
    multiverse::storage::DNSeedDB _db;
public:
    unsigned int _maxConnectFailTimes=0;
private:
    //test
    int _maxNumber;
    std::set<int> _rdmNumber; 
    void initRandomTool(int maxNumber)
    {
        _maxNumber=maxNumber;
        _rdmNumber.clear();
        srand((unsigned)time(0));
    } 
    int getRandomIndex()
    {
        int newNum;
        do{
            newNum=rand()%_maxNumber;
        }while(_rdmNumber.count(newNum));
        _rdmNumber.insert(newNum);
        return newNum;
    }

};
}
}
#endif