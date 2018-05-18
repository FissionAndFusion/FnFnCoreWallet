// Copyright (c) 2016-2018 The LoMoCoin developers
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
: strHost(strHostIn.substr(0,strHostIn.find(':'))), 
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
    ss << strHost << ":" << nPort;
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
    size_t s = strHostIn.find(':');
    if (s != string::npos)
    {
        port = atoi(strHostIn.substr(s + 1).c_str());
    }
    if (port <= 0 || port > 0xFFFF)
    {
        port = nPortDefault;
    }
    return (unsigned short)port;
}
