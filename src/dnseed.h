// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef MULTIVERSE_DNSEED_H
#define MULTIVERSE_DNSEED_H

#include "config.h"
#include "mvbase.h"
#include "mvdnseedservice.h"
#include "mvpeernet.h"
#include <boost/asio.hpp>
#include "walleve/netio/netio.h"

namespace multiverse
{

namespace network
{
class CMvDNSeedPeer : public CMvPeer
{
public:
  CMvDNSeedPeer(walleve::CPeerNet *pPeerNetIn, walleve::CIOClient *pClientIn, uint64 nNonceIn,
                bool fInBoundIn, uint32 nMsgMagicIn, uint32 nHsTimerIdIn);
  ~CMvDNSeedPeer() {}
  bool fIsTestPeer;

protected:
  virtual bool HandshakeCompleted() override;
};
} // namespace network

class CDNSeed : public network::CMvPeerNet
{
public:
  CDNSeed();
  ~CDNSeed();
  bool CheckPeerVersion(uint32 nVersionIn, uint64 nServiceIn, const std::string &subVersionIn) override;
  virtual int GetPrimaryChainHeight();
  virtual bool HandlePeerRecvMessage(walleve::CPeer *pPeer, int nChannel, int nCommand,
                                     walleve::CWalleveBufStream &ssPayload) override;
  virtual bool HandlePeerHandshaked(walleve::CPeer *pPeer, uint32 nTimerId) override;
  virtual void BuildHello(walleve::CPeer *pPeer, walleve::CWalleveBufStream &ssPayload) override;
  void DnseedTestConnSuccess(walleve::CPeer *pPeer);

protected:
  bool WalleveHandleInitialize() override;
  void WalleveHandleDeinitialize() override;
  void ClientFailToConnect(const boost::asio::ip::tcp::endpoint &epRemote) override;
  virtual void DestroyPeer(walleve::CPeer *pPeer) override;
  virtual void ProcessAskFor(walleve::CPeer *pPeer) override;
  virtual walleve::CPeer *CreatePeer(walleve::CIOClient *pClient, uint64 nNonce, bool fInBound) override;

  const CMvNetworkConfig *NetworkConfig()
  {
    return dynamic_cast<const CMvNetworkConfig *>(walleve::IWalleveBase::WalleveConfig());
  }
  const CMvStorageConfig *StorageConfig()
  {
    return dynamic_cast<const CMvStorageConfig *>(walleve::IWalleveBase::WalleveConfig());
  }
  int BeginFilterList();
  void FilterAddressList();
  void RequestGetTrustedHeight();
  void IOThreadFunc_test();
  void IOProcFilter(const boost::system::error_code &err);
  //trusted height
  void BeginVoteHeight();
  void VoteHeight(uint32 height);

protected:
  network::CMvDNSeedService dnseedService;
  std::string srtConfidentAddress;
  uint32 nConfidentHeight;
  std::vector<std::pair<uint32, int>> vVoteBox;
  bool fIsConfidentNodeCanConnect = false;
  std::vector<boost::asio::ip::tcp::endpoint> vTestListBuf;
  //timer
  walleve::CWalleveThread thrIOProc_test;
  boost::asio::io_service ioService_test;
  boost::asio::deadline_timer timerFilter;
};
} // namespace multiverse
#endif