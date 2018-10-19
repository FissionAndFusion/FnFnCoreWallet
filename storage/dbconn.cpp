// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "dbconn.h"

using namespace std;
using namespace multiverse::storage;

//////////////////////////////
// CMySQLLibrary
class CMySQLLibrary
{
public:
    CMySQLLibrary()
    {
        mysql_library_init(0,NULL,NULL);
    }
    ~CMySQLLibrary()
    {
        mysql_library_end();
    }
};

static CMySQLLibrary _myLibrary;

//////////////////////////////
// CMvDBConn

CMvDBConn::CMvDBConn()
{
    mysql_init(&dbConn);
}

CMvDBConn::~CMvDBConn()
{
    mysql_close(&dbConn);
}

bool CMvDBConn::Connect(const CMvDBConfig& config)
{
    boost::unique_lock<boost::mutex> lock(mtxConn);

    bool reconnect = 0;
    mysql_options(&dbConn, MYSQL_OPT_RECONNECT, &reconnect);
    return (mysql_real_connect(&dbConn,config.strHost.c_str(),config.strUser.c_str(),config.strPass.c_str(),
                                       config.strDBName.c_str(),config.nPort,NULL,0) != NULL);
}

void CMvDBConn::Disconnect()
{
    boost::unique_lock<boost::mutex> lock(mtxConn);
    mysql_close(&dbConn);
}

void CMvDBConn::Reset()
{
    boost::unique_lock<boost::mutex> lock(mtxConn);
    mysql_reset_connection(&dbConn); 
}

bool CMvDBConn::Query(const string& strQuery)
{
    boost::unique_lock<boost::mutex> lock(mtxConn);
    return (!mysql_real_query(&dbConn,strQuery.c_str(),strQuery.size()));
}

string CMvDBConn::ToEscString(const void* pBinary,size_t nBytes)
{
    char s[nBytes * 2 + 1];
    return string(s, mysql_real_escape_string(&dbConn,s,(const char*)pBinary,nBytes));
}

//////////////////////////////
// CMvDBTxn

CMvDBTxn::CMvDBTxn(CMvDBConn* pDBConn)
: CMvDBExclusive(pDBConn), fCompleted(false)
{
    mysql_autocommit(&dbConn, (bool)0);
}

CMvDBTxn::CMvDBTxn(CMvDBConn& dbConnIn)
: CMvDBExclusive(dbConnIn), fCompleted(false)
{
    mysql_autocommit(&dbConn, (bool)0);
}

CMvDBTxn::~CMvDBTxn()
{
    if (!fCompleted)
    {
        Abort();
    }
    mysql_autocommit(&dbConn, (bool)1);
}

bool CMvDBTxn::Query(const string& strQuery)
{
    return (!mysql_real_query(&dbConn,strQuery.c_str(),strQuery.size()));
}

bool CMvDBTxn::Commit()
{
    return (fCompleted = (!mysql_commit(&dbConn)));
}

void CMvDBTxn::Abort()
{
    fCompleted = true;
    mysql_rollback(&dbConn);
}

//////////////////////////////
// CMvDBRes

CMvDBRes::CMvDBRes(CMvDBConn* pDBConn,const string& strQuery,bool fUseResult)
: CMvDBExclusive(pDBConn), pResult(NULL), pLength(NULL), rowData(NULL)
{
    Execute(strQuery,fUseResult);
}

CMvDBRes::CMvDBRes(CMvDBConn& dbConnIn,const string& strQuery,bool fUseResult)
: CMvDBExclusive(dbConnIn), pResult(NULL), pLength(NULL), rowData(NULL)
{
    Execute(strQuery,fUseResult);
}

CMvDBRes::~CMvDBRes()
{
    if (pResult)
    {
        mysql_free_result(pResult);
    }
}

void CMvDBRes::Execute(const string& strQuery,bool fUseResult)
{
    if (!mysql_real_query(&dbConn,strQuery.c_str(),strQuery.size()))
    {
        pResult = fUseResult ? mysql_use_result(&dbConn) : mysql_store_result(&dbConn);
        if (pResult != NULL)
        {
            nField = mysql_num_fields(pResult);
        }
    }
}

int CMvDBRes::GetCount()
{
    return (!pResult ? -1 : (int)mysql_num_rows(pResult));
}

bool CMvDBRes::GetRow()
{
    if (pResult != NULL && (rowData = mysql_fetch_row(pResult)) != NULL)
    {
        pLength = mysql_fetch_lengths(pResult);
        return true;
    }
    return false;
}
