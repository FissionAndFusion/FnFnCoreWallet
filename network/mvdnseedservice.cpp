// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include "mvdnseedservice.h"

using namespace walleve;
using namespace multiverse;
using namespace multiverse::network;
using namespace multiverse::storage;

#define SEND_ADDRESS_LIMIT 20

bool CMvDNSeedService::Init(CMvDBConfig &config)
{
    if (!db.Init(config))
    {
        return false;
    }
    db.SelectAllNode(vActiveNodeList);
    return true;
}

bool CMvDNSeedService::Add2list(boost::asio::ip::tcp::endpoint newep, bool forceAdd)
{
    //过滤局域网地址
    if (!IsRoutable(newep.address()))
        return false;
    if (HasAddress(newep))
        return false;

    CSeedNode sn(newep);
    if (!forceAdd)
    {
        vNewNodeList.push_back(sn);
        return true;
    }
    else
    {
        if (db.InsertNode(sn))
        {
            activeListLocker.lock();
            vActiveNodeList.push_back(sn);
            activeListLocker.unlock();
            return true;
        }
        else
        {
            return false;
        }
    }
}

void CMvDNSeedService::GetSendAddressList(std::vector<CAddress> &list)
{
    std::vector<CSeedNode> snlist;
    size_t vSize = vActiveNodeList.size();
    if (vSize <= SEND_ADDRESS_LIMIT)
    {
        snlist = vActiveNodeList;
    }
    else
    {
        std::sort(vActiveNodeList.begin(), vActiveNodeList.end(), [=](CSeedNode s1, CSeedNode s2) {
            return s1.nScore > s2.nScore;
        });
        //To avoid all nodes getting exactly the same list
        size_t randomSize;
        randomSize = vSize >= SEND_ADDRESS_LIMIT * 2 ? SEND_ADDRESS_LIMIT * 2 : vSize;
        InitRandomTool(randomSize);
        for (size_t i = 0; i < vActiveNodeList.size() && (snlist.size() < SEND_ADDRESS_LIMIT); i++)
        {
            snlist.push_back(vActiveNodeList[GetRandomIndex()]);
        }
    }

    for (size_t i = 0; i < snlist.size(); i++)
    {
        list.push_back(CAddress(-1, snlist[i].ep));
    }
}

bool CMvDNSeedService::HasAddress(boost::asio::ip::tcp::endpoint ep)
{
    for (size_t i = 0; i < vActiveNodeList.size(); i++)
    {
        if (vActiveNodeList[i].ep == ep)
            return true;
    }
    return false;
}

void CMvDNSeedService::RecvAddressList(std::vector<CAddress> epList)
{
    for (size_t i = 0; i < epList.size(); i++)
    {
        CAddress *sa = &epList[i];
        boost::asio::ip::tcp::endpoint ep;
        sa->ssEndpoint.GetEndpoint(ep);
        Add2list(ep);
    }
}

bool CMvDNSeedService::UpdateNode(CSeedNode node)
{
    activeListLocker.lock();
    bool rzt = false;
    if (db.UpdateNode(node))
    {
        for (size_t i = 0; i < vActiveNodeList.size(); i++)
        {
            if (vActiveNodeList[i].ep == node.ep)
            {
                vActiveNodeList[i].ep = node.ep;
                vActiveNodeList[i].nScore = node.nScore;
                rzt = true;
                break;
            }
        }
    }
    activeListLocker.unlock();
    return rzt;
}

void CMvDNSeedService::RemoveNode(const boost::asio::ip::tcp::endpoint &ep)
{
    activeListLocker.lock();
    CSeedNode sn(ep);
    bool rzt = db.DeleteNode(sn);
    for (auto it = vActiveNodeList.begin(); it != vActiveNodeList.end(); it++)
    {
        if (it->ep == ep)
        {
            vActiveNodeList.erase(it);
            break;
        }
    }
    activeListLocker.unlock();
}

void CMvDNSeedService::GetAllNodeList4Filter(std::vector<boost::asio::ip::tcp::endpoint> &epList)
{
    for (CSeedNode &sn : vActiveNodeList)
    {
        epList.push_back(sn.ep);
    }
    for (CSeedNode &sn : vNewNodeList)
    {
        epList.push_back(sn.ep);
    }
}

void CMvDNSeedService::ResetNewNodeList()
{
    vNewNodeList.clear();
}

storage::CSeedNode *CMvDNSeedService::FindSeedNode(const boost::asio::ip::tcp::endpoint &ep)
{
    for (size_t i = 0; i < vActiveNodeList.size(); i++)
    {
        if (vActiveNodeList[i].ep == ep)
            return &vActiveNodeList[i];
    }
    return NULL;
}

storage::CSeedNode *CMvDNSeedService::AddNode(boost::asio::ip::tcp::endpoint &ep, bool forceAdd)
{

    if (Add2list(ep, forceAdd))
    {
        return FindSeedNode(ep);
    }
    return NULL;
}

storage::CSeedNode *CMvDNSeedService::AddNewNode(boost::asio::ip::tcp::endpoint &ep)
{
    if (vActiveNodeList.size() < SEND_ADDRESS_LIMIT)
    {
        return AddNode(ep, true);
    }
    else
    {
        return AddNode(ep, false);
    }
}

void CMvDNSeedService::GoodNode(storage::CSeedNode *node, CanTrust canTrust)
{
    node->nReconnTimes = 0;
    if (canTrust == CanTrust::dontKown)
    {
        return;
    }
    else if (canTrust == CanTrust::yes)
    {
        node->nScore += 10;
    }
    else if (canTrust == CanTrust::no)
    {
        node->nScore -= 10;
    }

    if (node->nScore > 100)
    {
        node->nScore = 100;
        return;
    }
    if (node->nScore <= -100)
    {
        RemoveNode(node->ep);
        return;
    }
    UpdateNode(*node);
}

bool CMvDNSeedService::BadNode(storage::CSeedNode *node)
{
    node->nReconnTimes--;
    if (node->nReconnTimes <= -nMaxConnectFailTimes)
    {
        RemoveNode(node->ep);
        return true;
    }
    UpdateNode(*node);
    return false;
}