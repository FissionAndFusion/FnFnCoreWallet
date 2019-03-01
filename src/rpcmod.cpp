// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "rpcmod.h"

#include <regex>

#include <boost/assign/list_of.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/regex.hpp>

#include "address.h"
#include "template.h"
#include "version.h"
#include "rpc/auto_rpc.h"

using namespace std;
using namespace multiverse;
using namespace walleve;
using namespace json_spirit;
using namespace multiverse::rpc;
namespace fs = boost::filesystem;

///////////////////////////////
// CRPCMod

CRPCMod::CRPCMod()
: IIOModule("rpcmod")
{
    pHttpServer = NULL;
    pCoreProtocol = NULL;
    pService = NULL;
    
    std::map<std::string,RPCFunc>  temp_map = boost::assign::map_list_of
                /* System */
                ("help",                  &CRPCMod::RPCHelp)
                ("stop",                  &CRPCMod::RPCStop)
                ("version",               &CRPCMod::RPCVersion)
                /* Network */
                ("getpeercount",          &CRPCMod::RPCGetPeerCount)
                ("listpeer",              &CRPCMod::RPCListPeer)
                ("addnode",               &CRPCMod::RPCAddNode)
                ("removenode",            &CRPCMod::RPCRemoveNode)
                /* Worldline & TxPool */
                ("getforkcount",          &CRPCMod::RPCGetForkCount)
                ("listfork",              &CRPCMod::RPCListFork)
                ("getgenealogy",          &CRPCMod::RPCGetForkGenealogy)
                ("getblocklocation",      &CRPCMod::RPCGetBlockLocation)
                ("getblockcount",         &CRPCMod::RPCGetBlockCount)
                ("getblockhash",          &CRPCMod::RPCGetBlockHash)
                ("getblock",              &CRPCMod::RPCGetBlock)
                ("gettxpool",             &CRPCMod::RPCGetTxPool)
                // ("removependingtx",       &CRPCMod::RPCRemovePendingTx)
                ("gettransaction",        &CRPCMod::RPCGetTransaction)
                ("sendtransaction",       &CRPCMod::RPCSendTransaction)
                ("getforkheight",         &CRPCMod::RPCGetForkHeight)
                /* Wallet */
                ("listkey",               &CRPCMod::RPCListKey)
                ("getnewkey",             &CRPCMod::RPCGetNewKey)
                ("encryptkey",            &CRPCMod::RPCEncryptKey)
                ("lockkey",               &CRPCMod::RPCLockKey)
                ("unlockkey",             &CRPCMod::RPCUnlockKey)
                ("importprivkey",         &CRPCMod::RPCImportPrivKey)
                ("importkey",             &CRPCMod::RPCImportKey)
                ("exportkey",             &CRPCMod::RPCExportKey)
                ("addnewtemplate",        &CRPCMod::RPCAddNewTemplate)
                ("importtemplate",        &CRPCMod::RPCImportTemplate)
                ("exporttemplate",        &CRPCMod::RPCExportTemplate)
                ("validateaddress",       &CRPCMod::RPCValidateAddress)
                ("resyncwallet",          &CRPCMod::RPCResyncWallet)
                ("getbalance",            &CRPCMod::RPCGetBalance)
                ("listtransaction",       &CRPCMod::RPCListTransaction)
                ("sendfrom",              &CRPCMod::RPCSendFrom)
                ("createtransaction",     &CRPCMod::RPCCreateTransaction)
                ("signtransaction",       &CRPCMod::RPCSignTransaction)
                ("signmessage",           &CRPCMod::RPCSignMessage)
                ("listaddress",           &CRPCMod::RPCListAddress)
                ("exportwallet",          &CRPCMod::RPCExportWallet)
                ("importwallet",          &CRPCMod::RPCImportWallet)
                ("makeorigin",            &CRPCMod::RPCMakeOrigin)
                /* Util */
                ("verifymessage",         &CRPCMod::RPCVerifyMessage)
                ("makekeypair",           &CRPCMod::RPCMakeKeyPair)
                ("getpubkeyaddress",      &CRPCMod::RPCGetPubKeyAddress)
                ("gettemplateaddress",    &CRPCMod::RPCGetTemplateAddress)
                ("maketemplate",          &CRPCMod::RPCMakeTemplate)
                ("decodetransaction",     &CRPCMod::RPCDecodeTransaction)
                /* Mint */
                ("getwork",               &CRPCMod::RPCGetWork)
                ("submitwork",            &CRPCMod::RPCSubmitWork)
                ;
    mapRPCFunc = temp_map; 
}

CRPCMod::~CRPCMod()
{
}

bool CRPCMod::WalleveHandleInitialize()
{
    if (!WalleveGetObject("httpserver",pHttpServer))
    {
        WalleveError("Failed to request httpserver\n");
        return false;
    }

    if (!WalleveGetObject("coreprotocol",pCoreProtocol))
    {
        WalleveError("Failed to request coreprotocol\n");
        return false;
    }
    
    if (!WalleveGetObject("service",pService))
    {
        WalleveError("Failed to request service\n");
        return false;
    }

    return true;
}

void CRPCMod::WalleveHandleDeinitialize()
{
    pHttpServer = NULL;
    pCoreProtocol = NULL;
    pService = NULL;
}

bool CRPCMod::HandleEvent(CWalleveEventHttpReq& eventHttpReq)
{
    auto lmdMask = [] (const string& data) -> string {
        //remove all sensible information such as private key
        // or passphrass from log content

        //log for debug mode
        boost::regex ptnSec(R"raw(("privkey"|"passphrase"|"oldpassphrase")(\s*:\s*)(".*?"))raw", boost::regex::perl);
        return boost::regex_replace(data, ptnSec, string(R"raw($1$2"***")raw"));
    };

    uint64 nNonce = eventHttpReq.nNonce;

    string strResult;
    try
    {
        // check version
        string strVersion = eventHttpReq.data.mapHeader["url"].substr(1);
        if (!strVersion.empty())
        {
            if (!CheckVersion(strVersion))
            {
                throw CRPCException(RPC_VERSION_OUT_OF_DATE,
                    string("Out of date version. Server version is v") + MV_VERSION_STR
                    + ", but client version is v" + strVersion);
            }
        }

        bool fArray;
        CRPCReqVec vecReq = DeserializeCRPCReq(eventHttpReq.data.strContent, fArray);
        CRPCRespVec vecResp;
        for (auto& spReq : vecReq)
        {
            CRPCErrorPtr spError;
            CRPCResultPtr spResult;
            try
            {
                map<string,RPCFunc>::iterator it = mapRPCFunc.find(spReq->strMethod);
                if (it == mapRPCFunc.end())
                {
                    throw CRPCException(RPC_METHOD_NOT_FOUND, "Method not found");
                }

                WalleveDebug("request : %s\n", lmdMask(spReq->Serialize()).c_str());

                spResult = (this->*(*it).second)(spReq->spParam);
            }
            catch (CRPCException& e)
            {
                spError = CRPCErrorPtr(new CRPCError(e));
            }
            catch (exception& e)
            {
                spError = CRPCErrorPtr(new CRPCError(RPC_MISC_ERROR, e.what()));
            }

            if (spError)
            {
                vecResp.push_back(MakeCRPCRespPtr(spReq->valID, spError));
            }
            else if (spResult)
            {
                vecResp.push_back(MakeCRPCRespPtr(spReq->valID, spResult));
            }
            else
            {
                // no result means no return
            }
        }

        if (fArray)
        {
            strResult = SerializeCRPCResp(vecResp);
        }
        else if (vecResp.size() > 0)
        {
            strResult = vecResp[0]->Serialize();
        }
        else
        {
            // no result means no return
        }
    }
    catch(CRPCException& e)
    {
        auto spError = MakeCRPCErrorPtr(e);
        CRPCResp resp(e.valData, spError);
        strResult = resp.Serialize();
    }
    catch(exception& e)
    {
        cout << "error: " << e.what() << endl;
        auto spError = MakeCRPCErrorPtr(RPC_MISC_ERROR, e.what());
        CRPCResp resp(Value(), spError);
        strResult = resp.Serialize();
    }

    WalleveDebug("response : %s\n", lmdMask(strResult).c_str());

    // no result means no return
    if (!strResult.empty())
    {
        JsonReply(nNonce, strResult);
    }

    return true;
}

bool CRPCMod::HandleEvent(CWalleveEventHttpBroken& eventHttpBroken)
{
    (void)eventHttpBroken;
    return true;
}

void CRPCMod::JsonReply(uint64 nNonce, const std::string& result)
{
    CWalleveEventHttpRsp eventHttpRsp(nNonce);
    eventHttpRsp.data.nStatusCode = 200;
    eventHttpRsp.data.mapHeader["content-type"] = "application/json";
    eventHttpRsp.data.mapHeader["connection"] = "Keep-Alive";
    eventHttpRsp.data.mapHeader["server"] = "multiverse-rpc";
    eventHttpRsp.data.strContent = result + "\n";

    pHttpServer->DispatchEvent(&eventHttpRsp);
}

bool CRPCMod::CheckWalletError(MvErr err)
{
    switch (err)
    {
    case MV_ERR_WALLET_NOT_FOUND:
        throw CRPCException(RPC_INVALID_REQUEST,"Missing wallet");
        break;
    case MV_ERR_WALLET_IS_LOCKED:
        throw CRPCException(RPC_WALLET_UNLOCK_NEEDED,
                           "Wallet is locked,enter the wallet passphrase with walletpassphrase first.");
    case MV_ERR_WALLET_IS_UNLOCKED:
        throw CRPCException(RPC_WALLET_ALREADY_UNLOCKED,"Wallet is already unlocked");
        break;
    case MV_ERR_WALLET_IS_ENCRYPTED:
        throw CRPCException(RPC_WALLET_WRONG_ENC_STATE,"Running with an encrypted wallet, "
                                                      "but encryptwallet was called");
        break;
    case MV_ERR_WALLET_IS_UNENCRYPTED:
        throw CRPCException(RPC_WALLET_WRONG_ENC_STATE,"Running with an unencrypted wallet, "
                                                      "but walletpassphrasechange/walletlock was called.");
        break;
    default:
        break;
    }
    return (err == MV_OK);
}

crypto::CPubKey CRPCMod::GetPubKey(const string& addr)
{
    crypto::CPubKey pubkey;
    CMvAddress address(addr);
    if (!address.IsNull())
    {
        if (!address.GetPubKey(pubkey))
        {
            throw CRPCException(RPC_INVALID_PARAMETER, "Invalid address, should be pubkey address");
        }
    }
    else
    {
        pubkey.SetHex(addr);
    }
    return pubkey;
}

