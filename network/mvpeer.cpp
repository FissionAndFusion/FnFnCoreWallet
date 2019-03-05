// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "mvpeer.h"
#include "mvpeernet.h"
#include "crypto.h"

using namespace std;
using namespace walleve;
using namespace multiverse::network;

//////////////////////////////
// CMvPeer

CMvPeer::CMvPeer(CPeerNet *pPeerNetIn, CIOClient* pClientIn,uint64 nNonceIn,
                     bool fInBoundIn,uint32 nMsgMagicIn,uint32 nHsTimerIdIn)
: CPeer(pPeerNetIn,pClientIn,nNonceIn,fInBoundIn),nMsgMagic(nMsgMagicIn),nHsTimerId(nHsTimerIdIn)
{   
}   
    
CMvPeer::~CMvPeer()
{
}

void CMvPeer::Activate()
{
    CPeer::Activate();

    nVersion = 0;
    nService = 0;
    nNonceFrom = 0;
    nTimeDelta = 0;
    nTimeHello = 0;
    strSubVer.clear();
    nStartingHeight = 0;

    Read(MESSAGE_HEADER_SIZE,boost::bind(&CMvPeer::HandshakeReadHeader,this));
    if (!fInBound)
    {
        SendHello();
    }
}

bool CMvPeer::IsHandshaked()
{
    return (nHsTimerId == 0);
}

bool CMvPeer::SendMessage(int nChannel,int nCommand,CWalleveBufStream& ssPayload)
{
    CMvPeerMessageHeader hdrSend;    
    hdrSend.nMagic = nMsgMagic;
    hdrSend.nType  = CMvPeerMessageHeader::GetMessageType(nChannel,nCommand);
    hdrSend.nPayloadSize = ssPayload.GetSize();
    hdrSend.nPayloadChecksum = multiverse::crypto::CryptoHash(ssPayload.GetData(),ssPayload.GetSize()).Get32();
    hdrSend.nHeaderChecksum = hdrSend.GetHeaderChecksum();

    if (!hdrSend.Verify())
    {
        return false;
    }
    
    WriteStream() << hdrSend << ssPayload;
    Write();
    return true;
}

uint32 CMvPeer::Request(CInv& inv,uint32 nTimerId)
{
    uint32 nPrevTimerId = 0;
    map<CInv,uint32>::iterator it = mapRequest.find(inv);
    if (it != mapRequest.end())
    {
        nPrevTimerId = (*it).second;
        (*it).second = nTimerId;
    }
    else
    {
        mapRequest.insert(make_pair(inv,nTimerId));
    }
    return nPrevTimerId;
}
    
uint32 CMvPeer::Responded(CInv& inv)
{   
    uint32 nTimerId = 0;
    std::map<CInv,uint32>::iterator it = mapRequest.find(inv);
    if (it != mapRequest.end())
    {
        nTimerId = (*it).second;
        mapRequest.erase(it);
    }
    return nTimerId;
}

void CMvPeer::AskFor(const uint256& hashFork, const vector<CInv>& vInv)
{
    BOOST_FOREACH(const CInv& inv,vInv)
    {
        queAskFor.push(make_pair(hashFork,inv));
    }
}   
    
bool CMvPeer::FetchAskFor(uint256& hashFork,CInv& inv)
{
    if (IsWriteable() && !queAskFor.empty())
    {
        hashFork = queAskFor.front().first;
        inv = queAskFor.front().second;
        queAskFor.pop();
        return true;
    }
    return false;
}

void CMvPeer::SendHello()
{
    CWalleveBufStream ssPayload;
    (static_cast<CMvPeerNet*>(pPeerNet))->BuildHello(this,ssPayload);
    SendMessage(MVPROTO_CHN_NETWORK,MVPROTO_CMD_HELLO,ssPayload);
    nTimeHello = GetTime();
}

void CMvPeer::SendHelloAck()
{
    SendMessage(MVPROTO_CHN_NETWORK,MVPROTO_CMD_HELLO_ACK);
}

