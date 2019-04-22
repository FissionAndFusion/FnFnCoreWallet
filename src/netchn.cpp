// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "netchn.h"
#include "schedule.h"
#include "virtualpeernet.h"
#include <boost/bind.hpp>
#include <limits>

using namespace std;
using namespace walleve;
using namespace multiverse;
using boost::asio::ip::tcp;

#define PUSHTX_TIMEOUT		(1000)

//////////////////////////////
// CConcurrentPeerNetData

bool CConcurrentPeerNetData::IsForkSynchronized(const uint256& hashFork) const
{
    ReadLocker lock(rwUnsync);
    map<uint256, set<uint64> >::const_iterator it = mapUnsync.find(hashFork);
    return (it == mapUnsync.end() || (*it).second.empty());
}

 std::vector<std::pair<uint64, CNetChannelPeer>> CConcurrentPeerNetData::KeyValues() const
 {
    std::vector<std::pair<uint64, CNetChannelPeer>> keyValues;
    {
        ReadLocker lock(rwPeer);
        for (map<uint64,CNetChannelPeer>::const_iterator it = mapPeer.begin();it != mapPeer.end();++it)
        {
            keyValues.push_back(std::make_pair(it->first, it->second));
        }
    }
    return keyValues;
 }

 void CConcurrentPeerNetData::ActivePeer(uint64 nNonce, uint64 nService, const uint256& forkHash)
 {
    WriteLocker wlockPeer(rwPeer, boost::defer_lock);
    WriteLocker wlockUnSync(rwUnsync, boost::defer_lock);
    boost::lock(wlockPeer, wlockUnSync);

    mapPeer[nNonce] = CNetChannelPeer(nService,forkHash);
    mapUnsync[forkHash].insert(nNonce);
 }

 void CConcurrentPeerNetData::DeactivePeer(uint64 nNonce)
 {
    WriteLocker wlockPeer(rwPeer, boost::defer_lock);
    WriteLocker wlockUnSync(rwUnsync, boost::defer_lock);
    boost::lock(wlockPeer, wlockUnSync);

    map<uint64,CNetChannelPeer>::iterator it = mapPeer.find(nNonce);
    if (it != mapPeer.end())
    {
        for (auto& subFork: (*it).second.mapSubscribedFork)
        {
            mapUnsync[subFork.first].erase(nNonce);
        }
        mapPeer.erase(nNonce);
    }
 }

void CConcurrentPeerNetData::SubscribeForks(uint64 nNonce, const std::vector<uint256>& hashForks)
{
    WriteLocker wlockPeer(rwPeer, boost::defer_lock);
    WriteLocker wlockUnSync(rwUnsync, boost::defer_lock);
    boost::lock(wlockPeer, wlockUnSync);

    auto iter = mapPeer.find(nNonce);
    if(iter != mapPeer.end())
    {
        for(const uint256& hash : hashForks)
        {
            iter->second.Subscribe(hash);
            mapUnsync[hash].insert(nNonce);
        }
    }
}

void CConcurrentPeerNetData::UnsubscribeForks(uint64 nNonce, const std::vector<uint256>& hashForks)
{
    WriteLocker wlockPeer(rwPeer, boost::defer_lock);
    WriteLocker wlockUnSync(rwUnsync, boost::defer_lock);
    boost::lock(wlockPeer, wlockUnSync);

    auto iter = mapPeer.find(nNonce);
    if(iter != mapPeer.end())
    {
        for(const uint256& hash : hashForks)
        {
            iter->second.Unsubscribe(hash);
            mapUnsync[hash].erase(nNonce);
        }       
    }
}

void CConcurrentPeerNetData::AddKnownTx(uint64 nNonce, const uint256& hashFork, const std::vector<uint256>& vTxHash)
{
    WriteLocker wlockPeer(rwPeer);
    mapPeer[nNonce].AddKnownTx(hashFork,vTxHash);
}

bool CConcurrentPeerNetData::SetPeerSyncStatus(uint64 nNonce, const uint256& hashFork, bool fSync, bool& fInverted)
{
    WriteLocker wlockPeer(rwPeer);
    CNetChannelPeer& peer = mapPeer[nNonce];
    return peer.SetSyncStatus(hashFork,fSync,fInverted);
}

void CConcurrentPeerNetData::DeletePeerUnSyncByFork(uint64 nNonce, const uint256& hashFork)
{
    WriteLocker wlockUnSync(rwUnsync);
    mapUnsync[hashFork].erase(nNonce);
}