CTemplatePtr CRPCMod::MakeTemplate(const CTemplateRequest& obj)
{
    CTemplatePtr ptr;
    if (obj.strType == "weighted")
    {
        const int64& nRequired = obj.weighted.nRequired;
        const vector<CTemplatePubKeyWeight>& vecPubkeys = obj.weighted.vecPubkeys;
        vector<pair<crypto::CPubKey,unsigned char> > vPubKey;
        for (auto& info: vecPubkeys)
        {
            crypto::CPubKey pubkey = GetPubKey(info.strKey);
            int nWeight = info.nWeight;
            if (nWeight <= 0 || nWeight >= 256)
            {
                throw CRPCException(RPC_INVALID_PARAMETER, "Invalid parameter, missing weight");
            }
            vPubKey.push_back(make_pair(pubkey,(unsigned char)nWeight));
        }
        ptr = CTemplatePtr(new CTemplateWeighted(vPubKey,nRequired));
    }
    else if (obj.strType == "multisig")
    {
        const int64& nRequired = obj.multisig.nRequired;
        const vector<string>& vecPubkeys = obj.multisig.vecPubkeys;
        vector<crypto::CPubKey> vPubKey;
        for (auto& key : vecPubkeys)
        {
            vPubKey.push_back(GetPubKey(key));
        } 
        ptr = CTemplatePtr(new CTemplateMultiSig(vPubKey,nRequired));
    }
    else if (obj.strType == "fork")
    {
        const string& strRedeem = obj.fork.strRedeem;
        const string& strFork = obj.fork.strFork;
        CMvAddress address(strRedeem);
        if (address.IsNull())
        {
            throw CRPCException(RPC_INVALID_PARAMETER, "Invalid parameter, missing redeem address");
        }
        uint256 hashFork(strFork);
        ptr = CTemplatePtr(new CTemplateFork(static_cast<CDestination&>(address),hashFork));
    }
    else if (obj.strType == "mint")
    {
        const string& strMint = obj.mint.strMint;
        const string& strSpent = obj.mint.strSpent;
        crypto::CPubKey keyMint = GetPubKey(strMint);
        CMvAddress addrSpent(strSpent);
        if (addrSpent.IsNull())
        {
            throw CRPCException(RPC_INVALID_PARAMETER, "Invalid parameter, missing spent address");
        }
        ptr = CTemplatePtr(new CTemplateMint(keyMint,static_cast<CDestination&>(addrSpent)));
    }
    else if (obj.strType == "delegate")
    {
        const string& strDelegate = obj.delegate.strDelegate;
        const string& strOwner = obj.delegate.strOwner;
        crypto::CPubKey keyDelegate = GetPubKey(strDelegate);
        CMvAddress addrOwner(strOwner);
        if (addrOwner.IsNull())
        {
            throw CRPCException(RPC_INVALID_PARAMETER, "Invalid parameter, missing owner address");
        }
        ptr = CTemplatePtr(new CTemplateDelegate(keyDelegate,static_cast<CDestination&>(addrOwner)));
    }
    else {
        throw CRPCException(RPC_INVALID_PARAMS, string("template type error. type: ") + obj.strType);
    }
    return ptr;
}

void CRPCMod::ListDestination(vector<CDestination>& vDestination)
{
    set<crypto::CPubKey> setPubKey;
    set<CTemplateId> setTid;
    pService->GetPubKeys(setPubKey);
    pService->GetTemplateIds(setTid);

    vDestination.clear();
    BOOST_FOREACH(const crypto::CPubKey& pubkey,setPubKey)
    {
        vDestination.push_back(CDestination(pubkey));
    }
    BOOST_FOREACH(const CTemplateId& tid,setTid)
    {
        vDestination.push_back(CDestination(tid));
    }
}

bool CRPCMod::CheckVersion(string& strVersion)
{
    int nMajor, nMinor, nRevision;
    if (!ResolveVersion(strVersion, nMajor, nMinor, nRevision))
    {
        return false;
    }

    strVersion = FormatVersion(nMajor, nMinor, nRevision);
    if (nMajor != MV_VERSION_MAJOR || nMinor != MV_VERSION_MINOR)
    {
        return false;
    }

    return true;
}

/* System */
CRPCResultPtr CRPCMod::RPCHelp(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CHelpParam>(param);
    string command = spParam->strCommand;
    return MakeCHelpResultPtr(Help(EModeType::CONSOLE, command));
}

CRPCResultPtr CRPCMod::RPCStop(CRPCParamPtr param)
{
    pService->Shutdown();
    return MakeCStopResultPtr("multiverse server stopping");
}

CRPCResultPtr CRPCMod::RPCVersion(CRPCParamPtr param)
{
    string strVersion = string("Multiverse server version is v") + MV_VERSION_STR;
    return MakeCVersionResultPtr(strVersion);
}

/* Network */
CRPCResultPtr CRPCMod::RPCGetPeerCount(CRPCParamPtr param)
{
    return MakeCGetPeerCountResultPtr(pService->GetPeerCount());
}

CRPCResultPtr CRPCMod::RPCListPeer(CRPCParamPtr param)
{
    vector<network::CMvPeerInfo> vPeerInfo;
    pService->GetPeers(vPeerInfo);

    auto spResult = MakeCListPeerResultPtr();
    BOOST_FOREACH(const network::CMvPeerInfo& info,vPeerInfo)
    {
        CListPeerResult::CPeer peer;
        peer.strAddress = info.strAddress;
        peer.strServices = UIntToHexString(info.nService);
        peer.nLastsend = info.nLastSend;
        peer.nLastrecv = info.nLastRecv;
        peer.nConntime = info.nActive;
        peer.strVersion = FormatVersion(info.nVersion);
        peer.strSubver = info.strSubVer;
        peer.fInbound = info.fInBound;
        peer.nHeight = info.nStartingHeight;
        peer.nBanscore = info.nScore;
        spResult->vecPeer.push_back(peer);
    }

    return spResult;
}

CRPCResultPtr CRPCMod::RPCAddNode(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CAddNodeParam>(param);
    string strNode = spParam->strNode;

    if (!pService->AddNode(CNetHost(strNode,WalleveConfig()->nPort)))
    {
        throw CRPCException(RPC_CLIENT_INVALID_IP_OR_SUBNET,"Failed to add node.");
    }

    return MakeCAddNodeResultPtr(string("Add node successfully: ") + strNode);
}

CRPCResultPtr CRPCMod::RPCRemoveNode(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CRemoveNodeParam>(param);
    string strNode = spParam->strNode;

    if (!pService->RemoveNode(CNetHost(strNode,WalleveConfig()->nPort)))
    {
        throw CRPCException(RPC_CLIENT_INVALID_IP_OR_SUBNET,"Failed to remove node.");
    }

    return MakeCRemoveNodeResultPtr(string("Remove node successfully: ") + strNode);
}

CRPCResultPtr CRPCMod::RPCGetForkCount(CRPCParamPtr param)
{
    return MakeCGetForkCountResultPtr(pService->GetForkCount());
}

CRPCResultPtr CRPCMod::RPCListFork(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CListForkParam>(param);
    vector<pair<uint256,CProfile> > vFork;
    pService->ListFork(vFork, spParam->fAll);

    auto spResult = MakeCListForkResultPtr();
    for (size_t i = 0; i < vFork.size(); i++)
    {
        CProfile& profile = vFork[i].second;
        spResult->vecProfile.push_back({ vFork[i].first.GetHex(), profile.strName, profile.strSymbol,
                                         profile.IsIsolated(), profile.IsPrivate(), profile.IsEnclosed(),
                                         CMvAddress(profile.destOwner).ToString() });
    }

    return spResult;
}

CRPCResultPtr CRPCMod::RPCGetForkGenealogy(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CGetGenealogyParam>(param);

    //getgenealogy (-f="fork")
    uint256 fork;
    if (!GetForkHashOfDef(spParam->strFork, fork))
    {
    	throw CRPCException(RPC_INVALID_PARAMETER, "Invalid fork");
    }

    vector<pair<uint256,int> > vAncestry;
    vector<pair<int,uint256> > vSubline;
    if (!pService->GetForkGenealogy(fork,vAncestry,vSubline))
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Unknown fork");
    }

    auto spResult = MakeCGetGenealogyResultPtr();
    for (int i = vAncestry.size();i > 0;i--)
    {
        spResult->vecAncestry.push_back({vAncestry[i - 1].first.GetHex(), vAncestry[i - 1].second});
    }
    for (std::size_t i = 0;i < vSubline.size();i++)
    {
        spResult->vecSubline.push_back({vSubline[i].second.GetHex(), vSubline[i].first});
    }
    return spResult;
}

CRPCResultPtr CRPCMod::RPCGetBlockLocation(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CGetBlockLocationParam>(param);
    
    //getblocklocation <"block">
    uint256 hashBlock;
    hashBlock.SetHex(spParam->strBlock);

    uint256 fork;
    int height;
    if (!pService->GetBlockLocation(hashBlock,fork,height))
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Unknown block");
    }

    auto spResult = MakeCGetBlockLocationResultPtr();
    spResult->strFork = fork.GetHex();
    spResult->nHeight = height;
    return spResult;
}

CRPCResultPtr CRPCMod::RPCGetBlockCount(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CGetBlockCountParam>(param);

    //getblockcount (-f="fork")
    uint256 hashFork;
	if (!GetForkHashOfDef(spParam->strFork, hashFork))
    {
    	throw CRPCException(RPC_INVALID_PARAMETER, "Invalid fork");
    }
    
    if (!pService->HaveFork(hashFork))
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Unknown fork");
    }

    return MakeCGetBlockCountResultPtr(pService->GetBlockCount(hashFork));
}

CRPCResultPtr CRPCMod::RPCGetBlockHash(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CGetBlockHashParam>(param);

    //getblockhash <height> (-f="fork")
    int nHeight = spParam->nHeight;

    uint256 hashFork;
    if (!GetForkHashOfDef(spParam->strFork, hashFork))
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Invalid fork");
    }
    
    if (!pService->HaveFork(hashFork))
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Unknown fork");
    }

    vector<uint256> vBlockHash;
    if (!pService->GetBlockHash(hashFork,nHeight,vBlockHash))
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Block number out of range.");
    }

    auto spResult = MakeCGetBlockHashResultPtr();
    for (const uint256& hash: vBlockHash)
    {
        spResult->vecHash.push_back(hash.GetHex());
    }

    return spResult;
}

CRPCResultPtr CRPCMod::RPCGetBlock(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CGetBlockParam>(param);
    
    //getblock <"block">
    uint256 hashBlock;
    hashBlock.SetHex(spParam->strBlock);

    CBlock block;
    uint256 fork;
    int height;
    if (!pService->GetBlock(hashBlock,block,fork,height))
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Unknown block");
    }

    return MakeCGetBlockResultPtr(BlockToJSON(hashBlock,block,fork,height));
}

