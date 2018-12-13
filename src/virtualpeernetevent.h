// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_VIRTUAL_PEEREVENT_H
#define MULTIVERSE_VIRTUAL_PEEREVENT_H

#include <boost/variant.hpp>

#include "walleve/walleve.h"
#include "mvproto.h"
#include "block.h"
#include "mvpeerevent.h"

using namespace multiverse::network;

namespace multiverse
{

class CVirtualPeerNetEventListener : virtual public walleve::CWalleveEventListener
{
public:
    virtual ~CVirtualPeerNetEventListener() {}
    DECLARE_EVENTHANDLER(CMvEventPeerActive);
    DECLARE_EVENTHANDLER(CMvEventPeerDeactive);
};

}
#endif // MULTIVERSE_VIRTUAL_PEEREVENT_H
