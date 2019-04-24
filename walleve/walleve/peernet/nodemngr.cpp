// Copyright (c) 2016-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php

#include "nodemngr.h"
#include "walleve/util.h"
#include <vector>

using namespace std;
using namespace walleve;
using boost::asio::ip::tcp;

///////////////////////////////
// CNodeManager



CNodeManager::CNodeManager() 
{
}

void CNodeManager::AddNew(const tcp::endpoint& ep,const string& strName,const boost::any& data)
{
    std::cout << "Node Manager AddNew " << ep.address().to_string() << ":" << ep.port() 
        << std::endl;
    if (mapNode.insert(make_pair(ep,CNode(ep,uint256(),strName,data))).second)
    {
        mapIdle.insert(make_pair(GetTime(),ep)); 
        if (mapIdle.size() > MAX_IDLENODES)
        {
            RemoveInactiveNodes();
        }
    }

    std::cout << "mapNode size " << mapNode.size() << std::endl;
    for(const auto& node : mapNode)
    {
        std::cout << "node ep [KEY] " << node.first.address().to_string() << ":" << node.first.port()
            << std::endl;
        std::cout << "node info [VALUE] " << node.second.nodeAddr.ToString() 
            << " retires " << node.second.nRetries << std::endl;
    }

    std::cout << "mapIdle size " << mapIdle.size() << std::endl;
    for(const auto& idle : mapIdle)
    {
        std::cout << "idle time " << idle.first << " ep " << 
            idle.second.address().to_string() << ":" << idle.second.port()
            << std::endl;
    }
}

void CNodeManager::Remove(const tcp::endpoint& ep)
{
    map<tcp::endpoint,CNode>::iterator mi = mapNode.find(ep);
    if (mi != mapNode.end())
    {
        std::cout << "Remove mapNode erase " << ep.address().to_string()
            << " port " << ep.port() << std::endl; 
        mapNode.erase(mi);

        for (multimap<int64,tcp::endpoint>::iterator it = mapIdle.begin();
             it != mapIdle.end();++it)
        {
            if ((*it).second == ep)
            {
                mapIdle.erase(it);
                break;
            }
        }
    }
}

string CNodeManager::GetName(const tcp::endpoint& ep)
{
    map<tcp::endpoint,CNode>::iterator mi = mapNode.find(ep);
    if (mi != mapNode.end())
    {
        return (*mi).second.strName;
    }
    return "";
}

bool CNodeManager::GetData(const tcp::endpoint& ep,boost::any& dataRet)
{
    map<tcp::endpoint,CNode>::iterator mi = mapNode.find(ep);
    if (mi != mapNode.end())
    {
        CNode& node = (*mi).second;
        if (!node.data.empty())
        {
            dataRet = node.data;
            return true;
        }
    }
    return false;
}

bool CNodeManager::SetData(const tcp::endpoint& ep,const boost::any& dataIn)
{
    map<tcp::endpoint,CNode>::iterator mi = mapNode.find(ep);
    if (mi != mapNode.end())
    {
        CNode& node = (*mi).second;
        node.data = dataIn;
        return true;
    }
    return false;
}

bool CNodeManager::GetNodeId(const boost::asio::ip::tcp::endpoint& ep,uint256& addr)
{
    map<tcp::endpoint,CNode>::iterator mi = mapNode.find(ep);
    if (mi != mapNode.end())
    {
        CNode& node = (*mi).second;
        if (node.nodeAddr.size() != 0)
        {
            addr = node.nodeAddr;
            return true;
        }
    }
    return false;
}

bool CNodeManager::SetNodeId(const boost::asio::ip::tcp::endpoint& ep,const uint256& addr)
{
    map<tcp::endpoint,CNode>::iterator mi = mapNode.find(ep);
    if (mi != mapNode.end())
    {
        CNode& node = (*mi).second;
        node.nodeAddr = addr;
        return true;
    }
    return false;
}

void CNodeManager::Clear()
{
    mapNode.clear();
    mapIdle.clear();
    mapRemoteEPNodeId.clear();
}