CRPCResultPtr CRPCMod::RPCGetTxPool(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CGetTxPoolParam>(param);

    //gettxpool (-f="fork") (-d|-nod*detail*)
    uint256 hashFork;
    if (!GetForkHashOfDef(spParam->strFork, hashFork))
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Invalid fork");
    }
    
    if (!pService->HaveFork(hashFork))
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Unknown fork");
    }

    bool fDetail = spParam->fDetail.IsValid() ? bool(spParam->fDetail) : false;
    
    vector<pair<uint256,size_t> > vTxPool;
    pService->GetTxPool(hashFork,vTxPool);

    auto spResult = MakeCGetTxPoolResultPtr();
    if (!fDetail)
    {
        size_t nTotalSize = 0;
        for (std::size_t i = 0;i < vTxPool.size();i++)
        {
            nTotalSize += vTxPool[i].second;
        }
        spResult->nCount = vTxPool.size();
        spResult->nSize = nTotalSize;
    }
    else
    {
        for (std::size_t i = 0;i < vTxPool.size();i++)
        {
            spResult->vecList.push_back({vTxPool[i].first.GetHex(), vTxPool[i].second});
        }
    }
    
    return spResult;
}

// CRPCResultPtr CRPCMod::RPCRemovePendingTx(CRPCParamPtr param)
// {
//     auto spParam = CastParamPtr<CRemovePendingTxParam>(param);

//     uint256 txid;
//     txid.SetHex(spParam->strTxid);

//     if (!pService->RemovePendingTx(txid))
//     {
//         throw CRPCException(RPC_INVALID_REQUEST, "This transaction is not in tx pool");
//     }

//     return MakeCRemovePendingTxResultPtr(string("Remove tx successfully: ") + spParam->strTxid);
// }

CRPCResultPtr CRPCMod::RPCGetTransaction(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CGetTransactionParam>(param);
    uint256 txid;
    txid.SetHex(spParam->strTxid);

    CTransaction tx;
    uint256 hashFork;
    int nHeight;

    if (!pService->GetTransaction(txid,tx,hashFork,nHeight))
    {
        throw CRPCException(RPC_INVALID_REQUEST, "No information available about transaction");
    }

    auto spResult = MakeCGetTransactionResultPtr();
    if (spParam->fSerialized)
    {
        CWalleveBufStream ss;
        ss << tx;
        spResult->strSerialization = ToHexString((const unsigned char*)ss.GetData(),ss.GetSize());
        return spResult;
    }

    int nDepth = nHeight < 0 ? 0 : pService->GetBlockCount(hashFork) - nHeight;
    spResult->transaction = TxToJSON(txid,tx,hashFork,nDepth);
    return spResult;
}

CRPCResultPtr CRPCMod::RPCSendTransaction(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CSendTransactionParam>(param);

    vector<unsigned char> txData = ParseHexString(spParam->strTxdata);
    CWalleveBufStream ss;
    ss.Write((char *)&txData[0],txData.size());
    CTransaction rawTx;
    try
    {
        ss >> rawTx;
    }
    catch (const std::exception &e)
    {
        throw CRPCException(RPC_DESERIALIZATION_ERROR, "TX decode failed");
    }
    MvErr err = pService->SendTransaction(rawTx);
    if (err != MV_OK)
    {
        throw CRPCException(RPC_TRANSACTION_REJECTED,string("Tx rejected : ")
                                                    + MvErrString(err));
    }
    
    return MakeCSendTransactionResultPtr(rawTx.GetHash().GetHex());
}

CRPCResultPtr CRPCMod::RPCGetForkHeight(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CGetForkHeightParam>(param);

    //getforkheight (-f="fork")
    uint256 hashFork;
    if (!GetForkHashOfDef(spParam->strFork, hashFork))
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Invalid fork");
    }
    
    if (!pService->HaveFork(hashFork))
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Unknown fork");
    }
    
    return MakeCGetForkHeightResultPtr(pService->GetForkHeight(hashFork));
}

/* Wallet */
CRPCResultPtr CRPCMod::RPCListKey(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CListKeyParam>(param);

    set<crypto::CPubKey> setPubKey;
    pService->GetPubKeys(setPubKey);

    auto spResult = MakeCListKeyResultPtr();
    BOOST_FOREACH(const crypto::CPubKey& pubkey,setPubKey)
    {
        int nVersion;
        bool fLocked;
        int64 nAutoLockTime;
        if (pService->GetKeyStatus(pubkey,nVersion,fLocked,nAutoLockTime))
        {
            CListKeyResult::CPubkey p;
            p.strKey = pubkey.GetHex();
            p.nVersion = nVersion;
            p.fLocked = fLocked;
            if (!fLocked && nAutoLockTime > 0)
            {
                p.nTimeout = (nAutoLockTime - GetTime());
            }
            spResult->vecPubkey.push_back(p);
        }
    } 
    return spResult;
}

CRPCResultPtr CRPCMod::RPCGetNewKey(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CGetNewKeyParam>(param);

    if (spParam->strPassphrase.empty())
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Passphrase must be nonempty");
    }
    crypto::CCryptoString strPassphrase = spParam->strPassphrase.c_str();
    crypto::CPubKey pubkey;
    if (!pService->MakeNewKey(strPassphrase,pubkey))
    {
        throw CRPCException(RPC_WALLET_ERROR,"Failed add new key.");
    }

    return MakeCGetNewKeyResultPtr(pubkey.ToString());
}

CRPCResultPtr CRPCMod::RPCEncryptKey(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CEncryptKeyParam>(param);

    //encryptkey <"pubkey"> <-new="passphrase"> <-old="oldpassphrase">
    crypto::CPubKey pubkey;
    pubkey.SetHex(spParam->strPubkey);

    if (spParam->strPassphrase.empty())
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Passphrase must be nonempty");
    }
    crypto::CCryptoString strPassphrase = spParam->strPassphrase.c_str();

    if (spParam->strOldpassphrase.empty())
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Old passphrase must be nonempty");
    }
    crypto::CCryptoString strOldPassphrase = spParam->strOldpassphrase.c_str();

    if (!pService->HaveKey(pubkey))
    {
        throw CRPCException(RPC_INVALID_ADDRESS_OR_KEY,"Unknown key");
    }
    if (!pService->EncryptKey(pubkey,strPassphrase,strOldPassphrase))
    {
        throw CRPCException(RPC_WALLET_PASSPHRASE_INCORRECT,"The passphrase entered was incorrect.");
    }

    return MakeCEncryptKeyResultPtr(string("Encrypt key successfully: ") + spParam->strPubkey);
}

CRPCResultPtr CRPCMod::RPCLockKey(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CLockKeyParam>(param);

    CMvAddress address(spParam->strPubkey);
    if(address.IsTemplate())
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "This method only accepts pubkey or pubkey address as parameter rather than template address you supplied.");
    }

    crypto::CPubKey pubkey;
    if(address.IsPubKey())
    {
        address.GetPubKey(pubkey);
    }
    else
    {
        pubkey.SetHex(spParam->strPubkey);
    }

    int nVersion;
    bool fLocked;
    int64 nAutoLockTime; 
    if (!pService->GetKeyStatus(pubkey,nVersion,fLocked,nAutoLockTime))
    {
        throw CRPCException(RPC_INVALID_ADDRESS_OR_KEY,"Unknown key");
    }
    if (!fLocked && !pService->Lock(pubkey))
    {
        throw CRPCException(RPC_WALLET_ERROR,"Failed to lock key");
    }
    return MakeCLockKeyResultPtr(string("Lock key successfully: ") + spParam->strPubkey);
}

CRPCResultPtr CRPCMod::RPCUnlockKey(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CUnlockKeyParam>(param);

    CMvAddress address(spParam->strPubkey);
    if(address.IsTemplate())
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "This method only accepts pubkey or pubkey address as parameter rather than template address you supplied.");
    }
  
    crypto::CPubKey pubkey;
    if(address.IsPubKey())
    {
        address.GetPubKey(pubkey);
    }
    else
    {
        pubkey.SetHex(spParam->strPubkey);
    }

    if (spParam->strPassphrase.empty())
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Passphrase must be nonempty");
    }
        
    crypto::CCryptoString strPassphrase = spParam->strPassphrase.c_str();
    int64 nTimeout = 0;
    if (spParam->nTimeout.IsValid())
    {
         nTimeout = spParam->nTimeout;
    }

    int nVersion;
    bool fLocked;
    int64 nAutoLockTime; 
    if (!pService->GetKeyStatus(pubkey,nVersion,fLocked,nAutoLockTime))
    {
        throw CRPCException(RPC_INVALID_ADDRESS_OR_KEY,"Unknown key");
    }

    if (!fLocked)
    {
        throw CRPCException(RPC_WALLET_ALREADY_UNLOCKED,"Key is already unlocked");
    }

    if (!pService->Unlock(pubkey,strPassphrase,nTimeout))
    {
        throw CRPCException(RPC_WALLET_PASSPHRASE_INCORRECT,"The passphrase entered was incorrect.");
    }
        
    return MakeCUnlockKeyResultPtr(string("Unlock key successfully: ") + spParam->strPubkey);
}

CRPCResultPtr CRPCMod::RPCImportPrivKey(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CImportPrivKeyParam>(param);

    //importprivkey <"privkey"> <"passphrase">
    uint256 nPriv;
    if (nPriv.SetHex(spParam->strPrivkey) != spParam->strPrivkey.size())
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Invalid private key");
    }

    if (spParam->strPassphrase.empty())
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Passphrase must be nonempty");
    }

    crypto::CCryptoString strPassphrase = spParam->strPassphrase.c_str();

    crypto::CKey key;
    if (!key.SetSecret(crypto::CCryptoKeyData(nPriv.begin(),nPriv.end())))
    {
        throw CRPCException(RPC_INVALID_PARAMETER,"Invalid private key");
    }
    if (pService->HaveKey(key.GetPubKey()))
    {
        throw CRPCException(RPC_WALLET_ERROR,"Already have key");
    }
    if (!strPassphrase.empty())
    {
        key.Encrypt(strPassphrase);
    }
    if (!pService->AddKey(key))
    {
        throw CRPCException(RPC_WALLET_ERROR,"Failed to add key");
    }
    if (!pService->SynchronizeWalletTx(CDestination(key.GetPubKey())))
    {
        throw CRPCException(RPC_WALLET_ERROR,"Failed to sync wallet tx");
    }

    return MakeCImportPrivKeyResultPtr(key.GetPubKey().GetHex());
}

