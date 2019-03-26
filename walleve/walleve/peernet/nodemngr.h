// Copyright (c) 2016-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  WALLEVE_NODEMANAGER_H
#define  WALLEVE_NODEMANAGER_H

#include "walleve/type.h"
#include "walleve/util.h"
#include <map>
#include <string>
#include <boost/asio.hpp>
#include <boost/any.hpp>
#include <boost/bimap.hpp>

namespace walleve
{

class CNode
{
public:
    CNode() {}
    CNode(const boost::asio::ip::tcp::endpoint& epIn, const walleve::CUniqueAddress& macAddrIn, const std::string& strNameIn,
          const boost::any& dataIn) 
    : ep(epIn),macAddr(macAddrIn),strName(strNameIn),data(dataIn),nRetries(0) {}
public:
    boost::asio::ip::tcp::endpoint ep;
    walleve::CUniqueAddress macAddr;
    std::string strName; 
    boost::any data;
    int nRetries;
};

class CNodeManager
{
public:
    CNodeManager();
    void AddNew(const boost::asio::ip::tcp::endpoint& ep,const std::string& strName,
                const boost::any& data);
    void Remove(const boost::asio::ip::tcp::endpoint& ep);
    std::string GetName(const boost::asio::ip::tcp::endpoint& ep);
    bool GetData(const boost::asio::ip::tcp::endpoint& ep,boost::any& dataRet);
    bool SetData(const boost::asio::ip::tcp::endpoint& ep,const boost::any& dataIn);
    bool GetMacAddress(const boost::asio::ip::tcp::endpoint& ep,CUniqueAddress& addr);
    bool SetMacAddress(const boost::asio::ip::tcp::endpoint& ep,const CUniqueAddress& addr);
    void Clear();
    void Ban(const walleve::CUniqueAddress& address,int64 nBanTo);
    bool Employ(boost::asio::ip::tcp::endpoint& ep);
    void Dismiss(const boost::asio::ip::tcp::endpoint& ep,bool fForceRetry);
    void Retrieve(std::vector<CNode>& vNode);
    int GetCandidateNodeCount(){ return mapNode.size(); }

    void AddNewEndPointMac(const boost::asio::ip::tcp::endpoint& ep, const walleve::CUniqueAddress& addr);
    void RemoveEndPointMac(const boost::asio::ip::tcp::endpoint& ep);
protected:
    void RemoveInactiveNodes();
protected:
    enum {MAX_IDLENODES = 512,REMOVE_COUNT = 16};
    enum {RETRY_INTERVAL_BASE = 30,MAX_RETRIES = 16,MAX_IDLETIME = 28800};
    std::map<boost::asio::ip::tcp::endpoint,CNode> mapNode;
    std::multimap<int64,boost::asio::ip::tcp::endpoint> mapIdle;

    std::map<boost::asio::ip::tcp::endpoint, walleve::CUniqueAddress> mapRemoteEPMac;
};

} // namespace walleve

#endif //WALLEVE_NODEMANAGER_H
