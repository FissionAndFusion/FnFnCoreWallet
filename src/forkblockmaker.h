// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_FORKBLOCKMAKER_H
#define  MULTIVERSE_FORKBLOCKMAKER_H

#include "mvbase.h"
#include "event.h"
#include "key.h"

namespace multiverse
{

class CForkBlockMakerHashAlgo
{
public: 
    CForkBlockMakerHashAlgo(const std::string& strAlgoIn,int64 nHashRateIn) : strAlgo(strAlgoIn),nHashRate(nHashRateIn) {}
    virtual ~CForkBlockMakerHashAlgo(){}
    const std::string strAlgo;
    int64 nHashRate;
public:
    virtual uint256 Hash(const std::vector<unsigned char>& vchData) = 0;
};

class CForkBlockMakerProfile
{
public:
    CForkBlockMakerProfile() {}
    CForkBlockMakerProfile(int nAlgoIn,const CDestination& dest,const uint256& nPrivKey) 
    : nAlgo(nAlgoIn),destMint(dest) 
    {
        keyMint.SetSecret(crypto::CCryptoKeyData(nPrivKey.begin(),nPrivKey.end()));
        BuildTemplate();
    }
    
    bool IsValid() const { return (templMint != NULL); }
    bool BuildTemplate();
    std::size_t GetSignatureSize() const
    {
        std::size_t size = templMint->GetTemplateData().size() + 64;
        walleve::CVarInt var(size);
        return (size + walleve::GetSerializeSize(var));
    }
    const CDestination GetDestination() const { return (CDestination(templMint->GetTemplateId())); }
public:
    int nAlgo;
    CDestination destMint;
    crypto::CKey keyMint;
    CTemplateMintPtr templMint;
};

// BlockMaker for forknode of supernode
class CForkBlockMaker : public IBlockMaker, virtual public CMvBlockMakerEventListener
{
public:
    CForkBlockMaker();
    ~CForkBlockMaker();
    bool HandleEvent(CMvEventBlockMakerUpdate& eventUpdate) override;
protected:
    bool WalleveHandleInitialize() override;
    void WalleveHandleDeinitialize() override;
    bool WalleveHandleInvoke() override;
    void WalleveHandleHalt() override;
    bool Interrupted() { return (nMakerStatus != ForkMakerStatus::MAKER_RUN); }
    bool Wait(long nSeconds);
    bool Wait(long nSeconds,const uint256& hashPrimaryBlock);
    void ArrangeBlockTx(CBlock& block,const uint256& hashFork,const CForkBlockMakerProfile& profile);
    bool SignBlock(CBlock& block,const CForkBlockMakerProfile& profile);
    bool DispatchBlock(const CBlock& block);
    void ProcessDelegatedProofOfStake(CBlock& block,const CDelegateAgreement& agreement,const int32 nPrevHeight);
    void ProcessExtended(const CDelegateAgreement& agreement,const uint256& hashPrimaryBlock,
                                                               int64 nPrimaryBlockTime,const int32 nPrimaryBlockHeight);
    bool CreateDelegatedBlock(CBlock& block,const uint256& hashFork,const CForkBlockMakerProfile& profile,std::size_t nWeight);
    void CreatePiggyback(const CForkBlockMakerProfile& profile,const CDelegateAgreement& agreement,const CBlock& refblock,const int32 nPrevHeight); 
    void CreateExtended(const CForkBlockMakerProfile& profile,const CDelegateAgreement& agreement,
                        const uint256& hashRefBlock,const std::set<uint256>& setFork,const int32 nPrimaryBlockHeight,int64 nTime); 
    bool GetAvailableDelegatedProfile(const std::vector<CDestination>& vBallot,std::vector<CForkBlockMakerProfile*>& vProfile);
    bool GetAvailableExtendedFork(std::set<uint256>& setFork);
private:
    enum class ForkMakerStatus : int {MAKER_RUN=0,MAKER_RESET=1,MAKER_EXIT=2,MAKER_HOLD=3,MAKER_SKIP=4};
    enum class ForkExtendMakerStatus : int {MAKER_RUN=0,MAKER_RESET=1,MAKER_EXIT=2,MAKER_HOLD=3,MAKER_SKIP=4};
    void BlockMakerThreadFunc();
    void ExtendedMakerThreadFunc();

protected:
    mutable boost::shared_mutex rwAccess;
    walleve::CWalleveThread thrMaker;
    walleve::CWalleveThread thrExtendedMaker;
    boost::mutex mutex;
    boost::condition_variable cond;
    boost::condition_variable condExtend;
    ForkMakerStatus nMakerStatus;
    ForkExtendMakerStatus nExtendMakerStatus;
    uint256 hashLastBlock;
    int64 nLastBlockTime;
    int32 nLastBlockHeight;
    uint256 nLastAgreement;
    std::size_t nLastWeight;
    CDelegateAgreement currentAgreement;
    std::map<int,CForkBlockMakerHashAlgo*> mapHashAlgo;
    std::map<CDestination,CForkBlockMakerProfile> mapDelegatedProfile;
    ICoreProtocol* pCoreProtocol;
    IWorldLine* pWorldLine;
    IForkManager* pForkManager;
    ITxPool* pTxPool;
    IDispatcher* pDispatcher;
    IConsensus* pConsensus;
};

}  // namespace multiverse

#endif // MULTIVERSE_FORKBLOCKMAKER_H