void CConcurrentPeerNetData::InsertPeerUnSyncByFork(uint64 nNonce, const uint256& hashFork)
{
    WriteLocker wlockUnSync(rwUnsync);
    mapUnsync[hashFork].insert(nNonce);
}

bool CConcurrentPeerNetData::IsPeerEmpty()
{
    ReadLocker rlockPeer(rwPeer);
    return mapPeer.empty();
}

void CConcurrentPeerNetData::MakeTxInvByFork(const uint256& hashFork, const std::vector<uint256>& vTxPool, std::vector<std::pair<uint64,VecInv>>& InvData)
{
    WriteLocker wlockPeer(rwPeer);
    for (map<uint64,CNetChannelPeer>::iterator it = mapPeer.begin();it != mapPeer.end();++it)
    {
        CNetChannelPeer& peer = (*it).second;
        if (peer.IsSubscribed(hashFork))
        {
            VecInv invs;
            peer.MakeTxInv(hashFork,vTxPool,invs,network::CInv::MAX_INV_COUNT);
            InvData.push_back(std::make_pair(it->first, invs));
        }
    }
}

//////////////////////////////
// CNetChannelPeer
void CNetChannelPeer::CNetChannelPeerFork::AddKnownTx(const vector<uint256>& vTxHash)
{
    ClearExpiredTx(vTxHash.size());
    for(const uint256& txid : vTxHash)
    {
        setKnownTx.insert(CPeerKnownTx(txid));
    }
}

void CNetChannelPeer::CNetChannelPeerFork::ClearExpiredTx(size_t nReserved)
{
    CPeerKnownTxSetByTime& index = setKnownTx.get<1>();
    int64 nExpiredAt = GetTime() - NETCHANNEL_KNOWNINV_EXPIREDTIME;
    size_t nMaxSize = NETCHANNEL_KNOWNINV_MAXCOUNT - nReserved;
    CPeerKnownTxSetByTime::iterator it = index.begin();
    while (it != index.end() && ((*it).nTime <= nExpiredAt || index.size() > nMaxSize))
    {
        index.erase(it++);
    }
}

bool CNetChannelPeer::IsSynchronized(const uint256& hashFork) const
{
    map<uint256,CNetChannelPeerFork>::const_iterator it = mapSubscribedFork.find(hashFork);
    if (it != mapSubscribedFork.end())
    {
        return (*it).second.fSynchronized;
    }
    return false;
}

bool CNetChannelPeer::SetSyncStatus(const uint256& hashFork,bool fSync,bool& fInverted)
{
    map<uint256,CNetChannelPeerFork>::iterator it = mapSubscribedFork.find(hashFork);
    if (it != mapSubscribedFork.end())
    {
        fInverted = ((*it).second.fSynchronized != fSync);
        (*it).second.fSynchronized = fSync;
        return true;
    }
    return false;
}

void CNetChannelPeer::AddKnownTx(const uint256& hashFork,const vector<uint256>& vTxHash)
{
    map<uint256,CNetChannelPeerFork>::iterator it = mapSubscribedFork.find(hashFork);
    if (it != mapSubscribedFork.end())
    {
        (*it).second.AddKnownTx(vTxHash);
    }
}

void CNetChannelPeer::MakeTxInv(const uint256& hashFork,const vector<uint256>& vTxPool,
                                                        vector<network::CInv>& vInv,size_t nMaxCount)
{
    map<uint256,CNetChannelPeerFork>::iterator it = mapSubscribedFork.find(hashFork);
    if (it != mapSubscribedFork.end())
    {
        vector<uint256> vTxHash;
        CNetChannelPeerFork& peerFork = (*it).second;
        for(const uint256& txid : vTxPool)
        {
            if (vInv.size() >= nMaxCount)
            {
                break;
            }
            else if (!(*it).second.IsKnownTx(txid))
            {
                vInv.push_back(network::CInv(network::CInv::MSG_TX,txid));
                vTxHash.push_back(txid);
            }
        }
        peerFork.AddKnownTx(vTxHash);
    }
}

//////////////////////////////
// CNetChannel 

CNetChannel::CNetChannel(const uint nThreadIn, const bool fAffinityIn)
: IMvNetChannel(nThreadIn, fAffinityIn)
{
    pPeerNet = NULL;
    pCoreProtocol = NULL;
    pWorldLine = NULL;
    pTxPool = NULL;
    pService = NULL;
    pDispatcher = NULL;
    nodeType = NODE_TYPE::NODE_TYPE_FNFN;
}

CNetChannel::~CNetChannel()
{
}

