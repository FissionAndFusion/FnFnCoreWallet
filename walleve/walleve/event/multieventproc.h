// Copyright (c) 2016-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef WALLEVE_MULTI_EVENT_PROC_H
#define WALLEVE_MULTI_EVENT_PROC_H

#include <atomic>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <chrono>
#include <map>
#include <vector>

#include "walleve/base/base.h"
#include "walleve/event/event.h"
#include "walleve/type.h"
#include "walleve/util.h"

namespace walleve
{

class CWalleveMultiEventQueue
{
public:
    struct CEventNode
    {
        uint64 nTime;
        std::atomic<int> nFlag;
        boost::shared_ptr<CWalleveEvent> spEvent;
        boost::shared_ptr<CEventNode> spNext;

        enum FlagType
        {
            LAST,
            MIDDLE,
            INVALID,
        };

        CEventNode(CWalleveEvent* pEventIn = NULL)
          : spEvent(pEventIn), spNext(NULL), nFlag(LAST),
            nTime(std::chrono::steady_clock::now().time_since_epoch().count())
        {
        }
    };

public:
    CWalleveMultiEventQueue();
    ~CWalleveMultiEventQueue();
    void AddNew(CWalleveEvent* pEvent);
    boost::shared_ptr<CEventNode> Fetch(boost::shared_ptr<CEventNode> spPrevNode);
    void Reset();
    void Interrupt();

protected:
    void NotifyRead(boost::shared_ptr<CEventNode> spNode);
    bool ContinueExecute(boost::shared_ptr<CEventNode> spNode);

protected:
    std::atomic<uint64> nTime;
    std::atomic<bool> fAbort;

    boost::mutex mtxRead;
    boost::condition_variable condRead;
    std::multimap<uint64, boost::shared_ptr<CEventNode>> mapRead;
    std::atomic<size_t> nReadSize;

    boost::mutex mtxWrite;
    std::map<uint64, boost::shared_ptr<CEventNode>> mapWrite;
};

class CWalleveMultiEventProc : public IWalleveBase
{
public:
    CWalleveMultiEventProc(const std::string& walleveOwnKeyIn, const size_t nThreadIn = 1, const bool fAffinityIn = false);
    void PostEvent(CWalleveEvent* pEvent);

protected:
    bool WalleveHandleInvoke() override;
    void WalleveHandleHalt() override;
    void EventThreadFunc();

protected:
    size_t nThreadNum;
    bool fAffinity;
    std::vector<CWalleveThread> vecThrEventQue;
    CWalleveMultiEventQueue queEvent;
};

} // namespace walleve

#endif //WALLEVE_MULTI_EVENT_PROC_H
