// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_TXPOOL_H
#define  MULTIVERSE_TXPOOL_H

#include "mvbase.h"

namespace multiverse
{

class CTxPool : public ITxPool
{
public:
    CTxPool();
    ~CTxPool();
    bool Exists(const uint256& txid);
    void Clear();
    std::size_t Count(const uint256& fork) const;
    MvErr AddNew(const CTransaction& tx);
    void Remove(const uint256& txid);
    bool Get(const uint256& txid,CTransaction& tx) const;
    bool Arrange(uint256& fork,std::vector<std::pair<uint256,CTransaction> >& vtx,std::size_t nMaxSize);
    bool FetchInputs(uint256& fork,const CTransaction& tx,std::vector<CTxOutput>& vUnspent);
    void ForkUpdate(uint256& fork,const std::vector<CBlockTx>& vAddNew,const std::vector<uint256>& vRemove);
protected:
    bool WalleveHandleInitialize();
    void WalleveHandleDeinitialize();
    bool WalleveHandleInvoke();
    void WalleveHandleHalt();
protected:
    boost::shared_mutex rwAccess;
    ICoreProtocol* pCoreProtocol;
    IWorldLine* pWorldLine;
};

} // namespace multiverse

#endif //MULTIVERSE_TXPOOL_H

