// Copyright (c) 2016-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  WALLEVE_NETIO_H
#define  WALLEVE_NETIO_H

#include "walleve/event/eventproc.h"
namespace walleve
{

class IIOProc : public IWalleveBase
{   
public:
    IIOProc(const std::string& walleveOwnKeyIn) : IWalleveBase(walleveOwnKeyIn) {}
    virtual bool DispatchEvent(CWalleveEvent* pEvent) = 0;
};

class IIOModule : public CWalleveEventProc 
{
public:
    IIOModule(const std::string& walleveOwnKeyIn, const size_t nThreadIn = 1) : CWalleveEventProc(walleveOwnKeyIn, nThreadIn) {}
};

} // namespace walleve

#endif //WALLEVE_NETIO_H

