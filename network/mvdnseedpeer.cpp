// Copyright (c) 2017-2018 The Multiverse xp

#include "mvdnseedpeer.h"
#include "crypto.h"
#include "mvpeernet.h"
#include "dnseedservice.h"

using namespace std;
using namespace walleve;
using namespace multiverse::network;


CMvDNSeedPeer::CMvDNSeedPeer(CPeerNet *pPeerNetIn, CIOClient* pClientIn,uint64 nNonceIn,
                     bool fInBoundIn,uint32 nMsgMagicIn,uint32 nHsTimerIdIn)
: CMvPeer(pPeerNetIn,pClientIn,nNonceIn,fInBoundIn,nMsgMagicIn,nHsTimerIdIn)
{   
    
}   
    
CMvDNSeedPeer::~CMvDNSeedPeer()
{
}


bool CMvDNSeedPeer::HandshakeCompletd()
{
    //std::cout<<"HandshakeCompletd()"<<std::endl;
    // if (!((CMvPeerNet*)pPeerNet)->HandlePeerHandshaked(this,nHsTimerId))
    // {
    //     return false;
    // }
     nHsTimerId = 0;
     Read(MESSAGE_HEADER_SIZE,boost::bind(&CMvDNSeedPeer::HandleReadHeader,this));
     if(DNSeedService::getInstance()->isDNSeedService())
     {
         ((CMvPeerNet*)pPeerNet)->dnseedTestConnSuccess(this);
     }else{
         SendMessage(MVPROTO_CHN_NETWORK,MVPROTO_CMD_GETDNSEED);     
     }
     
    return true;
}