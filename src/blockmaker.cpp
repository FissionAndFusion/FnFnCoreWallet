// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "blockmaker.h"
#include "address.h"
using namespace std;
using namespace walleve;
using namespace multiverse;

#define INITIAL_HASH_RATE      1024
#define WAIT_AGREEMENT_TIME    (BLOCK_TARGET_SPACING - 5)
#define WAIT_NEWBLOCK_TIME     (BLOCK_TARGET_SPACING - 5)

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
  nMakerStatus(MAKER_HOLD),hashLastBlock(0),nLastBlockTime(0),nLastBlockHeight(0),nLastAgreement(0),nLastWeight(0)
{
    pCoreProtocol = NULL;
    pWorldLine = NULL;
    pTxPool = NULL;
    pDispatcher = NULL;
    pConsensus = NULL;
    mapHashAlgo[CM_BLAKE512] = new CHashAlgo_Blake512(INITIAL_HASH_RATE);    
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

    if (!WalleveGetObject("txpool",pTxPool))
    {
        WalleveLog("Failed to request txpool\n");
        return false;
    }

    if (!WalleveGetObject("dispatcher",pDispatcher))
    {
        WalleveLog("Failed to request dispatcher\n");
        return false;
    }

    if (!WalleveGetObject("consensus",pConsensus))
    {
        WalleveLog("Failed to request consensus\n");
        return false;
    }

    if (!MintConfig()->destMPVss.IsNull() && MintConfig()->keyMPVss != 0)
    { 
        CBlockMakerProfile profile(0,MintConfig()->destMPVss,MintConfig()->keyMPVss);
        if (profile.IsValid())
        {
            mapDelegatedProfile.insert(make_pair(profile.GetDestination(),profile));
        }
    }

    if (!MintConfig()->destBlake512.IsNull() && MintConfig()->keyBlake512 != 0)
    { 
        CBlockMakerProfile profile(CM_BLAKE512,MintConfig()->destBlake512, MintConfig()->keyBlake512);
        if (profile.IsValid())
        {
            mapWorkProfile.insert(make_pair(CM_BLAKE512,profile));
        }
    }

    return true;
}

void CBlockMaker::WalleveHandleDeinitialize()
{
    pCoreProtocol = NULL;
    pWorldLine = NULL;
    pTxPool = NULL;
    pDispatcher = NULL;
    pConsensus = NULL;

    mapWorkProfile.clear();
    mapDelegatedProfile.clear();
}

bool CBlockMaker::WalleveHandleInvoke()
{
    if (!IBlockMaker::WalleveHandleInvoke())
    {
        return false;
    }

    if (!pWorldLine->GetLastBlock(pCoreProtocol->GetGenesisBlockHash(),hashLastBlock,nLastBlockHeight,nLastBlockTime))
    {
        return false;
    }

    if (!mapWorkProfile.empty() || !mapDelegatedProfile.empty())
    {
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
        hashLastBlock = eventUpdate.data.hashBlock;
        nLastBlockTime = eventUpdate.data.nBlockTime;
        nLastBlockHeight = eventUpdate.data.nBlockHeight;
        nLastAgreement = eventUpdate.data.nAgreement;
        nLastWeight = eventUpdate.data.nWeight;
    }
    cond.notify_all();
    
    return true;
}

bool CBlockMaker::Wait(long nSeconds)
{
    boost::system_time const timeout = boost::get_system_time()
                                       + boost::posix_time::seconds(nSeconds);
    boost::unique_lock<boost::mutex> lock(mutex);
    while (nMakerStatus == MAKER_RUN)
    {
        if (!cond.timed_wait(lock,timeout))
        {
            break;
        }
    }
    return (nMakerStatus == MAKER_RUN);
}

bool CBlockMaker::WaitAgreement(CBlockMakerAgreement& agree,int64 nTimeAgree,int nHeight)
{
    int64 nWait = nTimeAgree - WalleveGetNetTime();
    if (nWait > 0 && !Wait(nWait))
    {
        return false;
    }
    pConsensus->GetAgreement(nHeight,agree.nAgreement,agree.nWeight,agree.vBallot);
    return true;
}

