// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "rpcmod.h"

#include <boost/assign/list_of.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

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
                ("removependingtx",       &CRPCMod::RPCRemovePendingTx)
                ("gettransaction",        &CRPCMod::RPCGetTransaction)
                ("sendtransaction",       &CRPCMod::RPCSendTransaction)
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
                /* Util */
                ("verifymessage",         &CRPCMod::RPCVerifyMessage)
                ("makekeypair",           &CRPCMod::RPCMakeKeyPair)
                ("getpubkeyaddress",      &CRPCMod::RPCGetPubKeyAddress)
                ("gettemplateaddress",    &CRPCMod::RPCGetTemplateAddress)
                ("maketemplate",          &CRPCMod::RPCMakeTemplate)
                ("decodetransaction",     &CRPCMod::RPCDecodeTransaction)
                ("makeorigin",            &CRPCMod::RPCMakeOrigin)
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
        WalleveLog("Failed to request httpserver\n");
        return false;
    }

    if (!WalleveGetObject("coreprotocol",pCoreProtocol))
    {
        WalleveLog("Failed to request coreprotocol\n");
        return false;
    }
    
    if (!WalleveGetObject("service",pService))
    {
        WalleveLog("Failed to request service\n");
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
    WalleveLog("request : %s\n", eventHttpReq.data.strContent.c_str());
    uint64 nNonce = eventHttpReq.nNonce;

    string strResult;
    try
    {
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
    }

    WalleveLog("response : %s\n", strResult.c_str());

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
        peer.fBanscore = info.nScore;
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
    vector<pair<uint256,CProfile> > vFork;
    pService->ListFork(vFork);

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

    uint256 fork = GetForkHash(spParam->strFork);
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
    for (std::size_t i = 0;i > vSubline.size();i++)
    {
        spResult->vecSubline.push_back({vSubline[i].second.GetHex(), vSubline[i].first});
    }
    return spResult;
}

CRPCResultPtr CRPCMod::RPCGetBlockLocation(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CGetBlockLocationParam>(param);

    uint256 hash(spParam->strBlock);
    uint256 fork;
    int height;
    if (!pService->GetBlockLocation(hash,fork,height))
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
    uint256 fork = GetForkHash(spParam->strFork);
    return MakeCGetBlockCountResultPtr(pService->GetBlockCount(fork));
}

CRPCResultPtr CRPCMod::RPCGetBlockHash(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CGetBlockHashParam>(param);

    int nHeight = spParam->nHeight;
    uint256 fork = GetForkHash(spParam->strFork);
    uint256 hash = 0;
    if (!pService->GetBlockHash(fork,nHeight,hash))
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Block number out of range.");
    }

    return MakeCGetBlockHashResultPtr(hash.GetHex());
}

CRPCResultPtr CRPCMod::RPCGetBlock(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CGetBlockParam>(param);
    
    uint256 hash(spParam->strBlock);
    CBlock block;
    uint256 fork;
    int height;
    if (!pService->GetBlock(hash,block,fork,height))
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Unknown block");
    }

    return MakeCGetBlockResultPtr(BlockToJSON(hash,block,fork,height));
}

CRPCResultPtr CRPCMod::RPCGetTxPool(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CGetTxPoolParam>(param);

    uint256 fork = GetForkHash(spParam->strFork);
    bool fDetail = spParam->fDetail.IsValid() ? bool(spParam->fDetail) : false;
    
    vector<pair<uint256,size_t> > vTxPool;
    pService->GetTxPool(fork,vTxPool);

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

CRPCResultPtr CRPCMod::RPCRemovePendingTx(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CRemovePendingTxParam>(param);

    uint256 txid;
    txid.SetHex(spParam->strTxid);
    if (!pService->RemovePendingTx(txid))
    {
        throw CRPCException(RPC_INVALID_REQUEST, "This transaction is not in tx pool");
    }

    return MakeCRemovePendingTxResultPtr(string("Remove tx successfully: ") + spParam->strTxid);
}

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
            ostringstream oss;
            oss << "ver=" << nVersion;
            if (fLocked)
            {
                oss << ";locked";
            }
            else
            {
                oss << ";unlocked";
                if (nAutoLockTime > 0)
                {
                    oss <<";timeout=" << (nAutoLockTime - GetTime());
                }
            }

            spResult->vecPubkey.push_back({pubkey.GetHex(), oss.str()});
        }
    } 
    return spResult;
}

