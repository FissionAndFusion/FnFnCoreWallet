// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "template.h"
#include <walleve/stream/datastream.h>

using namespace std;
using namespace walleve;
using namespace multiverse::crypto;

//////////////////////////////
// CTemplateGeneric
CTemplatePtr CTemplateGeneric::CreateTemplatePtr(uint16 nTemplateTypeIn,const vector<unsigned char>& vchDataIn)
{
    CTemplatePtr ptr;
    if (nTemplateTypeIn == TEMPLATE_WEIGHTED)
    {
        ptr = CTemplatePtr(new CTemplateWeighted(vchDataIn));
    }
    else if (nTemplateTypeIn == TEMPLATE_MULTISIG)
    {
        ptr = CTemplatePtr(new CTemplateMultiSig(vchDataIn));
    }
    else if (nTemplateTypeIn == TEMPLATE_FORK)
    {
        ptr = CTemplatePtr(new CTemplateFork(vchDataIn));
    }
    else if (nTemplateTypeIn == TEMPLATE_MINT)
    {
        ptr = CTemplatePtr(new CTemplateMint(vchDataIn));
    }
    else if (nTemplateTypeIn == TEMPLATE_DELEGATE)
    {
        ptr = CTemplatePtr(new CTemplateDelegate(vchDataIn));
    }
    return ptr;
}

CTemplatePtr CTemplateGeneric::CreateTemplatePtr(const vector<unsigned char>& vchTemplateIn)
{
    if (vchTemplateIn.size() < 2)
    {
        return CTemplatePtr();
    }
    uint16 nTemplateTypeIn;
    vector<unsigned char> vchDataIn;
    nTemplateTypeIn = vchTemplateIn[1] | (((uint16)vchTemplateIn[1]) << 8);
    vchDataIn.assign(vchTemplateIn.begin() + 2,vchTemplateIn.end());
    return CreateTemplatePtr(nTemplateTypeIn,vchDataIn);
}

CTemplatePtr CTemplateGeneric::GetTemplatePtr(const CTemplateId& tid,const vector<unsigned char>& vchSig)
{
    CTemplatePtr ptr = CreateTemplatePtr(tid.GetType(),vchSig);
    if (ptr != NULL && ptr->GetTemplateId() != tid)
    {
        ptr.reset();
    }
    return ptr;
}
 
bool CTemplateGeneric::IsTxSpendableType(const CDestination& dest)
{
    CTemplateId tid;
    if (dest.GetTemplateId(tid))
    {
        uint16 nType = tid.GetType();
        return (nType == TEMPLATE_WEIGHTED || nType == TEMPLATE_MULTISIG);
    }
    return false;
}

const string CTemplateGeneric::GetTypeName(uint16 nTemplateTypeIn)
{
    const char* pszType[] = {"pubkey","weighted","multisig","fork","mint","delegate"};
    if (nTemplateTypeIn < TEMPLATE_MAX)
    {
        return string(pszType[nTemplateTypeIn]);
    }
    return string();
}

//////////////////////////////
// CTemplateWeighted

CTemplateWeighted::CTemplateWeighted(const vector<pair<CPubKey,unsigned char> >& vPubKeyIn,int nRequiredIn)
: CTemplateGeneric(TEMPLATE_WEIGHTED),nRequired(nRequiredIn)
{
    int nWeightTotal = 0;
    for (int i = 0;i < vPubKeyIn.size();i++)
    {
        nWeightTotal += (int)vPubKeyIn[i].second;
        mapPubKeyWeight.insert(vPubKeyIn[i]);
    }
    if (nRequired > 0 && nRequired < 256 && nWeightTotal >= nRequired && mapPubKeyWeight.size() == vPubKeyIn.size() && vPubKeyIn.size() <= 16)
    {
        BuildTemplate();
    }
}