bool CNetChannel::WalleveHandleInitialize()
{
    if (!WalleveGetObject("virtualpeernet",pPeerNet))
    {
        WalleveError("Failed to request peer net\n");
        return false;
    }

    if (!WalleveGetObject("coreprotocol",pCoreProtocol))
    {
        WalleveError("Failed to request coreprotocol\n");
        return false;
    }

    if (!WalleveGetObject("worldline",pWorldLine))
    {
        WalleveError("Failed to request worldline\n");
        return false;
    }

    if (!WalleveGetObject("txpool",pTxPool))
    {
        WalleveError("Failed to request txpool\n");
        return false;
    }

    if (!WalleveGetObject("service",pService))
    {
        WalleveError("Failed to request service\n");
        return false;
    }

    if (!WalleveGetObject("dispatcher",pDispatcher))
    {
        WalleveError("Failed to request dispatcher\n");
        return false;
    }

    return true;
}

void CNetChannel::WalleveHandleDeinitialize()
{
    pPeerNet = NULL;
    pCoreProtocol = NULL;
    pWorldLine = NULL;
    pTxPool = NULL;
    pService = NULL;
    pDispatcher = NULL;
}

bool CNetChannel::WalleveHandleInvoke()
{
    {
        boost::unique_lock<boost::mutex> lock(mtxPushTx);
        nTimerPushTx = 0;
    }
    return network::IMvNetChannel::WalleveHandleInvoke(); 
}

void CNetChannel::WalleveHandleHalt()
{
    {
        boost::unique_lock<boost::mutex> lock(mtxPushTx);
        if (nTimerPushTx != 0)
        {
            WalleveCancelTimer(nTimerPushTx);
            nTimerPushTx = 0;
        }
        setPushTxFork.clear(); 
    }
    
    network::IMvNetChannel::WalleveHandleHalt();
    {
        boost::recursive_mutex::scoped_lock scoped_lock(mtxSched);
        mapSched.clear();
    }
}

int32 CNetChannel::GetPrimaryChainHeight()
{
    uint256 hashBlock = uint64(0);
    int32 nHeight = 0;
    int64 nTime = 0;
    if (pWorldLine->GetLastBlock(pCoreProtocol->GetGenesisBlockHash(),hashBlock,nHeight,nTime))
    {
        return nHeight;
    }
    return 0;
}

bool CNetChannel::IsForkSynchronized(const uint256& hashFork) const
{
    return conPeerNetData.IsForkSynchronized(hashFork);
}

void CNetChannel::BroadcastBlockInv(const uint256& hashFork,const uint256& hashBlock,const set<uint64>& setKnownPeer)
{
    network::CMvEventPeerInv eventInv(0,hashFork);
    eventInv.sender = "netchannel";
    eventInv.data.push_back(network::CInv(network::CInv::MSG_BLOCK,hashBlock));

    std::vector<std::pair<uint64,CNetChannelPeer>> vChannelPeer = conPeerNetData.KeyValues();
    for(const auto& channelPeer : vChannelPeer)
    {
        const uint64& nNonce = channelPeer.first;
        if (!setKnownPeer.count(nNonce) && channelPeer.second.IsSubscribed(hashFork))
        {
            eventInv.nNonce = nNonce;
            pPeerNet->DispatchEvent(&eventInv);
        }
    }
    
    network::CMvEventPeerInv eventDownInv(std::numeric_limits<uint64>::max(), hashFork);
    eventDownInv.sender = "netchannel";
    eventDownInv.data.push_back(network::CInv(network::CInv::MSG_BLOCK,hashBlock));
    pPeerNet->DispatchEvent(&eventDownInv);
}

void CNetChannel::BroadcastTxInv(const uint256& hashFork)
{
    boost::unique_lock<boost::mutex> lock(mtxPushTx);
    if (nTimerPushTx == 0)
    {
        if (!PushTxInv(hashFork))
        {
            setPushTxFork.insert(hashFork);
        }
        nTimerPushTx = WalleveSetTimer(PUSHTX_TIMEOUT,boost::bind(&CNetChannel::PushTxTimerFunc,this,_1));
    }
    else
    {
        setPushTxFork.insert(hashFork);
    }
}

