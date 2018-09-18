// Copyright (c) 2016-2018 The LoMoCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  WALLEVE_PEERNET_H
#define  WALLEVE_PEERNET_H

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
    CPeerService(const boost::asio::ip::tcp::endpoint& epListenIn,std::size_t nMaxInBoundsIn)
    : epListen(epListenIn),nMaxInBounds(nMaxInBoundsIn) 
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
    std::size_t nMaxOutBounds;
    unsigned short nPortDefault;
};

class CPeerNet : public CIOProc, virtual public CWallevePeerEventListener
{
public:
    CPeerNet(const std::string& walleveOwnKeyIn);
    ~CPeerNet();
    void ConfigNetwork(CPeerNetConfig& config);
    void HandlePeerClose(CPeer * pPeer);
    void HandlePeerViolate(CPeer *pPeer);
    void HandlePeerError(CPeer *pPeer);
    virtual void HandlePeerWriten(CPeer *pPeer);
protected:
    void EnterLoop();
    void LeaveLoop();
    void HeartBeat();
    void Timeout(uint64 nNonce,uint32 nTimerId);
    std::size_t GetMaxOutBoundCount();
    bool ClientAccepted(const boost::asio::ip::tcp::endpoint& epService,CIOClient *pClient);
    bool ClientConnected(CIOClient *pClient);
    void ClientFailToConnect(const boost::asio::ip::tcp::endpoint& epRemote);
    void HostResolved(const CNetHost& host,const boost::asio::ip::tcp::endpoint& ep);
    CPeer* AddNewPeer(CIOClient *pClient,bool fInBound);
    void RewardPeer(CPeer *pPeer,const CEndpointManager::Bonus& bonus);
    void RemovePeer(CPeer *pPeer,const CEndpointManager::CloseReason& reason);
    CPeer* GetPeer(uint64 nNonce);
    void AddNewNode(const CNetHost& host);
    void AddNewNode(const boost::asio::ip::tcp::endpoint& epNode,
                    const std::string& strName = "",const boost::any& data = boost::any());
    void RemoveNode(const CNetHost& host);
    void RemoveNode(const boost::asio::ip::tcp::endpoint& epNode);
    std::string GetNodeName(const boost::asio::ip::tcp::endpoint& epNode);
    bool GetNodeData(const boost::asio::ip::tcp::endpoint& epNode,boost::any& data); 
    bool SetNodeData(const boost::asio::ip::tcp::endpoint& epNode,const boost::any& data);
    void RetrieveGoodNode(std::vector<CNodeAvail>& vGoodNode,int64 nActiveTime,std::size_t nMaxCount);
    virtual std::string GetLocalIP();
    virtual CPeer* CreatePeer(CIOClient *pClient,uint64 nNonce,bool fInBound);
    virtual void DestroyPeer(CPeer* pPeer);
    virtual CPeerInfo* GetPeerInfo(CPeer* pPeer,CPeerInfo* pInfo = NULL);
    bool HandleEvent(CWalleveEventPeerNetGetIP& eventGetIP); 
    bool HandleEvent(CWalleveEventPeerNetGetCount& eventGetCount); 
    bool HandleEvent(CWalleveEventPeerNetGetPeers& eventGetPeers); 
    bool HandleEvent(CWalleveEventPeerNetAddNode& eventAddNode); 
    bool HandleEvent(CWalleveEventPeerNetRemoveNode& eventRemoveNode); 
    bool HandleEvent(CWalleveEventPeerNetGetBanned& eventGetBanned); 
    bool HandleEvent(CWalleveEventPeerNetSetBan& eventSetBan); 
    bool HandleEvent(CWalleveEventPeerNetClrBanned& eventClrBanned);
    bool HandleEvent(CWalleveEventPeerNetReward& eventReward);
    bool HandleEvent(CWalleveEventPeerNetClose& eventClose);
    int GetCandidateNodeCount(){return epMngr.GetCandidateNodeCount();}
protected:
    CPeerNetConfig confNetwork; 
    boost::asio::ip::address localIP;
private:
    CEndpointManager epMngr;
    std::map<uint64,CPeer*> mapPeer;
};

} // namespace walleve

#endif //WALLEVE_PEERNET_H


