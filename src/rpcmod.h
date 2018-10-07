// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_RPCMOD_H
#define  MULTIVERSE_RPCMOD_H

#include <boost/function.hpp>

#include "mvbase.h"
#include "walleve/walleve.h"
#include "json/json_spirit.h"
#include "rpcjson.h"
#include "rpc/rpc.h"

namespace multiverse
{

class CRPCMod : public walleve::IIOModule, virtual public walleve::CWalleveHttpEventListener
{
public:
    typedef CRPCResultPtr (CRPCMod::*RPCFunc)(CRPCParamPtr param);
    CRPCMod();
    ~CRPCMod();
    bool HandleEvent(walleve::CWalleveEventHttpReq& eventHttpReq);
    bool HandleEvent(walleve::CWalleveEventHttpBroken& eventHttpBroken);
protected:
    bool WalleveHandleInitialize();
    void WalleveHandleDeinitialize();
    const CMvNetworkConfig* WalleveConfig()
    {
        return dynamic_cast<const CMvNetworkConfig*>(walleve::IWalleveBase::WalleveConfig());
    }

    void JsonReply(uint64 nNonce,const std::string& result);

    int GetInt(const rpc::CRPCInt64& i,int valDefault)
    {
        return i.IsValid() ? int(i) : valDefault;
    }
    uint GetUint(const rpc::CRPCUint64& i,uint valDefault)
    {
        return i.IsValid() ? uint64(i) : valDefault;
    }
    const uint256 GetForkHash(const rpc::CRPCString& hex)
    {
        uint256 fork;
        if (hex.IsValid())
        {
            fork.SetHex(hex);
        }
        if (fork == 0) fork = pCoreProtocol->GetGenesisBlockHash();
        return fork;
    }
    bool CheckWalletError(MvErr err);
    multiverse::crypto::CPubKey GetPubKey(const std::string& addr);
    CTemplatePtr MakeTemplate(const rpc::CTemplateRequest& obj);
    void ListDestination(std::vector<CDestination>& vDestination);
private:
    /* System */
    CRPCResultPtr RPCHelp(CRPCParamPtr param);
    CRPCResultPtr RPCStop(CRPCParamPtr param);
    /* Network */
    CRPCResultPtr RPCGetPeerCount(CRPCParamPtr param);
    CRPCResultPtr RPCListPeer(CRPCParamPtr param);
    CRPCResultPtr RPCAddNode(CRPCParamPtr param);
    CRPCResultPtr RPCRemoveNode(CRPCParamPtr param);
    /* Worldline & TxPool */
    CRPCResultPtr RPCGetForkCount(CRPCParamPtr param);
    CRPCResultPtr RPCListFork(CRPCParamPtr param);
    CRPCResultPtr RPCGetForkGenealogy(CRPCParamPtr param);
    CRPCResultPtr RPCGetBlockLocation(CRPCParamPtr param);
    CRPCResultPtr RPCGetBlockCount(CRPCParamPtr param);
    CRPCResultPtr RPCGetBlockHash(CRPCParamPtr param);
    CRPCResultPtr RPCGetBlock(CRPCParamPtr param);
    CRPCResultPtr RPCGetTxPool(CRPCParamPtr param);
    CRPCResultPtr RPCRemovePendingTx(CRPCParamPtr param);
    CRPCResultPtr RPCGetTransaction(CRPCParamPtr param);
    CRPCResultPtr RPCSendTransaction(CRPCParamPtr param);
    /* Wallet */
    CRPCResultPtr RPCListKey(CRPCParamPtr param);
    CRPCResultPtr RPCGetNewKey(CRPCParamPtr param);
    CRPCResultPtr RPCEncryptKey(CRPCParamPtr param);
    CRPCResultPtr RPCLockKey(CRPCParamPtr param);
    CRPCResultPtr RPCUnlockKey(CRPCParamPtr param);
    CRPCResultPtr RPCImportPrivKey(CRPCParamPtr param);
    CRPCResultPtr RPCImportKey(CRPCParamPtr param);
    CRPCResultPtr RPCExportKey(CRPCParamPtr param);
    CRPCResultPtr RPCAddNewTemplate(CRPCParamPtr param);
    CRPCResultPtr RPCImportTemplate(CRPCParamPtr param);
    CRPCResultPtr RPCExportTemplate(CRPCParamPtr param);
    CRPCResultPtr RPCValidateAddress(CRPCParamPtr param);
    CRPCResultPtr RPCResyncWallet(CRPCParamPtr param);
    CRPCResultPtr RPCGetBalance(CRPCParamPtr param);
    CRPCResultPtr RPCListTransaction(CRPCParamPtr param);
    CRPCResultPtr RPCSendFrom(CRPCParamPtr param);
    CRPCResultPtr RPCCreateTransaction(CRPCParamPtr param);
    CRPCResultPtr RPCSignTransaction(CRPCParamPtr param);
    CRPCResultPtr RPCSignMessage(CRPCParamPtr param);
    CRPCResultPtr RPCListAddress(CRPCParamPtr param);
    CRPCResultPtr RPCExportWallet(CRPCParamPtr param);
    CRPCResultPtr RPCImportWallet(CRPCParamPtr param);
    CRPCResultPtr RPCMakeOrigin(CRPCParamPtr param);
    /* Util */
    CRPCResultPtr RPCVerifyMessage(CRPCParamPtr param);
    CRPCResultPtr RPCMakeKeyPair(CRPCParamPtr param);
    CRPCResultPtr RPCGetPubKeyAddress(CRPCParamPtr param);
    CRPCResultPtr RPCGetTemplateAddress(CRPCParamPtr param);
    CRPCResultPtr RPCMakeTemplate(CRPCParamPtr param);
    CRPCResultPtr RPCDecodeTransaction(CRPCParamPtr param);
    /* Mint */
    CRPCResultPtr RPCGetWork(CRPCParamPtr param);
    CRPCResultPtr RPCSubmitWork(CRPCParamPtr param);
protected:
    walleve::IIOProc *pHttpServer;
    ICoreProtocol *pCoreProtocol;
    IService *pService;
private:
    std::map<std::string,RPCFunc> mapRPCFunc;
};

} // namespace multiverse

#endif //MULTIVERSE_RPCMOD_H