CRPCResultPtr CRPCMod::RPCImportKey(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CImportKeyParam>(param);

    vector<unsigned char> vchKey = ParseHexString(spParam->strPubkey);
    crypto::CKey key;
    if (!key.Load(vchKey))
    {
        throw CRPCException(RPC_INVALID_PARAMS,"Failed to verify serialized key");
    }
    if (key.GetVersion() == 0)
    {
        throw CRPCException(RPC_INVALID_PARAMS, "Can't import the key with empty passphrase");
    }
    if (pService->HaveKey(key.GetPubKey()))
    {
        throw CRPCException(RPC_WALLET_ERROR,"Already have key");
    }
    
    if (!pService->AddKey(key))
    {
        throw CRPCException(RPC_WALLET_ERROR,"Failed to add key");
    }
    if (!pService->SynchronizeWalletTx(CDestination(key.GetPubKey())))
    {
        throw CRPCException(RPC_WALLET_ERROR,"Failed to sync wallet tx");
    }

    return MakeCImportKeyResultPtr(key.GetPubKey().GetHex());
}

CRPCResultPtr CRPCMod::RPCExportKey(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CExportKeyParam>(param);

    crypto::CPubKey pubkey;
    pubkey.SetHex(spParam->strPubkey);

    if (!pService->HaveKey(pubkey))
    {
        throw CRPCException(RPC_INVALID_ADDRESS_OR_KEY,"Unknown key");
    }
    vector<unsigned char> vchKey;
    if (!pService->ExportKey(pubkey,vchKey))
    {
        throw CRPCException(RPC_WALLET_ERROR,"Failed to export key");
    }

    return MakeCExportKeyResultPtr(ToHexString(vchKey));
}

CRPCResultPtr CRPCMod::RPCAddNewTemplate(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CAddNewTemplateParam>(param);
    CTemplatePtr ptr = MakeTemplate(spParam->data);
    if (ptr == NULL || ptr->IsNull())
    {
        throw CRPCException(RPC_INVALID_PARAMETER,"Invalid parameters,failed to make template");
    }
    if (!pService->AddTemplate(ptr))
    {
        throw CRPCException(RPC_WALLET_ERROR,"Failed to add template");
    }
    if (!pService->SynchronizeWalletTx(CDestination(ptr->GetTemplateId())))
    {
        throw CRPCException(RPC_WALLET_ERROR,"Failed to sync wallet tx");
    }

    return MakeCAddNewTemplateResultPtr(CMvAddress(ptr->GetTemplateId()).ToString());
}

CRPCResultPtr CRPCMod::RPCImportTemplate(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CImportTemplateParam>(param);
    vector<unsigned char> vchTemplate = ParseHexString(spParam->strData);
    CTemplatePtr ptr = CTemplateGeneric::CreateTemplatePtr(vchTemplate);
    if (ptr == NULL || ptr->IsNull())
    {
        throw CRPCException(RPC_INVALID_PARAMETER,"Invalid parameters,failed to make template");
    }
    if (pService->HaveTemplate(ptr->GetTemplateId()))
    {
        throw CRPCException(RPC_WALLET_ERROR,"Already have this template");
    }
    if (!pService->AddTemplate(ptr))
    {
        throw CRPCException(RPC_WALLET_ERROR,"Failed to add template");
    }
    if (!pService->SynchronizeWalletTx(CDestination(ptr->GetTemplateId())))
    {
        throw CRPCException(RPC_WALLET_ERROR,"Failed to sync wallet tx");
    }
    
    return MakeCImportTemplateResultPtr(CMvAddress(ptr->GetTemplateId()).ToString()); 
}

CRPCResultPtr CRPCMod::RPCExportTemplate(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CExportTemplateParam>(param);
    CMvAddress address(spParam->strAddress);
    if (address.IsNull())
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Invalid address");
    }

    CTemplateId tid;
    if (!address.GetTemplateId(tid))
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Invalid address");
    }

    CTemplatePtr ptr;
    if (!pService->GetTemplate(tid,ptr))
    {
        throw CRPCException(RPC_WALLET_ERROR,"Unkown template");
    }
    vector<unsigned char> vchTemplate;
    ptr->Export(vchTemplate);

    return MakeCExportTemplateResultPtr(ToHexString(vchTemplate));
}

CRPCResultPtr CRPCMod::RPCValidateAddress(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CValidateAddressParam>(param);

    CMvAddress address(spParam->strAddress);
    bool isValid = !address.IsNull();

    auto spResult = MakeCValidateAddressResultPtr();
    spResult->fIsvalid = isValid;
    if (isValid)
    {
        auto& addressData = spResult->addressdata;

        addressData.strAddress = address.ToString();
        if (address.IsPubKey())
        {
            crypto::CPubKey pubkey;
            address.GetPubKey(pubkey);
            bool isMine = pService->HaveKey(pubkey);
            addressData.fIsmine = isMine;
            addressData.strType = "pubkey";
            addressData.strPubkey = pubkey.GetHex();
        }
        else if (address.IsTemplate())
        {
            CTemplateId tid;
            address.GetTemplateId(tid);
            CTemplatePtr ptr;
            uint16 nType = tid.GetType();
            bool isMine = pService->GetTemplate(tid,ptr);
            addressData.fIsmine = isMine;
            addressData.strType = "template";
            addressData.strTemplate = CTemplateGeneric::GetTypeName(nType);
            if (isMine)
            {
                auto& templateData = addressData.templatedata;

                vector<unsigned char> vchTemplate;
                ptr->Export(vchTemplate);
                templateData.strHex = ToHexString(vchTemplate);
                templateData.strType = CTemplateGeneric::GetTypeName(nType);
                if (nType == TEMPLATE_WEIGHTED)
                {
                    CTemplateWeighted* p = (CTemplateWeighted*)ptr.get();
                    templateData.weighted.nSigsrequired = p->nRequired;
                    for (map<crypto::CPubKey,unsigned char>::iterator it = p->mapPubKeyWeight.begin();
                         it != p->mapPubKeyWeight.end();++it)
                    {
                        CTemplatePubKeyWeight pubkey;
                        pubkey.strKey = CMvAddress((*it).first).ToString();
                        pubkey.nWeight = (*it).second;
                        templateData.weighted.vecAddresses.push_back(pubkey);
                    }
                }
                else if (nType == TEMPLATE_MULTISIG)
                {
                    CTemplateMultiSig* p = (CTemplateMultiSig*)ptr.get();
                    templateData.multisig.nSigsrequired = p->nRequired;
                    for (map<crypto::CPubKey,unsigned char>::iterator it = p->mapPubKeyWeight.begin();
                         it != p->mapPubKeyWeight.end();++it)
                    {
                        templateData.multisig.vecAddresses.push_back(CMvAddress((*it).first).ToString());
                    }
                }
                else if (nType == TEMPLATE_FORK)
                {
                    CTemplateFork* p = (CTemplateFork*)ptr.get();
                    templateData.fork.strFork = p->hashFork.GetHex();
                    templateData.fork.strRedeem = CMvAddress(p->destRedeem).ToString();
                }
                else if (nType == TEMPLATE_MINT)
                {
                    CTemplateMint* p = (CTemplateMint*)ptr.get();
                    templateData.mint.strMint = CMvAddress(p->keyMint).ToString();
                    templateData.mint.strSpent = CMvAddress(p->destSpend).ToString();
                }
                else if (nType == TEMPLATE_DELEGATE)
                {
                    CTemplateDelegate* p = (CTemplateDelegate*)ptr.get();
                    templateData.delegate.strDelegate = CMvAddress(p->keyDelegate).ToString();
                    templateData.delegate.strOwner = CMvAddress(p->destOwner).ToString();
                }
            }
        }
    }
    return spResult;
}

CRPCResultPtr CRPCMod::RPCResyncWallet(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CResyncWalletParam>(param);
    if (spParam->strAddress.IsValid())
    {
        CMvAddress address(spParam->strAddress);
        if (address.IsNull())
        {
            throw CRPCException(RPC_INVALID_PARAMETER, "Invalid address");
        }
        if (!pService->SynchronizeWalletTx(static_cast<CDestination&>(address)))
        {
            throw CRPCException(RPC_WALLET_ERROR, "Failed to resync wallet tx");
        }
    }
    else
    {
        if (!pService->ResynchronizeWalletTx())
        {
            throw CRPCException(RPC_WALLET_ERROR, "Failed to resync wallet tx");
        }
    }
    return MakeCResyncWalletResultPtr("Resync wallet successfully.");
}

CRPCResultPtr CRPCMod::RPCGetBalance(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CGetBalanceParam>(param);

    //getbalance (-f="fork") (-a="address")
    uint256 hashFork;
    if (!GetForkHashOfDef(spParam->strFork, hashFork))
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Invalid fork");
    }
    
    if (!pService->HaveFork(hashFork))
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Unknown fork");
    }
    
    vector<CDestination> vDest;
    if (spParam->strAddress.IsValid())
    {
        CMvAddress address(spParam->strAddress);
        if (address.IsNull())
        {
            throw CRPCException(RPC_INVALID_PARAMETER, "Invalid address");
        }
        vDest.push_back(static_cast<CDestination&>(address));
    }
    else 
    {
        ListDestination(vDest);
    }

    auto spResult = MakeCGetBalanceResultPtr();
    BOOST_FOREACH(const CDestination& dest,vDest)
    {
        CWalletBalance balance;
        if (pService->GetBalance(dest,hashFork,balance))
        {
            CGetBalanceResult::CBalance b;
            b.strAddress = CMvAddress(dest).ToString();
            b.fAvail = ValueFromAmount(balance.nAvailable);
            b.fLocked = ValueFromAmount(balance.nLocked);
            b.fUnconfirmed = ValueFromAmount(balance.nUnconfirmed);
            spResult->vecBalance.push_back(b);
        }
    }

    return spResult;
}

CRPCResultPtr CRPCMod::RPCListTransaction(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CListTransactionParam>(param);

    int nCount = GetUint(spParam->nCount, 10);
    int nOffset = GetInt(spParam->nOffset, 0);
    if (nCount <= 0)
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Negative, zero or out of range count");
    }
    
    vector<CWalletTx> vWalletTx;
    if (!pService->ListWalletTx(nOffset,nCount,vWalletTx))
    {
        throw CRPCException(RPC_WALLET_ERROR, "Failed to list transactions");
    }

    auto spResult = MakeCListTransactionResultPtr();
    BOOST_FOREACH(const CWalletTx& wtx,vWalletTx)
    {
        spResult->vecTransaction.push_back(WalletTxToJSON(wtx));
    }
    return spResult;
}

