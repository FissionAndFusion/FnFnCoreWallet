// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include "dnseed.h"
#include "version.h"
#include <boost/bind.hpp>
#include <boost/any.hpp>
#include "mvdnseedpeer.h"
#include "mvproto.h"
#include "config.h"

using namespace walleve;
using namespace multiverse;
using boost::asio::ip::tcp;
using namespace multiverse::storage;
using namespace multiverse::network;
using namespace std;

#define HANDSHAKE_TIMEOUT           5
#define TIMING_FILTER_INTERVAL      (boost::posix_time::seconds(10*60))//(12*60*60)
#define TIMING_GET_TRUSTED_HEIGHT   (boost::posix_time::seconds(1*60))
#define FORGIVE_HEIGHT_ERROR_VALUE  5

CDNSeed::CDNSeed()
:timerFilter(ioService_test,TIMING_FILTER_INTERVAL),
 thrIOProc_test("dnseedTimerFilter",boost::bind(&CDNSeed::IOThreadFunc_test,this)),
 timer_th(ioService_th,TIMING_GET_TRUSTED_HEIGHT),
 thrIOProc_th("dnseedTimerTrustedHeight",boost::bind(&CDNSeed::IOThreadFunc_th,this)),
 _confidentHeight(0)
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
                         StorageConfig()->strDBName,
                         StorageConfig()->strDBUser,
                         StorageConfig()->strDBPass);
    _dnseedService.init(dbConfig);
    _dnseedService._maxConnectFailTimes=NetworkConfig()->nMaxTimes2ConnectFail;
    this->_confidentAddress=NetworkConfig()->strConfidentAddress;

    CPeerNetConfig config;
    //启动端口监听
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

    /*If you need to get the latest altitude in real time, 
     *you can release the following code and modify the timing filter logic appropriately
    */
    // if(!this->_confidentAddress.empty())
    // {
    //     if (!WalleveThreadStart(thrIOProc_th))
    //     {
    //         WalleveLog("Failed to start GetTrustHeight thread \n");
    //         return false;
    //     }
    // }

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
    return _confidentHeight;
}

CPeer* CDNSeed::CreatePeer(CIOClient *pClient,uint64 nNonce,bool fInBound)
{
    uint32_t nTimerId = SetTimer(nNonce,HANDSHAKE_TIMEOUT);
    CMvDNSeedPeer *pPeer = new CMvDNSeedPeer(this,pClient,nNonce,fInBound,nMagicNum,nTimerId);
    if(!fInBound) pPeer->_isTestPeer=true;
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
                 
                _dnseedService.addNewNode(ep);

                std::vector<CAddress> vAddrs;
                _dnseedService.getSendAddressList(vAddrs);
                WalleveLog(" MVPROTO_CMD_GETADDRESS [%s:%d] height:%d  sendNum:%d\n",
                            ep.address().to_string().c_str(),ep.port(),pMvPeer->nStartingHeight,vAddrs.size());
                
                CWalleveBufStream ss;
                ss << vAddrs;
                return pMvPeer->SendMessage(MVPROTO_CHN_NETWORK,MVPROTO_CMD_ADDRESS,ss);
            }
            break;
        // case MVPROTO_CMD_ADDRESS:
        //     {   // betwen DNSeed servers change to test
        //         WalleveLog("xp [receive] MVPROTO_CMD_ADDRESS");
        //         std::vector<CAddress> vAddrs;
        //         ssPayload >> vAddrs;
        //         _dnseedService.recvAddressList(vAddrs);
        //         return true;
        //     }   
        //     break;
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

void CDNSeed::IOThreadFunc_test()
{
    ioService_test.reset(); 
    timerFilter.async_wait(boost::bind(&CDNSeed::IOProcFilter,this,_1));
    ioService_test.run();
    timerFilter.cancel();
    WalleveLog("dnseedservice stop\n");
}

