// Copyright (c) 2016-2018 The LoMoCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  WALLEVE_HTTPCOOKIE_H
#define  WALLEVE_HTTPCOOKIE_H

#include <map>
#include <string>
#include <boost/algorithm/string.hpp>
#include <boost/date_time.hpp>

namespace walleve
{

class CHttpCookie
{
public:
    static const boost::posix_time::ptime INVALID_PTIME;
    CHttpCookie();
    CHttpCookie(const std::string& strFromResp);
    CHttpCookie(const std::string& strNameIn,const std::string& strValueIn,
                const std::string& strDomainIn = "",
                const std::string& strPathIn = "",
                const boost::posix_time::ptime& ptExpiresIn = INVALID_PTIME,
                bool fSecureIn = false, bool fHttpOnlyIn = false);
    ~CHttpCookie();
    bool IsNull();
    bool IsPersistent();
    bool LoadFromResp(const std::string& strFromResp);
    void Delete();
    const std::string BuildSetCookie();
protected:
    void SetNull();
public:
    std::string strName;
    std::string strValue;
    std::string strDomain;
    std::string strPath;
    boost::posix_time::ptime ptExpires;
    bool fSecure;
    bool fHttpOnly;
};

} // namespace walleve
#endif //WALLEVE_HTTPCOOKIE_H