bool CTemplateWeighted::BuildTxSignature(const uint256& hash,const vector<unsigned char>& vchKeySig,
                                                             const vector<unsigned char>& vchPreSig,
                                                             vector<unsigned char>& vchSig,bool& fCompleted)
{
    vector<unsigned char> vchKeys;
    if (!vchPreSig.empty())
    {
        if (!VerifyTemplateData(vchPreSig))
        {
            return false;
        }
        vchKeys.assign(vchPreSig.begin() + vchData.size(),vchPreSig.end());
    }
    vchKeys.insert(vchKeys.end(),vchKeySig.begin(),vchKeySig.end());
  
    int nWeight = 0;
    if (!GetSignedWeight(hash,vchKeys,nWeight))
    {
        return false;
    }
    fCompleted = (nWeight >= nRequired);

    vchSig = vchData;
    vchSig.insert(vchSig.end(),vchKeys.begin(),vchKeys.end());
    return true;
}

bool CTemplateWeighted::GetSignedWeight(const uint256& hash,const vector<unsigned char>& vchSig,int& nWeight)
{
    int nSigned = vchSig.size() / 64;
    if (nSigned == 0 || nSigned * 64 != vchSig.size())
    {
        return false;
    }
    nWeight = 0;
    set<CPubKey> setSigned;
    vector<unsigned char>::const_iterator it = vchSig.begin();
    for (int i = 0;i < nSigned;i++,it += 64)
    {
        vector<unsigned char> vchKeySig(it,it + 64);
        map<CPubKey,unsigned char>::iterator mi;
        for (mi = mapPubKeyWeight.begin(); mi != mapPubKeyWeight.end(); ++mi)
        {
            CPubKey pubkey = (*mi).first;
            if (!setSigned.count(pubkey))
            {
                if (pubkey.Verify(hash,vchKeySig))
                {
                    setSigned.insert(pubkey);
                    nWeight += (*mi).second;
                    break;
                }
            }
        }
        if (mi == mapPubKeyWeight.end())
        {
            return false;
        }
    }
    return true;
}

bool CTemplateWeighted::SetTemplateData(const vector<unsigned char>& vchDataIn)
{
    if (vchDataIn.size() < 2)
    {
        return false;
    }
    CWalleveIDataStream is(vchDataIn);
    unsigned char r,n;
    is >> r >> n;
    if (n > 16 || vchDataIn.size() != 2 + ((int)n) * 33)
    {
        return false;
    }
    mapPubKeyWeight.clear();
    int nWeightTotal = 0;
    for (int i = 0;i < n;i++)
    {
        CPubKey pubkey;
        unsigned char weight;
        is >> pubkey >> weight;
        nWeightTotal += weight; 
        mapPubKeyWeight.insert(make_pair(pubkey,weight));
    }
    if (mapPubKeyWeight.size() != n || nWeightTotal <= r)
    {
        return false;
    }
    nRequired = r;

    BuildTemplate();

    return true;
}

void CTemplateWeighted::BuildTemplateData()
{
    vchData.clear();
    CWalleveODataStream os(vchData);
    os << (unsigned char)nRequired << (unsigned char)mapPubKeyWeight.size();
    for (map<CPubKey,unsigned char>::iterator it = mapPubKeyWeight.begin();it != mapPubKeyWeight.end();++it)
    {
        os << (*it).first << (*it).second;
    }
}

bool CTemplateWeighted::VerifySignature(const uint256& hash,const vector<unsigned char>& vchSig,bool fBlock)
{
    if (fBlock)
    {
        return false;
    }
    int nWeight = 0;
    return (GetSignedWeight(hash,vchSig,nWeight) && nWeight > nRequired);
}

//////////////////////////////
// CTemplateMultiSig

CTemplateMultiSig::CTemplateMultiSig(const vector<CPubKey>& vPubKeyIn,int nRequiredIn)
: CTemplateWeighted(TEMPLATE_MULTISIG,nRequiredIn)
{
    for (int i = 0;i < vPubKeyIn.size();i++)
    {
        mapPubKeyWeight.insert(make_pair(vPubKeyIn[i],1));
    }
    int nWeightTotal = vPubKeyIn.size();
    if (nRequired > 0 && vPubKeyIn.size() >= nRequired && mapPubKeyWeight.size() == vPubKeyIn.size() && vPubKeyIn.size() <= 16)
    {
        BuildTemplate();
    }
}