CRPCResultPtr CRPCMod::RPCSendFrom(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CSendFromParam>(param);

    //sendfrom <"from"> <"to"> <$amount$> ($txfee$) (-f="fork") (-d="data")
    CMvAddress from(spParam->strFrom);
    if (from.IsNull())
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Invalid from address");
    }

    CMvAddress to(spParam->strTo);
    if (to.IsNull())
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Invalid to address");
    }

    int64 nAmount = AmountFromValue(spParam->fAmount);
    
    int64 nTxFee = MIN_TX_FEE;
    if (spParam->fTxfee.IsValid())
    {
        nTxFee = AmountFromValue(spParam->fTxfee);
        if (nTxFee < MIN_TX_FEE)
        {
            nTxFee = MIN_TX_FEE;
        }
    }
    
    uint256 hashFork;
    if (!GetForkHashOfDef(spParam->strFork, hashFork))
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Invalid fork");
    }
    
    if (!pService->HaveFork(hashFork))
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Unknown fork");
    }
    
    vector<unsigned char> vchData;
    if (spParam->strData.IsValid())
    {
        vchData = ParseHexString(spParam->strData);
    }

    CTransaction txNew;
    if (!pService->CreateTransaction(hashFork,from,to,nAmount,nTxFee,vchData,txNew))
    {
        throw CRPCException(RPC_WALLET_ERROR,"Failed to create transaction");
    }
    bool fCompleted = false;
    if (!pService->SignTransaction(txNew,fCompleted))
    {
        throw CRPCException(RPC_WALLET_ERROR,"Failed to sign transaction");
    }
    if (!fCompleted)
    {
        throw CRPCException(RPC_WALLET_ERROR,"The signature is not completed");
    }
    MvErr err = pService->SendTransaction(txNew);
    if (err != MV_OK)
    {
        throw CRPCException(RPC_TRANSACTION_REJECTED,string("Tx rejected : ")
                                                    + MvErrString(err));
    }

    return MakeCSendFromResultPtr(txNew.GetHash().GetHex());
}

CRPCResultPtr CRPCMod::RPCCreateTransaction(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CCreateTransactionParam>(param);

    //createtransaction <"from"> <"to"> <$amount$> ($txfee$) (-f="fork") (-d="data")
    CMvAddress from(spParam->strFrom);
    if (from.IsNull())
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Invalid from address");
    }

    CMvAddress to(spParam->strTo);
    if (to.IsNull())
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Invalid to address");
    }

    int64 nAmount = AmountFromValue(spParam->fAmount);
    
    int64 nTxFee = MIN_TX_FEE;
    if (spParam->fTxfee.IsValid())
    {
        nTxFee = AmountFromValue(spParam->fTxfee);
        if (nTxFee < MIN_TX_FEE)
        {
            nTxFee = MIN_TX_FEE;
        }
    }

    uint256 hashFork;
    if (!GetForkHashOfDef(spParam->strFork, hashFork))
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Invalid fork");
    }
    
    if (!pService->HaveFork(hashFork))
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Unknown fork");
    }
    
    vector<unsigned char> vchData;
    if (spParam->strData.IsValid())
    {
        vchData = ParseHexString(spParam->strData);
    }
    CTransaction txNew;
    if (!pService->CreateTransaction(hashFork,from,to,nAmount,nTxFee,vchData,txNew))
    {
        throw CRPCException(RPC_WALLET_ERROR,"Failed to create transaction");
    }

    CWalleveBufStream ss;
    ss << txNew;

    return MakeCCreateTransactionResultPtr(
        ToHexString((const unsigned char*)ss.GetData(),ss.GetSize()));
}

CRPCResultPtr CRPCMod::RPCSignTransaction(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CSignTransactionParam>(param);

    vector<unsigned char> txData = ParseHexString(spParam->strTxdata);
    CWalleveBufStream ss;
    ss.Write((char *)&txData[0],txData.size());
    CTransaction rawTx;
    try
    {
        ss >> rawTx;
    }
    catch (const std::exception &e)
    {
        throw CRPCException(RPC_DESERIALIZATION_ERROR, "TX decode failed");
    }

    bool fCompleted = false;
    if (!pService->SignTransaction(rawTx,fCompleted))
    {
        throw CRPCException(RPC_WALLET_ERROR,"Failed to sign transaction");
    }

    CWalleveBufStream ssNew;
    ssNew << rawTx;

    auto spResult = MakeCSignTransactionResultPtr();
    spResult->strHex = ToHexString((const unsigned char*)ssNew.GetData(),ssNew.GetSize());
    spResult->fComplete = fCompleted;
    return spResult;
}

CRPCResultPtr CRPCMod::RPCSignMessage(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CSignMessageParam>(param);

    crypto::CPubKey pubkey;
    pubkey.SetHex(spParam->strPubkey);

    string strMessage = spParam->strMessage;

    int nVersion;
    bool fLocked;
    int64 nAutoLockTime; 
    if (!pService->GetKeyStatus(pubkey,nVersion,fLocked,nAutoLockTime))
    {
        throw CRPCException(RPC_INVALID_ADDRESS_OR_KEY,"Unknown key");
    }
    if (fLocked)
    {
        throw CRPCException(RPC_WALLET_UNLOCK_NEEDED,"Key is locked");
    }
    
    const string strMessageMagic = "Multiverse Signed Message:\n";
    CWalleveBufStream ss;
    ss << strMessageMagic;
    ss << strMessage;
    vector<unsigned char> vchSig;
    if (!pService->SignSignature(pubkey,crypto::CryptoHash(ss.GetData(),ss.GetSize()),vchSig))
    {
        throw CRPCException(RPC_WALLET_ERROR,"Failed to sign message");
    }

    return MakeCSignMessageResultPtr(ToHexString(vchSig));
}

CRPCResultPtr CRPCMod::RPCListAddress(CRPCParamPtr param)
{
    auto spResult = MakeCListAddressResultPtr();
    vector<CDestination> vDes;
    ListDestination(vDes);
    for(const auto& des : vDes)
    {
        CListAddressResult::CAddressdata addressData;
        addressData.strAddress = CMvAddress(des).ToString();
        if(des.IsPubKey())
        {
            addressData.strType = "pubkey";
            crypto::CPubKey pubkey;
            des.GetPubKey(pubkey);
            addressData.strPubkey = pubkey.GetHex();
        }
        else if(des.IsTemplate())
        {
            addressData.strType = "template";

            CTemplateId tid;
            des.GetTemplateId(tid);
            CTemplatePtr ptr;
            uint16 nType = tid.GetType();
            pService->GetTemplate(tid,ptr);
            addressData.strTemplate = CTemplateGeneric::GetTypeName(nType);

            vector<unsigned char> vchTemplate;
            ptr->Export(vchTemplate);

            auto& templateData = addressData.templatedata;
            templateData.strHex = ToHexString(vchTemplate);
            templateData.strType = CTemplateGeneric::GetTypeName(nType);
            switch(nType)
            {
                case TEMPLATE_WEIGHTED:
                    {
                        CTemplateWeighted* p = dynamic_cast<CTemplateWeighted*>(ptr.get());
                        templateData.weighted.nSigsrequired = p->nRequired;
                        for (const auto& it : p->mapPubKeyWeight)
                        {
                            CTemplatePubKeyWeight pubkey;
                            pubkey.strKey = CMvAddress(it.first).ToString();
                            pubkey.nWeight = it.second;
                            templateData.weighted.vecAddresses.push_back(pubkey);
                        }
                    }
                    break;
                case TEMPLATE_MULTISIG:
                    {
                        CTemplateMultiSig* p = dynamic_cast<CTemplateMultiSig*>(ptr.get());
                        templateData.multisig.nSigsrequired = p->nRequired;
                        Array addresses;
                        for (const auto& it : p->mapPubKeyWeight)
                        {
                            templateData.multisig.vecAddresses.push_back(CMvAddress(it.first).ToString());
                        }
                    }
                    break;
                case TEMPLATE_FORK:
                    {
                        CTemplateFork* p = dynamic_cast<CTemplateFork*>(ptr.get());
                        templateData.fork.strFork = p->hashFork.GetHex();
                        templateData.fork.strRedeem = CMvAddress(p->destRedeem).ToString();
                    }
                    break;
                case TEMPLATE_MINT:
                    {
                        CTemplateMint* p = dynamic_cast<CTemplateMint*>(ptr.get());
                        templateData.mint.strMint = CMvAddress(p->keyMint).ToString();
                        templateData.mint.strSpent = CMvAddress(p->destSpend).ToString();
                    }
                    break;
                case TEMPLATE_DELEGATE:
                    {
                        CTemplateDelegate* p = dynamic_cast<CTemplateDelegate*>(ptr.get());
                        templateData.delegate.strDelegate = CMvAddress(p->keyDelegate).ToString();
                        templateData.delegate.strOwner = CMvAddress(p->destOwner).ToString();
                    }
                    break;
                default:
                    break;
            }
        }
        else
        {
            continue;
        }
        spResult->vecAddressdata.push_back(addressData);
    }

    return spResult;
}

CRPCResultPtr CRPCMod::RPCExportWallet(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CExportWalletParam>(param);

    fs::path pSave(string(spParam->strPath));
    //check if the file name given is available
    if(!pSave.is_absolute())
    {
        throw CRPCException(RPC_WALLET_ERROR, "Must be an absolute path.");
    }
    if(is_directory(pSave))
    {
        throw CRPCException(RPC_WALLET_ERROR, "Cannot export to a folder.");
    }
    if(exists(pSave))
    {
        throw CRPCException(RPC_WALLET_ERROR, "File has been existed.");
    }
    if(pSave.filename() == "." || pSave.filename() == "..")
    {
        throw CRPCException(RPC_WALLET_ERROR, "Cannot export to a folder.");
    }

    if(!exists(pSave.parent_path()) && !create_directories(pSave.parent_path()))
    {
        throw CRPCException(RPC_WALLET_ERROR, "Failed to create directories.");
    }

    Array aAddr;
    vector<CDestination> vDes;
    ListDestination(vDes);
    for(const auto& des : vDes)
    {
        if (des.IsPubKey())
        {
            Object oKey;
            oKey.push_back(Pair("address", CMvAddress(des).ToString()));

            crypto::CPubKey pubkey;
            des.GetPubKey(pubkey);
            vector<unsigned char> vchKey;
            if (!pService->ExportKey(pubkey, vchKey))
            {
                throw CRPCException(RPC_WALLET_ERROR, "Failed to export key");
            }
            oKey.push_back(Pair("hex", ToHexString(vchKey)));
            aAddr.push_back(oKey);
        }

        if (des.IsTemplate())
        {
            Object oTemp;
            CMvAddress address(des);

            oTemp.push_back(Pair("address", address.ToString()));

            CTemplateId tid;
            if (!address.GetTemplateId(tid))
            {
                throw CRPCException(RPC_INVALID_PARAMETER, "Invalid template address");
            }
            CTemplatePtr ptr;
            if (!pService->GetTemplate(tid, ptr))
            {
                throw CRPCException(RPC_WALLET_ERROR, "Unkown template");
            }
            vector<unsigned char> vchTemplate;
            ptr->Export(vchTemplate);

            oTemp.push_back(Pair("hex", ToHexString(vchTemplate)));

            aAddr.push_back(oTemp);
        }
    }
    //output them together to file
    try
    {
        std::ofstream ofs(pSave.string(), std::ios::out);
        if (!ofs)
        {
            throw runtime_error("write error");
        }

        write_stream(Value(aAddr), ofs, pretty_print);
        ofs.close();
    }
    catch(...)
    {
        throw CRPCException(RPC_WALLET_ERROR, "filesystem_error - failed to write.");
    }

    return MakeCExportWalletResultPtr(string("Wallet file has been saved at: ") + pSave.string());
}

