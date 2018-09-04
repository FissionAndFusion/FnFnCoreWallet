// Copyright (c) 2017-2018 The Multiverse xp

#include "mvdnseedpeer.h"
#include "crypto.h"
#include "mvpeernet.h"
#include "dnseedservice.h"

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

// void CMvDNSeedPeer::Activate()
// {
//     CMvPeer::Activate();
// }

// void CMvDNSeedPeer::SendHello()
// {
//     CWalleveBufStream ssPayload;
//     ((CMvPeerNet*)pPeerNet)->BuildHello(this,ssPayload);
//     SendMessage(MVPROTO_CHN_NETWORK,MVPROTO_CMD_HELLO,ssPayload);
//     nTimeHello = GetTime();
// }

// void CMvDNSeedPeer::SendHelloAck()
// {
//     SendMessage(MVPROTO_CHN_NETWORK,MVPROTO_CMD_HELLO_ACK);
// }

// bool CMvDNSeedPeer::HandshakeReadCompletd()
// {
//     CWalleveBufStream& ss = ReadStream();
//     uint256 hash = multiverse::crypto::CryptoHash(ss.GetData(),ss.GetSize());
//     if (hdrRecv.nPayloadChecksum == hash.Get32() && hdrRecv.GetChannel() == MVPROTO_CHN_NETWORK)
//     {
//         int64 nTimeRecv = GetTime();
//         int nCmd = hdrRecv.GetCommand();
//         try
//         {
//             if (nCmd == MVPROTO_CMD_HELLO)
//             {
//                 if (nVersion != 0)
//                 {
//                     return false;
//                 }
//                 int64 nTime;
//                 ss >> nVersion >> nService >> nTime >> nNonceFrom >> strSubVer >> nStartingHeight;
//                 nTimeDelta = nTime - nTimeRecv;
//                 if (!fInBound)
//                 {
//                     nTimeDelta += (nTimeRecv - nTimeHello) / 2;
//                     SendHelloAck();
//                     return HandshakeCompletd();
//                 }
//                 SendHello();
//                 Read(MESSAGE_HEADER_SIZE,boost::bind(&CMvDNSeedPeer::HandshakeReadHeader,this));
//                 return true;
//             }
//             else if (nCmd == MVPROTO_CMD_HELLO_ACK && fInBound)
//             {
//                 if (nVersion == 0)
//                 {
//                     return false;
//                 }
//                 nTimeDelta += (nTimeRecv - nTimeHello) / 2;
//                 return HandshakeCompletd();
//             }
//         }
//         catch (...) {}
//     }
//     return false;
// }

// bool CMvDNSeedPeer::HandleReadCompleted()
// {
//     CWalleveBufStream& ss = ReadStream();
//     uint256 hash = multiverse::crypto::CryptoHash(ss.GetData(),ss.GetSize());
//     if (hdrRecv.nPayloadChecksum == hash.Get32())
//     {
//         try
//         {
//             if (((CMvPeerNet*)pPeerNet)->HandlePeerRecvMessage(this,hdrRecv.GetChannel(),hdrRecv.GetCommand(),ss))
//             {
//                 Read(MESSAGE_HEADER_SIZE,boost::bind(&CMvDNSeedPeer::HandleReadHeader,this));
//                 return true;
//             }
//         }
//         catch (...)
//         {}
//     }
//     return false;
// }
bool CMvDNSeedPeer::HandshakeCompletd()
{
    //std::cout<<"HandshakeCompletd()"<<std::endl;
    // if (!((CMvPeerNet*)pPeerNet)->HandlePeerHandshaked(this,nHsTimerId))
    // {
    //     return false;
    // }
     nHsTimerId = 0;
     Read(MESSAGE_HEADER_SIZE,boost::bind(&CMvDNSeedPeer::HandleReadHeader,this));
     if(!DNSeedService::getInstance()->isDNSeedService())
     {
         SendMessage(MVPROTO_CHN_NETWORK,MVPROTO_CMD_GETDNSEED);
     }else{
         //断开连接
         //获取高度
     }
     
    return true;
}