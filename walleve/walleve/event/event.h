// Copyright (c) 2016-2018 The LoMoCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  WALLEVE_EVENT_H
#define  WALLEVE_EVENT_H

#include "walleve/stream/stream.h"

namespace walleve
{

#define DECLARE_EVENTHANDLER(Type) virtual bool HandleEvent(Type& e) {return HandleEventDefault(e);}
enum
{
    WALLEVE_EVENT_PEER_BASE = (1 << 8),
    WALLEVE_EVENT_HTTP_BASE = (2 << 8),
    WALLEVE_EVENT_USER_BASE = (128 << 8),
};

class CWalleveEvent;
class CWalleveEventListener
{
public:
    virtual ~CWalleveEventListener() {}
    virtual bool DispatchEvent(CWalleveEvent* pEvent);
    virtual bool HandleEventDefault(CWalleveEvent&) {return true;}
    virtual bool HandleEvent(CWalleveEvent&e) {return HandleEventDefault(e);}
};

class CWalleveEvent
{
    friend class CWalleveStream;
public:
    CWalleveEvent(uint64 nNonceIn,int nTypeIn)
        : nNonce(nNonceIn), nType(nTypeIn) {} 
    virtual ~CWalleveEvent() {}
    virtual bool Handle(CWalleveEventListener& listener) {return listener.HandleEvent(*this);}
    virtual void Free() {delete this;}
protected:
    virtual void WalleveSerialize(CWalleveStream& s,SaveType&) {}
    virtual void WalleveSerialize(CWalleveStream& s,LoadType&) {}
    virtual void WalleveSerialize(CWalleveStream& s,std::size_t& serSize) {}
public:
    uint64 nNonce;
    int nType;
};

template <int type,typename L,typename D,typename R>
class CWalleveEventCategory : public CWalleveEvent
{
    friend class CWalleveStream;
public:
    CWalleveEventCategory(uint64 nNonceIn)
        : CWalleveEvent(nNonceIn,type) {}
    virtual ~CWalleveEventCategory() {}
    virtual bool Handle(CWalleveEventListener& listener) 
    {
        try
        {
            return (dynamic_cast<L&>(listener)).HandleEvent(*this);
        }
        catch (std::bad_cast&) 
        {
            return listener.HandleEvent(*this);
        }
        catch (...) {}
        return false;
    }
protected:
    template <typename O>
    void WalleveSerialize(walleve::CWalleveStream& s,O& opt)
    {
        s.Serialize(data,opt);
    }
public:
    D data;
    R result;
};

} // namespace walleve

#endif //WALLEVE_EVENT_H