void CNetChannel::SubscribeFork(const uint256& hashFork)
{
    {
        boost::recursive_mutex::scoped_lock scoped_lock(mtxSched);
        if (!mapSched.insert(make_pair(hashFork,CSchedule())).second)
        {
            return;
        }
    }

    network::CMvEventPeerSubscribe eventSubscribe(0ULL,pCoreProtocol->GetGenesisBlockHash());
    eventSubscribe.data.push_back(hashFork);

    std::vector<std::pair<uint64,CNetChannelPeer>> vChannelPeer = conPeerNetData.KeyValues();
    for(const auto& channelPeer : vChannelPeer)
    {
        eventSubscribe.nNonce = channelPeer.first;
        pPeerNet->DispatchEvent(&eventSubscribe);
        DispatchGetBlocksEvent(channelPeer.first,hashFork);
    }
}

void CNetChannel::UnsubscribeFork(const uint256& hashFork)
{
    {
        boost::recursive_mutex::scoped_lock scoped_lock(mtxSched);
        if (!mapSched.erase(hashFork))
        {
            return;
        }
    }

    network::CMvEventPeerUnsubscribe eventUnsubscribe(0ULL,pCoreProtocol->GetGenesisBlockHash());
    eventUnsubscribe.data.push_back(hashFork);

    std::vector<std::pair<uint64,CNetChannelPeer>> vChannelPeer = conPeerNetData.KeyValues();
    for(const auto& channelPeer : vChannelPeer)
    {
        eventUnsubscribe.nNonce = channelPeer.first;
        pPeerNet->DispatchEvent(&eventUnsubscribe);
    }
}

bool CNetChannel::IsContains(const uint256& hashFork)
{
    boost::recursive_mutex::scoped_lock scoped_lock(mtxSched);
    return mapSched.find(hashFork) != mapSched.end();
}

void CNetChannel::EnableSuperNode(bool fIsFork)
{
    nodeType = fIsFork ? NODE_TYPE::NODE_TYPE_FORK : NODE_TYPE::NODE_TYPE_ROOT;
}

bool CNetChannel::HandleEvent(network::CMvEventPeerActive& eventActive)
{
    uint64 nNonce = eventActive.nNonce;
    if ((eventActive.data.nService & network::NODE_NETWORK))
    {
        DispatchGetBlocksEvent(nNonce,pCoreProtocol->GetGenesisBlockHash());
        
        network::CMvEventPeerSubscribe eventSubscribe(nNonce,pCoreProtocol->GetGenesisBlockHash());
        {
            boost::recursive_mutex::scoped_lock scoped_lock(mtxSched);
            for (map<uint256,CSchedule>::iterator it = mapSched.begin();it != mapSched.end();++it)
            {
                if ((*it).first != pCoreProtocol->GetGenesisBlockHash())
                {
                    eventSubscribe.data.push_back((*it).first);
                }
            }
        }
        if (!eventSubscribe.data.empty())
        {
            pPeerNet->DispatchEvent(&eventSubscribe);
        }
    }
    
    conPeerNetData.ActivePeer(nNonce, eventActive.data.nService, 
        pCoreProtocol->GetGenesisBlockHash());
    
    NotifyPeerUpdate(nNonce,true,eventActive.data);
    return true;
}

bool CNetChannel::HandleEvent(network::CMvEventPeerDeactive& eventDeactive)
{
    uint64 nNonce = eventDeactive.nNonce;
    {
        boost::recursive_mutex::scoped_lock scoped_lock(mtxSched);
        for (map<uint256,CSchedule>::iterator it = mapSched.begin();it != mapSched.end();++it)
        {
            CSchedule& sched = (*it).second;
            set<uint64> setSchedPeer;
            sched.RemovePeer(nNonce,setSchedPeer);

            for(const uint64 nNonceSched : setSchedPeer)
            {
                SchedulePeerInv(nNonceSched,(*it).first,sched);
            }
        }
    }
    
    conPeerNetData.DeactivePeer(nNonce);
    
    NotifyPeerUpdate(nNonce,false,eventDeactive.data);    

    return true;
}

bool CNetChannel::HandleEvent(network::CMvEventPeerSubscribe& eventSubscribe)
{
    uint64 nNonce = eventSubscribe.nNonce;
    uint256& hashFork = eventSubscribe.hashFork;
    if (hashFork == pCoreProtocol->GetGenesisBlockHash())
    {
        conPeerNetData.SubscribeForks(nNonce, eventSubscribe.data);
        
        for(const uint256& hash : eventSubscribe.data)
        {
            boost::recursive_mutex::scoped_lock scoped_lock(mtxSched);
            if (mapSched.count(hash))
            {
                DispatchGetBlocksEvent(nNonce,hash);
            }   
        }
        
    }
    else
    {
        DispatchMisbehaveEvent(nNonce,CEndpointManager::DDOS_ATTACK,"eventSubscribe");
    }

    return true;
}

