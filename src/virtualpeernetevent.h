// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_FORKPEEREVENT_H
#define MULTIVERSE_FORKPEEREVENT_H

#include <boost/variant.hpp>

#include "walleve/walleve.h"
#include "mvproto.h"
#include "block.h"
#include "mvpeerevent.h"

using namespace multiverse::network;

namespace multiverse
{


enum class ecForkEventType : int
{
    //FORK NODE PEER NET EVENT

    FK_EVENT_NODE_MAX
};

class CFkNodeEventListener : virtual public walleve::CWalleveEventListener
{
public:
    virtual ~CFkNodeEventListener() {}
    DECLARE_EVENTHANDLER(CMvEventPeerActive);
    DECLARE_EVENTHANDLER(CMvEventPeerDeactive);
};

}
#endif //MULTIVERSE_FORKPEEREVENT_H
