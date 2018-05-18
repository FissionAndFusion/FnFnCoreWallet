// Copyright (c) 2016-2018 The LoMoCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  WALLEVE_HTTP_EVENT_H
#define  WALLEVE_HTTP_EVENT_H

#include "walleve/event/event.h"
#include "walleve/http/httptype.h"

namespace walleve
{

enum
{
    WALLEVE_EVENT_HTTP_IDLE = WALLEVE_EVENT_HTTP_BASE,
    WALLEVE_EVENT_HTTP_REQ,
    WALLEVE_EVENT_HTTP_RSP,
    WALLEVE_EVENT_HTTP_GET,
    WALLEVE_EVENT_HTTP_GETRSP,
    WALLEVE_EVENT_HTTP_ABORT,
    WALLEVE_EVENT_HTTP_BROKEN,
};

class CWalleveHttpEventListener;

#define TYPE_HTTPEVENT(type,body) 	\
	CWalleveEventCategory<type,CWalleveHttpEventListener,body,bool>

typedef TYPE_HTTPEVENT(WALLEVE_EVENT_HTTP_REQ,CWalleveHttpReq) CWalleveEventHttpReq;
typedef TYPE_HTTPEVENT(WALLEVE_EVENT_HTTP_RSP,CWalleveHttpRsp) CWalleveEventHttpRsp;
typedef TYPE_HTTPEVENT(WALLEVE_EVENT_HTTP_GET,CWalleveHttpGet) CWalleveEventHttpGet;
typedef TYPE_HTTPEVENT(WALLEVE_EVENT_HTTP_GETRSP,CWalleveHttpRsp) CWalleveEventHttpGetRsp;
typedef TYPE_HTTPEVENT(WALLEVE_EVENT_HTTP_ABORT,CWalleveHttpAbort) CWalleveEventHttpAbort;
typedef TYPE_HTTPEVENT(WALLEVE_EVENT_HTTP_BROKEN,CWalleveHttpBroken) CWalleveEventHttpBroken;

class CWalleveHttpEventListener : virtual public CWalleveEventListener
{
public:
    virtual ~CWalleveHttpEventListener() {}
    DECLARE_EVENTHANDLER(CWalleveEventHttpReq);
    DECLARE_EVENTHANDLER(CWalleveEventHttpRsp);
    DECLARE_EVENTHANDLER(CWalleveEventHttpAbort);
    DECLARE_EVENTHANDLER(CWalleveEventHttpGet);
    DECLARE_EVENTHANDLER(CWalleveEventHttpGetRsp);
    DECLARE_EVENTHANDLER(CWalleveEventHttpBroken);
};

} // namespace walleve

#endif //WALLEVE_HTTP_EVENT_H