bool CMvPeer::ParseMessageHeader()
{
    try
    {
        ReadStream() >> hdrRecv;
        return (hdrRecv.nMagic == nMsgMagic && hdrRecv.Verify());
    }
    catch (exception& e)
    {
        StdError(__PRETTY_FUNCTION__, e.what());
    }
    return false;
}

bool CMvPeer::HandshakeReadHeader()
{
    if (!ParseMessageHeader())
    {
        return false;
    }

    if (hdrRecv.nPayloadSize != 0)
    {
        Read(hdrRecv.nPayloadSize,boost::bind(&CMvPeer::HandshakeReadCompleted,this));
        return true;
    }
    return HandshakeReadCompleted();
}

bool CMvPeer::HandleReadHeader()
{
    if (!ParseMessageHeader())
    {
        return false;
    }

    if (hdrRecv.nPayloadSize != 0)
    {
        Read(hdrRecv.nPayloadSize,boost::bind(&CMvPeer::HandleReadCompleted,this));
        return true;
    }
    return HandleReadCompleted();
}

bool CMvPeer::HandshakeReadCompleted()
{
    CWalleveBufStream& ss = ReadStream();
    uint256 hash = multiverse::crypto::CryptoHash(ss.GetData(),ss.GetSize());
    if (hdrRecv.nPayloadChecksum == hash.Get32() && hdrRecv.GetChannel() == MVPROTO_CHN_NETWORK)
    {
        int64 nTimeRecv = GetTime();
        int nCmd = hdrRecv.GetCommand();
        try
        {
            if (nCmd == MVPROTO_CMD_HELLO)
            {
                if (nVersion != 0)
                {
                    return false;
                }
                int64 nTime;
                std::vector<unsigned char> macData;
                ss >> nVersion >> nService >> nTime >> nNonceFrom >> strSubVer >> nStartingHeight >> macData;
                
                CMacAddress macAddr(macData);
                macAddress = macAddr;

                nTimeDelta = nTime - nTimeRecv;
                if (!fInBound)
                {
                    nTimeDelta += (nTimeRecv - nTimeHello) / 2;
                    SendHelloAck();
                    return HandshakeCompleted();
                }
                SendHello();
                Read(MESSAGE_HEADER_SIZE,boost::bind(&CMvPeer::HandshakeReadHeader,this));
                return true;
            }
            else if (nCmd == MVPROTO_CMD_HELLO_ACK && fInBound)
            {
                if (nVersion == 0)
                {
                    return false;
                }
                nTimeDelta += (nTimeRecv - nTimeHello) / 2;
                return HandshakeCompleted();
            }
        }
        catch (exception& e)
        {
            StdError(__PRETTY_FUNCTION__, e.what());
        }
    }
    return false;
}

bool CMvPeer::HandshakeCompleted()
{
    if (!(static_cast<CMvPeerNet*>(pPeerNet))->HandlePeerHandshaked(this,nHsTimerId))
    {
        return false;
    }
    nHsTimerId = 0;
    Read(MESSAGE_HEADER_SIZE,boost::bind(&CMvPeer::HandleReadHeader,this)); 
    return true;
}

bool CMvPeer::HandleReadCompleted()
{
    CWalleveBufStream& ss = ReadStream();
    uint256 hash = multiverse::crypto::CryptoHash(ss.GetData(),ss.GetSize());
    if (hdrRecv.nPayloadChecksum == hash.Get32())
    {
        try
        {
            if ((static_cast<CMvPeerNet*>(pPeerNet))->HandlePeerRecvMessage(this,hdrRecv.GetChannel(),hdrRecv.GetCommand(),ss))
            {
                Read(MESSAGE_HEADER_SIZE,boost::bind(&CMvPeer::HandleReadHeader,this));
                return true;
            }
        }
        catch (exception& e)
        {
            StdError(__PRETTY_FUNCTION__, e.what());
        }
    }
    return false;
}

