// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "delegate.h"

#include "walleve/util.h"

#include "rpc/auto_protocol.h"
#include "template.h"
#include "transaction.h"

using namespace std;
using namespace walleve;
using namespace multiverse::crypto;

//////////////////////////////
// CTemplateDelegate

CTemplateDelegate::CTemplateDelegate(const multiverse::crypto::CPubKey& keyDelegateIn, const CDestination& destOwnerIn)
  : CTemplate(TEMPLATE_DELEGATE), keyDelegate(keyDelegateIn), destOwner(destOwnerIn)
{
}

CTemplateDelegate* CTemplateDelegate::clone() const
{
    return new CTemplateDelegate(*this);
}

bool CTemplateDelegate::GetSignDestination(const CTransaction& tx, const std::vector<uint8>& vchSig,
                                                std::set<CDestination>& setSubDest, std::vector<uint8>& vchSubSig) const
{
    return false;

    if (!CTemplate::GetSignDestination(tx, vchSig, setSubDest, vchSubSig))
    {
        return false;
    }

    setSubDest.clear();
    if (tx.sendTo.GetTemplateId() == nId)
    {
        setSubDest.insert(CDestination(keyDelegate));
    }
    else
    {
        setSubDest.insert(tx.sendTo);
    }
    
    return true;
}

void CTemplateDelegate::GetTemplateData(rpc::CTemplateResponse& obj, CDestination&& destInstance) const
{
    obj.delegate.strDelegate = destInstance.SetPubKey(keyDelegate).ToString();
    obj.delegate.strOwner = (destInstance = destOwner).ToString();
}

bool CTemplateDelegate::BuildVssSignature(const uint256& hash, const vector<uint8>& vchVssSig, vector<uint8>& vchSig)
{
    if (!keyDelegate.Verify(hash, vchVssSig))
    {
        return false;
    }

    std::vector<uint8> vchTemp;
    walleve::CWalleveODataStream os(vchTemp, CDestination::DESTINATION_SIZE + vchData.size() + vchVssSig.size());
    os << CDestination(nId);
    vchTemp.insert(vchTemp.end(), vchData.begin(), vchData.end());
    vchTemp.insert(vchTemp.end(), vchVssSig.begin(), vchVssSig.end());
    vchSig = move(vchTemp);
    return true;
}

bool CTemplateDelegate::ValidateParam() const
{
    if (!keyDelegate)
    {
        return false;
    }

    if (!IsTxSpendable(destOwner))
    {
        return false;
    }

    return true;
}

bool CTemplateDelegate::SetTemplateData(const std::vector<uint8>& vchDataIn)
{
    CWalleveIDataStream is(vchDataIn);
    try
    {
        is >> keyDelegate >> destOwner;
    }
    catch (const std::exception& e)
    {
        StdError(__PRETTY_FUNCTION__, e.what());
        return false;
    }

    return true;
}

bool CTemplateDelegate::SetTemplateData(const rpc::CTemplateRequest& obj, CDestination&& destInstance)
{
    if (obj.strType != GetTypeName(TEMPLATE_DELEGATE))
    {
        return false;
    }

    const string& strDelegate = obj.delegate.strDelegate;
    if (!destInstance.SetPubKey(obj.delegate.strDelegate))
    {
        return false;
    }
    keyDelegate = destInstance.GetPubKey();

    if (!destInstance.ParseString(obj.delegate.strOwner))
    {
        return false;
    }
    destOwner = destInstance;

    return true;
}

void CTemplateDelegate::BuildTemplateData()
{
    vchData.clear();
    CWalleveODataStream os(vchData);
    os << keyDelegate << destOwner;
}

bool CTemplateDelegate::VerifyTxSignature(const uint256& hash, const uint256& hashAnchor, const CDestination& destTo,
                                          const vector<uint8>& vchSig, bool& fCompleted) const
{
    if (destTo.GetTemplateId() == nId)
    {
        return CDestination(keyDelegate).VerifyTxSignature(hash, hashAnchor, destTo, vchSig, fCompleted);
    }
    else
    {
        return destTo.VerifyTxSignature(hash, hashAnchor, destTo, vchSig, fCompleted);
    }
}

bool CTemplateDelegate::VerifyBlockSignature(const uint256& hash, const vector<uint8>& vchSig) const
{
    return keyDelegate.Verify(hash, vchSig);
}