bool CBlockMaker::WaitDelegatedBlock(int64 nPredictedTime,const uint256& nAgreement,
                                     uint256& hashNewBlock,int64& nNewBlockTime,int& nNewBlockHeight)
{
    hashNewBlock    = 0;
    nNewBlockTime   = 0;
    nNewBlockHeight = 0;

    boost::system_time const timeout = boost::get_system_time()
                                       + boost::posix_time::seconds(nPredictedTime - WalleveGetNetTime());
    boost::unique_lock<boost::mutex> lock(mutex);
    while (nMakerStatus == MAKER_RUN)
    {
        if (!cond.timed_wait(lock,timeout))
        {
            break;
        }
    }
    if (nMakerStatus == MAKER_RESET)
    {
        hashNewBlock = hashLastBlock;
        nNewBlockTime = nLastBlockTime;
        nNewBlockHeight = nLastBlockHeight;

        if (nAgreement == nLastAgreement)
        {
            nMakerStatus = MAKER_RUN;
            return true;
        }
    }
    return false;
}

void CBlockMaker::PrepareBlock(CBlock& block,const uint256& hashPrev,int64 nPrevTime,int nPrevHeight,const CBlockMakerAgreement& agreement)
{
    block.SetNull();
    block.nType = CBlock::BLOCK_PRIMARY;
    block.nTimeStamp = nPrevTime + BLOCK_TARGET_SPACING;
    block.hashPrev = hashPrev;
    CProofOfSecretShare proof; 
    proof.nWeight = agreement.nWeight;
    proof.nAgreement = agreement.nAgreement;
    proof.Save(block.vchProof);
    if (agreement.nAgreement != 0)
    {
        pConsensus->GetProof(nPrevHeight + 1,block.vchProof);
    }
}

void CBlockMaker::ArrangeBlockTx(CBlock& block,const uint256& hashFork,const CBlockMakerProfile& profile)
{
    size_t nMaxTxSize = MAX_BLOCK_SIZE - GetSerializeSize(block) - profile.GetSignatureSize();
    int64 nTotalTxFee = 0;
    pTxPool->ArrangeBlockTx(hashFork,nMaxTxSize,block.vtx,nTotalTxFee); 
    block.hashMerkle = block.CalcMerkleTreeRoot();
}

bool CBlockMaker::SignBlock(CBlock& block,const CBlockMakerProfile& profile)
{
    uint256 hashSig = block.GetHash();
    vector<unsigned char> vchMintSig;
    if (!profile.keyMint.Sign(hashSig,vchMintSig))
    {
        return false;
    }
    return profile.templMint->BuildBlockSignature(hashSig,vchMintSig,block.vchSig);
}

bool CBlockMaker::DispatchBlock(CBlock& block)
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

bool CBlockMaker::CreateProofOfWorkBlock(CBlock& block)
{
    int nConsensus = CM_BLAKE512;
    map<int,CBlockMakerProfile>::iterator it = mapWorkProfile.find(nConsensus);
    if (it == mapWorkProfile.end())
    {
        return false;
    } 

    CBlockMakerProfile& profile = (*it).second;
    CDestination destSendTo = profile.GetDestination();

    int nAlgo = nConsensus;
    int nBits;
    int64 nReward;
    if (!pWorldLine->GetProofOfWorkTarget(block.hashPrev,nAlgo,nBits,nReward))
    {
        return false;
    }

    CTransaction& txMint = block.txMint;
    txMint.nType = CTransaction::TX_WORK;
    txMint.hashAnchor = block.hashPrev;
    txMint.sendTo = destSendTo;
    txMint.nAmount = nReward;
    
    block.vchProof.resize(block.vchProof.size() + CProofOfHashWorkCompact::PROOFHASHWORK_SIZE);
    CProofOfHashWorkCompact proof;
    proof.nAlgo = nAlgo;
    proof.nBits = nBits;
    proof.nNonce = 0;
    proof.Save(block.vchProof);

    if (!CreateProofOfWork(block,mapHashAlgo[profile.nAlgo]))
    {
        return false;
    }

    ArrangeBlockTx(block,pCoreProtocol->GetGenesisBlockHash(),profile);

    return SignBlock(block,profile);
}

