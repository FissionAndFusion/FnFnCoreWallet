// Copyright (c) 2016-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef WALLEVE_PEERNET_H
#define WALLEVE_PEERNET_H

#include "walleve/netio/ioproc.h"
#include "walleve/peernet/peerevent.h"
#include "walleve/peernet/peer.h"
#include "walleve/peernet/peerinfo.h"
#include "walleve/peernet/epmngr.h"
#include <string>
#include <vector>
#include <map>
#include <boost/asio.hpp>
#include <boost/any.hpp>

namespace walleve
{

class CPeerService
{
public:
    CPeerService(const boost::asio::ip::tcp::endpoint& epListenIn, std::size_t nMaxInBoundsIn)
        : epListen(epListenIn), nMaxInBounds(nMaxInBoundsIn)
    {
    }

public:
    boost::asio::ip::tcp::endpoint epListen;
    std::size_t nMaxInBounds;
};

class CPeerNetConfig
{
public:
    std::vector<CPeerService> vecService;
    std::vector<CNetHost> vecNode;
    CNetHost gateWayNode;
    std::size_t nMaxOutBounds;
    unsigned short nPortDefault;
};

class CPeerNet : public CIOProc, virtual public CWallevePeerEventListener
{
public:
    CPeerNet(const std::string& walleveOwnKeyIn);
    ~CPeerNet();
    void ConfigNetwork(CPeerNetConfig& config);
    void HandlePeerClose(CPeer* pPeer);
    void HandlePeerViolate(CPeer* pPeer);
    void HandlePeerError(CPeer* pPeer);
    virtual void HandlePeerWriten(CPeer* pPeer);

protected:
    void EnterLoop() override;
    void LeaveLoop() override;
    void HeartBeat() override;
    void Timeout(uint64 nNonce, uint32 nTimerId) override;
    std::size_t GetMaxOutBoundCount() override;
    bool ClientAccepted(const boost::asio::ip::tcp::endpoint& epService, CIOClient* pClient) override;
    bool ClientConnected(CIOClient* pClient) override;
    void ClientFailToConnect(const boost::asio::ip::tcp::endpoint& epRemote) override;
    void HostResolved(const CNetHost& host, const boost::asio::ip::tcp::endpoint& ep) override;
    CPeer* AddNewPeer(CIOClient* pClient, bool fInBound);
    void RewardPeer(CPeer* pPeer, const CEndpointManager::Bonus& bonus);
    void RemovePeer(CPeer* pPeer, const CEndpointManager::CloseReason& reason);
    CPeer* GetPeer(uint64 nNonce);
    CPeer* GetPeer(const boost::asio::ip::tcp::endpoint& epNode);
    void AddNewNode(const CNetHost& host);
    void AddNewNode(const boost::asio::ip::tcp::endpoint& epNode,
                    const std::string& strName = "", const boost::any& data = boost::any());
    void RemoveNode(const CNetHost& host);
    void RemoveNode(const boost::asio::ip::tcp::endpoint& epNode);
    std::string GetNodeName(const boost::asio::ip::tcp::endpoint& epNode);
    bool GetNodeData(const boost::asio::ip::tcp::endpoint& epNode, boost::any& data);
    bool SetNodeData(const boost::asio::ip::tcp::endpoint& epNode, const boost::any& data);
    bool GetNodeRemoteId(const boost::asio::ip::tcp::endpoint& epNode, uint256& addr);
    bool SetNodeRemoteId(const boost::asio::ip::tcp::endpoint& epNode, const uint256& addr);
    void RetrieveGoodNode(std::vector<CNodeAvail>& vGoodNode, int64 nActiveTime, std::size_t nMaxCount);
    void AddNewGateWay(const boost::asio::ip::tcp::endpoint& epGateWay, const boost::asio::ip::tcp::endpoint& epNode);
    virtual std::string GetLocalIP();
    virtual CPeer* CreatePeer(CIOClient* pClient, uint64 nNonce, bool fInBound);
    virtual void DestroyPeer(CPeer* pPeer);
    virtual CPeerInfo* GetPeerInfo(CPeer* pPeer, CPeerInfo* pInfo = NULL);
    bool HandleEvent(CWalleveEventPeerNetGetIP& eventGetIP) override;
    bool HandleEvent(CWalleveEventPeerNetGetCount& eventGetCount) override;
    bool HandleEvent(CWalleveEventPeerNetGetPeers& eventGetPeers) override;
    bool HandleEvent(CWalleveEventPeerNetAddNode& eventAddNode) override;
    bool HandleEvent(CWalleveEventPeerNetRemoveNode& eventRemoveNode) override;
    bool HandleEvent(CWalleveEventPeerNetGetBanned& eventGetBanned) override;
    bool HandleEvent(CWalleveEventPeerNetSetBan& eventSetBan) override;
    bool HandleEvent(CWalleveEventPeerNetClrBanned& eventClrBanned) override;
    bool HandleEvent(CWalleveEventPeerNetReward& eventReward) override;
    bool HandleEvent(CWalleveEventPeerNetClose& eventClose) override;
    int GetCandidateNodeCount() { return epMngr.GetCandidateNodeCount(); }

    void AddPeerRecord(CPeer* pPeer);
    bool AddRemotePeerId(CPeer* pPeer, const uint256& hashId, bool fIsInbound);
protected:
    CPeerNetConfig confNetwork;
    boost::asio::ip::address localIP;
    std::set<boost::asio::ip::address> setIP;
private:
    CEndpointManager epMngr;
    std::map<uint64, CPeer*> mapPeer;
};

} // namespace walleve

#endif //WALLEVE_PEERNET_H
