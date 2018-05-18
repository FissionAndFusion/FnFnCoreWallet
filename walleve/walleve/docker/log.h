// Copyright (c) 2016-2018 The LoMoCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  WALLEVE_LOG_H
#define  WALLEVE_LOG_H

#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <sstream>
#include <locale>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/date_time.hpp>

namespace walleve
{

class CWalleveLog
{
public:
    CWalleveLog() 
    : pFile(NULL),fNewLine(false) 
    {
    }
    
    ~CWalleveLog()
    {
        if (pFile != NULL)
        {
            fclose(pFile);
        }
    }
    
    bool SetLogFilePath(const std::string& strPathLog)
    {
        pFile = fopen(strPathLog.c_str(), "a");
        fNewLine = true;
        return (pFile != NULL);
    }

    virtual void operator()(const char *walleveKey,const char *strPrefix,const char *pszFormat,va_list ap)
    {
        if (pFile != NULL)
        {
            boost::mutex::scoped_lock scoped_lock(mutex);
            if (fNewLine && pszFormat[0] != '\n')
            {
                fprintf(pFile,"%s <%s> %s",GetLocalTime().c_str(),walleveKey,strPrefix);
            }

            fNewLine = (pszFormat[strlen(pszFormat) - 1] == '\n');

            vfprintf(pFile, pszFormat, ap);
            fflush(pFile);
        }
    }

protected:
    std::string GetLocalTime()
    {
        using namespace boost::posix_time;
        time_facet *facet = new time_facet("%Y-%m-%d %H:%M:%S");
        std::stringstream ss;
        ss.imbue(std::locale(std::locale("C"), facet));
        ss << second_clock::universal_time();
        return ss.str();
    }
protected:
    FILE *pFile;
    boost::mutex mutex;
    bool fNewLine;
};

} // namespace walleve

#endif //WALLEVE_LOG_H