bool CNetChannel::HandleEvent(network::CMvEventPeerUnsubscribe& eventUnsubscribe)
{
    uint64 nNonce = eventUnsubscribe.nNonce;
    uint256& hashFork = eventUnsubscribe.hashFork;
    if (hashFork == pCoreProtocol->GetGenesisBlockHash())
    {
        conPeerNetData.UnsubscribeForks(nNonce, eventUnsubscribe.data);
    }
    else
    {
        DispatchMisbehaveEvent(nNonce,CEndpointManager::DDOS_ATTACK,"eventUnsubscribe");
    }

    return true;
}

bool CNetChannel::HandleEvent(network::CMvEventPeerInv& eventInv)
{
    uint64 nNonce = eventInv.nNonce;
    uint256& hashFork = eventInv.hashFork;

    try 
    {
        if (eventInv.data.size() > network::CInv::MAX_INV_COUNT)
        {
            throw runtime_error("Inv count overflow.");
        }

        {
            boost::recursive_mutex::scoped_lock scoped_lock(mtxSched);
            CSchedule& sched = GetSchedule(hashFork);
            vector<uint256> vTxHash;
            for(const network::CInv& inv : eventInv.data)
            {
                if ((inv.nType == network::CInv::MSG_TX && !pTxPool->Exists(inv.nHash)) 
                    || (inv.nType == network::CInv::MSG_BLOCK && !pWorldLine->Exists(inv.nHash)))
                {
                    sched.AddNewInv(inv,nNonce);
                    if (inv.nType == network::CInv::MSG_TX)
                    {
                        vTxHash.push_back(inv.nHash);
                    }
                }
            }
            if (!vTxHash.empty())
            {
                conPeerNetData.AddKnownTx(nNonce, hashFork, vTxHash);
            }
            SchedulePeerInv(nNonce,hashFork,sched);
        }
    }
    catch (...)
    {
        DispatchMisbehaveEvent(nNonce,CEndpointManager::DDOS_ATTACK,"eventInv");
    }
    return true;
}

bool CNetChannel::HandleEvent(network::CMvEventPeerGetData& eventGetData)
{
    uint64 nNonce = eventGetData.nNonce;
    uint256& hashFork = eventGetData.hashFork;
    std::string flow = eventGetData.flow;
    for(const network::CInv& inv : eventGetData.data)
    {
        if (inv.nType == network::CInv::MSG_TX)
        {
            network::CMvEventPeerTx eventTx(nNonce,hashFork);

            if("up" == flow)
            {
                eventTx.nNonce = std::numeric_limits<uint64>::max();
                eventTx.sender = "netchannel";
            }

            if (pTxPool->Get(inv.nHash,eventTx.data))
            {
                pPeerNet->DispatchEvent(&eventTx);
            }
        }
        else if (inv.nType == network::CInv::MSG_BLOCK)
        {
            network::CMvEventPeerBlock eventBlock(nNonce,hashFork);

            if("up" == flow)
            {
                eventBlock.nNonce = std::numeric_limits<uint64>::max();
                eventBlock.sender = "netchannel";
            }

            if (pWorldLine->GetBlock(inv.nHash,eventBlock.data))
            {
                pPeerNet->DispatchEvent(&eventBlock);
            }
        }
    }
    return true;
}

bool CNetChannel::HandleEvent(network::CMvEventPeerGetBlocks& eventGetBlocks)
{
    uint64 nNonce = eventGetBlocks.nNonce;
    uint256& hashFork = eventGetBlocks.hashFork;
    vector<uint256> vBlockHash;
    std::string flow = eventGetBlocks.flow;
    if (!pWorldLine->GetBlockInv(hashFork,eventGetBlocks.data,vBlockHash,MAX_GETBLOCKS_COUNT))
    {
        DispatchMisbehaveEvent(nNonce,CEndpointManager::DDOS_ATTACK,"eventGetBlocks");
        return true;
    }
    network::CMvEventPeerInv eventInv(nNonce,hashFork);
    for(const uint256& hash : vBlockHash)
    {
        eventInv.data.push_back(network::CInv(network::CInv::MSG_BLOCK,hash));
    }

    if("up" == flow)
    {
        eventInv.nNonce = std::numeric_limits<uint64>::max();
        eventInv.sender = "netchannel";
    }

    pPeerNet->DispatchEvent(&eventInv);
    return true;
}

