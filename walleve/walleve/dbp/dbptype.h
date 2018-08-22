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
};

class CWalleveDbpSub: public CWalleveDbpRequest
{
public:

};

class CWalleveDbpMethod: public CWalleveDbpRequest
{
public:

};

class CWalleveDbpBroken
{
public:
    bool fEventStream;
};
} // namespace walleve
#endif//WALLEVE_DBP_TYPE_H