// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_NETWORK_H
#define  MULTIVERSE_NETWORK_H

#include "virtualpeernet.h"
#include "config.h"

namespace multiverse
{

class CNetwork : public CVirtualPeerNet
{
public:
    CNetwork();
    ~CNetwork();
    bool CheckPeerVersion(uint32 nVersionIn,uint64 nServiceIn,const std::string& subVersionIn) override;
protected:
    bool WalleveHandleInitialize() override;
    void WalleveHandleDeinitialize() override;
    const CMvNetworkConfig * NetworkConfig()
    {
        return dynamic_cast<const CMvNetworkConfig *>(walleve::IWalleveBase::WalleveConfig());
    }
    const CMvStorageConfig * StorageConfig()
    {
        return dynamic_cast<const CMvStorageConfig *>(walleve::IWalleveBase::WalleveConfig());
    }
};

} // namespace multiverse

#endif //MULTIVERSE_NETWORK_H

