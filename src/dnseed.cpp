// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include "dnseed.h"
#include "version.h"
#include <boost/bind.hpp>
#include <boost/any.hpp>
#include "dnseedservice.h"
#include "mvdnseedpeer.h"
#include "mvproto.h"
#include "config.h"

using namespace walleve;
using namespace multiverse;
using boost::asio::ip::tcp;
using namespace multiverse::storage;
using namespace multiverse::network;
using namespace std;

#define HANDSHAKE_TIMEOUT               5
#define TIMING_FILTER_INTERVAL (boost::posix_time::seconds(60*60))//(24*60*60)


CDNSeed::CDNSeed()
:timerFilter(ioService,TIMING_FILTER_INTERVAL),
 thrIOProc("dnseedTimerFilter",boost::bind(&CDNSeed::IOThreadFunc,this)),
 _newestHeight(0)
{
}

CDNSeed::~CDNSeed()
{
}

bool CDNSeed::WalleveHandleInitialize()
{

    //标记为 DNSeedServer
    CMvDBConfig dbConfig(StorageConfig()->strDBHost,
                         StorageConfig()->nDBPort,
                         StorageConfig()->strDNSeedDBName,
                         StorageConfig()->strDBUser,
                         StorageConfig()->strDBPass);
    DNSeedService* dns=DNSeedService::getInstance();
    dns->init(dbConfig);
    dns->enableDNSeedServer();
    dns->_maxConnectFailTimes=NetworkConfig()->nMaxTimes2ConnectFail;
    
    CPeerNetConfig config;
    //启动端口监听
    config.vecService.push_back(CPeerService(tcp::endpoint(tcp::v4(), NetworkConfig()->nDNSeedPort),
                                                 NetworkConfig()->nMaxInBounds));
    config.nMaxOutBounds = NetworkConfig()->nMaxOutBounds;
    config.nPortDefault = NetworkConfig()->nDNSeedPort;

    ConfigNetwork(config);

    if (!WalleveThreadStart(thrIOProc))
    {
        WalleveLog("Failed to start filter timer thread\n");
        return false;
    }

    return true;
}

void CDNSeed::WalleveHandleDeinitialize()
{
    network::CMvPeerNet::WalleveHandleDeinitialize();
    DNSeedService::Release();
}

bool CDNSeed::CheckPeerVersion(uint32 nVersionIn,uint64 nServiceIn,const std::string& subVersionIn)
{
    (void)subVersionIn;
    if (nVersionIn < MIN_PROTO_VERSION || (nServiceIn & network::NODE_NETWORK) == 0)
    {
        return false;
    }
    return true;
}

void CDNSeed::BuildHello(CPeer *pPeer,CWalleveBufStream& ssPayload)
{
    uint64 nNonce = pPeer->GetNonce();
    int64 nTime = WalleveGetNetTime();
    int nHeight = this->GetPrimaryChainHeight();
    ssPayload << nVersion << nService << nTime << nNonce << subVersion << nHeight; 
}

int CDNSeed::GetPrimaryChainHeight()
{
    return this->_newestHeight;
}

CPeer* CDNSeed::CreatePeer(CIOClient *pClient,uint64 nNonce,bool fInBound)
{
    uint32_t nTimerId = SetTimer(nNonce,HANDSHAKE_TIMEOUT);
    CMvDNSeedPeer *pPeer = new CMvDNSeedPeer(this,pClient,nNonce,fInBound,nMagicNum,nTimerId);
    if (pPeer == NULL)
    {   
        CancelTimer(nTimerId);
    }
    return pPeer;
}

void CDNSeed::DestroyPeer(CPeer* pPeer)
{
    CMvPeer *pMvPeer = static_cast<CMvPeer *>(pPeer);
    if (pMvPeer->IsHandshaked())
    {
        // CMvEventPeerDeactive *pEventDeactive = new CMvEventPeerDeactive(pMvPeer->GetNonce());
        // if (pEventDeactive != NULL)
        // {
        //     pEventDeactive->data = CAddress(pMvPeer->nService,pMvPeer->GetRemote());
        //     pNetChannel->PostEvent(pEventDeactive);
        // }
    }
    CPeerNet::DestroyPeer(pPeer);
}

void CDNSeed::ProcessAskFor(CPeer* pPeer)
{
    uint256 hashFork;
    CInv inv;
    CMvPeer *pMvPeer = static_cast<CMvPeer *>(pPeer);
    if (pMvPeer->FetchAskFor(hashFork,inv))
    {
        // CMvEventPeerGetData* pEventGetData = new CMvEventPeerGetData(pMvPeer->GetNonce(),hashFork);
        // if (pEventGetData != NULL)
        // {
        //     pEventGetData->data.push_back(inv);
        //     pNetChannel->PostEvent(pEventGetData);
        // }
    }
}

