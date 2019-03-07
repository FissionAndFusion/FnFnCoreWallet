// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_PEER_H
#define  MULTIVERSE_PEER_H

#include "mvproto.h"
#include "walleve/walleve.h"
#include <boost/bind.hpp>

namespace multiverse 
{

namespace network
{

class CMvPeer : public walleve::CPeer
{
public:
    CMvPeer(walleve::CPeerNet *pPeerNetIn, walleve::CIOClient* pClientIn,uint64 nNonceIn,
            bool fInBoundIn,uint32 nMsgMagicIn,uint32 nHsTimerIdIn);
    ~CMvPeer();
    void Activate() override;
    bool IsHandshaked();
    bool SendMessage(int nChannel,int nCommand,walleve::CWalleveBufStream& ssPayload);
    bool SendMessage(int nChannel,int nCommand)
    {
        walleve::CWalleveBufStream ssPayload;
        return SendMessage(nChannel,nCommand,ssPayload);
    }
    uint32 Request(CInv& inv,uint32 nTimerId);
    uint32 Responded(CInv& inv);
    void AskFor(const uint256& hashFork, const std::vector<CInv>& vInv);
    bool FetchAskFor(uint256& hashFork,CInv& inv);
protected:
    void SendHello();
    void SendHelloAck();
    bool ParseMessageHeader();
    bool HandshakeReadHeader();
    bool HandshakeReadCompleted();
    virtual bool HandshakeCompleted();
    bool HandleReadHeader();
    bool HandleReadCompleted();
public:
    int nVersion;
    uint64 nService;
    uint64 nNonceFrom;
    int64 nTimeDelta;
    int64 nTimeHello;
    std::string strSubVer;
    int nStartingHeight;
    walleve::CMacAddress remoteMacAddress;
protected:
    uint32 nMsgMagic;
    uint32 nHsTimerId;
    CMvPeerMessageHeader hdrRecv;

    std::map<CInv,uint32> mapRequest;
    std::queue<std::pair<uint256,CInv> > queAskFor;
};

class CMvPeerInfo : public walleve::CPeerInfo
{
public:
    int  nVersion;
    uint64 nService;
    std::string strSubVer;
    int nStartingHeight;
};

} // namespace network
} // namespace multiverse

#endif //MULTIVERSE_PEER_H

