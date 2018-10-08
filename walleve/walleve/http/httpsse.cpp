// Copyright (c) 2016-2018 The LoMoCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "httpsse.h"
#include "walleve/util.h"
#include <string.h>
#include <boost/foreach.hpp>

using namespace std;
using namespace boost;
using namespace walleve;

///////////////////////////////
// CHttpEventStream

CHttpEventStream::CHttpEventStream()
: strEntry(""),nEventId(0)  
{
}

CHttpEventStream::CHttpEventStream(const std::string& strEntryIn)
: strEntry(strEntryIn),nEventId(0)
{
}

CHttpEventStream::CHttpEventStream(const CHttpEventStream& es)
: strEntry(es.strEntry),nEventId(es.nEventId)
{
}

CHttpEventStream::~CHttpEventStream()
{
}

const string& CHttpEventStream::GetEntry()
{
    return strEntry;
}

void CHttpEventStream::RegisterEvent(const string& strEventName,CHttpSSEGenerator *pGenerator)
{
    boost::unique_lock<boost::mutex> lock(mtxEvent);
    auto it = mapGenerator.find(strEventName);
    if (it != mapGenerator.end())
    {
       it->second = std::unique_ptr<CHttpSSEGenerator>(pGenerator);
    }
    else
    {
        mapGenerator.insert(std::make_pair(strEventName,std::unique_ptr<CHttpSSEGenerator>(pGenerator)));
    }
}

void CHttpEventStream::UnregisterEvent(const string& strEventName)
{
    boost::unique_lock<boost::mutex> lock(mtxEvent);
    mapGenerator.erase(strEventName);
}

void CHttpEventStream::ResetData(const std::string& strEventName)
{
    boost::unique_lock<boost::mutex> lock(mtxEvent);
    auto it = mapGenerator.find(strEventName);
    if (it != mapGenerator.end())
    {
        (*it).second->ResetData();
    }
}

bool CHttpEventStream::UpdateEventData(const string& strEventName,CHttpSSEData& data)
{
    boost::unique_lock<boost::mutex> lock(mtxEvent);
    auto it = mapGenerator.find(strEventName);
    if (it != mapGenerator.end())
    {
        if ((*it).second->UpdateData(data,nEventId + 1)) 
        {
            nEventId++;
            return true;
        }
    }
    return false;
}

bool CHttpEventStream::ConstructResponse(uint64 nLastEventId,CWalleveHttpRsp& rsp)
{
    boost::unique_lock<boost::mutex> lock(mtxEvent);

    if (nLastEventId >= nEventId)
    {
        return false;
    }

    rsp.nStatusCode = 200;
    rsp.mapHeader["content-type"] = "text/event-stream";
    rsp.mapHeader["connection"] = "Keep-Alive";
    
    ostringstream oss;
    for (auto it = mapGenerator.begin();
         it != mapGenerator.end();++it)
    {
        vector<string> vEventData;
        (*it).second->GenerateEventData(nLastEventId,nEventId,vEventData);
        BOOST_FOREACH(const string& strData,vEventData)
        {
            oss << "event: " << (*it).first << "\ndata: " << strData << "\n\n"; 
        }
    }

    oss << "id: " << nEventId << "\ndata: " << GetTime() << "\n\n";
    rsp.strContent = oss.str();
    return true;
}
