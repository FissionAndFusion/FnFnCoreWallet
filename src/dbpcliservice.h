// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_DBP_CLIENT_SERVICE_H
#define MULTIVERSE_DBP_CLIENT_SERVICE_H

#include "walleve/walleve.h"
#include "event.h"
namespace multiverse
{

class CDbpCliService : public walleve::IIOModule, virtual public CDBPEventListener
{
public:
    CDbpCliService();
    virtual ~CDbpCliService() noexcept;

    bool HandleEvent(CMvEventDbpBroken& event) override;
    bool HandleEvent(CMvEventDbpAdded& event) override;
    
protected:
    bool WalleveHandleInitialize() override;
    void WalleveHandleDeinitialize() override;
protected:
    walleve::IIOProc* pDbpClient;
    walleve::IIOProc* pDbpServer;
}; 
} // namespace multiverse

#endif // MULTIVERSE_DBP_CLIENT_SERVICE_H