CRPCResultPtr CRPCMod::RPCGetNewKey(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CGetNewKeyParam>(param);

    crypto::CCryptoString strPassphrase;
    if (spParam->strPassphrase.IsValid())
    {
        strPassphrase = spParam->strPassphrase.c_str();
    }
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

    crypto::CPubKey pubkey;
    pubkey.SetHex(spParam->strPubkey);
    crypto::CCryptoString strPassphrase,strOldPassphrase;
    strPassphrase = spParam->strPassphrase.c_str();
    if (spParam->strOldpassphrase.IsValid())
    {
        strOldPassphrase = spParam->strOldpassphrase.c_str();
    }
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

    crypto::CCryptoString strPassphrase;
    strPassphrase = spParam->strPassphrase.c_str();
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

    uint256 nPriv(spParam->strPrivkey);
    crypto::CCryptoString strPassphrase;
    if (spParam->strPassphrase.IsValid())
    {
        strPassphrase = spParam->strPassphrase.c_str();
    }

    crypto::CKey key;
    if (!key.SetSecret(crypto::CCryptoKeyData(nPriv.begin(),nPriv.end())))
    {
        throw CRPCException(RPC_INVALID_ADDRESS_OR_KEY,"Invalid private key");
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
    CTemplateId tid;
    if (!address.GetTemplateId(tid))
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Invalid address, should be template address");
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
    uint256 hashFork = GetForkHash(spParam->strFork);
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

    CMvAddress from(spParam->strFrom);
    CMvAddress to(spParam->strTo);
    if (from.IsNull() || to.IsNull())
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Invalid address");
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
    uint256 hashFork = GetForkHash(spParam->strFork);

    CTransaction txNew;
    if (!pService->CreateTransaction(hashFork,from,to,nAmount,nTxFee,vector<unsigned char>(),txNew))
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

    CMvAddress from(spParam->strFrom);
    CMvAddress to(spParam->strTo);
    if (from.IsNull() || to.IsNull())
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Invalid address");
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
    uint256 hashFork = GetForkHash(spParam->strFork);
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
        if(des.IsPubKey())
        {
            addressData.strType = "pubkey";
            addressData.pubkey.strKey = des.GetHex();
            addressData.pubkey.strAddress = CMvAddress(des).ToString();
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

    fs::path pSave(spParam->strPath);
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
        write_stream(Value(aAddr), ofs, pretty_print);
        ofs.close();
    }
    catch(const fs::filesystem_error& e)
    {
        throw CRPCException(RPC_WALLET_ERROR, "filesystem_error");
    }

    return MakeCExportWalletResultPtr(string("Wallet file has been saved at: ") + pSave.string());
}

CRPCResultPtr CRPCMod::RPCImportWallet(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CImportWalletParam>(param);

    fs::path pLoad(spParam->strPath);
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
        read_stream(ifs, vWallet);
        ifs.close();
    }
    catch(const fs::filesystem_error& e)
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
        //import keys
        if(addr.IsPubKey())
        {
            vector<unsigned char> vchKey = ParseHexString(sHex);
            crypto::CKey key;
            if (!key.Load(vchKey))
            {
                throw CRPCException(RPC_INVALID_PARAMS, "Failed to verify serialized key");
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

/* Util */
CRPCResultPtr CRPCMod::RPCVerifyMessage(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CVerifyMessageParam>(param);

    crypto::CPubKey pubkey;
    pubkey.SetHex(spParam->strPubkey);
    string strMessage = spParam->strMessage;
    vector<unsigned char> vchSig = ParseHexString(spParam->strSig);
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
    pubkey.SetHex(spParam->strPubkey);
    CDestination dest(pubkey);

    return MakeCGetPubkeyAddressResultPtr(CMvAddress(dest).ToString());
}

CRPCResultPtr CRPCMod::RPCGetTemplateAddress(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CGetTemplateAddressParam>(param);
    CTemplateId tid;
    tid.SetHex(spParam->strTid);
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

CRPCResultPtr CRPCMod::RPCMakeOrigin(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CMakeOriginParam>(param);
    
    uint256 hashPrev(spParam->strPrev);
    CMvAddress address(spParam->strAddress);
    int64 nAmount = AmountFromValue(spParam->fAmount);
    string strIdent = spParam->strIdent;

    CBlock blockPrev;
    uint256 fork;
    int height;
    if (!pService->GetBlock(hashPrev,blockPrev,fork,height))
    {
        throw CRPCException(RPC_INVALID_PARAMETER, "Unknown prev block");
    }

    CBlock block;
    block.nVersion   = 1;
    block.nType      = CBlock::BLOCK_ORIGIN;
    block.nTimeStamp = blockPrev.nTimeStamp + BLOCK_TARGET_SPACING;
    block.hashPrev   = hashPrev;
    block.vchProof = vector<uint8>(strIdent.begin(),strIdent.end());

    CTransaction& tx = block.txMint;
    tx.nType = CTransaction::TX_GENESIS;
    tx.sendTo  = static_cast<CDestination>(address);
    tx.nAmount = nAmount;

    CWalleveBufStream ss;
    ss << block;
    
    auto spResult = MakeCMakeOriginResultPtr();
    spResult->strHash = block.GetHash().GetHex();
    spResult->strHex = ToHexString((const unsigned char*)ss.GetData(),ss.GetSize());
    return spResult;
}


// /* Mint */
CRPCResultPtr CRPCMod::RPCGetWork(CRPCParamPtr param)
{
    auto spParam = CastParamPtr<CGetWorkParam>(param);
    uint256 hashPrev;
    if (!pService->GetBlockHash(pCoreProtocol->GetGenesisBlockHash(),-1,hashPrev))
    {
        throw CRPCException(RPC_INTERNAL_ERROR, "The primary chain is invalid.");
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
    if (addrSpent.IsNull())
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
