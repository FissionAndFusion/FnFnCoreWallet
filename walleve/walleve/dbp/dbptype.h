#ifndef WALLEVE_DBP_TYPE_H
#define WALLEVE_DBP_TYPE_H

namespace walleve{

class CWalleveDbpContent
{
public:
};

class CWalleveDbpRequest: public CWalleveDbpContent
{
public:

};

class CWalleveDbpRespond: public CWalleveDbpContent
{
public:

};

class CWalleveDbpConnect: public CWalleveDbpRequest
{
public:
    bool isReconnect;
    std::string session;
    int32 version;
    std::string client;
};

class CWalleveDbpSub: public CWalleveDbpRequest
{
public:
    std::string id;
    std::string name;
};

class CWalleveDbpUnSub: public CWalleveDbpRequest
{
public:
    std::string id;
};

class CWalleveDbpNoSub: public CWalleveDbpRespond
{
public:
    std::string id;
    std::string error;
};

class CWalleveDbpReady: public CWalleveDbpRespond
{
public:
    std::string id;    
};
class CWalleveDbpMethod: public CWalleveDbpRequest
{
public:
    // param name => param value
    typedef std::map<std::string,std::string> ParamMap;
public:
    std::string method;
    std::string id;
    ParamMap params;
};

class CWalleveDbpError: public CWalleveDbpRespond
{
public:

};

class CWalleveDbpConnected: public CWalleveDbpRespond
{
public:
    std::string session;
};

class CWalleveDbpFailed: public CWalleveDbpRespond
{
public:
    std::string reason; // failed reason
    std::string session; // for delete session map
    std::vector<int32> versions;
};

class CWalleveDbpBroken
{
public:
    bool fEventStream;
};
} // namespace walleve
#endif//WALLEVE_DBP_TYPE_H