// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "forkblockmaker.h"
#include "address.h"
#include "template/proof.h"
#include "template/delegate.h"

using namespace std;
using namespace walleve;
using namespace multiverse;

#define INITIAL_HASH_RATE      1024
#define WAIT_AGREEMENT_TIME    (BLOCK_TARGET_SPACING / 2)
#define WAIT_NEWBLOCK_TIME     (BLOCK_TARGET_SPACING + 5)

//////////////////////////////
// CForkBlockMakerHashAlgo
class CHashAlgo_MPVSS : public multiverse::CForkBlockMakerHashAlgo
{
public:
    CHashAlgo_MPVSS(int64 nHashRateIn) : CForkBlockMakerHashAlgo("mpvss",nHashRateIn) {}
    uint256 Hash(const std::vector<unsigned char>& vchData)
    {
        return crypto::CryptoHash(&vchData[0],vchData.size());
    } 
};

//////////////////////////////
// CBlockMakerProfile 
bool CForkBlockMakerProfile::BuildTemplate()
{
    crypto::CPubKey pubkey;
    if (destMint.GetPubKey(pubkey) && pubkey == keyMint.GetPubKey())
    {
        return false;
    }
    if (nAlgo == CM_MPVSS)
    {
        templMint = CTemplateMint::CreateTemplatePtr(new CTemplateDelegate(keyMint.GetPubKey(),destMint));
    }
    else if (nAlgo < CM_MAX)
    {
        templMint = CTemplateMint::CreateTemplatePtr(new CTemplateProof(keyMint.GetPubKey(),destMint));
    }
    return (templMint != NULL);
}

//////////////////////////////
// CForkBlockMaker 

CForkBlockMaker::CForkBlockMaker()
: thrMaker("blockmaker",boost::bind(&CForkBlockMaker::BlockMakerThreadFunc,this)), 
  thrExtendedMaker("extendedmaker",boost::bind(&CForkBlockMaker::ExtendedMakerThreadFunc,this)), 
  nMakerStatus(ForkMakerStatus::MAKER_HOLD),hashLastBlock(uint64(0)),nLastBlockTime(0),
  nLastBlockHeight(0),nLastAgreement(uint64(0)),nLastWeight(0)
{
    pCoreProtocol = NULL;
    pWorldLine = NULL;
    pForkManager = NULL;
    pTxPool = NULL;
    pDispatcher = NULL;
    pConsensus = NULL;
    mapHashAlgo[CM_MPVSS] = new CHashAlgo_MPVSS(INITIAL_HASH_RATE); 
}

CForkBlockMaker::~CForkBlockMaker()
{
    for (map<int,CForkBlockMakerHashAlgo*>::iterator it = mapHashAlgo.begin();it != mapHashAlgo.end();++it)
    {
        delete ((*it).second);
    }
    mapHashAlgo.clear();
}
    
bool CForkBlockMaker::HandleEvent(CMvEventBlockMakerUpdate& eventUpdate)
{
    {
        boost::unique_lock<boost::mutex> lock(mutex);
        nMakerStatus = ForkMakerStatus::MAKER_RESET;
        hashLastBlock = eventUpdate.data.hashBlock;
        nLastBlockTime = eventUpdate.data.nBlockTime;
        nLastBlockHeight = eventUpdate.data.nBlockHeight;
        nLastAgreement = eventUpdate.data.nAgreement;
        nLastWeight = eventUpdate.data.nWeight;
    }
    cond.notify_all();
    
    return true;
}

