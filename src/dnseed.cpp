#include "dnseed.h"
#include "version.h"
#include <boost/bind.hpp>
#include <boost/any.hpp>
#include "dnseedservice.h"
#include "mvdnseedpeer.h"

using namespace walleve;
using namespace multiverse;
using boost::asio::ip::tcp;

CDNSeed::CDNSeed()
//: CPeerNet("peernet")
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
    DNSeedService::getInstance()->init(dbConfig);
    DNSeedService::getInstance()->enableDNSeedServer();
    
    CPeerNetConfig config;
    //启动端口监听6816
    config.vecService.push_back(CPeerService(tcp::endpoint(tcp::v4(), NetworkConfig()->nDNSeedPort),
                                                 NetworkConfig()->nMaxInBounds));
    config.nMaxOutBounds = NetworkConfig()->nMaxOutBounds;
    config.nPortDefault = NetworkConfig()->nDNSeedPort;

    ConfigNetwork(config);

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

int CDNSeed::GetPrimaryChainHeight()
{
    return 0;
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

bool CDNSeed::HandlePeerRecvMessage(CPeer *pPeer,int nChannel,int nCommand,CWalleveBufStream& ssPayload)
{
    CMvPeer *pMvPeer = static_cast<CMvPeer *>(pPeer);
    if (nChannel == MVPROTO_CHN_NETWORK)
    {
        switch (nCommand)
        {
        case MVPROTO_CMD_GETADDRESS:
            {
                vector<CNodeAvail> vNode;
                RetrieveGoodNode(vNode,NODE_ACTIVE_TIME,500);
                vector<CAddress> vAddr;
                BOOST_FOREACH(const CNodeAvail& node,vNode)
                {
                    if (node.data.type() == typeid(uint64) && IsRoutable(node.ep.address()))
                    {
                        uint64 nService = boost::any_cast<uint64>(node.data);
                        vAddr.push_back(CAddress(nService,node.ep));
                    }
                }
                CWalleveBufStream ss;
                ss << vAddr;
                return pMvPeer->SendMessage(MVPROTO_CHN_NETWORK,MVPROTO_CMD_ADDRESS,ss);
            }
            break;
        case MVPROTO_CMD_ADDRESS:
            if (!fEnclosed)
            {
                vector<CAddress> vAddr;
                ssPayload >> vAddr;
                if (vAddr.size() > 500)
                {
                    return false;
                }
                BOOST_FOREACH(CAddress& addr,vAddr)
                {
                    tcp::endpoint ep;
                    addr.ssEndpoint.GetEndpoint(ep);          
                    if ((addr.nService & NODE_NETWORK) == NODE_NETWORK 
                         && IsRoutable(ep.address()) && !setDNSeed.count(ep))
                    {   
                        AddNewNode(ep,ep.address().to_string(),boost::any(addr.nService));
                    }
                }
                
                tcp::endpoint ep_ = pMvPeer->GetRemote();
                if (setDNSeed.count(ep_))
                {
                    RemoveNode(ep_);
                }
                this->_dnseed.add2list(ep_); 
                return true;
            }
            break;
        case MVPROTO_CMD_PING:
            return pMvPeer->SendMessage(MVPROTO_CHN_NETWORK,MVPROTO_CMD_PONG,ssPayload);
            break;
        case MVPROTO_CMD_PONG:
            return true;
            break;
        case MVPROTO_CMD_GETDNSEED:
            {
                WalleveLog("xp [receive] MVPROTO_CMD_GETDNSEED\n");
                std::vector<CAddress> vAddrs;
                this->_dnseed.getAddressList(vAddrs);
                CWalleveBufStream ss;
                ss << vAddrs;
                return pMvPeer->SendMessage(MVPROTO_CHN_NETWORK,MVPROTO_CMD_DNSEED,ss);
            }
            break;
        case MVPROTO_CMD_DNSEED:
            {
                WalleveLog("xp [receive] MVPROTO_CMD_DNSEED\n");
                std::vector<CAddress> vAddrs;
                ssPayload >> vAddrs;
                this->_dnseed.recvAddressList(vAddrs);
                
                std::vector<tcp::endpoint> eplist;
                this->_dnseed.getConnectAddressList(eplist);
                for(size_t i=0;i<eplist.size();i++)
                {
                    tcp::endpoint &cep=eplist[i];
                     std::cout<<cep.address().to_string()<<":"<<cep.port()<<std::endl;
                    // this->Connect(cep,10);
                    //this->AddNewNode(CNetHost(cep,cep.address().to_string(),boost::any(uint64(network::NODE_NETWORK))));
                }
                return true;
            }   
            break;
        default:
            break;
        }
    }
    return false;
}
