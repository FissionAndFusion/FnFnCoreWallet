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
    unsigned int nMaxConnectFailTimes=0;
    enum CanTrust{
        yes,no,dontKown
    };
public:
    bool Init(storage::CMvDBConfig & config);

    storage::SeedNode * FindSeedNode(const boost::asio::ip::tcp::endpoint& ep);
    storage::SeedNode * AddNewNode(boost::asio::ip::tcp::endpoint& ep);
    void GetSendAddressList(std::vector<CAddress> & list);
    void RecvAddressList(std::vector<CAddress> epList);
    bool UpdateNode(storage::SeedNode node);
    void RemoveNode(const boost::asio::ip::tcp::endpoint& ep);
    void GetAllNodeList4Filter(std::vector<boost::asio::ip::tcp::endpoint> &epList);
    void ResetNewNodeList();
    void GoodNode(storage::SeedNode* node,CanTrust canTrust);
    //Return result: whether to remove from the list 
    bool BadNode(storage::SeedNode* node);

protected:

    storage::SeedNode * AddNode(boost::asio::ip::tcp::endpoint& ep,bool forceAdd=false);
    bool HasAddress(boost::asio::ip::tcp::endpoint ep);   
    bool Add2list(boost::asio::ip::tcp::endpoint newep,bool forceAdd=false);
    
protected:
    std::mutex activeListLocker;
    std::vector<storage::SeedNode> vActiveNodeList;
    std::vector<storage::SeedNode> vNewNodeList;
    multiverse::storage::DNSeedDB db;
private:
    //test
    int nMaxNumber;
    std::set<int> setRdmNumber; 
    void InitRandomTool(int maxNumber)
    {
        nMaxNumber=maxNumber;
        setRdmNumber.clear();
        srand((unsigned)time(0));
    } 
    int GetRandomIndex()
    {
        int newNum;
        do{
            newNum=rand()%nMaxNumber;
        }while(setRdmNumber.count(newNum));
        setRdmNumber.insert(newNum);
        return newNum;
    }

};
}
}
#endif