bool CForkBlockMaker::WalleveHandleInitialize()
{
    if (!WalleveGetObject("coreprotocol",pCoreProtocol))
    {
        WalleveError("Failed to request coreprotocol\n");
        return false;
    }

    if (!WalleveGetObject("worldline",pWorldLine))
    {
        WalleveError("Failed to request worldline\n");
        return false;
    }

    if (!WalleveGetObject("forkmanager",pForkManager))
    {
        WalleveError("Failed to request forkmanager\n");
        return false;
    }

    if (!WalleveGetObject("txpool",pTxPool))
    {
        WalleveError("Failed to request txpool\n");
        return false;
    }

    if (!WalleveGetObject("dispatcher",pDispatcher))
    {
        WalleveError("Failed to request dispatcher\n");
        return false;
    }

    if (!WalleveGetObject("consensus",pConsensus))
    {
        WalleveError("Failed to request consensus\n");
        return false;
    }

    if (!ForkNodeMintConfig()->destMPVss.IsNull() && ForkNodeMintConfig()->keyMPVss != 0)
    { 
        CForkBlockMakerProfile profile(CM_MPVSS,ForkNodeMintConfig()->destMPVss,ForkNodeMintConfig()->keyMPVss);
        if (profile.IsValid())
        {
            mapDelegatedProfile.insert(make_pair(profile.GetDestination(),profile));
        }
        else
        {
            WalleveError("CForkBlockMakerProfile is invalid.\n");
            return false;
        }
        
    }
    else
    {
        WalleveError("ForkNodeMintConfig is invalid.\n");
        return false;
    }
    

    return true;
}
    
void CForkBlockMaker::WalleveHandleDeinitialize()
{
    pCoreProtocol = NULL;
    pWorldLine = NULL;
    pForkManager = NULL;
    pTxPool = NULL;
    pDispatcher = NULL;
    pConsensus = NULL;

    mapDelegatedProfile.clear();
}
    
bool CForkBlockMaker::WalleveHandleInvoke()
{
    if (!IBlockMaker::WalleveHandleInvoke())
    {
        return false;
    }

    if (!pWorldLine->GetLastBlock(pCoreProtocol->GetGenesisBlockHash(),hashLastBlock,nLastBlockHeight,nLastBlockTime))
    {
        return false;
    }

    if (!mapDelegatedProfile.empty())
    {
        nMakerStatus = ForkMakerStatus::MAKER_HOLD;

        if (!WalleveThreadDelayStart(thrMaker))
        {
            return false;
        }
        if (!WalleveThreadDelayStart(thrExtendedMaker))
        {
            return false;
        }
    }

    return true;
}
    
void CForkBlockMaker::WalleveHandleHalt()
{
    {
        boost::unique_lock<boost::mutex> lock(mutex);
        nMakerStatus = ForkMakerStatus::MAKER_EXIT;
    }
    cond.notify_all();

    thrMaker.Interrupt();
    WalleveThreadExit(thrMaker);

    thrExtendedMaker.Interrupt();
    WalleveThreadExit(thrExtendedMaker);
    IBlockMaker::WalleveHandleHalt();
}
    
bool CForkBlockMaker::Wait(long nSeconds)
{
    boost::system_time const timeout = boost::get_system_time()
                                       + boost::posix_time::seconds(nSeconds);
    boost::unique_lock<boost::mutex> lock(mutex);
    while (nMakerStatus == ForkMakerStatus::MAKER_RUN)
    {
        if (!cond.timed_wait(lock,timeout))
        {
            break;
        }
    }
    return (nMakerStatus == ForkMakerStatus::MAKER_RUN);
}
    
bool CForkBlockMaker::Wait(long nSeconds,const uint256& hashPrimaryBlock)
{
    boost::system_time const timeout = boost::get_system_time()
                                       + boost::posix_time::seconds(nSeconds);
    boost::unique_lock<boost::mutex> lock(mutex);
    while (hashPrimaryBlock == hashLastBlock && nMakerStatus != ForkMakerStatus::MAKER_EXIT)
    {
        if (!cond.timed_wait(lock,timeout))
        {
            return (hashPrimaryBlock == hashLastBlock && nMakerStatus != ForkMakerStatus::MAKER_EXIT);
        }
    } 
    return false;
}
    
