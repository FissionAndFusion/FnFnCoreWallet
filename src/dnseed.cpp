// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include "dnseed.h"
#include "version.h"
#include <boost/bind.hpp>
#include <boost/any.hpp>
#include "mvproto.h"
#include "config.h"

using namespace walleve;
using namespace multiverse;
using boost::asio::ip::tcp;
using namespace multiverse::storage;
using namespace multiverse::network;
using namespace std;

#define HANDSHAKE_TIMEOUT 5
#define TIMING_FILTER_INTERVAL (boost::posix_time::seconds(10 * 60)) //(12*60*60)
#define FORGIVE_HEIGHT_ERROR_VALUE 5

CMvDNSeedPeer::CMvDNSeedPeer(CPeerNet *pPeerNetIn, CIOClient *pClientIn, uint64 nNonceIn,
                             bool fInBoundIn, uint32 nMsgMagicIn, uint32 nHsTimerIdIn)
    : CMvPeer(pPeerNetIn, pClientIn, nNonceIn, fInBoundIn, nMsgMagicIn, nHsTimerIdIn),
      fIsTestPeer(false)
{
}

bool CMvDNSeedPeer::HandshakeCompleted()
{
    nHsTimerId = 0;
    Read(MESSAGE_HEADER_SIZE, boost::bind(&CMvDNSeedPeer::HandleReadHeader, this));
    if (fIsTestPeer)
    {
        ((CDNSeed *)pPeerNet)->DnseedTestConnSuccess(this);
    }
    return true;
}

CDNSeed::CDNSeed()
    : timerFilter(ioService_test, TIMING_FILTER_INTERVAL),
      thrIOProc_test("dnseedTimerFilter", boost::bind(&CDNSeed::IOThreadFunc_test, this)),
      nConfidentHeight(0)
{
}

CDNSeed::~CDNSeed()
{
}

bool CDNSeed::WalleveHandleInitialize()
{
    //config DNSeedServer
    CMvDBConfig dbConfig(StorageConfig()->strDBHost,
                         StorageConfig()->nDBPort,
                         StorageConfig()->strDBName,
                         StorageConfig()->strDBUser,
                         StorageConfig()->strDBPass);
    dnseedService.Init(dbConfig);
    dnseedService.nMaxConnectFailTimes = NetworkConfig()->nMaxTimes2ConnectFail;
    srtConfidentAddress = NetworkConfig()->strTrustAddress;

    Configure(NetworkConfig()->nMagicNum, PROTO_VERSION, network::NODE_NETWORK,
              FormatSubVersion(), !NetworkConfig()->vConnectTo.empty());

    CPeerNetConfig config;
    config.vecService.push_back(CPeerService(tcp::endpoint(tcp::v4(), NetworkConfig()->nDNSeedPort),
                                             NetworkConfig()->nMaxInBounds));
    config.nMaxOutBounds = NetworkConfig()->nMaxOutBounds;
    config.nPortDefault = NetworkConfig()->nDNSeedPort;

    ConfigNetwork(config);
    if (!WalleveThreadStart(thrIOProc_test))
    {
        WalleveLog("Failed to start filter timer thread\n");
        return false;
    }

    return true;
}

void CDNSeed::WalleveHandleDeinitialize()
{
    network::CMvPeerNet::WalleveHandleDeinitialize();
}

bool CDNSeed::CheckPeerVersion(uint32 nVersionIn, uint64 nServiceIn, const std::string &subVersionIn)
{
    (void)subVersionIn;
    if (nVersionIn < MIN_PROTO_VERSION || (nServiceIn & network::NODE_NETWORK) == 0)
    {
        return false;
    }
    return true;
}

void CDNSeed::BuildHello(CPeer *pPeer, CWalleveBufStream &ssPayload)
{
    uint64 nNonce = pPeer->GetNonce();
    int64 nTime = WalleveGetNetTime();
    int nHeight = GetPrimaryChainHeight();
    ssPayload << nVersion << nService << nTime << nNonce << subVersion << nHeight;
}

int CDNSeed::GetPrimaryChainHeight()
{
    return nConfidentHeight;
}

