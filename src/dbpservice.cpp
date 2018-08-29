#include "dbpservice.h"

#include <boost/assign/list_of.hpp>

using namespace multiverse;

CDbpService::CDbpService()
:walleve::IIOModule("dbpservice")
{
    pService = NULL;
    pDbpServer = NULL;

    std::map<std::string,bool> temp_map = boost::assign::map_list_of
                    ("added",true)
                    ("changed",true)
                    ("removed",true);
    
    currentTopicExistMap = temp_map;
}

CDbpService::~CDbpService()
{

}


bool CDbpService::WalleveHandleInitialize()
{
    if (!WalleveGetObject("service",pService))
    {
        WalleveLog("Failed to request service\n");
        return false;
    }

    if (!WalleveGetObject("dbpserver",pDbpServer))
    {
        WalleveLog("Failed to request dbpserver\n");
        return false;
    }

    return true;
}

void CDbpService::WalleveHandleDeinitialize()
{
    pDbpServer = NULL;
    pService = NULL;
}

bool CDbpService::HandleEvent(walleve::CWalleveEventDbpConnect& event)
{
    (void)event.data.client;

    if(event.data.version != 1)
    {
        // reply failed
        uint64 nonce = event.nNonce;
        std::vector<int> versions{1};
        walleve::CWalleveEventDbpFailed eventFailed(nonce);
        eventFailed.data.versions = versions;
        eventFailed.data.session  = event.data.session;
        pDbpServer->DispatchEvent(&eventFailed);
    }
    else
    {
        // reply normal
        uint64 nonce = event.nNonce;
        walleve::CWalleveEventDbpConnected eventConnected(nonce);
        eventConnected.data.session = event.data.session;
        pDbpServer->DispatchEvent(&eventConnected);
    }

    return true;
}

bool CDbpService::HandleEvent(walleve::CWalleveEventDbpSub& event)
{
    std::string id = event.data.id;
    std::string topicName = event.data.name;

    // if topic not exists
    if(currentTopicExistMap.count(topicName) == 0 )
    {
         // reply nosub
        uint64 nonce = event.nNonce;
        walleve::CWalleveEventDbpNoSub eventNoSub(nonce);
        eventNoSub.data.id = event.data.id;
        pDbpServer->DispatchEvent(&eventNoSub);
    }
    else
    {
        if(idSubedTopicsMap.count(id) == 0)
        {
            TopicSet topics{topicName};
            idSubedTopicsMap.insert(std::make_pair(id,topics));
        }
        else
        {
            auto & topics = idSubedTopicsMap[id];
            topics.insert(topicName);
        }

        idSubedNonceMap.insert(std::make_pair(id,event.nNonce));

        //reply ready
        uint64 nonce = event.nNonce;
        walleve::CWalleveEventDbpReady eventReady(nonce);
        eventReady.data.id = event.data.id;
        pDbpServer->DispatchEvent(&eventReady);
    }
    
    return true;
}

bool CDbpService::HandleEvent(walleve::CWalleveEventDbpUnSub& event)
{
    std::string id = event.data.id;    
    if(idSubedTopicsMap.count(id) != 0)
    {
        // unsub is actual delete subed topic
        idSubedTopicsMap.erase(id);
    }

    if(idSubedNonceMap.count(id) != 0)
    {
        idSubedNonceMap.erase(id);
    }
    
    return true;
}

bool CDbpService::HandleEvent(walleve::CWalleveEventDbpMethod& event)
{
    return true;
}