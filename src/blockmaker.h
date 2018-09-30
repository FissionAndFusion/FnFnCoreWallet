// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_BLOCKMAKER_H
#define  MULTIVERSE_BLOCKMAKER_H

#include "mvbase.h"
#include "event.h"
#include "key.h"
namespace multiverse
{

class CBlockMakerHashAlgo
{
public: 
    CBlockMakerHashAlgo(const std::string& strAlgoIn,int64 nHashRateIn) : strAlgo(strAlgoIn),nHashRate(nHashRateIn) {}
    virtual ~CBlockMakerHashAlgo(){}
    const std::string strAlgo;
    int64 nHashRate;
public:
    virtual uint256 Hash(const std::vector<unsigned char>& vchData) = 0;
};

class CBlockMakerProfile
{
public:
    CBlockMakerProfile() {}
    CBlockMakerProfile(int nAlgoIn,const CDestination& dest,const uint256& nPrivKey) 
    : nAlgo(nAlgoIn),destMint(dest) 
    {
        keyMint.SetSecret(crypto::CCryptoKeyData(nPrivKey.begin(),nPrivKey.end()));
        BuildTemplate();
    }
    
    bool IsValid() const { return (templMint != NULL); }
    bool BuildTemplate();
    std::size_t GetSignatureSize()
    {
        std::size_t size = templMint->GetTemplateDataSize() + 64;
        walleve::CVarInt var(size);
        return (size + walleve::GetSerializeSize(var));
    }
public:
    int nAlgo;
    CDestination destMint;
    crypto::CKey keyMint;
    CTemplatePtr templMint;
};

class CBlockMaker : public IBlockMaker, virtual public CMvBlockMakerEventListener
{
public:
    CBlockMaker();
    ~CBlockMaker();
    bool HandleEvent(CMvEventBlockMakerUpdate& eventUpdate);
protected:
    bool WalleveHandleInitialize();
    void WalleveHandleDeinitialize();
    bool WalleveHandleInvoke();
    void WalleveHandleHalt();
    bool Interrupted() { return (nMakerStatus != MAKER_RUN); }
    bool Wait(long nSeconds);
    bool CreateNewBlock(CBlock& block,const uint256& hashPrev,int64 nPrevTime,int nPrevHeight,const CBlockMakerAgreement& agreement);
    bool DispatchNewBlock(CBlock& block);
    bool SignBlock(CBlock& block,CBlockMakerProfile& profile);
    bool CreateDelegatedProofOfStake(CBlock& block,std::size_t nWeight,const CDestination& dest);
    bool CreateProofOfWork(CBlock& block,int nAlgo,const CDestination& dest);
    void CreatePiggyback(const CBlockMakerAgreement& agreement,const CBlock& refblock,int nPrevHeight); 
    bool WaitAgreement(CBlockMakerAgreement& agree,int64 nTimeAgree,int nHeight);
private:
    enum {MAKER_RUN=0,MAKER_RESET=1,MAKER_EXIT=2,MAKER_HOLD=3};
    void BlockMakerThreadFunc();
protected:
    mutable boost::shared_mutex rwAccess;
    walleve::CWalleveThread thrMaker;
    boost::mutex mutex;
    boost::condition_variable cond;
    int nMakerStatus;
    uint256 hashLastBlock;
    int64 nLastBlockTime;
    int nLastBlockHeight;
    std::map<int,CBlockMakerHashAlgo*> mapHashAlgo;
    std::map<int,CBlockMakerProfile> mapProfile;
    ICoreProtocol* pCoreProtocol;
    IWorldLine* pWorldLine;
    ITxPool* pTxPool;
    IDispatcher* pDispatcher;
    IConsensus* pConsensus;
};

} // namespace multiverse

#endif //MULTIVERSE_BLOCKMAKER_H

