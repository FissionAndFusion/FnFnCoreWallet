// Copyright (c) 2016-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  WALLEVE_TIMER_H
#define  WALLEVE_TIMER_H

#include "walleve/type.h"

#include <string>
#include <boost/function.hpp>
#include <boost/thread/thread_time.hpp>

namespace walleve
{

typedef boost::function<void(uint64)> TimerCallback;

class CWalleveTimer
{
public:
    CWalleveTimer() {}
    CWalleveTimer(const std::string& walleveKeyIn,uint32 nTimerIdIn,boost::system_time& tExpiryAtIn,
                  TimerCallback fnCallbackIn)
    : walleveKey(walleveKeyIn),nTimerId(nTimerIdIn),tExpiryAt(tExpiryAtIn),fnCallback(fnCallbackIn)
    {
    }
public:
    const std::string walleveKey;
    uint32 nTimerId;
    boost::system_time const tExpiryAt;
    TimerCallback fnCallback;
};

} // namespace walleve

#endif //WALLEVE_TIMER_H