bool CBlockMaker::ProcessDelegatedProofOfStake(CBlock& block,const CBlockMakerAgreement& agreement,int nPrevHeight)
{
    map<CDestination,CBlockMakerProfile>::iterator it = mapDelegatedProfile.find(agreement.vBallot[0]);
    if (it != mapDelegatedProfile.end())
    {
        CBlockMakerProfile& profile = (*it).second;
        if (CreateDelegatedBlock(block,pCoreProtocol->GetGenesisBlockHash(),profile,agreement.nWeight))
        {
            if (DispatchBlock(block))
            {
                CreatePiggyback(profile,agreement,block,nPrevHeight);
            }
        }
    }
    uint256 hashNewBlock = 0;
    int64 nNewBlockTime  = 0;
    int nNewBlockHeight  = 0;
    if (WaitDelegatedBlock(block.nTimeStamp + WAIT_NEWBLOCK_TIME,agreement.nAgreement,
                           hashNewBlock,nNewBlockTime,nNewBlockHeight))
    {
        ProcessExtended(agreement,hashNewBlock,nNewBlockTime,nNewBlockHeight);
        {
            boost::unique_lock<boost::mutex> lock(mutex);
            if (nMakerStatus == MAKER_RUN)
            {
                nMakerStatus = MAKER_RESET;
            }
        }
    }

    return (hashNewBlock != 0);
}

void CBlockMaker::ProcessExtended(const CBlockMakerAgreement& agreement,
                                  const uint256& hashPrimaryBlock,int64 nPrimaryBlockTime,int nPrimaryBlockHeight)
{
    vector<CBlockMakerProfile*> vProfile;
    set<uint256> setFork;

    if (!GetAvailiableDelegatedProfile(agreement.vBallot,vProfile)
        || !GetAvailiableExtendedFork(nPrimaryBlockTime,nPrimaryBlockHeight,setFork))
    {
        return;
    }

    int64 nTime = WalleveGetNetTime();
    if (nTime <= nPrimaryBlockTime)
    {
        nTime = nPrimaryBlockTime + EXTENDED_BLOCK_SPACING;
    }
    while (nTime - nPrimaryBlockTime < WAIT_NEWBLOCK_TIME)
    {
        int nIndex = ((nTime - nPrimaryBlockTime + (EXTENDED_BLOCK_SPACING - 1)) / EXTENDED_BLOCK_SPACING);
        const CBlockMakerProfile* pProfile = vProfile[nIndex % vProfile.size()];
        if (pProfile != NULL)
        {
            if (!Wait(nTime - WalleveGetNetTime()))
            {
                return;
            }
            CreateExtended(*pProfile,agreement,setFork,nTime);
        }
        nTime += EXTENDED_BLOCK_SPACING;
    }
}

bool CBlockMaker::CreateDelegatedBlock(CBlock& block,const uint256& hashFork,const CBlockMakerProfile& profile,size_t nWeight)
{
    CDestination destSendTo = profile.GetDestination();

    int64 nReward;
    if (!pWorldLine->GetDelegatedProofOfStakeReward(block.hashPrev,nWeight,nReward))
    {
        return false;
    }

    CTransaction& txMint = block.txMint;
    txMint.nType = CTransaction::TX_STAKE;
    txMint.hashAnchor = block.hashPrev;
    txMint.sendTo = destSendTo;
    txMint.nAmount = nReward;
        
    ArrangeBlockTx(block,hashFork,profile);

    return SignBlock(block,profile);
}