bool CDNSeed::HandlePeerRecvMessage(CPeer *pPeer,int nChannel,int nCommand,CWalleveBufStream& ssPayload)
{
    CMvPeer *pMvPeer = static_cast<CMvPeer *>(pPeer);
    if (nChannel == MVPROTO_CHN_NETWORK)
    {
        switch (nCommand)
        {
        case MVPROTO_CMD_GETADDRESS:
            {
                tcp::endpoint ep(pMvPeer->GetRemote().address(),NetworkConfig()->nPort);
                DNSeedService* dns=DNSeedService::getInstance();
                //In order to facilitate the rapid formation of early network, all nodes connected to DNseed are considered as tested nodes.
                dns->addNode(ep,true);
                std::vector<CAddress> vAddrs;
                dns->getSendAddressList(vAddrs);
                WalleveLog(" MVPROTO_CMD_GETADDRESS[%s:%d] height:%d  sendNum:%d\n",
                            ep.address().to_string().c_str(),ep.port(),pMvPeer->nStartingHeight,vAddrs.size());
                
                CWalleveBufStream ss;
                ss << vAddrs;
                return pMvPeer->SendMessage(MVPROTO_CHN_NETWORK,MVPROTO_CMD_ADDRESS,ss);
            }
            break;
        case MVPROTO_CMD_ADDRESS:
            {   // betwen DNSeed servers change to test
                WalleveLog("xp [receive] MVPROTO_CMD_ADDRESS");
                std::vector<CAddress> vAddrs;
                ssPayload >> vAddrs;
                DNSeedService::getInstance()->recvAddressList(vAddrs);
                return true;
            }   
            break;
        default:
            break;
        }
    }
    return false;
}


bool CDNSeed::HandlePeerHandshaked(CPeer *pPeer,uint32 nTimerId)
{
    CMvPeer *pMvPeer = static_cast<CMvPeer *>(pPeer);
    CancelTimer(nTimerId);
    if (!CheckPeerVersion(pMvPeer->nVersion,pMvPeer->nService,pMvPeer->strSubVer))
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

        string strName = GetNodeName(ep);
        if (strName == "dnseed")
        {
            setDNSeed.insert(ep);
        }

        SetNodeData(ep,boost::any(pMvPeer->nService));
    }
    WalleveUpdateNetTime(pMvPeer->GetRemote().address(),pMvPeer->nTimeDelta);

    return true;
}

void CDNSeed::IOThreadFunc()
{
    ioService.reset(); 
    timerFilter.async_wait(boost::bind(&CDNSeed::IOProcFilter,this,_1));
    ioService.run();
    timerFilter.cancel();
    WalleveLog("dnseedservice stop");
}

void CDNSeed::IOProcFilter(const boost::system::error_code& err)
{
    if(!err)
    {
        int testNum=this->filterAddressList();
         /* restart deadline timer */
        boost::posix_time::seconds needTime(testNum);
        if(needTime<TIMING_FILTER_INTERVAL)
        {
            needTime=TIMING_FILTER_INTERVAL;
        }else{
           // needTime=needTime;//You can also add a compensation value here
        }
        timerFilter.expires_at(timerFilter.expires_at() + needTime);
        timerFilter.async_wait(boost::bind(&CDNSeed::IOProcFilter,this,_1));

        
    }
}

int CDNSeed::filterAddressList()
{
    std::vector<tcp::endpoint> testList;
    DNSeedService::getInstance()->getAllNodeList4Filter(testList);
    DNSeedService::getInstance()->resetNewNodeList();
    for(tcp::endpoint cep : testList)
    {
        WalleveLog("[dnseed] filterAddressList:%s:%d\n"
                    ,cep.address().to_string().c_str(),cep.port());
        this->AddNewNode(CNetHost(cep,"activeTest",boost::any(uint64(network::NODE_NETWORK))));
    }
}

void CDNSeed::dnseedTestConnSuccess(walleve::CPeer *pPeer)
{
    tcp::endpoint ep=pPeer->GetRemote();
    //过滤正常连接获取地址的握手
    string strName = GetNodeName(ep);
    if (strName != "activeTest") return;
    // //获取高度对比 TODO
     uint32 peerHeight=((CMvPeer*)pPeer)->nStartingHeight;
    // this->_newestHeight=_newestHeight>peerHeight ? _newestHeight:peerHeight;
    DNSeedService::getInstance()->addNode(ep,true);
    
    //断开连接
    WalleveLog("[dnseed]TestSuccess:%s:%d  h:%d\n"
                    ,ep.address().to_string().c_str()
                    ,ep.port()
                    ,peerHeight);
    this->RemovePeer(pPeer,CEndpointManager::CloseReason::HOST_CLOSE);
}

void CDNSeed::ClientFailToConnect(const tcp::endpoint& epRemote)
{
    CPeerNet::ClientFailToConnect(epRemote);
    WalleveLog("ConnectFailTo>>>%s\n",epRemote.address().to_string().c_str());
    this->RemoveNode(epRemote);
    DNSeedService * dns=DNSeedService::getInstance();
    SeedNode * sn=dns->findSeedNode(epRemote);
    if(sn)
    {
        if(dns->badNode(sn))
        {
            dns->removeNode(epRemote);
        } 
    }
    
    
}
