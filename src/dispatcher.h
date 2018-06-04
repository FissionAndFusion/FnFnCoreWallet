// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_DISPATCHER_H
#define  MULTIVERSE_DISPATCHER_H

#include "mvbase.h"

namespace multiverse
{

class CDispatcher : public IDispatcher
{
public:
    CDispatcher();
    ~CDispatcher();
    MvErr AddNewBlock(CBlock& block);
    MvErr AddNewTx(CTransaction& tx);
protected:
    bool WalleveHandleInitialize();
    void WalleveHandleDeinitialize();
    bool WalleveHandleInvoke();
    void WalleveHandleHalt();
//    MvErr PushTxPool(const uint256& txid,CTransaction& tx);
protected:
    boost::shared_mutex rwAccess;
    ICoreProtocol* pCoreProtocol;
    IWorldLine* pWorldLine;
    ITxPool* pTxPool;
    IWallet* pWallet;
    IService* pService;
    IBlockMaker* pBlockMaker;
};

} // namespace multiverse

#endif //MULTIVERSE_DISPATCHER_H

