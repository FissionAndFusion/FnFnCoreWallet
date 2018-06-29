// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_TAMPLATE_H
#define  MULTIVERSE_TAMPLATE_H

#include "uint256.h"
#include "key.h"
#include "destination.h"
#include <vector>
#include <map>
#include <set>
#include <boost/shared_ptr.hpp>

enum TemplateType
{
    TEMPLATE_PUBKEY    = 0,
    TEMPLATE_WEIGHTED  = 1,
    TEMPLATE_MULTISIG  = 2,
    TEMPLATE_FORK      = 3,
    TEMPLATE_MINT      = 4, 
    TEMPLATE_DELEGATE  = 5, 
    TEMPLATE_MAX, 
};

class CTemplateGeneric;
typedef boost::shared_ptr<CTemplateGeneric> CTemplatePtr;

class CTemplateGeneric
{
public:
    CTemplateGeneric(uint16 nTemplateTypeIn) : nTemplateType(nTemplateTypeIn),nTemplateId(0) {}
    CTemplateGeneric(uint16 nTemplateTypeIn,const std::vector<unsigned char>& vchDataIn)
    : nTemplateType(nTemplateTypeIn)
    {
    }
    bool IsTxSpendable() const { return (nTemplateType == TEMPLATE_WEIGHTED || nTemplateType == TEMPLATE_MULTISIG); }
    uint16 GetTemplateType() const { return nTemplateType; }
    CTemplateId GetTemplateId() const { return nTemplateId; }
    void GetTemplateData(std::vector<unsigned char>& vchDataRet) const { vchDataRet = vchData; }
    void Export(std::vector<unsigned char>& vchTemplateRet) const
    {
        vchTemplateRet.clear();
        vchTemplateRet.reserve(2 + vchData.size());
        vchTemplateRet.push_back((unsigned char)(nTemplateType&0xff));
        vchTemplateRet.push_back((unsigned char)(nTemplateType>>8));
        vchTemplateRet.insert(vchTemplateRet.end(),vchData.begin(),vchData.end());
    }
    bool IsNull() const { return (nTemplateId == 0); }
    void SetNull() { nTemplateId = 0; }
    bool VerifyBlockSignature(const uint256& hash,const std::vector<unsigned char>& vchSig)
    {
        if (!VerifyTemplateData(vchSig))
        {
            return false;
        }
        std::vector<unsigned char> vchRemain(vchSig.begin() + vchData.size(),vchSig.end());
        return VerifySignature(hash,vchRemain,true);
    }
    bool VerifyTxSignature(const uint256& hash,const std::vector<unsigned char>& vchSig)
    {
        if (!VerifyTemplateData(vchSig))
        {
            return false;
        }
        std::vector<unsigned char> vchRemain(vchSig.begin() + vchData.size(),vchSig.end());
        return VerifySignature(hash,vchRemain,false);
    }
    virtual bool BuildBlockSignature(const uint256& hash,const std::vector<unsigned char>& vchMintSig,std::vector<unsigned char>& vchSig) { return false; }

    static CTemplatePtr CreateTemplatePtr(uint16 nTemplateTypeIn,const std::vector<unsigned char>& vchDataIn);
    static CTemplatePtr CreateTemplatePtr(const std::vector<unsigned char>& vchTemplateIn);
    static CTemplatePtr GetTemplatePtr(const CTemplateId& tid,const std::vector<unsigned char>& vchSig);
    static bool IsTxSpendableType(const CDestination& dest);
    static const std::string GetTypeName(uint16 nTemplateTypeIn);
protected:
    virtual bool SetTemplateData(const std::vector<unsigned char>& vchDataIn) { return false; }
    virtual void BuildTemplateData() {};
    void BuildTemplate()
    {
        BuildTemplateData();
        nTemplateId = CTemplateId(nTemplateType,multiverse::crypto::CryptoHash(&vchData[0],vchData.size()));
    }
    bool VerifyTemplateData(const std::vector<unsigned char>& vchSig) const 
    {
        return (vchSig.size() > vchData.size() && std::memcmp(&vchData[0],&vchSig[0],vchData.size()) == 0); 
    }
    virtual bool VerifySignature(const uint256& hash,const std::vector<unsigned char>& vchSig,bool fBlock) { return false; }
public:
    uint16  nTemplateType;
    CTemplateId nTemplateId;
    std::vector<unsigned char> vchData; 
};

