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

}
    
bool CForkBlockMaker::HandleEvent(CMvEventBlockMakerUpdate& eventUpdate)
{
    return false;
}

bool CForkBlockMaker::WalleveHandleInitialize()
{
    return false;
}
    
void CForkBlockMaker::WalleveHandleDeinitialize()
{

}
    
bool CForkBlockMaker::WalleveHandleInvoke()
{

}
    
void CForkBlockMaker::WalleveHandleHalt()
{

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

}
    
void CForkBlockMaker::ArrangeBlockTx(CBlock& block,const uint256& hashFork,const CForkBlockMakerProfile& profile)
{

}
    
bool CForkBlockMaker::SignBlock(CBlock& block,const CForkBlockMakerProfile& profile)
{
    return false;
}
    
bool CForkBlockMaker::DispatchBlock(CBlock& block)
{
    return false;
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