bool CTemplateMultiSig::SetTemplateData(const vector<unsigned char>& vchDataIn)
{
    if (vchDataIn.size() < 2)
    {
        return false;
    }
    CWalleveIDataStream is(vchDataIn);
    unsigned char r,n;
    is >> r >> n;
    if (r > n || n > 16 || vchDataIn.size() < 2 + ((int)n) * 32)
    {
        return false;
    }
    mapPubKeyWeight.clear();
    for (int i = 0;i < n;i++)
    {
        CPubKey pubkey;
        is >> pubkey;
        mapPubKeyWeight.insert(make_pair(pubkey,1));
    }
    if (mapPubKeyWeight.size() != n)
    {
        return false;
    }
    nRequired = r;

    BuildTemplate();

    return true;
}

void CTemplateMultiSig::BuildTemplateData()
{
    vchData.clear();
    CWalleveODataStream os(vchData);
    os << (unsigned char)nRequired << (unsigned char)mapPubKeyWeight.size();
    for (map<CPubKey,unsigned char>::iterator it = mapPubKeyWeight.begin();it != mapPubKeyWeight.end();++it)
    {
        os << (*it).first;
    }
}

//////////////////////////////
// CTemplateFork

CTemplateFork::CTemplateFork(const CDestination& destRedeemIn,const uint256& hashForkIn)
: CTemplateGeneric(TEMPLATE_FORK),destRedeem(destRedeemIn),hashFork(hashForkIn)
{
    if (destRedeem.IsPubKey() || IsTxSpendableType(destRedeem))
    {
        BuildTemplate();
    }
}

bool CTemplateFork::BuildTxSignature(const uint256& hash,const vector<unsigned char>& vchRedeemSig,vector<unsigned char>& vchSig)
{
    if (!VerifySignature(hash,vchRedeemSig,false))
    {
        return false;
    }
    vchSig = vchData;
    vchSig.insert(vchSig.end(),vchRedeemSig.begin(),vchRedeemSig.end());
    return true;
}

bool CTemplateFork::SetTemplateData(const vector<unsigned char>& vchDataIn)
{
    if (vchDataIn.size() < (32 + 33))
    {
        return false;
    }
    CWalleveIDataStream is(vchDataIn);
    is >> destRedeem.prefix >> destRedeem.data >> hashFork;
    if (!destRedeem.IsPubKey() && !IsTxSpendableType(destRedeem))
    {
        return false;
    }
    BuildTemplate();
    return true;
}

void CTemplateFork::BuildTemplateData()
{
    vchData.clear();
    CWalleveODataStream os(vchData);
    os << destRedeem.prefix << destRedeem.data << hashFork;
}

bool CTemplateFork::VerifySignature(const uint256& hash,const vector<unsigned char>& vchSig,bool fBlock)
{
    if (fBlock)
    {
        return false;
    }
    return destRedeem.VerifySignature(hash,vchSig);    
}

//////////////////////////////
// CTemplateMint

CTemplateMint::CTemplateMint(const CPubKey& keyMintIn,const CDestination& destSpendIn)
: CTemplateGeneric(TEMPLATE_MINT),keyMint(keyMintIn),destSpend(destSpendIn)
{
    if (destSpend.IsPubKey() || IsTxSpendableType(destSpend))
    {
        BuildTemplate();
    }
}

bool CTemplateMint::BuildTxSignature(const uint256& hash,const vector<unsigned char>& vchSpendSig,vector<unsigned char>& vchSig)
{
    if (!VerifySignature(hash,vchSpendSig,false))
    {
        return false;
    }
    vchSig = vchData;
    vchSig.insert(vchSig.end(),vchSpendSig.begin(),vchSpendSig.end());
    return true;
}

bool CTemplateMint::BuildBlockSignature(const uint256& hash,const vector<unsigned char>& vchMintSig,vector<unsigned char>& vchSig)
{
    if (!VerifySignature(hash,vchMintSig,true))
    {
        return false;
    }
    vchSig = vchData;
    vchSig.insert(vchSig.end(),vchMintSig.begin(),vchMintSig.end());
    return true;
}

