// Copyright (c) 2017-2018 The Multiverse developer xp

#include "dnseeddb.h"

using namespace multiverse::storage;
using namespace boost::asio::ip;

DNSeedDB::DNSeedDB()
{

}

DNSeedDB::~DNSeedDB()
{
    deinit();
}

bool DNSeedDB::init(const CMvDBConfig& config)
{
    if (!dbConn.Connect(config))
    {
        return false;
    }
    return CreateTable();
}

void DNSeedDB::deinit()
{
    dbConn.Disconnect();
}

void DNSeedDB::getBinaryCharV4V6(std::vector<unsigned char> & bytes
                        ,boost::asio::ip::address addr)
{ 
    if(addr.is_v4())
    {
        address_v4::bytes_type bt=addr.to_v4().to_bytes();
        for(size_t i=0;i<bt.size();i++) bytes.push_back(bt[i]);
    }else{
        address_v6::bytes_type bt=addr.to_v6().to_bytes();
        for(size_t i=0;i<bt.size();i++) bytes.push_back(bt[i]);
    }
}

bool DNSeedDB::insertNode(SeedNode &node)
{
    std::ostringstream oss;
    std::vector<unsigned char> bt;
    this->getBinaryCharV4V6(bt,node._ep.address());
    
    oss << "INSERT INTO dnseednode(address,port,score) "
              "VALUES("
        <<            "\'" << dbConn.ToEscString(bt) << "\',"
        <<            node._ep.port() << ","
        <<            node._score << ")";
    std::string str=oss.str();
    return dbConn.Query(oss.str());
}

bool DNSeedDB::deleteNode(SeedNode &node)
{
    std::ostringstream oss;
    std::vector<unsigned char> bt;
     this->getBinaryCharV4V6(bt,node._ep.address());
    oss << "DELETE FROM dnseednode"
        <<            "where address =\'" << dbConn.ToEscString(bt) << "\'";
    return dbConn.Query(oss.str());
}

bool DNSeedDB::updateNodeScore(SeedNode &node)
{
    std::ostringstream oss;
    std::vector<unsigned char> bt;
     this->getBinaryCharV4V6(bt,node._ep.address());
    oss << "UPDATE dnseednode SET score = "<< node._score   
                << " WHERE address = " << "\'" << dbConn.ToEscString(bt) << "\'"
                << "or id="<<18;
    std::string str=oss.str();
    return dbConn.Query(oss.str());
}

bool DNSeedDB::selectAllNode(std::vector<SeedNode> & nodeList)
{
    CMvDBRes res(dbConn,"SELECT id,address,port,score FROM dnseednode limit 100",true);
    while (res.GetRow())
    {
       
        SeedNode node;
        std::vector<unsigned char>  addbyte;
        short port;
        if (!res.GetField(0,node._id) || !res.GetField(1,addbyte) || !res.GetField(2,port) 
            || !res.GetField(3,node._score))
        {
            return false;
        }
        address_v4::bytes_type byte;
        for(size_t i=0;i<4;i++)
        {
            byte[i]=addbyte[i];
        }
        node._ep=tcp::endpoint(address(address_v4(byte)),port);
        nodeList.push_back(node);
    }
    return true;
}

bool DNSeedDB::CreateTable()
{
        return dbConn.Query("CREATE TABLE IF NOT EXISTS dnseednode("
                          "id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,"
                          "address VARBINARY(16) NOT NULL UNIQUE KEY,"
                          "port INT NOT NULL,"
                          "score INT NOT NULL)"
                       );
}

SeedNode* DNSeedDB::findOneWithAddress(std::string ip)
{
    address addr=address::from_string(ip);
    std::vector<unsigned char> bt;
    this->getBinaryCharV4V6(bt,addr);

    std::ostringstream oss;
    oss<<"SELECT id,address,port,score FROM dnseednode where address = \'"
    << dbConn.ToEscString(bt)<<"\'";
    std::string str=oss.str();
    CMvDBRes res(dbConn,str,true);
    while(res.GetRow())
    {
        SeedNode* node=new SeedNode();
        std::vector<unsigned char>  addbyte;
        short port;
        if (!res.GetField(0,node->_id) || !res.GetField(1,addbyte) || !res.GetField(2,port) 
            || !res.GetField(3,node->_score))
        {
            return 0;
        }
        address_v4::bytes_type byte;
        for(size_t i=0;i<4;i++)
        {
            byte[i]=addbyte[i];
        }
        node->_ep=tcp::endpoint(address(address_v4(byte)),port);
        return node;
    }
    return 0;
}