CPeer *CDNSeed::CreatePeer(CIOClient *pClient, uint64 nNonce, bool fInBound)
{
    uint32_t nTimerId = SetTimer(nNonce, HANDSHAKE_TIMEOUT);
    CMvDNSeedPeer *pPeer = new CMvDNSeedPeer(this, pClient, nNonce, fInBound, nMagicNum, nTimerId);
    if (!fInBound)
        pPeer->fIsTestPeer = true;
    if (pPeer == NULL)
    {
        CancelTimer(nTimerId);
    }
    return pPeer;
}

void CDNSeed::DestroyPeer(CPeer *pPeer)
{
    CPeerNet::DestroyPeer(pPeer);
}

void CDNSeed::ProcessAskFor(CPeer *pPeer)
{
    uint256 hashFork;
    CInv inv;
    CMvPeer *pMvPeer = static_cast<CMvPeer *>(pPeer);
    pMvPeer->FetchAskFor(hashFork, inv);
}

bool CDNSeed::HandlePeerRecvMessage(CPeer *pPeer, int nChannel, int nCommand, CWalleveBufStream &ssPayload)
{
    CMvPeer *pMvPeer = static_cast<CMvPeer *>(pPeer);
    if (nChannel == MVPROTO_CHN_NETWORK)
    {
        switch (nCommand)
        {
        case MVPROTO_CMD_GETADDRESS:
        {
            tcp::endpoint ep(pMvPeer->GetRemote().address(), NetworkConfig()->nPort);
            dnseedService.AddNewNode(ep);
            std::vector<CAddress> vAddrs;
            dnseedService.GetSendAddressList(vAddrs);
            WalleveLog(" MVPROTO_CMD_GETADDRESS [%s:%d] height:%d  sendNum:%d\n",
                       ep.address().to_string().c_str(), ep.port(), pMvPeer->nStartingHeight, vAddrs.size());
            CWalleveBufStream ss;
            ss << vAddrs;
            return pMvPeer->SendMessage(MVPROTO_CHN_NETWORK, MVPROTO_CMD_ADDRESS, ss);
        }
        break;
        default:
            break;
        }
    }
    return false;
}

bool CDNSeed::HandlePeerHandshaked(CPeer *pPeer, uint32 nTimerId)
{
    CMvPeer *pMvPeer = static_cast<CMvPeer *>(pPeer);
    CancelTimer(nTimerId);
    if (!CheckPeerVersion(pMvPeer->nVersion, pMvPeer->nService, pMvPeer->strSubVer))
    {
        return false;
    }
    if (!pMvPeer->IsInBound())
    {
        tcp::endpoint ep = pMvPeer->GetRemote();
        if (GetPeer(pMvPeer->nNonceFrom) != NULL)
        {
            RemoveNode(ep);
            return false;
        }

        SetNodeData(ep, boost::any(pMvPeer->nService));
    }
    WalleveUpdateNetTime(pMvPeer->GetRemote().address(), pMvPeer->nTimeDelta);

    return true;
}

void CDNSeed::IOThreadFunc_test()
{
    ioService_test.reset();
    timerFilter.async_wait(boost::bind(&CDNSeed::IOProcFilter, this, _1));
    ioService_test.run();
    timerFilter.cancel();
    WalleveLog("dnseedservice stop\n");
}

void CDNSeed::IOProcFilter(const boost::system::error_code &err)
{
    if (!err)
    {
        WalleveLog("Filter run\n");
        int testNum = BeginFilterList();
        /* restart deadline timer */
        boost::posix_time::seconds needTime(testNum);
        if (needTime < TIMING_FILTER_INTERVAL)
        {
            needTime = TIMING_FILTER_INTERVAL;
        }
        else
        {
            // needTime=needTime;//You can also add a compensation value here
        }
        timerFilter.expires_at(timerFilter.expires_at() + needTime);
        timerFilter.async_wait(boost::bind(&CDNSeed::IOProcFilter, this, _1));
    }
}

int CDNSeed::BeginFilterList()
{
    vTestListBuf.clear();
    dnseedService.GetAllNodeList4Filter(vTestListBuf);
    dnseedService.ResetNewNodeList();
    BeginVoteHeight();
    if (!srtConfidentAddress.empty())
    {
        RequestGetTrustedHeight();
    }
    else
    {
        FilterAddressList();
    }
    return vTestListBuf.size() + 1;
}

