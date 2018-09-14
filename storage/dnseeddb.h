// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef __DNSEED_DB__
#define __DNSEED_DB__

#include "dbconn.h"
#include <boost/asio.hpp>
#include <boost/any.hpp>

namespace multiverse
{
namespace storage
{

class SeedNode{
public:
    SeedNode(){}
    SeedNode(boost::asio::ip::tcp::endpoint ep,int score=0)
    {
        this->_id=-1;
        this->_ep=ep;
        this->_score=score;
    }
public:
    int _id;
    boost::asio::ip::tcp::endpoint _ep;
    int _score;
};

class DNSeedDB
{
public:
    DNSeedDB();
    ~DNSeedDB();

    bool init(const CMvDBConfig& config);
    bool insertNode(SeedNode &node);
    bool deleteNode(SeedNode &node);
    bool updateNode(SeedNode &node);
    bool selectAllNode(std::vector<SeedNode> & nodeList);
    bool findOneWithAddress(std::string ip,SeedNode &targetNode);
private:
    void deinit();
    bool CreateTable();
    void getBinaryCharV4V6(std::vector<unsigned char> & bytes
                            ,boost::asio::ip::address addr);
protected:
    CMvDBConn dbConn;
};

}
}
#endif 