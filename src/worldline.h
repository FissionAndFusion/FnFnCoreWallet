// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_WORLDLINE_H
#define  MULTIVERSE_WORLDLINE_H

#include "mvbase.h"
#include "blockbase.h"
#include <map>

namespace multiverse
{

class CWorldLine : public IWorldLine
{
public:
    CWorldLine();
    ~CWorldLine();
    MvErr AddNewBlock(CBlock& block,std::vector<CTransaction>& vtx); 
protected:
    bool WalleveHandleInitialize();
    void WalleveHandleDeinitialize();
    bool WalleveHandleInvoke();
    void WalleveHandleHalt();
    bool CheckContainer();
    bool RebuildContainer();
    bool InsertGenesisBlock(CBlock& block);
    MvErr GetBlockTx(CBlockTx& tx,storage::CBlockView& view);
protected:
    boost::shared_mutex rwAccess;
    ICoreProtocol *pCoreProtocol;
    storage::CBlockBase cntrBlock;
};

} // namespace multiverse

#endif //MULTIVERSE_WORLDLINE_H

