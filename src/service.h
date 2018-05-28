// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_SERVICE_H
#define  MULTIVERSE_SERVICE_H

#include "mvbase.h"

namespace multiverse
{

class CService : public IService
{
public:
    CService();
    ~CService();
    /* System */
    void Shutdown();
    /* Network */
    int GetPeerCount();
    bool AddNode(const walleve::CNetHost& node);
    void RemoveNode(const walleve::CNetHost& node);
    /* Worldline & Tx Pool*/
    int  GetForkCount();
    bool GetForkGenealogy(const uint256& hashFork,std::vector<uint256>& vAncestry,std::vector<uint256>& vSubline);
    bool GetForkFromBlock(const uint256 hashBlock,uint256& hashFork);
    int  GetBlockCount(const uint256& hashFork);
    bool GetBlockHash(const uint256& hashFork,int nHeight,uint256& hashBlock);
    bool GetBlock(const uint256& hash,CBlock& block,CBlockIndex** ppBlockIndex);
    void GetTxPool(const uint256& hashFork,std::vector<uint256>& vTxPool);
    bool GetTransaction(const uint256& txid,CTransaction& tx,std::vector<uint256>& vInBlock);
    MvErr SendTransaction(CTransaction& tx);
    /* Wallet */
    bool ImportKey(const crypto::CKey& key);
    bool ExportKey(const crypto::CPubKey& pubkey,int& nVersionRet,crypto::CCryptoCipher& cipherRet);
    bool SignSignature(const crypto::CPubKey& pubkey,const uint256& hash,std::vector<unsigned char>& vchSig);
    /* Util */
protected:
    bool WalleveHandleInitialize();
    void WalleveHandleDeinitialize();
    bool WalleveHandleInvoke();
    void WalleveHandleHalt();
protected:
    ICoreProtocol* pCoreProtocol;
    IWorldLine* pWorldLine;
};

} // namespace multiverse

#endif //MULTIVERSE_SERVICE_H

