// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_DBCONN_H
#define  MULTIVERSE_DBCONN_H

#include <mysql.h>
#include <boost/thread/thread.hpp>
#include "uint256.h"
#include "destination.h"

namespace multiverse
{
namespace storage
{

class CMvDBExclusive;

class CMvDBConfig
{
public:
    CMvDBConfig() {}
    CMvDBConfig(const std::string& strHostIn,int nPortIn,const std::string& strDBNameIn,
                const std::string& strUserIn,const std::string& strPassIn)
    : strHost(strHostIn),strDBName(strDBNameIn),strUser(strUserIn),strPass(strPassIn),nPort(nPortIn) {}
public:
    std::string strHost;
    std::string strDBName;
    std::string strUser;
    std::string strPass;
    int nPort;
};

class CMvDBConn
{
    friend class CMvDBExclusive;
public:
    CMvDBConn();
    ~CMvDBConn();
    bool Connect(const CMvDBConfig& config);
    void Disconnect();
    void Reset();
    
    bool Query(const std::string& strQuery);
    std::string ToEscString(const std::string& str);
    std::string ToEscString(const void* pBinary,std::size_t nBytes);
    std::string ToEscString(const uint256& hash)
    {
        return ToEscString(hash.begin(),32);
    }
    std::string ToEscString(const CDestination& dest)
    {
        unsigned char d[33];
        d[0] = dest.prefix;
        *(uint256*)&d[1] = dest.data;
        return ToEscString(&d,33);
    }
    std::string ToEscString(const std::vector<unsigned char>& vch)
    {
        return ToEscString(&vch[0],vch.size());
    }
protected:
    MYSQL dbConn;
    boost::mutex mtxConn;
};

class CMvDBExclusive
{
public:
    CMvDBExclusive(CMvDBConn* pDBConn) : dbConn(pDBConn->dbConn), lock(pDBConn->mtxConn) {}
    CMvDBExclusive(CMvDBConn& dbConnIn) : dbConn(dbConnIn.dbConn), lock(dbConnIn.mtxConn) {}
protected:
    MYSQL& dbConn;
    boost::unique_lock<boost::mutex> lock;
};

class CMvDBTxn : public CMvDBExclusive 
{
public:
    CMvDBTxn(CMvDBConn* pDBConn);
    CMvDBTxn(CMvDBConn& dbConnIn);
    ~CMvDBTxn();
    bool Query(const std::string& strQuery);
    bool Commit();
    void Abort(); 
protected:
    bool fCompleted;
};

class CMvDBRes : public CMvDBExclusive
{
public:
    CMvDBRes(CMvDBConn* pDBConn,const std::string& strQuery,bool fUseResult=false);
    CMvDBRes(CMvDBConn& dbConnIn,const std::string& strQuery,bool fUseResult=false);
    ~CMvDBRes();
    int GetCount();
    bool GetRow();
    template <typename T>
    bool GetField(int idx,T& t)
    {
        if (idx < nField && rowData[idx] != NULL)
        {
            try
            {
                std::stringstream ss(std::string(rowData[idx],pLength[idx]));
                if (sizeof(T) == 1)
                {
                   int i;
                   ss >> i;
                   t = (T)i;
                }
                else
                {
                    ss >> t;
                }
                return true;
            }
            catch (std::exception& e)
            {
                walleve::StdError(__PRETTY_FUNCTION__, e.what());
            }
        } 
        return false;
    }
    bool GetField(int idx,std::string& str)
    {
        if (idx < nField && rowData[idx] != NULL)
        {
            str = std::string(rowData[idx],pLength[idx]);
            return true;
        }
        return false;
    }
    bool GetField(int idx,std::vector<unsigned char>& vch)
    {
        if (idx < nField && rowData[idx] != NULL)
        {
            try
            {
                vch.assign(rowData[idx],rowData[idx] + pLength[idx]);
                return true;
            }
            catch (std::exception& e)
            {
                walleve::StdError(__PRETTY_FUNCTION__, e.what());
            }
        }
        return false;
    }
    bool GetField(int idx,uint256& hash)
    {
        if (idx < nField && rowData[idx] != NULL && pLength[idx] == 32)
        {
            hash = *((uint256* )rowData[idx]);
            return true;
        }
        return false;
    }
    bool GetField(int idx,CDestination& dest)
    {
        if (idx < nField && rowData[idx] != NULL && pLength[idx] == 33)
        {
            const unsigned char* p = (const unsigned char*)rowData[idx];
            dest.prefix = *p++;
            dest.data   = *((uint256* )p);
            return true;
        }
        return false;
    }
    bool GetBinary(int idx,unsigned char* data,std::size_t n)
    {
        if (idx < nField && rowData[idx] != NULL && pLength[idx] == n)
        {
            const unsigned char* p = (const unsigned char*)rowData[idx];
            while (n--)
            {
                *data++ = *p++;
            }
            return true;
        }
        return false;
    }
protected:
    void Execute(const std::string& strQuery,bool fUseResult);
protected:
    MYSQL_RES* pResult;
    unsigned int nField;
    unsigned long* pLength;
    MYSQL_ROW rowData;
};

} // namespace storage
} // namespace multiverse

#endif //MULTIVERSE_DBCONN_H

