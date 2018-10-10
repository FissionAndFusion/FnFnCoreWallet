// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_DBP_CLIENT_H
#define MULTIVERSE_DBP_CLIENT_H

#include "walleve/walleve.h"

namespace multiverse
{

class CMvDbpClient : public walleve::CIOProc
{
public:
    CMvDbpClient();
    virtual ~CMvDbpClient() noexcept;
protected:
    bool WalleveHandleInitialize() override;
    void WalleveHandleDeinitialize() override;
    void EnterLoop() override;
    void LeaveLoop() override;
};

} // namespace multiverse

#endif // MULTIVERSE_DBP_CLIENT_H