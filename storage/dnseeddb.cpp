// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include "dnseeddb.h"

using namespace multiverse::storage;
using namespace boost::asio::ip;

DNSeedDB::DNSeedDB()
{

}

DNSeedDB::~DNSeedDB()
{
    Deinit();
}

bool DNSeedDB::Init(const CMvDBConfig& config)
{
    if (!dbConn.Connect(config))
    {
        return false;
    }
    return CreateTable();
}

void DNSeedDB::Deinit()
{
    dbConn.Disconnect();
}

void DNSeedDB::GetBinaryCharV4V6(std::vector<unsigned char> & bytes
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

bool DNSeedDB::InsertNode(SeedNode &node)
{
    std::ostringstream oss;
    std::vector<unsigned char> bt;
    GetBinaryCharV4V6(bt,node.ep.address());
    
    oss << "INSERT INTO dnseednode(address,port,score) "
              "VALUES("
        <<            "\'" << dbConn.ToEscString(bt) << "\',"
        <<            node.ep.port() << ","
        <<            node.nScore << ")";
    std::string str=oss.str();
    return dbConn.Query(oss.str());
}

bool DNSeedDB::DeleteNode(SeedNode &node)
{
    std::ostringstream oss;
    std::vector<unsigned char> bt;
    GetBinaryCharV4V6(bt,node.ep.address());
    oss << "DELETE FROM dnseednode "
         << " WHERE address = " << "\'" << dbConn.ToEscString(bt) << "\'";
    return dbConn.Query(oss.str());
}

bool DNSeedDB::UpdateNode(SeedNode &node)
{
    std::ostringstream oss;
    std::vector<unsigned char> bt;
    GetBinaryCharV4V6(bt,node.ep.address());
    oss << "UPDATE dnseednode SET score = "<< node.nScore<< ",port = "<<node.ep.port() 
                << " WHERE address = " << "\'" << dbConn.ToEscString(bt) << "\'";
    return dbConn.Query(oss.str());
}

bool DNSeedDB::SelectAllNode(std::vector<SeedNode> & nodeList)
{
    CMvDBRes res(dbConn,"SELECT id,address,port,score FROM dnseednode ",true);
    while (res.GetRow())
    {
       
        SeedNode node;
        std::vector<unsigned char>  addbyte;
        short port;
        if (!res.GetField(0,node.nId) || !res.GetField(1,addbyte) || !res.GetField(2,port) 
            || !res.GetField(3,node.nScore))
        {
            return false;
        }
        address_v4::bytes_type byte;
        for(size_t i=0;i<4;i++)
        {
            byte[i]=addbyte[i];
        }
        node.ep=tcp::endpoint(address(address_v4(byte)),port);
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

bool DNSeedDB::FindOneWithAddress(std::string ip,SeedNode &targetNode)
{
    address addr=address::from_string(ip);
    std::vector<unsigned char> bt;
    GetBinaryCharV4V6(bt,addr);

    std::ostringstream oss;
    oss<<"SELECT id,address,port,score FROM dnseednode where address = \'"
    << dbConn.ToEscString(bt)<<"\'";
    std::string str=oss.str();
    CMvDBRes res(dbConn,str,true);
    while(res.GetRow())
    {
        std::vector<unsigned char>  addbyte;
        short port;
        if (!res.GetField(0,targetNode.nId) || !res.GetField(1,addbyte) 
            || !res.GetField(2,port)        || !res.GetField(3,targetNode.nScore))
        {
            return false;
        }
        address_v4::bytes_type byte;
        for(size_t i=0;i<4;i++)
        {
            byte[i]=addbyte[i];
        }
        targetNode.ep=tcp::endpoint(address(address_v4(byte)),port);
        return true;
    }
    return false;
}