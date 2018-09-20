// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef __DNSEED_SERVER__
#define __DNSEED_SERVER__

#include "walleve/walleve.h"
#include "dnseeddb.h"
#include "mvproto.h"
#include <mutex>

namespace multiverse
{

namespace network
{

class CMvDNSeedService
{
public:
  CMvDNSeedService() {}
  ~CMvDNSeedService() {}

public:
  unsigned int nMaxConnectFailTimes = 0;
  enum CanTrust
  {
    yes,
    no,
    dontKown
  };

public:
  bool Init(storage::CMvDBConfig &config);

  storage::CSeedNode *FindSeedNode(const boost::asio::ip::tcp::endpoint &ep);
  storage::CSeedNode *AddNewNode(boost::asio::ip::tcp::endpoint &ep);
  void GetSendAddressList(std::vector<CAddress> &list);
  void RecvAddressList(std::vector<CAddress> epList);
  bool UpdateNode(storage::CSeedNode node);
  void RemoveNode(const boost::asio::ip::tcp::endpoint &ep);
  void GetAllNodeList4Filter(std::vector<boost::asio::ip::tcp::endpoint> &epList);
  void ResetNewNodeList();
  void GoodNode(storage::CSeedNode *node, CanTrust canTrust);
  //Return result: whether to remove from the list
  bool BadNode(storage::CSeedNode *node);

protected:
  storage::CSeedNode *AddNode(boost::asio::ip::tcp::endpoint &ep, bool forceAdd = false);
  bool HasAddress(boost::asio::ip::tcp::endpoint ep);
  bool Add2list(boost::asio::ip::tcp::endpoint newep, bool forceAdd = false);

protected:
  std::mutex activeListLocker;
  std::vector<storage::CSeedNode> vActiveNodeList;
  std::vector<storage::CSeedNode> vNewNodeList;
  multiverse::storage::CDNSeedDB db;

private:
  //test
  int nMaxNumber;
  std::set<int> setRdmNumber;
  void InitRandomTool(int maxNumber)
  {
    nMaxNumber = maxNumber;
    setRdmNumber.clear();
    srand((unsigned)time(0));
  }
  int GetRandomIndex()
  {
    int newNum;
    do
    {
      newNum = rand() % nMaxNumber;
    } while (setRdmNumber.count(newNum));
    setRdmNumber.insert(newNum);
    return newNum;
  }
};
} // namespace network
} // namespace multiverse
#endif