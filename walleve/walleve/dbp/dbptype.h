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

class CWalleveDbpMethod: public CWalleveDbpRequest
{
public:
    std::string method;
    std::string id;
};

class CWalleveDbpBroken
{
public:
    bool fEventStream;
};
} // namespace walleve
#endif//WALLEVE_DBP_TYPE_H