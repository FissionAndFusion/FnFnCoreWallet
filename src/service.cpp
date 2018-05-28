// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "service.h"

using namespace std;
using namespace walleve;
using namespace multiverse;

extern void MvShutdown();
//////////////////////////////
// CService 

CService::CService()
{
    pCoreProtocol = NULL;
    pWorldLine = NULL;
}

CService::~CService()
{
}

bool CService::WalleveHandleInitialize()
{
    if (!WalleveGetObject("coreprotocol",pCoreProtocol))
    {
        WalleveLog("Failed to request coreprotocol\n");
        return false;
    }

    if (!WalleveGetObject("worldline",pWorldLine))
    {
        WalleveLog("Failed to request worldline\n");
        return false;
    }

    return true;
}

void CService::WalleveHandleDeinitialize()
{
    pCoreProtocol = NULL;
    pWorldLine = NULL;
}

bool CService::WalleveHandleInvoke()
{
    return true;
}

void CService::WalleveHandleHalt()
{
}

void CService::Shutdown()
{
    MvShutdown();
}

int CService::GetPeerCount()
{
    return 0;
}

bool CService::AddNode(const walleve::CNetHost& node)
{
    return false;
}

void CService::RemoveNode(const walleve::CNetHost& node)
{
}

int  CService::GetForkCount()
{
    return 0;
}

bool CService::GetForkGenealogy(const uint256& hashFork,vector<uint256>& vAncestry,vector<uint256>& vSubline)
{
    return false;
}

bool CService::GetForkFromBlock(const uint256 hashBlock,uint256& hashFork)
{
    return false;
}

int  CService::GetBlockCount(const uint256& hashFork)
{
    return 0;
}

bool CService::GetBlockHash(const uint256& hashFork,int nHeight,uint256& hashBlock)
{
    return false;
}

bool CService::GetBlock(const uint256& hash,CBlock& block,CBlockIndex** ppBlockIndex)
{
    return false;
}

void CService::GetTxPool(const uint256& hashFork,vector<uint256>& vTxPool)
{
}

bool CService::GetTransaction(const uint256& txid,CTransaction& tx,vector<uint256>& vInBlock)
{
    return false;
}

MvErr CService::SendTransaction(CTransaction& tx)
{
    return MV_OK;
}

bool CService::ImportKey(const crypto::CKey& key)
{
    return false;
}

bool CService::ExportKey(const crypto::CPubKey& pubkey,int& nVersionRet,crypto::CCryptoCipher& cipherRet)
{
    return false;
}

bool CService::SignSignature(const crypto::CPubKey& pubkey,const uint256& hash,vector<unsigned char>& vchSig)
{
    return false;
}
    
