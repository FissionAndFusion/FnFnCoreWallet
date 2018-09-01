#ifndef MULTIVERSE_DNSEED_H
#define MULTIVERSE_DNSEED_H

#include "config.h"
#include "mvbase.h"

namespace multiverse
{

class CDNSeed: public network::CMvPeerNet
{
public:
    CDNSeed();
    ~CDNSeed();
     bool CheckPeerVersion(uint32 nVersionIn,uint64 nServiceIn,const std::string& subVersionIn)override;
     virtual int GetPrimaryChainHeight()override;
     bool HandlePeerRecvMessage(walleve::CPeer *pPeer,int nChannel,int nCommand,
                               walleve::CWalleveBufStream& ssPayload)override;

protected:
    bool WalleveHandleInitialize();
    void WalleveHandleDeinitialize();
    walleve::CPeer* CreatePeer(walleve::CIOClient *pClient,uint64 nNonce,bool fInBound);

    const CMvNetworkConfig * NetworkConfig()
    {
        return dynamic_cast<const CMvNetworkConfig *>(walleve::IWalleveBase::WalleveConfig());
    }
    const CMvStorageConfig * StorageConfig()
    {
        return dynamic_cast<const CMvStorageConfig *>(walleve::IWalleveBase::WalleveConfig());
    }
protected:

};
}
#endif