void CDNSeed::FilterAddressList()
{
    for (tcp::endpoint cep : vTestListBuf)
    {
        WalleveLog("[dnseed] filterAddressList:%s:%d\n", cep.address().to_string().c_str(), cep.port());
        AddNewNode(CNetHost(cep, "activeTest", boost::any(uint64(network::NODE_NETWORK))));
    }
    vTestListBuf.clear();
}

void CDNSeed::DnseedTestConnSuccess(walleve::CPeer *pPeer)
{
    tcp::endpoint ep = pPeer->GetRemote();
    string strName = GetNodeName(ep);
    uint32 peerHeight = ((CMvPeer *)pPeer)->nStartingHeight;
    if (strName == "trustedNode")
    {
        nConfidentHeight = peerHeight;
        fIsConfidentNodeCanConnect = true;
        RemovePeer(pPeer, CEndpointManager::CloseReason::HOST_CLOSE);
        RemoveNode(ep);
        WalleveLog("[dnseed]Trusted height: %d\n", peerHeight);
        FilterAddressList();
        return;
    }
    if (strName != "activeTest")
        return;
    CSeedNode *sn = dnseedService.FindSeedNode(ep);
    if (srtConfidentAddress.empty() || !fIsConfidentNodeCanConnect)
    {
        VoteHeight(peerHeight);
    }
    if (sn)
    {
        CMvDNSeedService::CanTrust canTrust = CMvDNSeedService::CanTrust::dontKown;
        if (nConfidentHeight > 0)
        {
            if (abs((long)nConfidentHeight - (long)peerHeight) <= FORGIVE_HEIGHT_ERROR_VALUE)
            {
                canTrust = CMvDNSeedService::CanTrust::yes;
            }
            else
            {
                canTrust = CMvDNSeedService::CanTrust::no;
            }
        }
        dnseedService.GoodNode(sn, canTrust);
    }

    WalleveLog("[dnseed]TestSuccess:%s:%d  h:%d  score:%d\n",
               ep.address().to_string().c_str(), ep.port(), peerHeight, sn->nScore);
    RemovePeer(pPeer, CEndpointManager::CloseReason::HOST_CLOSE);
    RemoveNode(ep);
}

void CDNSeed::RequestGetTrustedHeight()
{
    if (!srtConfidentAddress.empty())
    {
        AddNewNode(CNetHost(srtConfidentAddress, NetworkConfig()->nPort,
                            "trustedNode", boost::any(uint64(network::NODE_NETWORK))));
    }
}

void CDNSeed::ClientFailToConnect(const tcp::endpoint &epRemote)
{
    CPeerNet::ClientFailToConnect(epRemote);
    WalleveLog("ConnectFailTo>>>%s\n", epRemote.address().to_string().c_str());

    CSeedNode *sn = dnseedService.FindSeedNode(epRemote);
    if (sn)
    {
        dnseedService.BadNode(sn);
    }

    string strName = GetNodeName(epRemote);
    if (strName == "trustedNode")
    {
        fIsConfidentNodeCanConnect = false;
        FilterAddressList();
    }
    RemoveNode(epRemote);
}

void CDNSeed::BeginVoteHeight()
{
    if (srtConfidentAddress.empty() || !fIsConfidentNodeCanConnect)
    {
        nConfidentHeight = 0;
        vVoteBox.clear();
    }
}

void CDNSeed::VoteHeight(uint32 height)
{
    //totest
    bool hadSame = false;
    uint32 sumH = 0;
    uint32 avgH = 0;
    if (nConfidentHeight > 0)
        return;
    for (auto vote : vVoteBox)
    {
        if (vote.first == height)
        {
            vote.second++;
            hadSame = true;
        }
        sumH += vote.first;
        if (vote.second >= 5)
        {
            nConfidentHeight = vote.first;
            WalleveLog("vote Height:%d \n", nConfidentHeight);
            return;
        }
    }
    if (!vVoteBox.empty())
        avgH = sumH / vVoteBox.size();
    if (!hadSame && vVoteBox.size() < 5)
    {
        vVoteBox.push_back(std::make_pair(height, 1));
    }
    else
    {
        for (auto it = vVoteBox.begin(); it != vVoteBox.end();)
        {
            if ((it->first - avgH) > (height - avgH) && it->second == 1)
            {
                it = vVoteBox.erase(it);
                vVoteBox.push_back(std::make_pair(height, 1));
            }
            else
                it++;
        }
    }
}