void CNodeManager::Ban(const uint256& address,int64 nBanTo)
{
    std::cout << "Ban Address " << address.ToString() << std::endl;
    std::cout << "BanTo " << nBanTo << std::endl;
    vector<tcp::endpoint> vNode;
    multimap<int64,tcp::endpoint>::iterator it = mapIdle.begin();
    while (it != mapIdle.upper_bound(nBanTo))
    {
       
       auto iter = mapRemoteEPNodeId.find(it->second);
       if(iter != mapRemoteEPNodeId.end() && iter->second == address)
       {
           std::cout << "vNode pushed back" << std::endl;
           vNode.push_back((*it).second);
           mapIdle.erase(it++);
       }
       else
       {
           ++it;
       }
       
    }

    for(const tcp::endpoint& ep : vNode)
    {
        mapIdle.insert(make_pair(nBanTo,ep));
    }
}

bool CNodeManager::Employ(tcp::endpoint& ep)
{
    std::cout << "Employ ep" << std::endl;
    std::cout << "mapIdle size " << mapIdle.size() << std::endl;
    for(const auto& idle : mapIdle)
    {
        std::cout << "idle time " << idle.first << " ep " << 
            idle.second.address().to_string() << ":" << idle.second.port()
            << std::endl;
    }
    multimap<int64,tcp::endpoint>::iterator it = mapIdle.begin();
    if (it != mapIdle.upper_bound(GetTime()))
    {
        ep = (*it).second;
        mapIdle.erase(it);
        std::cout << "Employed  true " << ep.address().to_string() << ":" << ep.port() 
            << std::endl;
        return true;
    }

     std::cout << "Employed  false " << std::endl;
    return false; 
}

void CNodeManager::Dismiss(const tcp::endpoint& ep,bool fForceRetry)
{
    std::cout << "Dismiss ep " << ep.address().to_string() << ":" << ep.port()
        << " ForceRetry " << fForceRetry << std::endl;
    map<tcp::endpoint,CNode>::iterator it = mapNode.find(ep);
    if (it != mapNode.end())
    {
        CNode& node = (*it).second;
        std::cout << "node old retries " << node.nRetries << std::endl;
        node.nRetries = (fForceRetry ? 0 : node.nRetries + 1);
        std::cout << "node new retries " << node.nRetries << std::endl;
        if (node.nRetries <= MAX_RETRIES)
        {
            int64 nIdleTo = GetTime() + (RETRY_INTERVAL_BASE << node.nRetries);
            std::cout << "IdleTo " << nIdleTo << std::endl;
            mapIdle.insert(make_pair(nIdleTo,node.ep));
            if (mapIdle.size() > MAX_IDLENODES)
            {
                RemoveInactiveNodes();
            }
        }
        else
        {
             std::cout << "[Dismiss] Remove mapNode erase " << ep.address().to_string()
            << " port " << ep.port() << std::endl; 
            mapNode.erase(it);
        }
    }
}

void CNodeManager::Retrieve(vector<CNode>& vNode)
{
    for (map<tcp::endpoint,CNode>::iterator it = mapNode.begin();it != mapNode.end();++it)
    {
        vNode.push_back((*it).second);
    }
}

void CNodeManager::RemoveInactiveNodes()
{
    int64 inactive = GetTime() + MAX_IDLETIME;
    int nRemoved = 0;

    multimap<int64,tcp::endpoint>::reverse_iterator rit = mapIdle.rbegin();
    while (rit != mapIdle.rend() && nRemoved < REMOVE_COUNT)
    {
        if ((*rit).first > inactive || nRemoved == 0)
        {
             std::cout << "[Inactive] Remove mapNode erase " << (*rit).second.address().to_string()
            << " port " << (*rit).second.port() << std::endl; 
            mapNode.erase((*rit).second);
            mapIdle.erase((++rit).base());
            nRemoved++;
        }
        else
        {
            break;
        }
    }
}

void CNodeManager::AddNewEndPointNodeId(const boost::asio::ip::tcp::endpoint& ep, const uint256& addr)
{
    mapRemoteEPNodeId[ep] = addr;
}

void CNodeManager::RemoveEndPointNodeId(const boost::asio::ip::tcp::endpoint& ep)
{
    auto it =  mapRemoteEPNodeId.find(ep);
    if(it != mapRemoteEPNodeId.end())
    {
        mapRemoteEPNodeId.erase(it);
    }
}