CRPCResultPtr CRPCMod::RPCImportWallet(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CImportWalletParam>(param);

    fs::path pLoad(string(spParam->strPath));
    //check if the file name given is available
    if(!pLoad.is_absolute())
    {
        throw CRPCException(RPC_WALLET_ERROR, "Must be an absolute path.");
    }
    if(!exists(pLoad) || is_directory(pLoad))
    {
        throw CRPCException(RPC_WALLET_ERROR, "File name is invalid.");
    }

    Value vWallet;
    try
    {
        fs::ifstream ifs(pLoad);
        if (!ifs)
        {
            throw runtime_error("read error");
        }

        read_stream(ifs, vWallet);
        ifs.close();
    }
    catch(...)
    {
        throw CRPCException(RPC_WALLET_ERROR, "Filesystem_error - failed to read.");
    }

    if(array_type != vWallet.type())
    {
        throw CRPCException(RPC_WALLET_ERROR, "Wallet file exported is invalid, check it and try again.");
    }

    Array aAddr;
    uint32 nKey = 0;
    uint32 nTemp = 0;
    for(const auto& oAddr : vWallet.get_array())
    {
        if(oAddr.get_obj()[0].name_ != "address" || oAddr.get_obj()[1].name_ != "hex")
        {
            throw CRPCException(RPC_WALLET_ERROR, "Data format is not correct, check it and try again.");
        }
        string sAddr = oAddr.get_obj()[0].value_.get_str(); //"address" field
        string sHex = oAddr.get_obj()[1].value_.get_str();  //"hex" field

        CMvAddress addr(sAddr);
        if (addr.IsNull())
        {
            throw CRPCException(RPC_WALLET_ERROR, "Data format is not correct, check it and try again.");
        }

        //import keys
        if(addr.IsPubKey())
        {
            vector<unsigned char> vchKey = ParseHexString(sHex);
            crypto::CKey key;
            if (!key.Load(vchKey))
            {
                throw CRPCException(RPC_INVALID_PARAMS, "Failed to verify serialized key");
            }
            if (key.GetVersion() == 0)
            {
                throw CRPCException(RPC_INVALID_PARAMS, "Can't import the key with empty passphrase");
            }
            if (pService->HaveKey(key.GetPubKey()))
            {
                continue;   //step to next one to continue importing
            }
            if (!pService->AddKey(key))
            {
                throw CRPCException(RPC_WALLET_ERROR, "Failed to add key");
            }
            if (!pService->SynchronizeWalletTx(CDestination(key.GetPubKey())))
            {
                throw CRPCException(RPC_WALLET_ERROR, "Failed to sync wallet tx");
            }
            aAddr.push_back(key.GetPubKey().GetHex());
            ++nKey;
        }

        //import templates
        if(addr.IsTemplate())
        {
            vector<unsigned char> vchTemplate = ParseHexString(sHex);
            CTemplatePtr ptr = CTemplateGeneric::CreateTemplatePtr(vchTemplate);
            if (ptr == NULL || ptr->IsNull())
            {
                throw CRPCException(RPC_INVALID_PARAMETER,"Invalid parameters,failed to make template");
            }
            if (pService->HaveTemplate(addr.GetTemplateId()))
            {
                continue;   //step to next one to continue importing
            }
            if (!pService->AddTemplate(ptr))
            {
                throw CRPCException(RPC_WALLET_ERROR,"Failed to add template");
            }
            if (!pService->SynchronizeWalletTx(CDestination(ptr->GetTemplateId())))
            {
                throw CRPCException(RPC_WALLET_ERROR,"Failed to sync wallet tx");
            }
            aAddr.push_back(CMvAddress(ptr->GetTemplateId()).ToString());
            ++nTemp;
        }
    }

    return MakeCImportWalletResultPtr(string("Imported ") + std::to_string(nKey)
                + string(" keys and ") + std::to_string(nTemp) + string(" templates."));
}

CRPCResultPtr CRPCMod::RPCMakeOrigin(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CMakeOriginParam>(param);
    
    //makeorigin <"prev"> <"owner"> <$amount$> <"name"> <"symbol"> <$reward$> (-i|-noi*isolated*) (-p|-nop*private*) (-e|-noe*enclosed*)
    uint256 hashPrev;
    hashPrev.SetHex(spParam->strPrev);

    CDestination destOwner = static_cast<CDestination>(CMvAddress(spParam->strOwner));
    if (destOwner.IsNull())
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Invalid owner");
    }
	
    int64 nAmount = AmountFromValue(spParam->fAmount);
    int64 nMintReward = AmountFromValue(spParam->fReward);

    if (spParam->strName.empty() || spParam->strName.size() > 128
        || spParam->strSymbol.empty() || spParam->strSymbol.size() > 16)
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Invalid name or symbol");
    }
 
    CBlock blockPrev;
    uint256 hashParent;
    int nJointHeight;
    if (!pService->GetBlock(hashPrev,blockPrev,hashParent,nJointHeight))
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Unknown prev block");
    }

    if (blockPrev.IsExtended() || blockPrev.IsVacant())
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Prev block should not be extended/vacant block");
    }

    CProfile profile;
    profile.strName      = spParam->strName;
    profile.strSymbol    = spParam->strSymbol;
    profile.destOwner    = destOwner;
    profile.hashParent   = hashParent; 
    profile.nJointHeight = nJointHeight; 
    profile.nMintReward  = nMintReward;
    profile.nMinTxFee    = MIN_TX_FEE;
    profile.SetFlag(spParam->fIsolated,spParam->fPrivate,spParam->fEnclosed);

    CBlock block;
    block.nVersion   = 1;
    block.nType      = CBlock::BLOCK_ORIGIN;
    block.nTimeStamp = blockPrev.nTimeStamp + BLOCK_TARGET_SPACING;
    block.hashPrev   = hashPrev;
    profile.Save(block.vchProof);

    CTransaction& tx = block.txMint;
    tx.nType = CTransaction::TX_GENESIS;
    tx.sendTo  = destOwner;
    tx.nAmount = nAmount;
    tx.vchData.assign(profile.strName.begin(),profile.strName.end());

    crypto::CPubKey pubkey;
    if (!destOwner.GetPubKey(pubkey))
    {
        throw CRPCException(RPC_INVALID_ADDRESS_OR_KEY,"Owner' address should be pubkey address");
    }

    int nVersion;
    bool fLocked;
    int64 nAutoLockTime; 
    if (!pService->GetKeyStatus(pubkey,nVersion,fLocked,nAutoLockTime))
    {
        throw CRPCException(RPC_INVALID_ADDRESS_OR_KEY,"Unknown key");
    }
    if (fLocked)
    {
        throw CRPCException(RPC_WALLET_UNLOCK_NEEDED,"Key is locked");
    }

    uint256 hashBlock = block.GetHash();
    if (!pService->SignSignature(pubkey,hashBlock,block.vchSig))
    {
        throw CRPCException(RPC_WALLET_ERROR,"Failed to sign message");
    }
 
    CWalleveBufStream ss;
    ss << block;
    
    auto spResult = MakeCMakeOriginResultPtr();
    spResult->strHash = hashBlock.GetHex();
    spResult->strHex = ToHexString((const unsigned char*)ss.GetData(),ss.GetSize());

    return spResult;
}

/* Util */
CRPCResultPtr CRPCMod::RPCVerifyMessage(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CVerifyMessageParam>(param);

    //verifymessage <"pubkey"> <"message"> <"sig">
    crypto::CPubKey pubkey;
    if (pubkey.SetHex(spParam->strPubkey) != spParam->strPubkey.size())
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Invalid pubkey");
    }

    string strMessage = spParam->strMessage;

    if (spParam->strSig.empty())
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Invalid sig");
    }
    vector<unsigned char> vchSig = ParseHexString(spParam->strSig);
    if (vchSig.size() == 0)
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Invalid sig");
    }

    const string strMessageMagic = "Multiverse Signed Message:\n";
    CWalleveBufStream ss;
    ss << strMessageMagic;
    ss << strMessage;
    
    return MakeCVerifyMessageResultPtr(
        pubkey.Verify(crypto::CryptoHash(ss.GetData(),ss.GetSize()),vchSig));
}

CRPCResultPtr CRPCMod::RPCMakeKeyPair(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CMakeKeyPairParam>(param);

    crypto::CCryptoKey key;
    crypto::CryptoMakeNewKey(key);
    
    auto spResult = MakeCMakeKeyPairResultPtr();
    spResult->strPrivkey = key.secret.GetHex();
    spResult->strPubkey = key.pubkey.GetHex();
    return spResult;
}

CRPCResultPtr CRPCMod::RPCGetPubKeyAddress(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CGetPubkeyAddressParam>(param);
    crypto::CPubKey pubkey;
    if (pubkey.SetHex(spParam->strPubkey) != spParam->strPubkey.size())
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Invalid pubkey");
    }

    CDestination dest(pubkey);

    return MakeCGetPubkeyAddressResultPtr(CMvAddress(dest).ToString());
}

CRPCResultPtr CRPCMod::RPCGetTemplateAddress(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CGetTemplateAddressParam>(param);
    CTemplateId tid;
    if (tid.SetHex(spParam->strTid) != spParam->strTid.size())
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Invalid tid");
    }

    CDestination dest(tid);

    return MakeCGetTemplateAddressResultPtr(CMvAddress(dest).ToString());
}

CRPCResultPtr CRPCMod::RPCMakeTemplate(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CMakeTemplateParam>(param);
    CTemplatePtr ptr = MakeTemplate(spParam->data);
    if (ptr == NULL || ptr->IsNull())
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Invalid parameters,failed to make template");
    }
    vector<unsigned char> vchTemplate;
    ptr->Export(vchTemplate);

    auto spResult = MakeCMakeTemplateResultPtr();
    spResult->strHex = ToHexString(vchTemplate);
    spResult->strAddress = CMvAddress(ptr->GetTemplateId()).ToString();
    return spResult;
}