bool CTemplateMint::SetTemplateData(const vector<unsigned char>& vchDataIn)
{
    if (vchDataIn.size() < (32 + 33))
    {
        return false;
    }
    CWalleveIDataStream is(vchDataIn);
    is >> keyMint >> destSpend.prefix >> destSpend.data;
    if (!destSpend.IsPubKey() && !IsTxSpendableType(destSpend))
    {
        return false;
    }
    BuildTemplate();
    return true;
}

void CTemplateMint::BuildTemplateData()
{
    vchData.clear();
    CWalleveODataStream os(vchData);
    os << keyMint << destSpend.prefix << destSpend.data;
}

bool CTemplateMint::VerifySignature(const uint256& hash,const vector<unsigned char>& vchSig,bool fBlock)
{
    if (fBlock)
    {
        return keyMint.Verify(hash,vchSig);
    }
    return destSpend.VerifySignature(hash,vchSig);
}

//////////////////////////////
// CTemplateDelegate

CTemplateDelegate::CTemplateDelegate(const CPubKey& keyDelegateIn,const CDestination& destOwnerIn)
: CTemplateGeneric(TEMPLATE_DELEGATE),keyDelegate(keyDelegateIn),destOwner(destOwnerIn)
{
    if (destOwner.IsPubKey() || IsTxSpendableType(destOwner))
    {
        BuildTemplate();
    }
}

bool CTemplateDelegate::BuildTxSignature(const CDestination& dest,const uint256& hash,const vector<unsigned char>& vchSpendSig,vector<unsigned char>& vchSig)
{
    if (!dest.VerifySignature(hash,vchSpendSig))
    {
        vchSig = vchData;
        CWalleveODataStream os(vchSig);
        os << dest.prefix << dest.data;
        vchSig.insert(vchSig.end(),vchSpendSig.begin(),vchSpendSig.end());
    }
    return true;
}

bool CTemplateDelegate::BuildVssSignature(const uint256& hash,const vector<unsigned char>& vchVssSig,vector<unsigned char>& vchSig)
{
    if (!VerifySignature(hash,vchVssSig,true))
    {
        return false;
    }
    vchSig = vchData;
    vchSig.insert(vchSig.end(),vchVssSig.begin(),vchVssSig.end());
    return true;
}

bool CTemplateDelegate::BuildBlockSignature(const uint256& hash,const vector<unsigned char>& vchMintSig,vector<unsigned char>& vchSig)
{
    if (!VerifySignature(hash,vchMintSig,true))
    {
        return false;
    }
    vchSig = vchData;
    vchSig.insert(vchSig.end(),vchMintSig.begin(),vchMintSig.end());
    return true;
}

bool CTemplateDelegate::SetTemplateData(const vector<unsigned char>& vchDataIn)
{
    if (vchDataIn.size() < (32 + 33))
    {
        return false;
    }
    CWalleveIDataStream is(vchDataIn);
    is >> keyDelegate >> destOwner.prefix >> destOwner.data;
    if (!destOwner.IsPubKey() && !IsTxSpendableType(destOwner))
    {
        return false;
    }
    BuildTemplate();
    return true;
}

void CTemplateDelegate::BuildTemplateData()
{
    vchData.clear();
    CWalleveODataStream os(vchData);
    os << keyDelegate << destOwner.prefix << destOwner.data;
}

bool CTemplateDelegate::VerifySignature(const uint256& hash,const vector<unsigned char>& vchSig,bool fBlock)
{
    if (fBlock)
    {
        return keyDelegate.Verify(hash,vchSig);
    }
    if (vchSig.size() == 64)
    {
        return keyDelegate.Verify(hash,vchSig);
    }
    if (vchSig.size() < 33)
    {
        return false;
    }
    CDestination dest;
    CWalleveIDataStream is(vchSig);
    is >> dest.prefix >> dest.data;
    vector<unsigned char> vchDestSig(vchSig.begin() + 33,vchSig.end());
    return dest.VerifySignature(hash,vchDestSig);
}





