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
    DNSeedService();
    ~DNSeedService(){}
public:
    unsigned int _maxConnectFailTimes=0;
    enum CanTrust{
        yes,no,dontKown
    };
public:
    bool init(storage::CMvDBConfig & config);

    storage::SeedNode * findSeedNode(const boost::asio::ip::tcp::endpoint& ep);
    storage::SeedNode * addNewNode(boost::asio::ip::tcp::endpoint& ep);
    void getSendAddressList(std::vector<CAddress> & list);
    void recvAddressList(std::vector<CAddress> epList);
    bool updateNode(storage::SeedNode node);
    void removeNode(const boost::asio::ip::tcp::endpoint& ep);
    void getAllNodeList4Filter(std::vector<boost::asio::ip::tcp::endpoint> &epList);
    void resetNewNodeList();
    void goodNode(storage::SeedNode* node,CanTrust canTrust);
    //Return result: whether to remove from the list 
    bool badNode(storage::SeedNode* node);

protected:

    storage::SeedNode * addNode(boost::asio::ip::tcp::endpoint& ep,bool forceAdd=false);
    bool hasAddress(boost::asio::ip::tcp::endpoint ep);   
    bool add2list(boost::asio::ip::tcp::endpoint newep,bool forceAdd=false);
    
protected:
    std::mutex _activeListLocker;
    std::vector<storage::SeedNode> _activeNodeList;
    std::vector<storage::SeedNode> _newNodeList;
    multiverse::storage::DNSeedDB _db;
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