bool CNetChannel::HandleEvent(network::CMvEventPeerTx& eventTx)
{
    uint64 nNonce = eventTx.nNonce;
    uint256& hashFork = eventTx.hashFork;
    CTransaction& tx = eventTx.data;
    uint256 txid = tx.GetHash();

    try
    {
        boost::recursive_mutex::scoped_lock scoped_lock(mtxSched);

        set<uint64> setSchedPeer,setMisbehavePeer;
        CSchedule& sched = GetSchedule(hashFork);

        if (!sched.ReceiveTx(nNonce,txid,tx,setSchedPeer))
        {
            throw runtime_error("Failed to receive tx");
        }
        
        uint256 hashForkAnchor;
        int32 nHeightAnchor;
        if (pWorldLine->GetBlockLocation(tx.hashAnchor,hashForkAnchor,nHeightAnchor)
            && hashForkAnchor == hashFork)
        {
            set<uint256> setMissingPrevTx;
            if (!GetMissingPrevTx(tx,setMissingPrevTx))
            {
                AddNewTx(hashFork,txid,sched,setSchedPeer,setMisbehavePeer);   
            }
            else
            {
                for(const uint256& prev : setMissingPrevTx)
                {
                    sched.AddOrphanTxPrev(txid,prev);
                    network::CInv inv(network::CInv::MSG_TX,prev);
                    if (!sched.Exists(inv))
                    {
                        for(const uint64 nNonceSched : setSchedPeer)
                        {
                            sched.AddNewInv(inv,nNonceSched);
                        }
                    }
                }
            }
        }
        else
        {
            sched.InvalidateTx(txid,setMisbehavePeer);
        } 
        PostAddNew(hashFork,sched,setSchedPeer,setMisbehavePeer);
    }
    catch (...)
    {
        DispatchMisbehaveEvent(nNonce,CEndpointManager::DDOS_ATTACK,"eventTx");
    }
    return true;
    
}

bool CNetChannel::HandleEvent(network::CMvEventPeerBlock& eventBlock)
{
    uint64 nNonce = eventBlock.nNonce;
    uint256& hashFork = eventBlock.hashFork; 
    CBlock& block = eventBlock.data;
    uint256 hash = block.GetHash();

    try
    {
        boost::recursive_mutex::scoped_lock scoped_lock(mtxSched);

        set<uint64> setSchedPeer,setMisbehavePeer;
        CSchedule& sched = GetSchedule(hashFork);
        
        if (!sched.ReceiveBlock(nNonce,hash,block,setSchedPeer))
        {
            throw runtime_error("Failed to receive block");
        }

        uint256 hashForkPrev;
        int32 nHeightPrev;
        if (pWorldLine->GetBlockLocation(block.hashPrev,hashForkPrev,nHeightPrev))
        {
            if (hashForkPrev == hashFork)
            {
               AddNewBlock(hashFork,hash,sched,setSchedPeer,setMisbehavePeer);
            }
            else
            {
                sched.InvalidateBlock(hash,setMisbehavePeer);
            }
        }
        else
        {
            sched.AddOrphanBlockPrev(hash,block.hashPrev);
        }

        PostAddNew(hashFork,sched,setSchedPeer,setMisbehavePeer);
    }
    catch (...)
    {
        DispatchMisbehaveEvent(nNonce,CEndpointManager::DDOS_ATTACK,"eventBlock");
    }
    return true; 
}

CSchedule& CNetChannel::GetSchedule(const uint256& hashFork)
{
    map<uint256,CSchedule>::iterator it = mapSched.find(hashFork);
    if (it == mapSched.end())
    {
        throw runtime_error("Unknown fork for scheduling.");
    }
    return ((*it).second);
}

void CNetChannel::NotifyPeerUpdate(uint64 nNonce,bool fActive,const network::CAddress& addrPeer)
{
    CNetworkPeerUpdate update;
    update.nPeerNonce = nNonce;
    update.fActive = fActive;
    update.addrPeer = addrPeer;
    pService->NotifyNetworkPeerUpdate(update);
}

void CNetChannel::DispatchGetBlocksEvent(uint64 nNonce,const uint256& hashFork)
{
    network::CMvEventPeerGetBlocks eventGetBlocks(nNonce,hashFork);
    if (pWorldLine->GetBlockLocator(hashFork,eventGetBlocks.data))
    {
        pPeerNet->DispatchEvent(&eventGetBlocks);
    }
}

void CNetChannel::DispatchAwardEvent(uint64 nNonce,CEndpointManager::Bonus bonus)
{
    CWalleveEventPeerNetReward eventReward(nNonce);
    eventReward.data = bonus;
    pPeerNet->DispatchEvent(&eventReward);
}

