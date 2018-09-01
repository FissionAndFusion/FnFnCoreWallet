// Copyright (c) 2017-2018 The Multiverse xp

#include "mvdnseedpeer.h"
#include "crypto.h"
#include "mvpeernet.h"

using namespace std;
using namespace walleve;
using namespace multiverse::network;

//////////////////////////////
// CMvPeer

CMvDNSeedPeer::CMvDNSeedPeer(CPeerNet *pPeerNetIn, CIOClient* pClientIn,uint64 nNonceIn,
                     bool fInBoundIn,uint32 nMsgMagicIn,uint32 nHsTimerIdIn)
: CMvPeer(pPeerNetIn,pClientIn,nNonceIn,fInBoundIn,nMsgMagicIn,nHsTimerIdIn)
{   
}   
    
CMvDNSeedPeer::~CMvDNSeedPeer()
{
}

void CMvDNSeedPeer::Activate()
{
    CMvPeer::Activate();
}