void CBlockMaker::CreatePiggyback(const CBlockMakerProfile& profile,const CBlockMakerAgreement& agreement,
                                  const CBlock& refblock,int nPrevHeight)
{
    CProofOfPiggyback proof;
    proof.nWeight = agreement.nWeight;
    proof.nAgreement = agreement.nAgreement;
    proof.hashRefBlock = refblock.GetHash();

    map<uint256,CForkStatus> mapForkStatus;
    pWorldLine->GetForkStatus(mapForkStatus);
    for (map<uint256,CForkStatus>::iterator it = mapForkStatus.begin();it != mapForkStatus.end();++it)
    {
        const uint256& hashFork = (*it).first;
        CForkStatus& status = (*it).second;
        if (hashFork != pCoreProtocol->GetGenesisBlockHash() 
            && status.nLastBlockHeight == nPrevHeight
            && status.nLastBlockTime < refblock.nTimeStamp)
        {
            CBlock block;
            block.nType = CBlock::BLOCK_SUBSIDIARY;
            block.nTimeStamp = refblock.nTimeStamp;
            block.hashPrev = status.hashLastBlock;
            proof.Save(block.vchProof);

            if (CreateDelegatedBlock(block,hashFork,profile,agreement.nWeight))
            {
                DispatchBlock(block);
            }
        }
    } 
}

void CBlockMaker::CreateExtended(const CBlockMakerProfile& profile,const CBlockMakerAgreement& agreement,
                                 const set<uint256>& setFork,int64 nTime)
{
    CProofOfSecretShare proof;
    proof.nWeight = agreement.nWeight;
    proof.nAgreement = agreement.nAgreement;
    BOOST_FOREACH(const uint256& hashFork,setFork)
    {
        uint256 hashLastBlock;
        int nLastBlockHeight;
        int64 nLastBlockTime;
        if (pTxPool->Count(hashFork) 
            && pWorldLine->GetLastBlock(hashFork,hashLastBlock,nLastBlockHeight,nLastBlockTime) 
            && nLastBlockTime < nTime)
        {
            CBlock block;
            block.nType = CBlock::BLOCK_EXTENDED;
            block.nTimeStamp = nTime;
            block.hashPrev = hashLastBlock;
            proof.Save(block.vchProof);
            CTransaction& txMint = block.txMint;
            txMint.nType = CTransaction::TX_STAKE;
            txMint.hashAnchor = hashLastBlock;
            txMint.sendTo = profile.GetDestination();
            txMint.nAmount = 0;
            ArrangeBlockTx(block,hashFork,profile);
            if (!block.vtx.empty() && SignBlock(block,profile))
            {
                DispatchBlock(block);
            }
        }
    }
}

