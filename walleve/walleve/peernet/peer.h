// Copyright (c) 2016-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  WALLEVE_PEER_H
#define  WALLEVE_PEER_H

#include "walleve/netio/ioproc.h"
#include "walleve/stream/stream.h"
#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <boost/function.hpp>

namespace walleve
{

class CPeerNet;

class CPeer
{
public:
    typedef boost::function<bool(bool&)> CompltFunc;

    CPeer(CPeerNet *pPeerNetIn, CIOClient* pClientIn,uint64 nNonceIn,bool fInBoundIn);
    virtual ~CPeer();
    uint64 GetNonce();
    bool IsInBound();
    bool IsWriteable();
    const boost::asio::ip::tcp::endpoint GetRemote();
    const boost::asio::ip::tcp::endpoint GetLocal();
    virtual void Activate();
protected:
    CWalleveBufStream& ReadStream();
    CWalleveBufStream& WriteStream();

    void Read(std::size_t nLength,CompltFunc fnComplt);
    void Write();

    void HandleRead(std::size_t nTransferred,CompltFunc fnComplt);
    void HandleWriten(std::size_t nTransferred);
public:
    int64 nTimeActive;
    int64 nTimeRecv;
    int64 nTimeSend;
protected:
    CPeerNet *pPeerNet; 
    CIOClient* pClient;
    uint64 nNonce;
    bool fInBound;

    CWalleveBufStream ssRecv;
    CWalleveBufStream ssSend[2];
    int indexStream;
    int indexWrite;
};

} // namespace walleve

#endif //WALLEVE_PEER_H