class CTemplateWeighted : public CTemplateGeneric
{
public:
    CTemplateWeighted(uint16 nTemplateTypeIn,int nRequiredIn) : CTemplateGeneric(nTemplateTypeIn),nRequired(nRequiredIn) {} 
    CTemplateWeighted(const std::vector<std::pair<multiverse::crypto::CPubKey,unsigned char> >& vPubKeyIn,int nRequiredIn);
    CTemplateWeighted(const std::vector<unsigned char>& vchDataIn) : CTemplateGeneric(TEMPLATE_WEIGHTED,vchDataIn)
    {
        if (SetTemplateData(vchDataIn))
        {
            BuildTemplate();
        }
    }
    CTemplateWeighted(uint16 nTemplateTypeIn,const std::vector<unsigned char>& vchDataIn) : CTemplateGeneric(nTemplateTypeIn,vchDataIn) {}
    virtual bool BuildTxSignature(const uint256& hash,const std::vector<unsigned char>& vchKeySig,
                                                      const std::vector<unsigned char>& vchPreSig,
                                                      std::vector<unsigned char>& vchSig,bool& fCompleted);
protected:
    bool SetTemplateData(const std::vector<unsigned char>& vchDataIn);
    void BuildTemplateData();
    bool GetSignedWeight(const uint256& hash,const std::vector<unsigned char>& vchSig,int& nWeight);
    bool VerifySignature(const uint256& hash,const std::vector<unsigned char>& vchSig,bool fBlock);
public:
    std::map<multiverse::crypto::CPubKey,unsigned char> mapPubKeyWeight;
    int nRequired;
};

class CTemplateMultiSig : public CTemplateWeighted
{
public:
    CTemplateMultiSig(const std::vector<multiverse::crypto::CPubKey>& vPubKeyIn,int nRequiredIn);
    CTemplateMultiSig(const std::vector<unsigned char>& vchDataIn) : CTemplateWeighted(TEMPLATE_MULTISIG,vchDataIn) 
    {
        if (SetTemplateData(vchDataIn))
        {
            BuildTemplate();
        }
    }
protected:
    bool SetTemplateData(const std::vector<unsigned char>& vchDataIn);
    void BuildTemplateData();
};

class CTemplateFork : public CTemplateGeneric
{
public:
    CTemplateFork(const CDestination& destRedeemIn,const uint256& hashForkIn);
    CTemplateFork(const std::vector<unsigned char>& vchDataIn) : CTemplateGeneric(TEMPLATE_FORK,vchDataIn)
    {
        if (SetTemplateData(vchDataIn))
        {
            BuildTemplate();
        }
    }
    bool BuildTxSignature(const uint256& hash,const std::vector<unsigned char>& vchRedeemSig,std::vector<unsigned char>& vchSig);
protected:
    bool SetTemplateData(const std::vector<unsigned char>& vchDataIn);
    void BuildTemplateData();
    bool VerifySignature(const uint256& hash,const std::vector<unsigned char>& vchSig,bool fBlock);
public:
    CDestination destRedeem;
    uint256 hashFork;
};

class CTemplateMint : public CTemplateGeneric
{
public:
    CTemplateMint(const multiverse::crypto::CPubKey& keyMintIn,const CDestination& destSpendIn);
    CTemplateMint(const std::vector<unsigned char>& vchDataIn) : CTemplateGeneric(TEMPLATE_MINT,vchDataIn) 
    {
        if (SetTemplateData(vchDataIn))
        {
            BuildTemplate();
        }
    }
    bool BuildTxSignature(const uint256& hash,const std::vector<unsigned char>& vchSpendSig,std::vector<unsigned char>& vchSig);
    bool BuildBlockSignature(const uint256& hash,const std::vector<unsigned char>& vchMintSig,std::vector<unsigned char>& vchSig);
protected:
    bool SetTemplateData(const std::vector<unsigned char>& vchDataIn);
    void BuildTemplateData();
    bool VerifySignature(const uint256& hash,const std::vector<unsigned char>& vchSig,bool fBlock);
public:
    multiverse::crypto::CPubKey keyMint;
    CDestination destSpend;
};

class CTemplateDelegate : public CTemplateGeneric
{
public:
    CTemplateDelegate(const multiverse::crypto::CPubKey& keyDelegateIn,const CDestination& destOwerIn);
    CTemplateDelegate(const std::vector<unsigned char>& vchDataIn) : CTemplateGeneric(TEMPLATE_DELEGATE,vchDataIn)
    {
        if (SetTemplateData(vchDataIn))
        {
            BuildTemplate();
        }
    }
    bool BuildTxSignature(const CDestination& dest,const uint256& hash,const std::vector<unsigned char>& vchSpendSig,
                          std::vector<unsigned char>& vchSig);
    bool BuildVssSignature(const uint256& hash,const std::vector<unsigned char>& vchVssSig,std::vector<unsigned char>& vchSig);
    bool BuildBlockSignature(const uint256& hash,const std::vector<unsigned char>& vchMintSig,std::vector<unsigned char>& vchSig);
protected:
    bool SetTemplateData(const std::vector<unsigned char>& vchDataIn);
    void BuildTemplateData();
    bool VerifySignature(const uint256& hash,const std::vector<unsigned char>& vchSig,bool fBlock);
public:
    multiverse::crypto::CPubKey keyDelegate;
    CDestination destOwner;
};

#endif //MULTIVERSE_TAMPLATE_H