CRPCResultPtr CRPCMod::RPCDecodeTransaction(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CDecodeTransactionParam>(param);
    vector<unsigned char> txData(ParseHexString(spParam->strTxdata));
    CWalleveBufStream ss;
    ss.Write((char *)&txData[0],txData.size());
    CTransaction rawTx;
    try
    {
        ss >> rawTx;
    }
    catch (const std::exception &e)
    {
        throw CRPCException(RPC_DESERIALIZATION_ERROR, "TX decode failed");
    }
    
    uint256 hashFork;
    int nHeight;
    if (!pService->GetBlockLocation(rawTx.hashAnchor,hashFork,nHeight))
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Unknown anchor block");
    }

    return MakeCDecodeTransactionResultPtr(TxToJSON(rawTx.GetHash(),rawTx,hashFork,-1));
}


// /* Mint */
CRPCResultPtr CRPCMod::RPCGetWork(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CGetWorkParam>(param);

    //getwork ("prev")
    uint256 hashPrev;
    if (!pService->GetBlockHash(pCoreProtocol->GetGenesisBlockHash(),-1,hashPrev))
    {
        throw CRPCException(RPC_INTERNAL_ERROR, "The primary chain is invalid.");
    }

    uint256 inPrev;
    if (inPrev.SetHex(spParam->strPrev) != spParam->strPrev.size())
    {
        throw CRPCException(RPC_INTERNAL_ERROR, "Invalid prev.");
    }

    auto spResult = MakeCGetWorkResultPtr();
    if (hashPrev == uint256(spParam->strPrev))
    {
        spResult->fResult = true;
        return spResult;
    }

    vector<unsigned char> vchWorkData;
    uint32 nPrevTime;
    int nAlgo,nBits;
    if (!pService->GetWork(vchWorkData,hashPrev,nPrevTime,nAlgo,nBits))
    {
        spResult->fResult = false;
        return spResult;
    }

    spResult->work.strPrevblockhash = hashPrev.GetHex();
    spResult->work.nPrevblocktime = nPrevTime;
    spResult->work.nAlgo = nAlgo;
    spResult->work.nBits = nBits;
    spResult->work.strData = ToHexString(vchWorkData);

    return spResult;
}

CRPCResultPtr CRPCMod::RPCSubmitWork(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CSubmitWorkParam>(param);
    vector<unsigned char> vchWorkData(ParseHexString(spParam->strData));
    CMvAddress addrSpent(spParam->strSpent);
    uint256 nPriv(spParam->strPrivkey);
    if (addrSpent.IsNull() || !addrSpent.IsPubKey())
    {
        throw CRPCException(RPC_INVALID_ADDRESS_OR_KEY,"Invalid spent address");
    }
    crypto::CKey key;
    if (!key.SetSecret(crypto::CCryptoKeyData(nPriv.begin(),nPriv.end())))
    {
        throw CRPCException(RPC_INVALID_ADDRESS_OR_KEY,"Invalid private key");
    }
    
    CTemplatePtr ptr = CTemplatePtr(new CTemplateMint(key.GetPubKey(),static_cast<CDestination&>(addrSpent)));
    if (ptr == NULL)
    {
        throw CRPCException(RPC_INVALID_ADDRESS_OR_KEY,"Invalid mint template");
    }
    uint256 hashBlock;
    MvErr err = pService->SubmitWork(vchWorkData,ptr,key,hashBlock);
    if (err != MV_OK)
    {
        throw CRPCException(RPC_INVALID_PARAMETER,string("Block rejected : ") + MvErrString(err));
    }

    return MakeCSubmitWorkResultPtr(hashBlock.GetHex());
}

CRPCResultPtr CRPCMod::SnRPCStop(CRPCParamPtr param)
{
    (void)param;
    return NULL;
}

CRPCResultPtr CRPCMod::SnRPCGetForkCount(CRPCParamPtr param)
{
    (void)param;
    return NULL;
}

CRPCResultPtr CRPCMod::SnRPCListFork(CRPCParamPtr param)
{
    (void)param;
    return NULL;
}

CRPCResultPtr CRPCMod::SnRPCGetBlockLocation(CRPCParamPtr param)
{
    (void)param;
    return NULL;
}

CRPCResultPtr CRPCMod::SnRPCGetBlockCount(CRPCParamPtr param)
{
    (void)param;
    return NULL;
}

CRPCResultPtr CRPCMod::SnRPCGetBlockHash(CRPCParamPtr param)
{
    (void)param;
    return NULL;
}

CRPCResultPtr CRPCMod::SnRPCGetBlock(CRPCParamPtr param)
{
    (void)param;
    return NULL;
}

CRPCResultPtr CRPCMod::SnRPCGetTxPool(CRPCParamPtr param)
{
    (void)param;
    return NULL;
}

CRPCResultPtr CRPCMod::SnRPCGetTransaction(CRPCParamPtr param)
{
    (void)param;
    return NULL;
}

CRPCResultPtr CRPCMod::SnRPCGetForkHeight(CRPCParamPtr param)
{
    (void)param;
    return NULL;
}

CRPCResultPtr CRPCMod::SnRPCSendTransaction(CRPCParamPtr param)
{
    (void)param;
    return NULL;
}

CSnRPCMod::CSnRPCMod()
{
    mapRPCFunc["stop"] = &CRPCMod::SnRPCStop;
    mapRPCFunc["getforkcount"] = &CRPCMod::SnRPCGetForkCount;
    mapRPCFunc["listfork"] = &CRPCMod::SnRPCListFork;
    mapRPCFunc["getblocklocation"] = &CRPCMod::SnRPCGetBlockLocation;
    mapRPCFunc["getblockcount"] = &CRPCMod::SnRPCGetBlockCount;
    mapRPCFunc["getblockhash"] = &CRPCMod::SnRPCGetBlockHash;
    mapRPCFunc["getblock"] = &CRPCMod::SnRPCGetBlock;
    mapRPCFunc["gettxpool"] = &CRPCMod::SnRPCGetTxPool;
    mapRPCFunc["gettransaction"] = &CRPCMod::SnRPCGetTransaction;
    mapRPCFunc["getforkheight"] = &CRPCMod::SnRPCGetForkHeight;
    mapRPCFunc["sendtransaction"] = &CRPCMod::SnRPCSendTransaction;
}

CSnRPCMod::~CSnRPCMod()
{
}

bool CSnRPCMod::WalleveHandleInitialize()
{
    CRPCMod::WalleveHandleInitialize();

    if (!WalleveGetObject("dbpservice", pDbpService))
    {
        WalleveLog("Failed to request DBP service\n");
        return false;
    }

    return true;
}

void CSnRPCMod::WalleveHandleDeinitialize()
{
    CRPCMod::WalleveHandleDeinitialize();
    pDbpService = NULL;
}

uint64 CSnRPCMod::GenNonce()
{
    uint64 nNonce;
    RAND_bytes((unsigned char*)&nNonce, sizeof(nNonce));
    while(nNonce <= 0xFF || nNonce == std::numeric_limits<uint64>::max())
    {
        RAND_bytes((unsigned char*)&nNonce, sizeof(nNonce));
    }
    return nNonce;
}

CRPCResultPtr CSnRPCMod::SnRPCStop(CRPCParamPtr param)
{
    walleve::CIOCompletionUntil ioCompltUntil(200000);
    CMvEventRPCRouteStop* pEvent = new CMvEventRPCRouteStop("");
    pEvent->data.pIoCompltUntil = &ioCompltUntil;
    pEvent->data.nNonce = GenNonce();
    if(!pEvent)
    {
        return NULL;
    }
    ioCompltUntil.Reset();
    pDbpService->PostEvent(pEvent);

    bool fResult = false;
    ioCompltUntil.WaitForComplete(fResult);
    if(!fResult)
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Timeout");
    }

    std::string reason = "[supernode]multiverse server stopping";
    return MakeCStopResultPtr(reason);
}

CRPCResultPtr CSnRPCMod::SnRPCGetForkCount(CRPCParamPtr param)
{
    walleve::CIOCompletionUntil ioCompltUntil(200000);
    CMvEventRPCRouteGetForkCount* pEvent = new CMvEventRPCRouteGetForkCount("");
    pEvent->data.pIoCompltUntil = &ioCompltUntil;
    pEvent->data.nNonce = GenNonce();
    if(!pEvent)
    {
        return NULL;
    }
    ioCompltUntil.Reset();
    pDbpService->PostEvent(pEvent);

    bool fResult = false;
    ioCompltUntil.WaitForComplete(fResult);
    if(!fResult)
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Timeout");
    }

    CMvRPCRouteGetForkCountRet ret = boost::any_cast<CMvRPCRouteGetForkCountRet>(ioCompltUntil.obj);
    return MakeCGetForkCountResultPtr(ret.count);
}

CRPCResultPtr CSnRPCMod::SnRPCListFork(CRPCParamPtr param)
{
    walleve::CIOCompletionUntil ioCompltUntil(200000);
    auto spParam = CastParamPtr<CListForkParam>(param);
    CMvEventRPCRouteListFork* pEvent = new CMvEventRPCRouteListFork("");
    pEvent->data.pIoCompltUntil = &ioCompltUntil;
    pEvent->data.nNonce = GenNonce();
    pEvent->data.fAll = spParam->fAll;
    if(!pEvent)
    {
        return NULL;
    }
    ioCompltUntil.Reset();
    pDbpService->PostEvent(pEvent);

    bool fResult = false;
    ioCompltUntil.WaitForComplete(fResult);
    if(!fResult)
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Timeout");
    }

    CMvRPCRouteListForkRet ret = boost::any_cast<CMvRPCRouteListForkRet>(ioCompltUntil.obj);
    auto vFork = ret.vFork;
    auto spResult = MakeCListForkResultPtr();
    for (size_t i = 0; i < vFork.size(); i++)
    {
        CMvRPCProfile& profile = vFork[i];
        spResult->vecProfile.push_back({ profile.strHex, profile.strName,
                                         profile.strSymbol, profile.fIsolated,
                                         profile.fPrivate, profile.fEnclosed,
                                         profile.address });
    }
    return spResult;
}