void CForkBlockMaker::ArrangeBlockTx(CBlock& block,const uint256& hashFork,const CForkBlockMakerProfile& profile)
{
    size_t nMaxTxSize = MAX_BLOCK_SIZE - GetSerializeSize(block) - profile.GetSignatureSize();
    int64 nTotalTxFee = 0;
    pTxPool->ArrangeBlockTx(hashFork,block.GetBlockTime(),nMaxTxSize,block.vtx,nTotalTxFee); 
    block.hashMerkle = block.CalcMerkleTreeRoot();
    block.txMint.nAmount += nTotalTxFee;
}
    
bool CForkBlockMaker::SignBlock(CBlock& block,const CForkBlockMakerProfile& profile)
{
    uint256 hashSig = block.GetHash();
    vector<unsigned char> vchMintSig;
    if (!profile.keyMint.Sign(hashSig,vchMintSig))
    {
        return false;
    }
    return profile.templMint->BuildBlockSignature(hashSig,vchMintSig,block.vchSig);
}
    
bool CForkBlockMaker::DispatchBlock(const CBlock& block)
{
    int nWait = block.nTimeStamp - WalleveGetNetTime();
    if (nWait > 0 && !Wait(nWait))
    {
        return false;
    }
    MvErr err = pDispatcher->AddNewBlock(block);
    if (err != MV_OK)
    {
        WalleveError("Dispatch new block failed (%d) : %s in ForkNode \n", err, MvErrString(err));
        return false;
    }
    return true;
}
    
void CForkBlockMaker::ProcessDelegatedProofOfStake(CBlock& block,const CDelegateAgreement& agreement,const int32 nPrevHeight)
{
    map<CDestination,CForkBlockMakerProfile>::iterator it = mapDelegatedProfile.find(agreement.vBallot[0]);
    if (it != mapDelegatedProfile.end())
    {
        CForkBlockMakerProfile& profile = (*it).second;
        CreatePiggyback(profile,agreement,block,nPrevHeight);
    }
}
    
void CForkBlockMaker::ProcessExtended(const CDelegateAgreement& agreement,const uint256& hashPrimaryBlock,
                                                               int64 nPrimaryBlockTime,const int32 nPrimaryBlockHeight)
{
    vector<CForkBlockMakerProfile*> vProfile;
    set<uint256> setFork;

    if (!GetAvailableDelegatedProfile(agreement.vBallot,vProfile) || !GetAvailableExtendedFork(setFork))
    {
        return;
    }

    int64 nTime = nPrimaryBlockTime + EXTENDED_BLOCK_SPACING * 
                  ((WalleveGetNetTime() - nPrimaryBlockTime + (EXTENDED_BLOCK_SPACING - 1)) / EXTENDED_BLOCK_SPACING);
    if (nTime < nPrimaryBlockTime + EXTENDED_BLOCK_SPACING)
    {
        nTime = nPrimaryBlockTime + EXTENDED_BLOCK_SPACING;
    }
    while (nTime - nPrimaryBlockTime < BLOCK_TARGET_SPACING)
    {
        int nIndex = (nTime - nPrimaryBlockTime) / EXTENDED_BLOCK_SPACING;
        const CForkBlockMakerProfile* pProfile = vProfile[nIndex % vProfile.size()];
        if (pProfile != NULL)
        {
            if (!Wait(nTime - WalleveGetNetTime(),hashPrimaryBlock))
            {
                return;
            }
            CreateExtended(*pProfile,agreement,setFork,nPrimaryBlockHeight,nTime);
        }
        nTime += EXTENDED_BLOCK_SPACING;
    }
}
    
bool CForkBlockMaker::CreateDelegatedBlock(CBlock& block,const uint256& hashFork,const CForkBlockMakerProfile& profile,std::size_t nWeight)
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
    txMint.sendTo = destSendTo;  // to mpvss template address
    txMint.nAmount = nReward;
        
    ArrangeBlockTx(block,hashFork,profile);

    return SignBlock(block,profile);
}
    
