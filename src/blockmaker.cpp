// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "blockmaker.h"
#include "address.h"
using namespace std;
using namespace walleve;
using namespace multiverse;

#define INITIAL_HASH_RATE      1024
//////////////////////////////
// CBlockMakerHashAlgo
class CHashAlgo_Blake512 : public multiverse::CBlockMakerHashAlgo
{
public:
    CHashAlgo_Blake512(int64 nHashRateIn) : CBlockMakerHashAlgo("blake512",nHashRateIn) {}
    uint256 Hash(const std::vector<unsigned char>& vchData)
    {
        return crypto::CryptoHash(&vchData[0],vchData.size());
    } 
};

//////////////////////////////
// CBlockMakerProfile 
bool CBlockMakerProfile::BuildTemplate()
{
    crypto::CPubKey pubkey;
    if (destMint.GetPubKey(pubkey) && pubkey == keyMint.GetPubKey())
    {
        return false;
    }
    if (nAlgo == CM_MPVSS)
    {
        templMint = CTemplatePtr(new CTemplateDelegate(keyMint.GetPubKey(),destMint));
    }
    else if (nAlgo < CM_MAX)
    {
        templMint = CTemplatePtr(new CTemplateMint(keyMint.GetPubKey(),destMint));
    }
    if (templMint != NULL && templMint->IsNull())
    {
        templMint.reset();
    }
    return (templMint != NULL);
}

//////////////////////////////
// CBlockMaker 

CBlockMaker::CBlockMaker()
: thrMaker("blockmaker",boost::bind(&CBlockMaker::BlockMakerThreadFunc,this)), 
  nMakerStatus(MAKER_HOLD),hashLastBlock(0),nLastBlockTime(0)
{
    pCoreProtocol = NULL;
    pWorldLine = NULL;
    pDispatcher = NULL;
    pWallet = NULL;
    mapHashAlgo[POWA_BLAKE512] = new CHashAlgo_Blake512(INITIAL_HASH_RATE);    
    
}

CBlockMaker::~CBlockMaker()
{
    for (map<int,CBlockMakerHashAlgo*>::iterator it = mapHashAlgo.begin();it != mapHashAlgo.end();++it)
    {
        delete ((*it).second);
    }
    mapHashAlgo.clear();
}

bool CBlockMaker::WalleveHandleInitialize()
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

    if (!WalleveGetObject("dispatcher",pDispatcher))
    {
        WalleveLog("Failed to request dispatcher\n");
        return false;
    }

    if (!WalleveGetObject("wallet",pWallet))
    {
        WalleveLog("Failed to request wallet\n");
        return false;
    }

    if (!MintConfig()->destMPVss.IsNull() && MintConfig()->keyMPVss != 0)
    { 
        CBlockMakerProfile profile(0,MintConfig()->destMPVss,MintConfig()->keyMPVss);
        if (profile.IsValid())
        {
            mapProfile.insert(make_pair(CM_MPVSS,profile));
        }
    }

    if (!MintConfig()->destBlake512.IsNull() && MintConfig()->keyBlake512 != 0)
    { 
        CBlockMakerProfile profile(POWA_BLAKE512,MintConfig()->destBlake512, MintConfig()->keyBlake512);
        if (profile.IsValid())
        {
            mapProfile.insert(make_pair(CM_BLAKE512,profile));
        }
    }

    return true;
}

void CBlockMaker::WalleveHandleDeinitialize()
{
    pCoreProtocol = NULL;
    pWorldLine = NULL;
    pDispatcher = NULL;
    pWallet = NULL;

    mapProfile.clear();
}

bool CBlockMaker::WalleveHandleInvoke()
{
    if (!IBlockMaker::WalleveHandleInvoke())
    {
        return false;
    }
    int nHeight;
    if (!pWorldLine->GetLastBlock(pCoreProtocol->GetGenesisBlockHash(),hashLastBlock,nHeight,nLastBlockTime))
    {
        return false;
    }

    if (!mapProfile.empty())
    {
        //nMakerStatus = MAKER_HOLD;
        nMakerStatus = MAKER_RUN;

        if (!WalleveThreadDelayStart(thrMaker))
        {
            return false;
        }
    }

    return true;
}

void CBlockMaker::WalleveHandleHalt()
{
    {
        boost::unique_lock<boost::mutex> lock(mutex);
        nMakerStatus = MAKER_EXIT;
    }
    cond.notify_all();
    WalleveThreadExit(thrMaker);
    IBlockMaker::WalleveHandleHalt();
}

bool CBlockMaker::HandleEvent(CMvEventBlockMakerUpdate& eventUpdate)
{
    {
        boost::unique_lock<boost::mutex> lock(mutex);
        nMakerStatus = MAKER_RESET;
        hashLastBlock = eventUpdate.data.first;
        nLastBlockTime = eventUpdate.data.second;
    }
    cond.notify_all();
    
    return true;
}

bool CBlockMaker::Wait(long nSeconds)
{
    boost::system_time const timeout = boost::get_system_time()
                                       + boost::posix_time::seconds(nSeconds);
    boost::unique_lock<boost::mutex> lock(mutex);
    while(nMakerStatus == MAKER_RUN)
    {
        if (!cond.timed_wait(lock,timeout))
        {
            break;
        }
    }
    return (nMakerStatus == MAKER_RUN);
}

