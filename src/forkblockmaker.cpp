// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "forkblockmaker.h"
#include "address.h"
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
// CForkBlockMaker 

CForkBlockMaker::CForkBlockMaker()
: thrMaker("blockmaker",boost::bind(&CForkBlockMaker::BlockMakerThreadFunc,this)), 
  thrExtendedMaker("extendedmaker",boost::bind(&CForkBlockMaker::ExtendedMakerThreadFunc,this)), 
  nMakerStatus(ForkMakerStatus::MAKER_HOLD),hashLastBlock(uint64(0)),nLastBlockTime(0),
  nLastBlockHeight(uint64(0)),nLastAgreement(uint64(0)),nLastWeight(0)
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
    WalleveThreadExit(thrMaker);
    WalleveThreadExit(thrExtendedMaker);
    IBlockMaker::WalleveHandleHalt();
}
    
bool CForkBlockMaker::Wait(long nSeconds)
{
    return false;
}
    
bool CForkBlockMaker::Wait(long nSeconds,const uint256& hashPrimaryBlock)
{
    return false;
}
    
void CForkBlockMaker::PrepareBlock(CBlock& block,const uint256& hashPrev,int64 nPrevTime,int nPrevHeight,const CBlockMakerAgreement& agreement)
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
        // TODO
        // pConsensus->GetProof(nPrevHeight + 1,block.vchProof);
    }
}
    
void CForkBlockMaker::ArrangeBlockTx(CBlock& block,const uint256& hashFork,const CForkBlockMakerProfile& profile)
{
    size_t nMaxTxSize = MAX_BLOCK_SIZE - GetSerializeSize(block) - profile.GetSignatureSize();
    int64 nTotalTxFee = 0;
    pTxPool->ArrangeBlockTx(hashFork,nMaxTxSize,block.vtx,nTotalTxFee); 
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
    
bool CForkBlockMaker::DispatchBlock(CBlock& block)
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
    
bool CForkBlockMaker::CreateProofOfWorkBlock(CBlock& block)
{
    return false;
}
    
void CForkBlockMaker::ProcessDelegatedProofOfStake(CBlock& block,const CBlockMakerAgreement& agreement,int nPrevHeight)
{

}
    
void CForkBlockMaker::ProcessExtended(const CBlockMakerAgreement& agreement,const uint256& hashPrimaryBlock,
                                                               int64 nPrimaryBlockTime,int nPrimaryBlockHeight)
{

}
    
bool CForkBlockMaker::CreateDelegatedBlock(CBlock& block,const uint256& hashFork,const CForkBlockMakerProfile& profile,std::size_t nWeight)
{
    return false;
}
    
bool CForkBlockMaker::CreateProofOfWork(CBlock& block,CForkBlockMakerHashAlgo* pHashAlgo)
{
    return false;
}
    
void CForkBlockMaker::CreatePiggyback(const CForkBlockMakerProfile& profile,const CBlockMakerAgreement& agreement,const CBlock& refblock,int nPrevHeight)
{

}
    
void CForkBlockMaker::CreateExtended(const CForkBlockMakerProfile& profile,const CBlockMakerAgreement& agreement,const std::set<uint256>& setFork,int nPrimaryBlockHeight,int64 nTime)
{

}
    
bool CForkBlockMaker::GetAvailiableDelegatedProfile(const std::vector<CDestination>& vBallot,std::vector<CForkBlockMakerProfile*>& vProfile)
{
    return false;
}
    
bool CForkBlockMaker::GetAvailiableExtendedFork(std::set<uint256>& setFork)
{
    return false;
}

void CForkBlockMaker::BlockMakerThreadFunc()
{

}
    
void CForkBlockMaker::ExtendedMakerThreadFunc()
{

}