void CForkBlockMaker::CreatePiggyback(const CForkBlockMakerProfile& profile,const CDelegateAgreement& agreement,const CBlock& refblock,const int32 nPrevHeight)
{
    CProofOfPiggyback proof;
    proof.nWeight = agreement.nWeight;
    proof.nAgreement = agreement.nAgreement;
    proof.hashRefBlock = hashLastBlock;

    map<uint256,CForkStatus> mapForkStatus;
    pWorldLine->GetForkStatus(mapForkStatus);
    for (map<uint256,CForkStatus>::iterator it = mapForkStatus.begin();it != mapForkStatus.end();++it)
    {
        const uint256& hashFork = (*it).first;
        CForkStatus& status = (*it).second;
        if (hashFork != pCoreProtocol->GetGenesisBlockHash() 
            && status.nLastBlockHeight == nPrevHeight
            && status.nLastBlockTime < nLastBlockTime)
        {
            CBlock block;
            block.nType = CBlock::BLOCK_SUBSIDIARY;
            block.nTimeStamp = nLastBlockTime;
            block.hashPrev = status.hashLastBlock;
            proof.Save(block.vchProof);

            if (CreateDelegatedBlock(block,hashFork,profile,agreement.nWeight))
            {
                DispatchBlock(block);
            }
        }
    }
}
    
