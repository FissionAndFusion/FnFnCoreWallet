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
    _isTestPeer=false;
}   
    
CMvDNSeedPeer::~CMvDNSeedPeer()
{
}


bool CMvDNSeedPeer::HandshakeCompletd()
{
     nHsTimerId = 0;
     Read(MESSAGE_HEADER_SIZE,boost::bind(&CMvDNSeedPeer::HandleReadHeader,this));
     if(_isTestPeer)
     {
         ((CMvPeerNet*)pPeerNet)->dnseedTestConnSuccess(this);
     }else{
         SendMessage(MVPROTO_CHN_NETWORK,MVPROTO_CMD_GETADDRESS);     
     }
     
    return true;
}

bool CMvDNSeedPeer::HandleReadCompleted()
{
    CWalleveBufStream& ss = ReadStream();
    uint256 hash = multiverse::crypto::CryptoHash(ss.GetData(),ss.GetSize());
    if (hdrRecv.nPayloadChecksum == hash.Get32())
    {
        try
        {
            if (((CMvPeerNet*)pPeerNet)->HandlePeerRecvMessage(this,hdrRecv.GetChannel(),hdrRecv.GetCommand(),ss))
            {
                if(hdrRecv.GetChannel()==MVPROTO_CHN_NETWORK && hdrRecv.GetCommand()==MVPROTO_CMD_ADDRESS)
                {
                    ((CMvPeerNet*)pPeerNet)->HandlePeerClose(this);
                }
                Read(MESSAGE_HEADER_SIZE,boost::bind(&CMvDNSeedPeer::HandleReadHeader,this));
                return true;
            }
        }
        catch (...)
        {}
    }
    return false;
}