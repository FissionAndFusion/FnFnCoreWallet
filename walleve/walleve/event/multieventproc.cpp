// Copyright (c) 2016-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "multieventproc.h"

#include <boost/bind.hpp>
#include <thread>

using namespace std;
using namespace walleve;

///////////////////////////////
// CWalleveMultiEventQueue

CWalleveMultiEventQueue::CWalleveMultiEventQueue()
    : fAbort(false), nTime(chrono::steady_clock::now().time_since_epoch().count())
{
}

CWalleveMultiEventQueue::~CWalleveMultiEventQueue()
{
    Reset();
}

void CWalleveMultiEventQueue::AddNew(CWalleveEvent* pEvent)
{
    shared_ptr<CEventNode> spNode = make_shared<CEventNode>(pEvent);
    if (!spNode)
    {
        return;
    }

    shared_ptr<CEventNode> spPrevNode;
    {
        unique_lock<mutex> lock(mtxWrite);
        if (fAbort)
        {
            return;
        }

        auto it = mapWrite.find(spNode->spEvent->nNonce);
        if (it == mapWrite.end())
        {
            mapWrite.insert(make_pair(spNode->spEvent->nNonce, spNode));
        }
        else
        {
            spPrevNode = it->second;
            it->second = spNode;
        }
    }

    if (spPrevNode)
    {
        int nOldFlag = spPrevNode->nFlag.exchange(CEventNode::MIDDLE);
        if (nOldFlag == CEventNode::INVALID)
        {
            spPrevNode.reset();
        }
        else
        {
            atomic_store_explicit(&spPrevNode->spNext, spNode, memory_order_release);
        }
    }
        
    if (!spPrevNode)
    {
        NotifyRead(spNode);
    }
}

shared_ptr<CWalleveMultiEventQueue::CEventNode> CWalleveMultiEventQueue::Fetch(shared_ptr<CEventNode> spPrevNode)
{
    if (spPrevNode)
    {
        int nOldFlag = spPrevNode->nFlag.exchange(CEventNode::INVALID);
        if (nOldFlag == CEventNode::LAST)
        {
            // if event queue is empty, remove this nonce from mapWrite
            unique_lock<mutex> lock(mtxWrite);
            auto it = mapWrite.find(spPrevNode->spEvent->nNonce);
            if (it != mapWrite.end() && (it->second == spPrevNode))
            {
                mapWrite.erase(it);
            }
        }
        else if (nOldFlag == CEventNode::MIDDLE)
        {
            // wait for writing thread
            shared_ptr<CEventNode> spNode;
            while((spNode = atomic_load_explicit(&spPrevNode->spNext, memory_order_relaxed)) == NULL)
                ;

            // continue to execute next event or not
            if (!fAbort && ContinueExecute(spNode))
            {
                return spNode;
            }
            else
            {
                NotifyRead(spNode);
            }
        }
        else
        {
            StdError(__PRETTY_FUNCTION__, string("EventNode nFlag error when fetch: ").append(to_string(nOldFlag)).c_str());
            return NULL;
        }
    }

    // wait for new event
    unique_lock<mutex> lock(mtxRead);
    while (!fAbort && mapRead.empty())
    {
        condRead.wait(lock);
    }
    if (!fAbort && !mapRead.empty())
    {
        --nReadSize;

        auto it = mapRead.begin();
        shared_ptr<CEventNode> spNode = it->second;
        nTime.store(spNode->nTime);
        mapRead.erase(it);

        return spNode;
    }

    return NULL;
}

void CWalleveMultiEventQueue::Reset()
{
    fAbort = false;
    {
        unique_lock<mutex> lock(mtxWrite);
        mapWrite.clear();
    }
    {
        unique_lock<mutex> lock(mtxRead);
        mapRead.clear();
    }
}

void CWalleveMultiEventQueue::Interrupt()
{
    fAbort = true;
    {
        unique_lock<mutex> lock(mtxWrite);
        mapWrite.clear();
    }
    {
        unique_lock<mutex> lock(mtxRead);
        mapRead.clear();
    }
    condRead.notify_all();
}

void CWalleveMultiEventQueue::NotifyRead(shared_ptr<CEventNode> spNode)
{
    {
        unique_lock<mutex> lock(mtxRead);
        if (fAbort)
        {
            return;
        }

        mapRead.insert(make_pair(spNode->nTime, spNode));
        ++nReadSize;
    }
    condRead.notify_one();
}

bool CWalleveMultiEventQueue::ContinueExecute(shared_ptr<CEventNode> spNode)
{
    return spNode && (spNode->nTime <= nTime || nReadSize == 0);
}

///////////////////////////////
// CWalleveMultiEventProc

CWalleveMultiEventProc::CWalleveMultiEventProc(const string& walleveOwnKeyIn, const size_t nThreadNumIn, const bool fAffinityIn)
 : IWalleveBase(walleveOwnKeyIn), nThreadNum(nThreadNumIn), fAffinity(fAffinityIn)
{
    if (nThreadNum == 0)
    {
        nThreadNum = 1;
    }

    vecThrEventQue.reserve(nThreadNum);
    auto nextCPU = fAffinity
        ? [](int& x) -> int { return x++ % thread::hardware_concurrency(); }
        : [](int& x) -> int { return -1; };
    
    int nCPU = 0;
    for (size_t i = 0; i < nThreadNum; ++i)
    {
        vecThrEventQue.emplace_back(walleveOwnKeyIn + "-namedeventq-" + to_string(i),
            bind(&CWalleveMultiEventProc::EventThreadFunc,this), nextCPU(nCPU));
    }
}

bool CWalleveMultiEventProc::WalleveHandleInvoke()
{   
    queEvent.Reset();

    for (CWalleveThread& thrEventQue : vecThrEventQue)
    {
        if (!WalleveThreadStart(thrEventQue))
        {
            return false;
        }
    }
    return true;
}   

void CWalleveMultiEventProc::WalleveHandleHalt()
{
    queEvent.Interrupt();
    
    for (CWalleveThread& thrEventQue : vecThrEventQue)
    {
        WalleveThreadExit(thrEventQue);
    }
}

void CWalleveMultiEventProc::PostEvent(CWalleveEvent *pEvent)
{
    queEvent.AddNew(pEvent);
}

void CWalleveMultiEventProc::EventThreadFunc()
{
    shared_ptr<CWalleveMultiEventQueue::CEventNode> spPreNode;
    shared_ptr<CWalleveMultiEventQueue::CEventNode> spNode;
    while ((spNode = queEvent.Fetch(spPreNode)) != NULL)
    {
        if (!spNode->spEvent->Handle(*this))
        {
            spPreNode.reset();
        }
        else
        {
            spPreNode = spNode;
        }
    }
}

