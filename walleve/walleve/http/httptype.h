// Copyright (c) 2016-2018 The LoMoCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  WALLEVE_HTTP_TYPE_H
#define  WALLEVE_HTTP_TYPE_H

#include "walleve/http/httpcookie.h"
#include "walleve/stream/stream.h"

#include <string.h>
#include <vector>
#include <map>

namespace walleve
{

class _iless
{
public:
    bool operator() (const std::string& a,const std::string& b) const
    {
        return (strcasecmp(a.c_str(),b.c_str()) < 0);
    }
};

typedef std::map<std::string,std::string> MAPKeyValue;
typedef std::map<std::string,std::string,_iless> MAPIKeyValue;
typedef std::map<std::string,CHttpCookie,_iless> MAPCookie;

class CWalleveHttpContent
{
    friend class CWalleveStream;
protected:
    void WalleveSerialize(CWalleveStream& s,SaveType&)
    {
        s.Write(&strContent[0],strContent.size());
    }
    void WalleveSerialize(CWalleveStream& s,LoadType&)
    {
        std::size_t size = s.GetSize();
        strContent.resize(size);
        s.Read(&strContent[0],size);
    }
    void WalleveSerialize(CWalleveStream& s,std::size_t& serSize)
    {
        (void)s;
        serSize += strContent.size();
    }
public:
    std::string strContentType;
    std::string strContent;
};

class CWalleveHttpReq : public CWalleveHttpContent
{
public:
    std::string strUser;
    MAPIKeyValue mapHeader;
    MAPKeyValue mapQuery;
    MAPIKeyValue mapCookie;
    int64 nTimeout;
};

class CWalleveHttpGet : public CWalleveHttpReq
{
public:
    std::string strIOModule;
    std::string strProtocol;
    std::string strPathCA;
    std::string strPathCert;
    std::string strPathPK;
    bool fVerifyPeer;
};

class CWalleveHttpRsp : public CWalleveHttpContent
{
public:
    MAPIKeyValue mapHeader;
    MAPCookie mapCookie;
    int nStatusCode;
};

class CWalleveHttpAbort
{
public:
    std::string strIOModule;
    std::vector<uint64> vNonce;
};

class CWalleveHttpBroken
{
public:
    bool fEventStream;
};

} // namespace walleve

#endif //WALLEVE_HTTP_TYPE_H

