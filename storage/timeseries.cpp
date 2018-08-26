// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "timeseries.h"

using namespace std;
using namespace boost::filesystem;
using namespace multiverse::storage;

//////////////////////////////
// CTimeSeries

const uint32 CTimeSeries::nMagicNum = 0x5E33A1EF;

CTimeSeries::CTimeSeries()
: cacheStream(FILE_CACHE_SIZE)
{
    nLastFile = 0;
}

CTimeSeries::~CTimeSeries()
{
}

bool CTimeSeries::Initialize(const path& pathLocationIn,const string& strPrefixIn)
{
    boost::unique_lock<boost::mutex> lock(mtxFile);

    if (!exists(pathLocationIn))
    {
        create_directories(pathLocationIn);
    }

    if (!is_directory(pathLocationIn))
    {
        return false;
    }
    pathLocation = pathLocationIn;
    strPrefix = strPrefixIn;
    nLastFile = 1;

    ResetCache();

    return CheckDiskSpace();
}

void CTimeSeries::Deinitialize()
{
    boost::unique_lock<boost::mutex> lock(mtxFile);

    ResetCache();
}

bool CTimeSeries::CheckDiskSpace()
{
    // 15M
    return (space(pathLocation).available > 15000000);
}

const std::string CTimeSeries::FileName(uint32 nFile)
{
    ostringstream oss;
    oss << strPrefix << "_" << setfill('0') << setw(4) << nFile << ".dat";
    return oss.str();
}

bool CTimeSeries::GetFilePath(uint32 nFile,string& strPath)
{
    path current = pathLocation / FileName(nFile);
    if (exists(current) && is_regular_file(current))
    {
        strPath = current.string();
        return true;
    }
    return false;
}

bool CTimeSeries::GetLastFilePath(uint32& nFile,std::string& strPath)
{
    for (;;)
    {
        path last = pathLocation / FileName(nLastFile);
        if (!exists(last))
        {
            FILE * fp = fopen(last.string().c_str(),"w+");
            if (fp == NULL)
            {
                break;
            }
            fclose(fp);
        }
        if (is_regular_file(last) && file_size(last) < MAX_FILE_SIZE - MAX_CHUNK_SIZE - 8)
        {
            nFile = nLastFile;
            strPath = last.string();
            return true;
        }
        nLastFile++;
    }
    return false;
}

void CTimeSeries::ResetCache()
{
    cacheStream.Clear();
    mapCachePos.clear(); 
}

bool CTimeSeries::VacateCache(uint32 nNeeded)
{
    const size_t nHdrSize = 12;

    while(cacheStream.GetBufFreeSpace() < nNeeded + nHdrSize)
    {
        CDiskPos diskpos;
        uint32 nSize = 0;

        try
        {
            cacheStream.Rewind();
            cacheStream >> diskpos >> nSize;
        }
        catch (...)
        {   
            return false;
        }

        mapCachePos.erase(diskpos); 
        cacheStream.Consume(nSize + nHdrSize);
    }
    return true;
}
