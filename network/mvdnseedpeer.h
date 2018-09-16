#ifndef __MVDNSEEDNET__H
#define __MVDNSEEDNET__H

#include "mvproto.h"
#include "walleve/walleve.h"
#include <boost/bind.hpp>
#include "mvpeer.h"

namespace multiverse 
{

namespace network
{

class CMvDNSeedPeer : public CMvPeer
{
public:
    CMvDNSeedPeer(walleve::CPeerNet *pPeerNetIn, walleve::CIOClient* pClientIn,uint64 nNonceIn,
            bool fInBoundIn,uint32 nMsgMagicIn,uint32 nHsTimerIdIn);
    ~CMvDNSeedPeer();
    
protected:
    virtual bool HandshakeCompletd()override;
    virtual bool HandleReadCompleted()override;


};

}
}

#endif