bool CBlockMaker::CreateNewBlock(CBlock& block,const uint256& hashPrev,int64 nPrevTime)
{
    block.nType = CBlock::BLOCK_PRIMARY;
    block.nTimeStamp = nPrevTime + BLOCK_TARGET_SPACING - 10;
    block.hashPrev = hashPrev;
   
    int nConsensus = CM_BLAKE512;
    map<int,CBlockMakerProfile>::iterator it = mapProfile.find(nConsensus);
    if (it == mapProfile.end())
    {
        return false;
    } 

    CBlockMakerProfile& profile = (*it).second;
    if (!CreateProofOfWork(block,profile.nAlgo,CDestination(profile.templMint->GetTemplateId())))
    {
        return false;
    }
   
    block.hashMerkle = block.CalcMerkleTreeRoot();
 
    if (!SignBlock(block,profile))
    {
        return false;
    }

    return true;
}

bool CBlockMaker::SignBlock(CBlock& block,CBlockMakerProfile& profile)
{
    uint256 hashSig = block.GetHash();
    vector<unsigned char> vchMintSig;
    if (!profile.keyMint.Sign(hashSig,vchMintSig))
    {
        return false;
    }
    return profile.templMint->BuildBlockSignature(hashSig,vchMintSig,block.vchSig);
}

bool CBlockMaker::CreateProofOfWork(CBlock& block,int nAlgo,const CDestination& dest)
{
    int nBits;
    int64 nReward;
    if (!pWorldLine->GetProofOfWorkTarget(block.hashPrev,nAlgo,nBits,nReward))
    {
        return false;
    }
    CTransaction& txMint = block.txMint;
    txMint.nType = CTransaction::TX_WORK;
    txMint.hashAnchor = block.hashPrev;
    txMint.sendTo = dest;
    txMint.nAmount = nReward;
    CProofOfHashWork proof;
    proof.nWeight = 0;
    proof.nAgreement = 0;
    proof.nAlgo = nAlgo;
    proof.nBits = nBits;
    proof.nNonce = 0;
    proof.Save(block.vchProof);
    if (WalleveGetNetTime() > block.nTimeStamp)
    {
        block.nTimeStamp = WalleveGetNetTime();
    }

    uint256 hashTarget = (~uint256(0) >> nBits);
    vector<unsigned char> vchProofOfWork;
    block.GetSerializedProofOfWorkData(vchProofOfWork);
    uint32& nTime = *((uint32*)&vchProofOfWork[4]);
    uint256& nNonce = *((uint256*)&vchProofOfWork[vchProofOfWork.size() - 4]);
    CBlockMakerHashAlgo* pHashAlgo = mapHashAlgo[nAlgo]; 
    int64& nHashRate = pHashAlgo->nHashRate;
    while (!Interrupted())
    {
        hashTarget = (~uint256(0) >> pCoreProtocol->GetProofOfWorkRunTimeBits(nBits,nTime,nLastBlockTime));
        for (int i = 0;i < nHashRate;i++)
        {
            uint256 hash = pHashAlgo->Hash(vchProofOfWork);
            if (hash <= hashTarget)
            {
                WalleveLog("Proof-of-work(%s) block found (%ld)\nhash : %s\ntarget : %s\n",
                           pHashAlgo->strAlgo.c_str(),nHashRate,hash.GetHex().c_str(),hashTarget.GetHex().c_str());
                return true;
            }
            nNonce++;
        }

        int64 nNetTime = WalleveGetNetTime();
        if (nTime + 1 < nNetTime)
        {
            nHashRate /= (nNetTime - nTime);
            nTime = nNetTime;
        }
        else if (nTime == nNetTime)
        {
            nHashRate *= 2;
        }
    }
    return false;
}

bool CBlockMaker::DispatchNewBlock(CBlock& block)
{
    int nWait = block.nTimeStamp - WalleveGetNetTime();
    if (nWait > 0 && !Wait(nWait))
    {
        return false;
    }
    MvErr err = pDispatcher->AddNewBlock(block);
    if (err != MV_OK)
    {
        WalleveLog("Dispatch new block failed (%s) : %s\n",err,MvErrString(err));
        return false;
    }
    return true;
}

void CBlockMaker::BlockMakerThreadFunc()
{
    const char* ConsensusMethodName[CM_MAX] = {"mpvss","blake512"};
    WalleveLog("Block maker started\n");
    for (map<int,CBlockMakerProfile>::iterator it = mapProfile.begin();it != mapProfile.end();++it)
    {
        CBlockMakerProfile& profile = (*it).second;
        WalleveLog("Profile [%s] : dest=%s,pubkey=%s\n",
                   ConsensusMethodName[(*it).first],
                   CMvAddress(profile.destMint).ToString().c_str(),
                   profile.keyMint.GetPubKey().GetHex().c_str());
    }
    for (;;)
    {
        uint256 hashPrev;
        int64 nPrevTime;
        {
            boost::unique_lock<boost::mutex> lock(mutex);
            while (nMakerStatus == MAKER_HOLD)
            {
                cond.wait(lock);
            }
            if (nMakerStatus == MAKER_EXIT)
            {
                break;
            }
            nMakerStatus = MAKER_RUN;
            hashPrev = hashLastBlock;
            nPrevTime = nLastBlockTime;
        }

        CBlock block;
        try
        {
            int nNextStatus = MAKER_HOLD;
            if (CreateNewBlock(block,hashPrev,nPrevTime))
            {
                if (!DispatchNewBlock(block))
                {
                    nNextStatus = MAKER_RESET;
                }
            }

            {
                boost::unique_lock<boost::mutex> lock(mutex);
                if (nMakerStatus == MAKER_RUN)
                {
                    nMakerStatus = nNextStatus;
                }
            }
        }
        catch (...)
        {
            break;
        }
    }

    WalleveLog("Block maker exited\n");
}