void CDNSeed::IOProcFilter(const boost::system::error_code& err)
{
    if(!err)
    {
        WalleveLog("Filter run\n");
        int testNum=this->beginFilterList();
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
void CDNSeed::IOThreadFunc_th()
{
    ioService_th.reset(); 
    timer_th.async_wait(boost::bind(&CDNSeed::IOProc_th,this,_1));
    ioService_th.run();
    timer_th.cancel();
    WalleveLog("Timer Of Get Trusted Height Stop\n");
}

void CDNSeed::IOProc_th(const boost::system::error_code& err)
{
    if(!err)
    {
        this->requestGetTrustedHeight();
        timer_th.expires_at(timer_th.expires_at() + TIMING_GET_TRUSTED_HEIGHT);
        timer_th.async_wait(boost::bind(&CDNSeed::IOProc_th,this,_1));  
    }
}

int CDNSeed::beginFilterList()
{
    _testListBuf.clear();
    _dnseedService.getAllNodeList4Filter(_testListBuf);
    _dnseedService.resetNewNodeList();
    this->beginVoteHeight();
    if(!this->_confidentAddress.empty())
    {
        this->requestGetTrustedHeight();
    }else{
        this->filterAddressList();
    }
    return _testListBuf.size()+1;
}

void CDNSeed::filterAddressList()
{  
    for(tcp::endpoint cep : _testListBuf)
    {
        WalleveLog("[dnseed] filterAddressList:%s:%d\n"
                    ,cep.address().to_string().c_str(),cep.port());
        this->AddNewNode(CNetHost(cep,"activeTest",boost::any(uint64(network::NODE_NETWORK))));
    }
    _testListBuf.clear();
}

void CDNSeed::dnseedTestConnSuccess(walleve::CPeer *pPeer)
{
    tcp::endpoint ep=pPeer->GetRemote();
    string strName = GetNodeName(ep);
    uint32 peerHeight=((CMvPeer*)pPeer)->nStartingHeight;
    if(strName == "trustedNode")
    {
        _confidentHeight=peerHeight;
        _isConfidentNodeCanConnect=true;
        this->RemovePeer(pPeer,CEndpointManager::CloseReason::HOST_CLOSE);
        this->RemoveNode(ep);
        WalleveLog("trusted height: %d\n",peerHeight);
        this->filterAddressList();
        return;
    }
    if (strName != "activeTest") return;
    SeedNode * sn=_dnseedService.findSeedNode(ep);
    if(_confidentAddress.empty() || !_isConfidentNodeCanConnect)
    {
        this->voteHeight(peerHeight);
    }
    if(sn)
    {
        DNSeedService::CanTrust canTrust=DNSeedService::CanTrust::dontKown;
        if(this->_confidentHeight>0)
        {
            if(abs((long)_confidentHeight - (long)peerHeight) <= FORGIVE_HEIGHT_ERROR_VALUE)
            {
                canTrust=DNSeedService::CanTrust::yes;
            }else{
                canTrust=DNSeedService::CanTrust::no;
            }
        }
        _dnseedService.goodNode(sn,canTrust);
    }

    WalleveLog("[dnseed]TestSuccess:%s:%d  h:%d  score:%d\n",
                ep.address().to_string().c_str(),ep.port(),peerHeight,sn->_score);
    this->RemovePeer(pPeer,CEndpointManager::CloseReason::HOST_CLOSE);
    this->RemoveNode(ep);
}

void CDNSeed::requestGetTrustedHeight()
{
    if(!this->_confidentAddress.empty())
    { 
        this->AddNewNode(CNetHost(_confidentAddress,NetworkConfig()->nPort,
                         "trustedNode",boost::any(uint64(network::NODE_NETWORK))));
    }
}

void CDNSeed::ClientFailToConnect(const tcp::endpoint& epRemote)
{
    CPeerNet::ClientFailToConnect(epRemote);
    WalleveLog("ConnectFailTo>>>%s\n",epRemote.address().to_string().c_str());
    
    SeedNode * sn=_dnseedService.findSeedNode(epRemote);
    if(sn)
    {
        _dnseedService.badNode(sn);
    }

    string strName = GetNodeName(epRemote);
    if(strName=="trustedNode")
    {
        _isConfidentNodeCanConnect=false;
        this->filterAddressList();
    }
    this->RemoveNode(epRemote);
}

void CDNSeed::beginVoteHeight()
{
    if(this->_confidentAddress.empty()|| !_isConfidentNodeCanConnect)
    {
        _confidentHeight=0;
        _voteBox.clear();
    }
}

void CDNSeed::voteHeight(uint32 height)
{
    //totest
    bool hadSame=false;
    uint32 sumH=0;
    uint32 avgH=0;
    if(_confidentHeight>0) return;
    for(auto vote : _voteBox)
    {
        if(vote.first ==height)
        {
            vote.second++;
            hadSame=true;
        }
        sumH+=vote.first;
        if(vote.second>=5)
        {
            _confidentHeight=vote.first;
            WalleveLog("vote Height:%d \n",_confidentHeight);
            return;
        }
    }
    if(!_voteBox.empty())avgH=sumH/_voteBox.size();
    if(!hadSame && _voteBox.size()<5)
    {
        _voteBox.push_back(std::make_pair(height,1));
    }else{
        for(auto it=_voteBox.begin();it!=_voteBox.end();)
        {
            if(abs(it->first-avgH)>abs(height-avgH)&& it->second==1)
            {
                it=_voteBox.erase(it);
                _voteBox.push_back(std::make_pair(height,1));
            }
            else it++;
        }
    }
}