CRPCResultPtr CSnRPCMod::SnRPCGetBlockLocation(CRPCParamPtr param)
{
    walleve::CIOCompletionUntil ioCompltUntil(200000);
    auto spParam = CastParamPtr<CGetBlockLocationParam>(param);
    CMvEventRPCRouteGetBlockLocation* pEvent = new CMvEventRPCRouteGetBlockLocation("");
    pEvent->data.pIoCompltUntil = &ioCompltUntil;
    pEvent->data.nNonce = GenNonce();
    pEvent->data.strBlock = spParam->strBlock;
    if(!pEvent)
    {
        return NULL;
    }
    ioCompltUntil.Reset();
    pDbpService->PostEvent(pEvent);

    bool fResult = false;
    ioCompltUntil.WaitForComplete(fResult);
    if(!fResult)
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Timeout");
    }

    CMvRPCRouteGetBlockLocationRet ret = boost::any_cast<CMvRPCRouteGetBlockLocationRet>(ioCompltUntil.obj);
    if (ret.strFork.empty())
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Unknown block");
    }

    auto spResult = MakeCGetBlockLocationResultPtr();
    spResult->strFork = ret.strFork;
    spResult->nHeight = ret.height;
    return spResult;
}

CRPCResultPtr CSnRPCMod::SnRPCGetBlockCount(CRPCParamPtr param)
{
    walleve::CIOCompletionUntil ioCompltUntil(200000);
    auto spParam = CastParamPtr<CGetBlockCountParam>(param);
    auto* pEvent = new CMvEventRPCRouteGetBlockCount("");
    pEvent->data.pIoCompltUntil = &ioCompltUntil;
    pEvent->data.nNonce = GenNonce();
    pEvent->data.strFork = spParam->strFork;
    if (!pEvent)
    {
        return NULL;
    }
    ioCompltUntil.Reset();
    pDbpService->PostEvent(pEvent);

    bool fResult = false;
    ioCompltUntil.WaitForComplete(fResult);
    if (!fResult)
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Timeout");
    }

    auto ret = boost::any_cast<CMvRPCRouteGetBlockCountRet>(ioCompltUntil.obj);
    uint256 hashFork;
    int height = 0;
    if (ret.exception == 0)
    {
        height = ret.height;
    }
    else if (ret.exception == 1)
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Invalid fork");
    }
    else if (ret.exception == 2)
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Unknown fork");
    }
    return MakeCGetBlockCountResultPtr(height);
}

CRPCResultPtr CSnRPCMod::SnRPCGetBlockHash(CRPCParamPtr param)
{
    walleve::CIOCompletionUntil ioCompltUntil(200000);
    auto spParam = CastParamPtr<CGetBlockHashParam>(param);
    auto* pEvent = new CMvEventRPCRouteGetBlockHash("");
    pEvent->data.pIoCompltUntil = &ioCompltUntil;
    pEvent->data.nNonce = GenNonce();
    pEvent->data.height = spParam->nHeight;
    pEvent->data.strFork = spParam->strFork;
    if(!pEvent)
    {
        return NULL;
    }
    ioCompltUntil.Reset();
    pDbpService->PostEvent(pEvent);

    bool fResult = false;
    ioCompltUntil.WaitForComplete(fResult);
    if(!fResult)
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Timeout");
    }

    auto ret = boost::any_cast<CMvRPCRouteGetBlockHashRet>(ioCompltUntil.obj);
    int height = 0;
    if (ret.exception == 0)
    {
    }
    else if (ret.exception == 1)
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Invalid fork");
    }
    else if (ret.exception == 2)
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Unknown fork");
    }
    else if (ret.exception == 3)
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Block number out of range.");
    }

    auto spResult = MakeCGetBlockHashResultPtr();
    for (const auto& hash: ret.vHash)
    {
        spResult->vecHash.push_back(hash);
    }
    return spResult;
}

CRPCResultPtr CSnRPCMod::SnRPCGetBlock(CRPCParamPtr param)
{
    walleve::CIOCompletionUntil ioCompltUntil(200000);
    auto spParam = CastParamPtr<CGetBlockParam>(param);
    auto* pEvent = new CMvEventRPCRouteGetBlock("");
    pEvent->data.pIoCompltUntil = &ioCompltUntil;
    pEvent->data.nNonce = GenNonce();
    pEvent->data.hash = spParam->strBlock;
    if (!pEvent)
    {
        return NULL;
    }
    ioCompltUntil.Reset();
    pDbpService->PostEvent(pEvent);

    bool fResult = false;
    ioCompltUntil.WaitForComplete(fResult);
    if(!fResult)
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Timeout");
    }

    auto ret = boost::any_cast<CMvRPCRouteGetBlockRet>(ioCompltUntil.obj);
    if (ret.exception == 1)
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Unknown block");
    }

    uint256 fork;
    fork.SetHex(ret.strFork);
    return MakeCGetBlockResultPtr(BlockToJSON(ret.block.GetHash(), ret.block, fork, ret.height));
}

CRPCResultPtr CSnRPCMod::SnRPCGetTxPool(CRPCParamPtr param)
{
    walleve::CIOCompletionUntil ioCompltUntil(200000);
    auto spParam = CastParamPtr<CGetTxPoolParam>(param);
    bool fDetail = spParam->fDetail.IsValid() ? bool(spParam->fDetail) : false;
    auto* pEvent = new CMvEventRPCRouteGetTxPool("");
    pEvent->data.pIoCompltUntil = &ioCompltUntil;
    pEvent->data.nNonce = GenNonce();
    pEvent->data.strFork = spParam->strFork;
    pEvent->data.fDetail = fDetail;
    if (!pEvent)
    {
        return NULL;
    }
    ioCompltUntil.Reset();
    pDbpService->PostEvent(pEvent);

    bool fResult = false;
    ioCompltUntil.WaitForComplete(fResult);
    if(!fResult)
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Timeout");
    }

    auto spResult = MakeCGetTxPoolResultPtr();
    auto ret = boost::any_cast<CMvRPCRouteGetTxPoolRet>(ioCompltUntil.obj);
    if (ret.exception == 0)
    {
    }
    else if (ret.exception == 1)
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Invalid fork");
    }
    else if (ret.exception == 2)
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Unknown fork");
    }

    if (!fDetail)
    {
        size_t nTotalSize = 0;
        for (std::size_t i = 0; i < ret.vTxPool.size(); i++)
        {
            nTotalSize += ret.vTxPool[i].second;
        }
        spResult->nCount = ret.vTxPool.size();
        spResult->nSize = nTotalSize;
    }
    else
    {
        for (std::size_t i = 0; i < ret.vTxPool.size(); i++)
        {
            spResult->vecList.push_back({ret.vTxPool[i].first, ret.vTxPool[i].second});
        }
    }
    return spResult;
}

CRPCResultPtr CSnRPCMod::SnRPCGetTransaction(CRPCParamPtr param)
{
    walleve::CIOCompletionUntil ioCompltUntil(200000);
    auto spParam = CastParamPtr<CGetTransactionParam>(param);
    auto* pEvent = new CMvEventRPCRouteGetTransaction("");
    pEvent->data.pIoCompltUntil = &ioCompltUntil;
    pEvent->data.nNonce = GenNonce();
    pEvent->data.strTxid = spParam->strTxid;
    if (!pEvent)
    {
        return NULL;
    }
    ioCompltUntil.Reset();
    pDbpService->PostEvent(pEvent);

    bool fResult = false;
    ioCompltUntil.WaitForComplete(fResult);
    if(!fResult)
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Timeout");
    }

    auto spResult = MakeCGetTransactionResultPtr();
    auto ret = boost::any_cast<CMvRPCRouteGetTransactionRet>(ioCompltUntil.obj);
    if (ret.exception == 0)
    {
    }
    else if (ret.exception == 1)
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "No information available about transaction");
    }

    if (spParam->fSerialized)
    {
        CWalleveBufStream ss;
        ss << ret.tx;
        spResult->strSerialization = ToHexString((const unsigned char*)ss.GetData(),ss.GetSize());
        return spResult;
    }

    uint256 txid, hashFork;
    txid.SetHex(spParam->strTxid);
    spResult->transaction = TxToJSON(txid, ret.tx, hashFork.SetHex(ret.strFork), ret.nDepth);
    return spResult;
}

CRPCResultPtr CSnRPCMod::SnRPCGetForkHeight(CRPCParamPtr param)
{
    walleve::CIOCompletionUntil ioCompltUntil(200000);
    auto spParam = CastParamPtr<CGetForkHeightParam>(param);
    auto* pEvent = new CMvEventRPCRouteGetForkHeight("");
    pEvent->data.pIoCompltUntil = &ioCompltUntil;
    pEvent->data.nNonce = GenNonce();
    pEvent->data.strFork = spParam->strFork;
    if (!pEvent)
    {
        return NULL;
    }
    ioCompltUntil.Reset();
    pDbpService->PostEvent(pEvent);

    bool fResult = false;
    ioCompltUntil.WaitForComplete(fResult);
    if(!fResult)
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Timeout");
    }

    auto ret = boost::any_cast<CMvRPCRouteGetForkHeightRet>(ioCompltUntil.obj);
    if (ret.exception == 0)
    {
    }
    else if (ret.exception == 1)
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Invalid fork");
    }
    else if (ret.exception == 2)
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Unknown fork");
    }
    return MakeCGetForkHeightResultPtr(ret.height);
}

CRPCResultPtr CSnRPCMod::SnRPCSendTransaction(CRPCParamPtr param)
{
    walleve::CIOCompletionUntil ioCompltUntil(200000);
    auto spParam = CastParamPtr<CSendTransactionParam>(param);
    vector<unsigned char> txData = ParseHexString(spParam->strTxdata);
    CWalleveBufStream ss;
    ss.Write((char *)&txData[0],txData.size());
    CTransaction rawTx;
    try
    {
        ss >> rawTx;
    }
    catch (const std::exception &e)
    {
        throw CRPCException(RPC_DESERIALIZATION_ERROR, "TX decode failed");
    }

    auto* pEvent = new CMvEventRPCRouteSendTransaction("");
    pEvent->data.pIoCompltUntil = &ioCompltUntil;
    pEvent->data.nNonce = GenNonce();
    pEvent->data.rawTx = rawTx;
    if (!pEvent)
    {
        return NULL;
    }
    ioCompltUntil.Reset();
    pDbpService->PostEvent(pEvent);

    bool fResult = false;
    ioCompltUntil.WaitForComplete(fResult);
    if(!fResult)
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Timeout");
    }

    auto ret = boost::any_cast<CMvRPCRouteSendTransactionRet>(ioCompltUntil.obj);
    if (ret.exception == 0)
    {
    }
    else if (ret.exception == 1)
    {
        throw CRPCException(RPC_TRANSACTION_REJECTED,string("Tx rejected : ") + MvErrString((MvErr)ret.err));
    }
    return MakeCSendTransactionResultPtr(rawTx.GetHash().GetHex());
}
