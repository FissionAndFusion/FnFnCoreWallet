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
    CMvDBConfig dbConfig(StorageConfig()->strDBHost,StorageConfig()->nDBPort
                        ,StorageConfig()->strDBName,StorageConfig()->strDBUser,StorageConfig()->strDBPass);
    DNSeedService* dns=DNSeedService::getInstance();
    dns->init(dbConfig);
    dns->enableDNSeedServer();
    dns->_maxConnectFailTimes=NetworkConfig()->nMaxTimes2ConnectFail;
    
    CPeerNetConfig config;
    //启动端口监听
    config.vecService.push_back(CPeerService(tcp::endpoint(tcp::v4(), NetworkConfig()->nPort),
                                                 NetworkConfig()->nMaxInBounds));
    config.nMaxOutBounds = NetworkConfig()->nMaxOutBounds;
    config.nPortDefault = NetworkConfig()->nPort;

    ConfigNetwork(config);

    if (!WalleveThreadStart(thrIOProc))
    {
        WalleveLog("Failed to start iothread\n");
        return false;
    }

    return true;
}

void CDNSeed::WalleveHandleDeinitialize()
{
    network::CMvPeerNet::WalleveHandleDeinitialize();
   
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
        case MVPROTO_CMD_GETDNSEED:
            {
                tcp::endpoint ep(pMvPeer->GetRemote().address(),NetworkConfig()->nPort);
                DNSeedService::getInstance()->addNode(ep);
                WalleveLog("[receive] MVPROTO_CMD_GETDNSEED      height:%d\n",pMvPeer->nStartingHeight);
                std::vector<CAddress> vAddrs;
                DNSeedService::getInstance()->getSendAddressList(vAddrs);
                
                CWalleveBufStream ss;
                ss << vAddrs;
                return pMvPeer->SendMessage(MVPROTO_CHN_NETWORK,MVPROTO_CMD_DNSEED,ss);
            }
            break;
        case MVPROTO_CMD_DNSEED:
            {   // betwen DNSeed servers change to test
                std::cout<<"xp [receive] MVPROTO_CMD_DNSEED"<<std::endl;
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
    cout<<"HandlePeerHandshaked()"<<endl;
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
         /* restart deadline timer */
        timerFilter.expires_at(timerFilter.expires_at() + TIMING_FILTER_INTERVAL);
        timerFilter.async_wait(boost::bind(&CDNSeed::IOProcFilter,this,_1));

        this->filterAddressList();
    }
}

void CDNSeed::filterAddressList()
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
