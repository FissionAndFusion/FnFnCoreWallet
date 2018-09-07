// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_RPCMOD_H
#define  MULTIVERSE_RPCMOD_H

#include "mvbase.h"
#include "walleve/walleve.h"
#include "rpcjson.h"
#include <boost/function.hpp>
namespace multiverse
{

class CRPCMod : public walleve::IIOModule, virtual public walleve::CWalleveHttpEventListener
{
public:
    typedef json_spirit::Value (CRPCMod::*RPCFunc)(const json_spirit::Array&,bool);
    CRPCMod();
    ~CRPCMod();
    bool HandleEvent(walleve::CWalleveEventHttpReq& eventHttpReq);
    bool HandleEvent(walleve::CWalleveEventHttpBroken& eventHttpBroken);
protected:
    bool WalleveHandleInitialize();
    void WalleveHandleDeinitialize();
    const CMvConfig* WalleveConfig()
    {
        return dynamic_cast<const CMvConfig *>(walleve::IWalleveBase::WalleveConfig());
    }

    void JsonReply(uint64 nNonce,const json_spirit::Value& result,
                                 const json_spirit::Value& id);
    void JsonError(uint64 nNonce,const json_spirit::Object& objError,
                                 const json_spirit::Value& id);
    int GetInt(const json_spirit::Array& params,int index,int valDefault)
    {
        return (params.size() > index ? params[index].get_int() : valDefault);
    }
    const uint256 GetForkHash(const json_spirit::Array& params,int index)
    {
        uint256 fork;
        if (params.size() > index) fork.SetHex(params[index].get_str());
        if (fork == 0) fork = pCoreProtocol->GetGenesisBlockHash();
        return fork;
    }
    bool CheckWalletError(MvErr err);
    multiverse::crypto::CPubKey GetPubKey(const json_spirit::Value& value);
    CTemplatePtr MakeTemplate(const std::string& strType,const json_spirit::Object& obj);
    void ListDestination(std::vector<CDestination>& vDestination);
private:
    /* System */
    json_spirit::Value RPCHelp(const json_spirit::Array& params,bool fHelp);
    json_spirit::Value RPCStop(const json_spirit::Array& params,bool fHelp);
    /* Network */
    json_spirit::Value RPCGetPeerCount(const json_spirit::Array& params,bool fHelp);
    json_spirit::Value RPCListPeer(const json_spirit::Array& params,bool fHelp);
    json_spirit::Value RPCAddNode(const json_spirit::Array& params,bool fHelp);
    json_spirit::Value RPCRemoveNode(const json_spirit::Array& params,bool fHelp);
    /* Worldline & TxPool */
    json_spirit::Value RPCGetForkCount(const json_spirit::Array& params,bool fHelp);
    json_spirit::Value RPCGetForkGenealogy(const json_spirit::Array& params,bool fHelp);
    json_spirit::Value RPCGetBlockLocation(const json_spirit::Array& params,bool fHelp);
    json_spirit::Value RPCGetBlockCount(const json_spirit::Array& params,bool fHelp);
    json_spirit::Value RPCGetBlockHash(const json_spirit::Array& params,bool fHelp);
    json_spirit::Value RPCGetBlock(const json_spirit::Array& params,bool fHelp);
    json_spirit::Value RPCGetTxPool(const json_spirit::Array& params,bool fHelp);
    json_spirit::Value RPCRemovePendingTx(const json_spirit::Array& params,bool fHelp);
    json_spirit::Value RPCGetTransaction(const json_spirit::Array& params,bool fHelp);
    json_spirit::Value RPCSendTransaction(const json_spirit::Array& params,bool fHelp);
    /* Wallet */
    json_spirit::Value RPCListKey(const json_spirit::Array& params,bool fHelp);
    json_spirit::Value RPCGetNewKey(const json_spirit::Array& params,bool fHelp);
    json_spirit::Value RPCEncryptKey(const json_spirit::Array& params,bool fHelp);
    json_spirit::Value RPCLockKey(const json_spirit::Array& params,bool fHelp);
    json_spirit::Value RPCUnlockKey(const json_spirit::Array& params,bool fHelp);
    json_spirit::Value RPCImportPrivKey(const json_spirit::Array& params,bool fHelp);
    json_spirit::Value RPCImportKey(const json_spirit::Array& params,bool fHelp);
    json_spirit::Value RPCExportKey(const json_spirit::Array& params,bool fHelp);
    json_spirit::Value RPCAddNewTemplate(const json_spirit::Array& params,bool fHelp);
    json_spirit::Value RPCImportTemplate(const json_spirit::Array& params,bool fHelp);
    json_spirit::Value RPCExportTemplate(const json_spirit::Array& params,bool fHelp);
    json_spirit::Value RPCValidateAddress(const json_spirit::Array& params,bool fHelp);
    json_spirit::Value RPCResyncWallet(const json_spirit::Array& params,bool fHelp);
    json_spirit::Value RPCGetBalance(const json_spirit::Array& params,bool fHelp);
    json_spirit::Value RPCListTransaction(const json_spirit::Array& params,bool fHelp);
    json_spirit::Value RPCSendFrom(const json_spirit::Array& params,bool fHelp);
    json_spirit::Value RPCCreateTransaction(const json_spirit::Array& params,bool fHelp);
    json_spirit::Value RPCSignTransaction(const json_spirit::Array& params,bool fHelp);
    json_spirit::Value RPCSignMessage(const json_spirit::Array& params,bool fHelp);
    /* Util */
    json_spirit::Value RPCVerifyMessage(const json_spirit::Array& params,bool fHelp);
    json_spirit::Value RPCMakeKeyPair(const json_spirit::Array& params,bool fHelp);
    json_spirit::Value RPCGetPubKeyAddress(const json_spirit::Array& params,bool fHelp);
    json_spirit::Value RPCGetTemplateAddress(const json_spirit::Array& params,bool fHelp);
    json_spirit::Value RPCMakeTemplate(const json_spirit::Array& params,bool fHelp);
    json_spirit::Value RPCDecodeTransaction(const json_spirit::Array& params,bool fHelp);
    /* Mint */
    json_spirit::Value RPCGetWork(const json_spirit::Array& params,bool fHelp);
    json_spirit::Value RPCSubmitWork(const json_spirit::Array& params,bool fHelp);
protected:
    walleve::IIOProc *pHttpServer;
    ICoreProtocol *pCoreProtocol;
    IService *pService;
private:
    std::map<std::string,RPCFunc> mapRPCFunc;
};

} // namespace multiverse

#endif //MULTIVERSE_RPCMOD_H

