// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_DISPATCHER_H
#define  MULTIVERSE_DISPATCHER_H

#include "mvbase.h"
#include "mvpeernet.h"

namespace multiverse
{

class CDispatcher : public IDispatcher
{
public:
    CDispatcher();
    ~CDispatcher();
    MvErr AddNewBlock(const CBlock& block,uint64 nNonce=0);
    MvErr AddNewTx(const CTransaction& tx,uint64 nNonce=0);
protected:
    bool WalleveHandleInitialize();
    void WalleveHandleDeinitialize();
    bool WalleveHandleInvoke();
    void WalleveHandleHalt();
    void UpdatePrimaryBlock(const CBlock& block,const CWorldLineUpdate& updateWorldLine,const CTxSetChange& changeTxSet);
    void ProcessForkTx(const CTransaction& tx,int nPrimaryHeight);
    void SyncForkHeight(int nPrimaryHeight);
protected:
    boost::shared_mutex rwAccess;
    ICoreProtocol* pCoreProtocol;
    IWorldLine* pWorldLine;
    ITxPool* pTxPool;
    IConsensus* pConsensus;
    IWallet* pWallet;
    IService* pService;
    IBlockMaker* pBlockMaker;
    network::IMvNetChannel* pNetChannel;
};

} // namespace multiverse

#endif //MULTIVERSE_DISPATCHER_H