void CNetChannel::DispatchMisbehaveEvent(uint64 nNonce,CEndpointManager::CloseReason reason,const std::string& strCaller)
{
    if (!strCaller.empty())
    {
        WalleveLog("DispatchMisbehaveEvent : %s\n",strCaller.c_str());
    }

    CWalleveEventPeerNetClose eventClose(nNonce);
    eventClose.data = reason;
    pPeerNet->DispatchEvent(&eventClose);
}

void CNetChannel::SchedulePeerInv(uint64 nNonce,const uint256& hashFork,CSchedule& sched)
{
    network::CMvEventPeerGetData eventGetData(nNonce,hashFork);
    bool fMissingPrev = false;
    bool fEmpty = true;
    if (sched.ScheduleBlockInv(nNonce,eventGetData.data,MAX_PEER_SCHED_COUNT,fMissingPrev,fEmpty))
    {
        if (fMissingPrev)
        {
            DispatchGetBlocksEvent(nNonce,hashFork);
        }
        else if (eventGetData.data.empty())
        {
            if (!sched.ScheduleTxInv(nNonce,eventGetData.data,MAX_PEER_SCHED_COUNT))
            {
                if(nNonce != std::numeric_limits<uint64>::max())
                {
                    DispatchMisbehaveEvent(nNonce,CEndpointManager::DDOS_ATTACK,"SchedulePeerInv1");
                }     
            }
        }
        SetPeerSyncStatus(nNonce,hashFork,fEmpty);
    }
    else
    {
        if(nNonce != std::numeric_limits<uint64>::max())
        {
            DispatchMisbehaveEvent(nNonce,CEndpointManager::DDOS_ATTACK,"SchedulePeerInv2");
        }
        
    }
    if (!eventGetData.data.empty())
    {
        pPeerNet->DispatchEvent(&eventGetData);
    }
}

bool CNetChannel::GetMissingPrevTx(CTransaction& tx,set<uint256>& setMissingPrevTx)
{
    setMissingPrevTx.clear();
    for(const CTxIn& txin : tx.vInput)
    {
        const uint256 &prev = txin.prevout.hash;
        if (!setMissingPrevTx.count(prev))
        {
            if (!pTxPool->Exists(prev) && !pWorldLine->ExistsTx(prev))
            {
                setMissingPrevTx.insert(prev);
            }
        }
    }
    return (!setMissingPrevTx.empty());
}

void CNetChannel::AddNewBlock(const uint256& hashFork,const uint256& hash,CSchedule& sched,
                              set<uint64>& setSchedPeer,set<uint64>& setMisbehavePeer)
{
    vector<uint256> vBlockHash;
    vBlockHash.push_back(hash);
    for (size_t i = 0;i < vBlockHash.size();i++)
    {
        uint256 hashBlock = vBlockHash[i];
        uint64 nNonceSender = 0;
        CBlock* pBlock = sched.GetBlock(hashBlock,nNonceSender);
        if (pBlock != NULL)
        {
            MvErr err = pDispatcher->AddNewBlock(*pBlock,nNonceSender);
            if (err == MV_OK)
            {
                for(const CTransaction& tx : pBlock->vtx)
                {
                    uint256 txid = tx.GetHash();
                    sched.RemoveInv(network::CInv(network::CInv::MSG_TX,txid),setSchedPeer);
                }

                set<uint64> setKnownPeer;
                sched.GetNextBlock(hashBlock,vBlockHash);
                sched.RemoveInv(network::CInv(network::CInv::MSG_BLOCK,hashBlock),setKnownPeer);
                
                if(nNonceSender != std::numeric_limits<uint64>::max())
                {
                    DispatchAwardEvent(nNonceSender,CEndpointManager::VITAL_DATA);
                    BroadcastBlockInv(hashFork,hashBlock,setKnownPeer);
                }
                
                setSchedPeer.insert(setKnownPeer.begin(),setKnownPeer.end());
            }
            else if (err == MV_ERR_ALREADY_HAVE && pBlock->IsVacant())
            {
                set<uint64> setKnownPeer;
                sched.GetNextBlock(hashBlock,vBlockHash);
                sched.RemoveInv(network::CInv(network::CInv::MSG_BLOCK,hashBlock),setKnownPeer);
                setSchedPeer.insert(setKnownPeer.begin(),setKnownPeer.end());
            }
            else
            {
                sched.InvalidateBlock(hashBlock,setMisbehavePeer);
            }
        }
    }
}

