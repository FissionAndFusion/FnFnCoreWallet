// Copyright (c) 2016-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "nethost.h"
#include <sstream>

using namespace std;
using namespace walleve;
using boost::asio::ip::tcp;

///////////////////////////////
// CNetHost
CNetHost::CNetHost()
: strHost(""),nPort(0),strName("")
{   
}   
    
CNetHost::CNetHost(const string& strHostIn,unsigned short nPortIn,const string& strNameIn,
                   const boost::any& dataIn)
: strHost(HostFromString(strHostIn)), 
  nPort(PortFromString(strHostIn,nPortIn)),
  strName(!strNameIn.empty() ? strNameIn : strHost),
  data(dataIn)
{   
}   

CNetHost::CNetHost(const tcp::endpoint& ep,const string& strNameIn,const boost::any& dataIn)
: strHost(ep.address().to_string()),nPort(ep.port()),
  strName(!strNameIn.empty() ? strNameIn : strHost),
  data(dataIn)
{
}

const string CNetHost::ToString() const
{
    stringstream ss;

    boost::asio::ip::address addr(boost::asio::ip::address::from_string(strHost));
    if(addr.is_v4())
    {
        ss << strHost << ":" << nPort;
    }

    if(addr.is_v6())
    {
        ss << "[" << strHost << "]" << ":" << nPort;
    }
    
    return ss.str();
}

const tcp::endpoint CNetHost::ToEndPoint() const
{
    boost::system::error_code ec;
    tcp::endpoint ep(boost::asio::ip::address::from_string(strHost,ec),nPort);
    return ((!ec && nPort) ? ep : tcp::endpoint());
}

unsigned short CNetHost::PortFromString(const string& strHostIn,unsigned short nPortDefault)
{
    int port = 0;
    if(IsV4FromString(strHostIn))
    {
        size_t s = strHostIn.find(':');
        if (s != string::npos)
        {
            port = atoi(strHostIn.substr(s + 1).c_str());
        }
    }
    else
    {
        size_t s = strHostIn.find("]:");
        if (s != string::npos)
        {
            port = atoi(strHostIn.substr(s + 2).c_str());
        }
    }
    
    if (port <= 0 || port > 0xFFFF)
    {
        port = nPortDefault;
    }
   
   
    return (unsigned short)port;
}

std::string CNetHost::HostFromString(const std::string& strHostIn)
{
    // IPv6 [host]:port
    // IPv4 host:port
    // refer to https://stackoverflow.com/questions/186829/how-do-ports-work-with-ipv6
    std::string host;
    if(IsV4FromString(strHostIn))
    {
        // IPv4
        host = strHostIn.substr(0,strHostIn.find(':'));
    }
    else
    {
        // IPv6
        size_t s = strHostIn.find("]:");
        if(s != std::string::npos)
        {
            host = strHostIn.substr(1,s - 1);
        }
        else
        {
            host = strHostIn;
        }
    }

    return host;
}

bool CNetHost::IsV4FromString(const std::string& strHostIn)
{
    auto dotDelim = strHostIn.find('.');
    if(dotDelim != std::string::npos)
    {
        return true;
    }
    else
    {
        return false;
    }
}
