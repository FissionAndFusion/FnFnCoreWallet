#ifndef  WALLEVE_DBP_EVENT_H
#define  WALLEVE_DBP_EVENT_H

#include "walleve/event/event.h"
#include "walleve/dbp/dbptype.h"

namespace walleve{

enum {
    WALLEVE_EVENT_DBP_REQ,
    WALLEVE_EVENT_DBP_RSP,
    WALLEVE_EVENT_DBP_CONNECT,
    WALLEVE_EVENT_DBP_SUB,
    WALLEVE_EVENT_DBP_METHOD
};

class CWalleveDBPEventListener;


#define TYPE_DBPEVENT(type,body) 	\
	CWalleveEventCategory<type,CWalleveDBPEventListener,body,bool>

typedef TYPE_DBPEVENT(WALLEVE_EVENT_DBP_REQ,CWalleveDbpRequest) CWalleveEventDbpRequest;
typedef TYPE_DBPEVENT(WALLEVE_EVENT_DBP_RSP,CWalleveDbpRespond) CWalleveEventDbpRespond;
typedef TYPE_DBPEVENT(WALLEVE_EVENT_DBP_CONNECT,CWalleveDbpConnect) CWalleveEventDbpConnect;
typedef TYPE_DBPEVENT(WALLEVE_EVENT_DBP_SUB,CWalleveDbpSub) CWalleveEventDbpSub;
typedef TYPE_DBPEVENT(WALLEVE_EVENT_DBP_METHOD,CWalleveDbpMethod) CWalleveEventDbpMethod;


class CWalleveDBPEventListener : virtual public CWalleveEventListener
{
public:
    virtual ~CWalleveDBPEventListener() {}
    DECLARE_EVENTHANDLER(CWalleveEventDbpRequest);
    DECLARE_EVENTHANDLER(CWalleveEventDbpRespond);
    DECLARE_EVENTHANDLER(CWalleveEventDbpConnect);
    DECLARE_EVENTHANDLER(CWalleveEventDbpSub);
    DECLARE_EVENTHANDLER(CWalleveEventDbpMethod);
};

}// namespace walleve
#endif // WALLEVE_DBP_EVENT_H