bool CBlockMaker::CreateProofOfWork(CBlock& block,CBlockMakerHashAlgo* pHashAlgo)
{
    const int64 nTimePrev = block.nTimeStamp - BLOCK_TARGET_SPACING;
    block.nTimeStamp -= 5;

    if (WalleveGetNetTime() > block.nTimeStamp)
    {
        block.nTimeStamp = WalleveGetNetTime();
    }

    CProofOfHashWorkCompact proof;
    proof.Load(block.vchProof);

    int nBits = proof.nBits;

    uint256 hashTarget = (~uint256(0) >> nBits);

    vector<unsigned char> vchProofOfWork;
    block.GetSerializedProofOfWorkData(vchProofOfWork);

    uint32& nTime = *((uint32*)&vchProofOfWork[4]);
    uint256& nNonce = *((uint256*)&vchProofOfWork[vchProofOfWork.size() - 32]);

    int64& nHashRate = pHashAlgo->nHashRate;

    while (!Interrupted())
    {
        hashTarget = (~uint256(0) >> pCoreProtocol->GetProofOfWorkRunTimeBits(nBits,nTime,nTimePrev));
        for (int i = 0;i < nHashRate;i++)
        {
            uint256 hash = pHashAlgo->Hash(vchProofOfWork);
            if (hash <= hashTarget)
            {
                block.nTimeStamp = nTime;
                proof.nNonce     = nNonce;
                proof.Save(block.vchProof);

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

bool CBlockMaker::GetAvailiableDelegatedProfile(const vector<CDestination>& vBallot,vector<CBlockMakerProfile*>& vProfile)
{
    int nAvailProfile = 0;
    vProfile.reserve(vBallot.size());
    BOOST_FOREACH(const CDestination& dest,vBallot)
    {
        map<CDestination,CBlockMakerProfile>::iterator it = mapDelegatedProfile.find(dest);
        if (it != mapDelegatedProfile.end())
        {
            vProfile.push_back(&(*it).second);
            ++nAvailProfile;
        }
        else
        {
            vProfile.push_back((CBlockMakerProfile*)NULL);
        }
    }
    
    return (!!nAvailProfile);
}

bool CBlockMaker::GetAvailiableExtendedFork(int64 nPrimaryBlockTime,int nPrimaryBlockHeight,set<uint256>& setFork)
{
    map<uint256,CForkStatus> mapForkStatus;
    pWorldLine->GetForkStatus(mapForkStatus);
    for (map<uint256,CForkStatus>::iterator it = mapForkStatus.begin();it != mapForkStatus.end();++it)
    {
        CProfile profile;
        const uint256& hashFork = (*it).first;
        CForkStatus& status = (*it).second;
        if (hashFork != pCoreProtocol->GetGenesisBlockHash() 
            && status.nLastBlockHeight == nPrimaryBlockHeight
            && status.nLastBlockTime >= nPrimaryBlockTime
            && pWorldLine->GetForkProfile(hashFork,profile) && !profile.IsEnclosed())
        {
            setFork.insert(hashFork);
        }
    }
    return (!setFork.empty());
}

void CBlockMaker::BlockMakerThreadFunc()
{
    const char* ConsensusMethodName[CM_MAX] = {"mpvss","blake512"};
    WalleveLog("Block maker started\n");
    for (map<int,CBlockMakerProfile>::iterator it = mapWorkProfile.begin();it != mapWorkProfile.end();++it)
    {
        CBlockMakerProfile& profile = (*it).second;
        WalleveLog("Profile [%s] : dest=%s,pubkey=%s\n",
                   ConsensusMethodName[(*it).first],
                   CMvAddress(profile.destMint).ToString().c_str(),
                   profile.keyMint.GetPubKey().GetHex().c_str());
    }
    for (map<CDestination,CBlockMakerProfile>::iterator it = mapDelegatedProfile.begin();
         it != mapDelegatedProfile.end();++it)
    {
        CBlockMakerProfile& profile = (*it).second;
        WalleveLog("Profile [%s] : dest=%s,pubkey=%s\n",
                   ConsensusMethodName[CM_MPVSS],
                   CMvAddress(profile.destMint).ToString().c_str(),
                   profile.keyMint.GetPubKey().GetHex().c_str());
    }
    for (;;)
    {
        uint256 hashPrev;
        int64 nPrevTime;
        int nPrevHeight;
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
            hashPrev = hashLastBlock;
            nPrevTime = nLastBlockTime;
            nPrevHeight = nLastBlockHeight;
            nMakerStatus = MAKER_RUN;
        }

        CBlockMakerAgreement agree;
        if (!WaitAgreement(agree,nPrevTime + WAIT_AGREEMENT_TIME,nPrevHeight + 1))
        {
            continue;
        }
       
        WalleveLog("GetAgreement : %s at height=%d, weight=%lu\n",agree.nAgreement.GetHex().c_str(),
                                                                  nPrevHeight,agree.nWeight); 
       
        CBlock block;
        try
        {
            int nNextStatus = MAKER_HOLD;
            
            PrepareBlock(block,hashPrev,nPrevTime,nPrevHeight,agree);
            
            if (!agree.IsProofOfWork())
            {
                if (!ProcessDelegatedProofOfStake(block,agree,nPrevHeight))
                {
                    agree = CBlockMakerAgreement();
                    PrepareBlock(block,hashPrev,nPrevTime + BLOCK_TARGET_SPACING,nPrevHeight,agree);
                }
            }

            if (agree.IsProofOfWork())
            {
                if (CreateProofOfWorkBlock(block))
                {
                    if (!DispatchBlock(block))
                    {
                        nNextStatus = MAKER_RESET;
                    }
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
