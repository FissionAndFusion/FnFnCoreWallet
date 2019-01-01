// Copyright (c) 2016-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  WALLEVE_NETHOST_H
#define  WALLEVE_NETHOST_H

#include <string>
#include <boost/asio.hpp>
#include <boost/any.hpp>

namespace walleve
{

class CNetHost
{
public:
    CNetHost();
    CNetHost(const std::string& strHostIn,unsigned short nPortIn,
             const std::string& strNameIn = "",const boost::any& dataIn = boost::any());
    CNetHost(const boost::asio::ip::tcp::endpoint& ep,
             const std::string& strNameIn = "",const boost::any& dataIn = boost::any());
    friend bool operator==(const CNetHost& a, const CNetHost& b)
    {
        return (a.strHost == b.strHost && a.nPort == b.nPort);
    }
    friend bool operator<(const CNetHost& a, const CNetHost& b)
    {
        return (a.strHost < b.strHost
                || (a.strHost == b.strHost && a.nPort < b.nPort));
    }
    const std::string ToString() const;
    const boost::asio::ip::tcp::endpoint ToEndPoint() const;
protected:
    unsigned short PortFromString(const std::string& strHostIn,unsigned short nPortDefault);
public:
    std::string strHost;
    unsigned short nPort;
    std::string strName;
    boost::any data;
};

} // namespace walleve

#endif //WALLEVE_NETHOST_H