void CNetChannel::AddNewTx(const uint256& hashFork,const uint256& txid,CSchedule& sched,
                           set<uint64>& setSchedPeer,set<uint64>& setMisbehavePeer)
{
    set<uint256> setTx;
    vector<uint256> vtx;

    vtx.push_back(txid);
    int nAddNewTx = 0;
    for (size_t i = 0;i < vtx.size();i++)
    {
        uint256 hashTx = vtx[i];
        uint64 nNonceSender = 0;
        CTransaction *pTx = sched.GetTransaction(hashTx,nNonceSender);
        if (pTx != NULL)
        {
            MvErr err = pDispatcher->AddNewTx(*pTx,nNonceSender);
            if (err == MV_OK)
            {
                sched.GetNextTx(hashTx,vtx,setTx);
                sched.RemoveInv(network::CInv(network::CInv::MSG_TX,hashTx),setSchedPeer);
                
                if(nNonceSender != std::numeric_limits<uint64>::max())
                {
                    DispatchAwardEvent(nNonceSender,CEndpointManager::MAJOR_DATA);
                }

                nAddNewTx++;
            }
            else if (err != MV_ERR_MISSING_PREV)
            {
                sched.InvalidateTx(hashTx,setMisbehavePeer);
            }
        }
    }
    if (nAddNewTx)
    {
        BroadcastTxInv(hashFork);
    }
}

void CNetChannel::PostAddNew(const uint256& hashFork,CSchedule& sched,
                             set<uint64>& setSchedPeer,set<uint64>& setMisbehavePeer)
{
    for(const uint64 nNonceSched : setSchedPeer)
    {
        if (!setMisbehavePeer.count(nNonceSched))
        {
            SchedulePeerInv(nNonceSched,hashFork,sched);
        }
    }

    for(const uint64 nNonceMisbehave : setMisbehavePeer)
    {
        if(nNonceMisbehave != std::numeric_limits<uint64>::max())
        {
            DispatchMisbehaveEvent(nNonceMisbehave,CEndpointManager::DDOS_ATTACK,"PostAddNew");
        }
    }
}

void CNetChannel::SetPeerSyncStatus(uint64 nNonce,const uint256& hashFork,bool fSync)
{ 
    bool fInverted = false;
    {
        if(!conPeerNetData.SetPeerSyncStatus(nNonce, hashFork, fSync, fInverted))
        {
            return;
        }
    }
    
    if (fInverted)
    {
        if (fSync)
        {
            conPeerNetData.DeletePeerUnSyncByFork(nNonce, hashFork);
            BroadcastTxInv(hashFork);
        }
        else
        {
            conPeerNetData.InsertPeerUnSyncByFork(nNonce, hashFork);
        }
    }
}

void CNetChannel::PushTxTimerFunc(uint32 nTimerId)
{
    boost::unique_lock<boost::mutex> lock(mtxPushTx);
    if (nTimerPushTx == nTimerId)
    {
        if (!setPushTxFork.empty())
        {
            set<uint256>::iterator it = setPushTxFork.begin();
            while (it != setPushTxFork.end())
            {
                if (PushTxInv(*it))
                {
                    setPushTxFork.erase(it++);
                }
                else
                {
                    ++it;
                }
            }
            nTimerPushTx = WalleveSetTimer(PUSHTX_TIMEOUT,boost::bind(&CNetChannel::PushTxTimerFunc,this,_1));
        }
        else
        {
            nTimerPushTx = 0;
        }
    }
}

bool CNetChannel::PushTxInv(const uint256& hashFork)
{
    // if (!IsForkSynchronized(hashFork))
    // {
    //     return false;
    // }

    bool fCompleted = true;
    vector<uint256> vTxPool;
    pTxPool->ListTx(hashFork,vTxPool);
    if (!vTxPool.empty() && !conPeerNetData.IsPeerEmpty())
    {
        
        std::vector<std::pair<uint64,CConcurrentPeerNetData::VecInv>> vecInvData;
        conPeerNetData.MakeTxInvByFork(hashFork,vTxPool,vecInvData);

        for(const auto& inv : vecInvData)
        {
            network::CMvEventPeerInv eventInv(inv.first,hashFork);
            eventInv.data = inv.second;
            if (!eventInv.data.empty())
            {
                pPeerNet->DispatchEvent(&eventInv);
                if (fCompleted && eventInv.data.size() == network::CInv::MAX_INV_COUNT)
                {
                    fCompleted = false;
                }
            }
        }
    }
    return fCompleted;
}
