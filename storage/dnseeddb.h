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
        this->nId=-1;
        this->ep=ep;
        this->nReconnTimes=0;
        this->nScore=score;
    }
public:
    int nId;
    boost::asio::ip::tcp::endpoint ep;
    int nScore;
    int nReconnTimes;
};

class DNSeedDB
{
public:
    DNSeedDB();
    ~DNSeedDB();

    bool Init(const CMvDBConfig& config);
    bool InsertNode(SeedNode &node);
    bool DeleteNode(SeedNode &node);
    bool UpdateNode(SeedNode &node);
    bool SelectAllNode(std::vector<SeedNode> & nodeList);
    bool FindOneWithAddress(std::string ip,SeedNode &targetNode);
private:
    void Deinit();
    bool CreateTable();
    void GetBinaryCharV4V6(std::vector<unsigned char> & bytes
                            ,boost::asio::ip::address addr);
protected:
    CMvDBConn dbConn;
};

}
}
#endif 