void CForkBlockMaker::CreateExtended(const CForkBlockMakerProfile& profile,const CDelegateAgreement& agreement,const std::set<uint256>& setFork,const int32 nPrimaryBlockHeight,int64 nTime)
{
    CProofOfSecretShare proof;
    proof.nWeight = agreement.nWeight;
    proof.nAgreement = agreement.nAgreement;
    for(const uint256& hashFork : setFork)
    {
        uint256 hashLastBlock;
        int32 nLastBlockHeight;
        int64 nLastBlockTime;
        if (pTxPool->Count(hashFork) 
            && pWorldLine->GetLastBlock(hashFork,hashLastBlock,nLastBlockHeight,nLastBlockTime)
            && nPrimaryBlockHeight == nLastBlockHeight
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
    
bool CForkBlockMaker::GetAvailableDelegatedProfile(const std::vector<CDestination>& vBallot,std::vector<CForkBlockMakerProfile*>& vProfile)
{
    int nAvailProfile = 0;
    vProfile.reserve(vBallot.size());
    for(const CDestination& dest : vBallot)
    {
        map<CDestination,CForkBlockMakerProfile>::iterator it = mapDelegatedProfile.find(dest);
        if (it != mapDelegatedProfile.end())
        {
            vProfile.push_back(&(*it).second);
            ++nAvailProfile;
        }
        else
        {
            vProfile.push_back((CForkBlockMakerProfile*)NULL);
        }
    }
    
    return (!!nAvailProfile);
}
    
bool CForkBlockMaker::GetAvailableExtendedFork(std::set<uint256>& setFork)
{
    map<uint256,CForkStatus> mapForkStatus;
    pWorldLine->GetForkStatus(mapForkStatus);
    for (map<uint256,CForkStatus>::iterator it = mapForkStatus.begin();it != mapForkStatus.end();++it)
    {
        CProfile profile;
        const uint256& hashFork = (*it).first;
        if (hashFork != pCoreProtocol->GetGenesisBlockHash() 
            && pForkManager->IsAllowed(hashFork)
            && pWorldLine->GetForkProfile(hashFork,profile) && !profile.IsEnclosed())
        {
            setFork.insert(hashFork);
        }
    }
    return (!setFork.empty());
}

void CForkBlockMaker::BlockMakerThreadFunc()
{
    const char* ConsensusMethodName[CM_MAX] = {"mpvss","blake512"};
    WalleveLog("Fork Block maker started\n");
    
    for (map<CDestination,CForkBlockMakerProfile>::iterator it = mapDelegatedProfile.begin();
         it != mapDelegatedProfile.end();++it)
    {
        CForkBlockMakerProfile& profile = (*it).second;
        WalleveLog("Profile [%s] : dest=%s,pubkey=%s\n",
                   ConsensusMethodName[CM_MPVSS],
                   CMvAddress(profile.destMint).ToString().c_str(),
                   profile.keyMint.GetPubKey().GetHex().c_str());
    }

    uint256 hashPrimaryBlock = uint64(0);
    int64 nPrimaryBlockTime  = 0;
    int32 nPrimaryBlockHeight  = 0;

    {
        boost::unique_lock<boost::mutex> lock(mutex);
        hashPrimaryBlock    = hashLastBlock;
        nPrimaryBlockTime   = nLastBlockTime;
        nPrimaryBlockHeight = nLastBlockHeight;
    }

    // run state machine
    for (;;)
    {   
        CDelegateAgreement agree;
        {
            boost::unique_lock<boost::mutex> lock(mutex);

            while((nMakerStatus == ForkMakerStatus::MAKER_HOLD || nMakerStatus == ForkMakerStatus::MAKER_SKIP)
                && nMakerStatus != ForkMakerStatus::MAKER_EXIT)
            {
                cond.wait(lock);
            }

            if (nMakerStatus == ForkMakerStatus::MAKER_EXIT)
            {
                break;
            }
        
            if(!pWorldLine->GetBlockDelegateAgreement(hashLastBlock, agree))
            {
                nMakerStatus = ForkMakerStatus::MAKER_SKIP;
                continue;
            }

            if(agree.vBallot.empty())
            {
                nMakerStatus = ForkMakerStatus::MAKER_SKIP;
                continue;
            }

            currentAgreement = agree;
            
            nMakerStatus = ForkMakerStatus::MAKER_RUN;
        }

        CBlock block;
        try
        {
            ForkMakerStatus nNextStatus = ForkMakerStatus::MAKER_HOLD;
            
            if (!agree.IsProofOfWork())
            {
                ProcessDelegatedProofOfStake(block,agree,nLastBlockHeight - 1); 
            }
            
            {
                boost::unique_lock<boost::mutex> lock(mutex);
                if (nMakerStatus == ForkMakerStatus::MAKER_RUN)
                {
                    nMakerStatus = nNextStatus;
                }
            }
        }
        catch (const std::exception& e)
        {
            WalleveError("Fork Block maker error: %s\n", e.what());
            break;
        }
    } // end for loop

    WalleveLog(" Fork Block maker exited\n");
}
    
void CForkBlockMaker::ExtendedMakerThreadFunc()
{
    uint256 hashPrimaryBlock = uint64(0);
    int64 nPrimaryBlockTime  = 0;
    int32 nPrimaryBlockHeight  = 0;

    {
        boost::unique_lock<boost::mutex> lock(mutex);
        hashPrimaryBlock = hashLastBlock;
    }

    WalleveLog("Extened fork block maker started, initial primary block hash = %s\n",hashPrimaryBlock.GetHex().c_str());

    for (;;)
    {
        CDelegateAgreement agree; 
        {
            boost::unique_lock<boost::mutex> lock(mutex);
            
            while((nMakerStatus == ForkMakerStatus::MAKER_HOLD || nMakerStatus == ForkMakerStatus::MAKER_SKIP) 
                && nMakerStatus != ForkMakerStatus::MAKER_EXIT)
            {
                cond.wait(lock);
            }

            if (nMakerStatus == ForkMakerStatus::MAKER_EXIT)
            {
                break;
            }

            if (currentAgreement.IsProofOfWork()
                || currentAgreement.nAgreement != nLastAgreement 
                || currentAgreement.nWeight != nLastWeight) 
            {
                hashPrimaryBlock = hashLastBlock;
                nMakerStatus = ForkMakerStatus::MAKER_SKIP;
                continue;
            }

            agree = currentAgreement;
            hashPrimaryBlock = hashLastBlock;
            nPrimaryBlockTime = nLastBlockTime;
            nPrimaryBlockHeight = nLastBlockHeight;
        }

        try
        {
            ProcessExtended(agree,hashPrimaryBlock,nPrimaryBlockTime,nPrimaryBlockHeight);
        }
        catch (const std::exception& e)
        {
            WalleveError("Extended fork block maker error: %s\n", e.what());
            break;
        }
    } // end for loop

    WalleveLog("Extended fork block maker exited \n");
}
