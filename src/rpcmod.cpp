// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "rpcmod.h"
#include <boost/assign/list_of.hpp>
using namespace std;
using namespace multiverse;
using namespace walleve;
using namespace json_spirit;

///////////////////////////////
// CRPCMod

CRPCMod::CRPCMod()
: IIOModule("rpcmod")
{
    pHttpServer = NULL;
    pCoreProtocol = NULL;
    pService = NULL;
    mapRPCFunc = boost::assign::map_list_of
                 ("help",                  &CRPCMod::RPCHelp)
                 ("stop",                  &CRPCMod::RPCStop)
                 ("getpeercount",          &CRPCMod::RPCGetPeerCount)
                 ("listpeerinfo",          &CRPCMod::RPCListPeer)
                 ("addnode",               &CRPCMod::RPCAddNode)
                 ("getforkcount",          &CRPCMod::RPCGetForkCount)
                 ("getgenealogy",          &CRPCMod::RPCGetForkGenealogy)
                 ("getlocation",           &CRPCMod::RPCGetBlockLocation)
                 ("getblockcount",         &CRPCMod::RPCGetBlockCount)
                 ("getblockhash",          &CRPCMod::RPCGetBlockHash)
                 ("getblock",              &CRPCMod::RPCGetBlock)
                 ("gettxpool",             &CRPCMod::RPCGetTxPool)
                 ("gettransaction",        &CRPCMod::RPCGetTransaction)
                 ("listkey",               &CRPCMod::RPCListKey)
                 ("getnewkey",             &CRPCMod::RPCGetNewKey)
                 ("encryptkey",            &CRPCMod::RPCEncryptKey)
                 ("lockkey",               &CRPCMod::RPCLockKey)
                 ("unlockkey",             &CRPCMod::RPCUnlockKey)
                 ("importprivkey",         &CRPCMod::RPCImportPrivKey)
                 ("importkey",             &CRPCMod::RPCImportKey)
                 ("exportkey",             &CRPCMod::RPCExportKey)
                 ("signmessage",           &CRPCMod::RPCSignMessage)
                 ("verifymessage",         &CRPCMod::RPCVerifyMessage)
                 ("makekeypair",           &CRPCMod::RPCMakeKeyPair)
#if 0
                 ("notify",                &CRPCMod::RPCNotify)
                 ("makekeypair",           &CRPCMod::RPCMakeKeyPair)
                 ("getblockcount",         &CRPCMod::RPCGetBlockCount)
                 ("getblockhash",          &CRPCMod::RPCGetBlockHash)
                 ("getblock",              &CRPCMod::RPCGetBlock)
                 ("getmininginfo",         &CRPCMod::RPCGetMiningInfo)
                 ("getrawmempool",         &CRPCMod::RPCGetRawMempool)
                 ("getrawtransaction",     &CRPCMod::RPCGetRawTransaction)
                 ("addnewwallet",          &CRPCMod::RPCAddNewWallet)
                 ("listwallet",            &CRPCMod::RPCListWallets)
                 ("addmultisigaddress",    &CRPCMod::RPCAddMultiSigAddress)
                 ("addcoldmintingaddress", &CRPCMod::RPCAddColdMintingAddress)
                 ("settxfee",              &CRPCMod::RPCSetTxFee)
                 ("encryptwallet",         &CRPCMod::RPCEncryptWallet)
                 ("walletlock",            &CRPCMod::RPCWalletLock)
                 ("walletpassphrase",      &CRPCMod::RPCWalletPassphrase)
                 ("walletpassphrasechange",&CRPCMod::RPCWalletPassphraseChange)
                 ("walletexport",          &CRPCMod::RPCWalletExport)
                 ("walletimport",          &CRPCMod::RPCWalletImport)
                 ("getinfo",               &CRPCMod::RPCGetInfo)
                 ("getaddressinfo",        &CRPCMod::RPCGetAddressInfo)
                 ("setaddressbook",        &CRPCMod::RPCSetAddressBook)
                 ("getaddressbook",        &CRPCMod::RPCGetAddressBook)
                 ("deladdressbook",        &CRPCMod::RPCDelAddressBook)
                 ("listaddressbook",       &CRPCMod::RPCListAddressBook)
                 ("listaccounts",          &CRPCMod::RPCListAccounts)
                 ("getnewaddress",         &CRPCMod::RPCGetNewAddress)
                 ("getaccountaddress",     &CRPCMod::RPCGetAccountAddress)
                 ("setaccount",            &CRPCMod::RPCSetAccount)
                 ("getaccount",            &CRPCMod::RPCGetAccount)
                 ("getaddressesbyaccount", &CRPCMod::RPCGetAddressesByAccount)
                 ("validateaddress",       &CRPCMod::RPCValidateAddress)
                 ("getbalance",            &CRPCMod::RPCGetBalance)
                 ("gettransaction",        &CRPCMod::RPCGetTransaction)
                 ("listtransactions",      &CRPCMod::RPCListTransactions)
                 ("listunspent",           &CRPCMod::RPCListUnspent)
                 ("listsinceblock",        &CRPCMod::RPCListSinceBlock)
                 ("listreceivedbyaddress", &CRPCMod::RPCListReceivedByAddress)
                 ("listreceivedbyaccount", &CRPCMod::RPCListReceivedByAccount)
                 ("getreceivedbyaddress",  &CRPCMod::RPCGetReceivedByAddress)
                 ("getreceivedbyaccount",  &CRPCMod::RPCGetReceivedByAccount)
                 ("sendtoaddress",         &CRPCMod::RPCSendToAddress)
                 ("sendfrom",              &CRPCMod::RPCSendFrom)
                 ("sendmany",              &CRPCMod::RPCSendMany)
                 ("createsendfromaddress", &CRPCMod::RPCCreateSendFromAddress)
                 ("importprivkey",         &CRPCMod::RPCImportPrivKey)
                 ("dumpprivkey",           &CRPCMod::RPCDumpPrivKey)
                 ("signmessage",           &CRPCMod::RPCSignMessage)
                 ("verifymessage",         &CRPCMod::RPCVerifyMessage)
                 ("createrawtransaction",  &CRPCMod::RPCCreateRawTransaction)
                 ("decoderawtransaction",  &CRPCMod::RPCDecodeRawTransaction)
                 ("signrawtransaction",    &CRPCMod::RPCSignRawTransaction)
                 ("sendrawtransaction",    &CRPCMod::RPCSendRawTransaction);
#endif
                 ;
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
    uint64 nNonce = eventHttpReq.nNonce;

    Value id = Value::null;
    try
    {
        // Parse request
        Value valRequest;
        if (!read_string(eventHttpReq.data.strContent, valRequest)
            || valRequest.type() != obj_type)
        {
            throw JSONRPCError(RPC_PARSE_ERROR, "Parse error");
        }
        const Object& request = valRequest.get_obj();

        // Parse id now so errors from here on will have the id
        id = find_value(request, "id");

        // Parse method
        Value valMethod = find_value(request, "method");
        if (valMethod.type() == null_type)
        {
            throw JSONRPCError(RPC_INVALID_REQUEST, "Missing method");
        }
        if (valMethod.type() != str_type)
        {
            throw JSONRPCError(RPC_INVALID_REQUEST, "Method must be a string");
        }
        string strMethod = valMethod.get_str();
        WalleveLog("RPC command : %s\n", strMethod.c_str());

        // Parse params
        Value valParams = find_value(request, "params");
        Array params;
        if (valParams.type() == array_type)
        {
            params = valParams.get_array();
        }
        else if (valParams.type() == null_type)
        {
            params = Array();
        }
        else
        {
            throw JSONRPCError(RPC_INVALID_REQUEST, "Params must be an array");
        }

        map<string,RPCFunc>::iterator it = mapRPCFunc.find(strMethod);
        if (it == mapRPCFunc.end())
        {
            throw JSONRPCError(RPC_METHOD_NOT_FOUND, "Method not found");
        }
        
        try
        {
            JsonReply(nNonce,(this->*(*it).second)(params,false),id);
        }
        catch (exception& e)
        {
            throw JSONRPCError(RPC_MISC_ERROR, e.what());
        }
    }
    catch (Object& objError)
    {
        JsonError(nNonce, objError, id);
    }
    catch (exception& e)
    {
        JsonError(nNonce,JSONRPCError(RPC_PARSE_ERROR,e.what()),id);
    }

    return true;
}

bool CRPCMod::HandleEvent(CWalleveEventHttpBroken& eventHttpBroken)
{
    return true;
}

void CRPCMod::JsonReply(uint64 nNonce,const Value& result, const Value& id)
{
    Object reply;
    reply.push_back(Pair("result", result));
    reply.push_back(Pair("error", Value::null));
    reply.push_back(Pair("id", id));

    CWalleveEventHttpRsp eventHttpRsp(nNonce);
    eventHttpRsp.data.nStatusCode = 200;
    eventHttpRsp.data.mapHeader["content-type"] = "application/json";
    eventHttpRsp.data.mapHeader["connection"] = "Keep-Alive";
    eventHttpRsp.data.mapHeader["server"] = "multivers-rpc";
    eventHttpRsp.data.strContent = write_string(Value(reply), false) + "\n";

    pHttpServer->DispatchEvent(&eventHttpRsp);
}

void CRPCMod::JsonError(uint64 nNonce,const Object& objError, const Value& id)
{
    int nStatus = 500;
    int code = find_value(objError, "code").get_int();
    if (code == RPC_INVALID_REQUEST) nStatus = 400;
    else if (code == RPC_METHOD_NOT_FOUND) nStatus = 404;

    Object reply;
    reply.push_back(Pair("result", Value::null));
    reply.push_back(Pair("error", objError));
    reply.push_back(Pair("id", id));

    CWalleveEventHttpRsp eventHttpRsp(nNonce);
    eventHttpRsp.data.nStatusCode = nStatus;
    eventHttpRsp.data.mapHeader["content-type"] = "application/json";
    eventHttpRsp.data.mapHeader["server"] = "multiverse-rpc";
    eventHttpRsp.data.strContent = write_string(Value(reply), false) + "\n";

    pHttpServer->DispatchEvent(&eventHttpRsp);
}

bool CRPCMod::CheckWalletError(MvErr err)
{
    switch (err)
    {
    case MV_ERR_WALLET_NOT_FOUND:
        throw JSONRPCError(RPC_INVALID_REQUEST,"Missing wallet");
        break;
    case MV_ERR_WALLET_IS_LOCKED:
        throw JSONRPCError(RPC_WALLET_UNLOCK_NEEDED,
                           "Wallet is locked,enter the wallet passphrase with walletpassphrase first.");
    case MV_ERR_WALLET_IS_UNLOCKED:
        throw JSONRPCError(RPC_WALLET_ALREADY_UNLOCKED,"Wallet is already unlocked");
        break;
    case MV_ERR_WALLET_IS_ENCRYPTED:
        throw JSONRPCError(RPC_WALLET_WRONG_ENC_STATE,"Running with an encrypted wallet, "
                                                      "but encryptwallet was called");
        break;
    case MV_ERR_WALLET_IS_UNENCRYPTED:
        throw JSONRPCError(RPC_WALLET_WRONG_ENC_STATE,"Running with an unencrypted wallet, "
                                                      "but walletpassphrasechange/walletlock was called.");
        break;
    default:
        break;
    }
    return (err == MV_OK);
}

Value CRPCMod::RPCHelp(const Array& params,bool fHelp)
{
    if (fHelp || params.size() > 1)
    {
        throw runtime_error(
            "help [command]\n"
            "List commands, or get help for a command.");
    }
    map<string,RPCFunc>::iterator it;
    if (params.size() > 0)
    {
        it = mapRPCFunc.find(params[0].get_str());
        if (it != mapRPCFunc.end())
        {
            try
            {
                Array params;
                (this->*(*it).second)(params,true);
            }
            catch (exception& e)
            {
                return string(e.what());
            }
        }
        else
        {
            return (string("help: unknown command: ") + params[0].get_str());
        }
    }
    else
    {
        string strRet;
        for (it = mapRPCFunc.begin();it != mapRPCFunc.end();++it)
        {
            try
            {
                Array params;
                (this->*(*it).second)(params,true);
            }
            catch (exception& e)
            {
                string strHelp = string(e.what());
                strRet += strHelp.substr(0, strHelp.find('\n')) + "\n";
            }
        }
        return strRet.substr(0,strRet.size() - 1);
    }

    return Value::null;
}

Value CRPCMod::RPCStop(const Array& params,bool fHelp)
{
    if (fHelp || params.size() != 0)
    {
        throw runtime_error(
            "stop\n"
            "Stop multiverse server.");
    }
    pService->Shutdown();
    return "multiverse server stopping";
}

Value CRPCMod::RPCGetPeerCount(const Array& params,bool fHelp)
{
    if (fHelp || params.size() != 0)
    {
        throw runtime_error(
            "getpeercount\n"
            "Returns the number of connections to other nodes.");
    }
    return pService->GetPeerCount();
}

Value CRPCMod::RPCListPeer(const Array& params,bool fHelp)
{
    if (fHelp || params.size() != 0)
    {
        throw runtime_error(
            "listpeer\n"
            "Returns data about each connected network node.");
    }
    return Value::null;
}

Value CRPCMod::RPCAddNode(const Array& params,bool fHelp)
{
    if (fHelp || params.size() != 1)
    {
        throw runtime_error(
            "addnode \"node\"\n"
            "Attempts add a node into the addnode list.\n");
    }
    string strNode = params[0].get_str();
/*
    if (!pService->AddNode(CNetHost(strNode,WalleveConfig()->nPort)))
    {
        throw JSONRPCError(RPC_CLIENT_INVALID_IP_OR_SUBNET,"Failed to add node.");
    }
*/
    return Value::null;
}

Value CRPCMod::RPCGetForkCount(const Array& params,bool fHelp)
{
    if (fHelp || params.size() != 0)
    {
        throw runtime_error(
            "getforkcount\n"
            "Returns the number of forks.");
    }
    return pService->GetForkCount();
}

Value CRPCMod::RPCGetForkGenealogy(const Array& params,bool fHelp)
{
    if (fHelp || params.size() > 1)
    {
        throw runtime_error(
            "getgenealogy [fork=0]\n"
            "Returns the list of ancestry and subline.");
    }
    uint256 fork = GetForkHash(params,0);
    vector<pair<uint256,int> > vAncestry;
    vector<pair<int,uint256> > vSubline;
    if (!pService->GetForkGenealogy(fork,vAncestry,vSubline))
    {
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Unknown fork");
    }
    Object ancestry;
    for (int i = vAncestry.size();i > 0;i--)
    {
        ancestry.push_back(Pair(vAncestry[i - 1].first.GetHex(),vAncestry[i - 1].second));
    }
    Object subline;
    for (int i = 0;i > vSubline.size();i++)
    {
        subline.push_back(Pair(vSubline[i].second.GetHex(),vSubline[i].first));
    }
    Object ret;
    ret.push_back(Pair("ancestry",ancestry));
    ret.push_back(Pair("subline",subline));
    return ret;
}

Value CRPCMod::RPCGetBlockLocation(const Array& params,bool fHelp)
{
    if (fHelp || params.size() != 1)
    {
        throw runtime_error(
            "getlocation <blockhash>\n"
            "Returns the location with given block.");
    }
    uint256 hash(params[0].get_str());
    uint256 fork;
    int height;
    if (!pService->GetBlockLocation(hash,fork,height))
    {
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Unknown block");
    }
    Object ret;
    ret.push_back(Pair(fork.GetHex(),height));
    return ret;
}

Value CRPCMod::RPCGetBlockCount(const Array& params,bool fHelp)
{
    if (fHelp || params.size() > 1)
    {
        throw runtime_error(
            "getblockcount [fork=0]\n"
            "Returns the number of blocks in the given fork.");
    }
    uint256 fork = GetForkHash(params,0);
    return pService->GetBlockCount(fork);
}

Value CRPCMod::RPCGetBlockHash(const Array& params,bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
    {
        throw runtime_error(
            "getblockhash <index> [fork=0]\n"
            "Returns hash of block in fork at <index>.");
    }
    int nHeight = params[0].get_int();
    uint256 fork = GetForkHash(params,1);
    uint256 hash = 0;
    if (!pService->GetBlockHash(fork,nHeight,hash))
    {
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Block number out of range.");
    }
    return (hash.GetHex());
}

Value CRPCMod::RPCGetBlock(const Array& params,bool fHelp)
{
    if (fHelp || params.size() != 1)
    {
        throw runtime_error(
            "getblock <hash>\n"
            "Returns details of a block with given block-hash.");
    }

    uint256 hash(params[0].get_str());
    CBlock block;
    uint256 fork;
    int height;
    if (!pService->GetBlock(hash,block,fork,height))
    {
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Unknown block");
    }

    return BlockToJSON(hash,block,fork,height);
}

Value CRPCMod::RPCGetTxPool(const Array& params,bool fHelp)
{
    if (fHelp || params.size() > 1)
    {
        throw runtime_error(
            "gettxpool [fork=0]\n"
            "Returns all transaction ids in memory pool for given fork.");
    }
    uint256 fork = GetForkHash(params,0);
    vector<uint256> vTxPool;
    pService->GetTxPool(fork,vTxPool);
    Array arrayTx;
    BOOST_FOREACH(const uint256& txid, vTxPool)
    {   
        arrayTx.push_back(txid.ToString());
    }
    return arrayTx;
}

Value CRPCMod::RPCGetTransaction(const Array& params,bool fHelp)
{
    if (fHelp || params.size() != 1)
    {
        throw runtime_error(
            "gettransaction <hash>\n"
            "Returns details of a transaction with given txid.");
    }

    return Value::null;
}

Value CRPCMod::RPCListKey(const Array& params,bool fHelp)
{
    if (fHelp || params.size() != 0)
    {
        throw runtime_error(
            "listkey\n"
            "Returns Object that has pubkey as keys, associated status as values.\n");
    }
    set<crypto::CPubKey> setPubKey;
    pService->GetPubKeys(setPubKey);

    Object ret;
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
            ret.push_back(Pair(pubkey.GetHex(),oss.str()));
        }
    } 
    return ret;
}

Value CRPCMod::RPCGetNewKey(const Array& params,bool fHelp)
{
    if (fHelp || params.size() > 1)
    {
        throw runtime_error(
            "getnewkey [passphrase]\n"
            "Returns a new pubkey for receiving payments.  "
            "If [passphrase] is specified (recommended), the key will be encrypted with [passphrase]. \n");
    }
    
    crypto::CCryptoString strPassphrase;
    if (params.size() != 0)
    {
        strPassphrase = params[0].get_str().c_str();
    }
    crypto::CPubKey pubkey;

    if (!pService->MakeNewKey(strPassphrase,pubkey))
    {
        throw JSONRPCError(RPC_WALLET_ERROR,"Failed add new key.");
    }

    return (pubkey.ToString());
}
Value CRPCMod::RPCEncryptKey(const Array& params,bool fHelp)
{
    if (fHelp || params.size() < 2 || params.size()  > 3)
    {
        throw runtime_error(
            "encryptkey <pubkey> <passphrase> [oldpassphrase]\n"
            "Encrypts the key assoiciated with <passphrase>.  "
            "For encrypted key, changes the passphrase for [oldpassphrase] to <passphrase> \n");
    }

    crypto::CPubKey pubkey;
    pubkey.SetHex(params[0].get_str());
    crypto::CCryptoString strPassphrase,strOldPassphrase;
    strPassphrase = params[1].get_str().c_str();
    if (params.size() > 2)
    {
        strOldPassphrase = params[2].get_str().c_str();
    }
    if (!pService->HaveKey(pubkey))
    {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY,"Unknown key");
    }
    if (!pService->EncryptKey(pubkey,strPassphrase,strOldPassphrase))
    {
        throw JSONRPCError(RPC_WALLET_PASSPHRASE_INCORRECT,"The passphrase entered was incorrect.");
    }
    return Value::null;
}

Value CRPCMod::RPCLockKey(const Array& params,bool fHelp)
{
    if (fHelp || params.size() != 1)
    {
        throw runtime_error(
            "lockkey <pubkey>\n"
            "Removes the encryption key from memory, locking the key.\n"
            "After calling this method, you will need to call unlockkey again."
            "before being able to call any methods which require the key to be unlocked.");
    }

    crypto::CPubKey pubkey;
    pubkey.SetHex(params[0].get_str());
   
    int nVersion;
    bool fLocked;
    int64 nAutoLockTime; 
    if (!pService->GetKeyStatus(pubkey,nVersion,fLocked,nAutoLockTime))
    {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY,"Unknown key");
    }
    if (!fLocked && !pService->Lock(pubkey))
    {
        throw JSONRPCError(RPC_WALLET_ERROR,"Failed to lock key");
    }
    return Value::null;
}

Value CRPCMod::RPCUnlockKey(const Array& params,bool fHelp)
{
    if (fHelp || params.size() < 2 || params.size() > 3)
    {
        throw runtime_error(
            "unlockkey <pubkey> <passphrase> [timeout=0]\n"
            "If [timeout] > 0,stores the wallet decryption key in memory for [timeout] seconds."
            "before being able to call any methods which require the key to be locked.");
    }

    crypto::CPubKey pubkey;
    pubkey.SetHex(params[0].get_str());
    crypto::CCryptoString strPassphrase;
    strPassphrase = params[1].get_str().c_str();
    int64 nTimeout = 0;
    if (params.size() > 2)
    {
         nTimeout = params[2].get_int64();
    }

    int nVersion;
    bool fLocked;
    int64 nAutoLockTime; 
    if (!pService->GetKeyStatus(pubkey,nVersion,fLocked,nAutoLockTime))
    {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY,"Unknown key");
    }
    if (!fLocked)
    {
        throw JSONRPCError(RPC_WALLET_ALREADY_UNLOCKED,"Key is already unlocked");
    }
    if (!pService->Unlock(pubkey,strPassphrase,nTimeout))
    {
        throw JSONRPCError(RPC_WALLET_PASSPHRASE_INCORRECT,"The passphrase entered was incorrect.");
    }
    return Value::null;
}

Value CRPCMod::RPCImportPrivKey(const Array& params,bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
    {
        throw runtime_error(
            "importprivkey <privkey> [passphrase]\n"
            "Adds a private key (as returned by dumpprivkey) to your wallet."
            "If [passphrase] is specified (recommended), the key will be encrypted with [passphrase]. \n");
    }
    uint256 nPriv(params[0].get_str());
    crypto::CCryptoString strPassphrase;
    if (params.size() == 2)
    {
        strPassphrase = params[1].get_str().c_str();
    }

    crypto::CKey key;
    if (!key.SetSecret(crypto::CCryptoKeyData(nPriv.begin(),nPriv.end())))
    {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY,"Invalid private key");
    }
    if (pService->HaveKey(key.GetPubKey()))
    {
        throw JSONRPCError(RPC_WALLET_ERROR,"Already have key");
    }
    if (!strPassphrase.empty())
    {
        key.Encrypt(strPassphrase);
    }
    if (!pService->AddKey(key))
    {
        throw JSONRPCError(RPC_WALLET_ERROR,"Failed to add key");
    }
    return key.GetPubKey().GetHex();
}

Value CRPCMod::RPCImportKey(const Array& params,bool fHelp)
{
    if (fHelp || params.size() != 1)
    {
        throw runtime_error(
            "importkey <serialized key>\n"
            "Adds a key (as returned by importkey) to your wallet.");
    }
    vector<unsigned char> vchKey = ParseHexString(params[0].get_str());
    crypto::CKey key;
    if (!key.Load(vchKey))
    {
        throw JSONRPCError(RPC_INVALID_PARAMS,"Failed to verify serialized key");
    }
    if (pService->HaveKey(key.GetPubKey()))
    {
        throw JSONRPCError(RPC_WALLET_ERROR,"Already have key");
    }
    
    if (!pService->AddKey(key))
    {
        throw JSONRPCError(RPC_WALLET_ERROR,"Failed to add key");
    }
    return key.GetPubKey().GetHex();
}

Value CRPCMod::RPCExportKey(const Array& params,bool fHelp)
{
    if (fHelp || params.size() != 1)
    {
        throw runtime_error(
            "exportkey <pubkey>\n"
            "Reveals the serialized key corresponding to <pubkey>.");
    }
    crypto::CPubKey pubkey;
    pubkey.SetHex(params[0].get_str());
    if (!pService->HaveKey(pubkey))
    {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY,"Unknown key");
    }
    vector<unsigned char> vchKey;
    if (!pService->ExportKey(pubkey,vchKey))
    {
        throw JSONRPCError(RPC_WALLET_ERROR,"Failed to export key");
    }
    return ToHexString(vchKey);
}

Value CRPCMod::RPCSignMessage(const Array& params,bool fHelp)
{
    if (fHelp || params.size() != 2)
    {
        throw runtime_error(
            "signmessage <pubkey> <message>\n"
            "Sign a message with the private key of an pubkey");
    }

    crypto::CPubKey pubkey;
    pubkey.SetHex(params[0].get_str());
    string strMessage = params[1].get_str();

    int nVersion;
    bool fLocked;
    int64 nAutoLockTime; 
    if (!pService->GetKeyStatus(pubkey,nVersion,fLocked,nAutoLockTime))
    {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY,"Unknown key");
    }
    if (fLocked)
    {
        throw JSONRPCError(RPC_WALLET_UNLOCK_NEEDED,"Key is locked");
    }
    
    const string strMessageMagic = "Multiverse Signed Message:\n";
    CWalleveBufStream ss;
    ss << strMessageMagic;
    ss << strMessage;
    vector<unsigned char> vchSig;
    if (!pService->SignSignature(pubkey,crypto::CryptoHash(ss.GetData(),ss.GetSize()),vchSig))
    {
        throw JSONRPCError(RPC_WALLET_ERROR,"Failed to sign message");
    }

    return ToHexString(vchSig);
}

Value CRPCMod::RPCVerifyMessage(const Array& params,bool fHelp)
{
    if (fHelp || params.size() != 3)
    {
        throw runtime_error(
            "verifymessage <pubkey> <message> <signature>\n"
            "Verify a signed message");
    }

    crypto::CPubKey pubkey;
    pubkey.SetHex(params[0].get_str());
    string strMessage = params[1].get_str();
    vector<unsigned char> vchSig = ParseHexString(params[2].get_str());
    const string strMessageMagic = "Multiverse Signed Message:\n";
    CWalleveBufStream ss;
    ss << strMessageMagic;
    ss << strMessage;
    
    return pubkey.Verify(crypto::CryptoHash(ss.GetData(),ss.GetSize()),vchSig);
}

Value CRPCMod::RPCMakeKeyPair(const Array& params,bool fHelp)
{
    if (fHelp || params.size() != 0)
    {
        throw runtime_error(
            "makekeypair\n"
            "Make a public/private key pair.\n");
    }

    crypto::CCryptoKey key;
    crypto::CryptoMakeNewKey(key);
    
    Object result;
    result.push_back(Pair("privkey", key.secret.GetHex()));
    result.push_back(Pair("pubkey", key.pubkey.GetHex()));
    return result;
}
#if 0
Value CRPCMod::RPCNotify(const Array& params,bool fHelp)
{
    if (fHelp || params.size() != 1)
    {
        throw runtime_error(
            "notify <type>\n"
            "Setup special event notify."
            "<type> is blockchain/network/wallet/tx");
    }

    return Value::null;
}

Value CRPCMod::RPCMakeKeyPair(const Array& params,bool fHelp)
{
    if (fHelp || params.size() != 0)
    {
        throw runtime_error(
            "makekeypair\n"
            "Make a public/private key pair.\n");
    }

    CKey key;
    key.MakeNewKey(true);

    bool fCompressed;
    CSecret vchSecret = key.GetSecret(fCompressed);

    Object result;
    result.push_back(Pair("privkey", CLoMoSecret(vchSecret, fCompressed).ToString()));
    result.push_back(Pair("pubkey", HexStr(key.GetPubKey().Raw())));
    result.push_back(Pair("address", CLoMoAddress(key.GetPubKey().GetID()).ToString()));
    return result;
}

Value CRPCMod::RPCGetBlockCount(const Array& params,bool fHelp)
{
    if (fHelp || params.size() != 0)
    {
        throw runtime_error(
            "getblockcount\n"
            "Returns the number of blocks in the longest block chain.");
    }

    return pService->GetBestChainHeight();
}

Value CRPCMod::RPCGetBlockHash(const Array& params,bool fHelp)
{
    if (fHelp || params.size() != 1)
    {
        throw runtime_error(
            "getblockhash <index>\n"
            "Returns hash of block in best-block-chain at <index>.");
    }

    int nHeight = params[0].get_int();
    uint256 hash = 0;
    if (!pService->GetBlockHash(nHeight,hash))
    {
        throw runtime_error("Block number out of range.");
    }
    return (hash.GetHex());
}

Value CRPCMod::RPCGetBlock(const Array& params,bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 3)
    {
        throw runtime_error(
            "getblock <hash> [txinfo] [txdetails]\n"
            "txinfo optional to print more detailed tx info\n"
            "txdetails optional to print even more detailed tx info\n"
            "Returns details of a block with given block-hash.");
    }

    uint256 hash(params[0].get_str());
    bool fTxInfo = params.size() > 1 ? params[1].get_bool() : false;
    bool fTxDetails = params.size() > 2 ? params[2].get_bool() : false;

    CAssembledBlock block;
    if (!pService->GetBlock(hash,block))
    {
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Block not found in main chain");
    }
    return BlockToJSON(block, fTxInfo, fTxDetails);
}

Value CRPCMod::RPCGetMiningInfo(const Array& params,bool fHelp)
{
    if (fHelp || params.size() != 0)
    {
        throw runtime_error(
            "getmininginfo\n"
            "Returns an object containing mining-related information.");
    }

    size_t nTransactions;
    size_t nSerializedSize;
    int64  nDelayTime;
    pService->GetTxPoolStatus(nTransactions,nSerializedSize,nDelayTime);

    Object obj;
    obj.push_back(Pair("blocks",        pService->GetBestChainHeight()));
    obj.push_back(Pair("stakecoins",    ValueFromAmount(pService->GetStakeCoins())));
    obj.push_back(Pair("pooledtx",      (uint64_t)nTransactions));
    obj.push_back(Pair("pooledtxsize",  (uint64_t)nSerializedSize));
    obj.push_back(Pair("pooledtxdelay", (uint64_t)nDelayTime));
    obj.push_back(Pair("testnet",       WalleveConfig()->fTestNet));
    obj.push_back(Pair("errors",        WalleveGetWarnings()));
    return obj;
}

Value CRPCMod::RPCGetRawMempool(const Array& params,bool fHelp)
{
    if (fHelp || params.size() != 0)
    {
        throw runtime_error(
            "getrawmempool\n"
            "Returns all transaction ids in memory pool.");
    }

    vector<uint256> vTxPool;
    pService->GetTxPool(vTxPool);
    Array arrayTx;
    BOOST_FOREACH(const uint256& txid, vTxPool)
    {
        arrayTx.push_back(txid.ToString());
    }
    return arrayTx;
}

Value CRPCMod::RPCGetRawTransaction(const Array& params,bool fHelp)
{

   if (fHelp || params.size() < 1 || params.size() > 2)
   {
        throw runtime_error(
            "getrawtransaction <txid> [verbose=0]\n"
            "If verbose=0, returns a string that is\n"
            "serialized, hex-encoded data for <txid>.\n"
            "If verbose is non-zero, returns an Object\n"
            "with information about <txid>.");
    }

    uint256 txid(params[0].get_str());
    bool fVerbose = false;
    if (params.size() > 1)
    {
        fVerbose = (params[1].get_int() != 0);
    }

    CAssembledTx atx;
    if (!pService->GetAssembledTx(txid,atx))
    {
        throw JSONRPCError(RPC_INVALID_REQUEST, "No information available about transaction");
    }

    CWalleveBufStream ss;
    ss << ((CTransaction)atx);
    string strHex = HexStr(ss.GetData(),ss.GetData() + ss.GetSize());
    if (!fVerbose)
    {
        return strHex;
    }

    Object result;
    result.push_back(Pair("hex", strHex));
    TxToJSON(atx,pService->GetBestChainHeight(),result);
    return result;
}

Value CRPCMod::RPCAddNewWallet(const Array& params,bool fHelp)
{
    if (fHelp || params.size() != 1)
    {
        throw runtime_error(
            "addnewwallet <\"walletname\">\n"
            "Add a new wallet by the name of [\"walletname\"].");
    }
    
    string strWallet = params[0].get_str();
    if (pService->GetWallet(strWallet) != NULL)
    {
        throw JSONRPCError(RPC_WALLET_ERROR,"Wallet is exists.");
    }
    if (!pService->AddNewWallet(strWallet))
    {
        throw JSONRPCError(RPC_WALLET_ERROR,"Failed add new wallet.");
    }
    return Value::null;
}

Value CRPCMod::RPCListWallets(const Array& params,bool fHelp)
{
    if (fHelp || params.size() != 0)
    {
        throw runtime_error(
            "listwallet\n"
            "Returns Object that has wallet names as keys, wallet balances as values.");
    }
    vector<pair<string,int64> > vWalletRet;
    pService->ListWallets(vWalletRet);
    Object ret;
    BOOST_FOREACH(const PAIRTYPE(string, int64)& walletBalance, vWalletRet)
    {
        ret.push_back(Pair(walletBalance.first, ValueFromAmount(walletBalance.second)));
    }
    return ret;
}

Value CRPCMod::RPCAddMultiSigAddress(const Array& params,bool fHelp)
{
    if (fHelp || params.size() < 2 || params.size() > 3)
    {
        throw runtime_error(
            "addmultisigaddress <nrequired> <'[\"key\",\"key\"]'> [account]\n"
            "Add a nrequired-to-sign multisignature address to the wallet\"\n"
            "each key is a bitcoin address or hex-encoded public key\n"
            "If [account] is specified, assign address to [account].");
    }

    int nRequired = params[0].get_int();
    if (nRequired < 1)
    {
        throw JSONRPCError(RPC_INVALID_PARAMETER,"Require at least one key to redeem");
    }

    const Array& keys = params[1].get_array();
    if ((int)keys.size() < nRequired)
    {
        throw JSONRPCError(RPC_INVALID_PARAMETER,"Not enough keys supplied");
    }

    string strAccount = GetAccount(params,2);

    // Gather public keys
    string strWallet = GetWalletName(strURL);
    vector<CKey> pubkeys;
    pubkeys.resize(keys.size());
    
    for (unsigned int i = 0; i < keys.size(); i++)
    {
        CPubKey vchPubKey;
        const std::string& ks = keys[i].get_str();
        // Case 1: lomocoin address and we have full public key:
        CLoMoAddress address(ks);
        if (address.IsValid())
        {
            CKeyID keyID;
            if (!address.GetKeyID(keyID))
            {
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY,
                                   strprintf("%s does not refer to a key",ks.c_str()));
            }
            CPubKey vchPubKey;
            if (!CheckWalletError(pService->ExportPubKey(strWallet,keyID, vchPubKey)))
            {
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY,
                                   strprintf("No full public key for address %s",ks.c_str()));
            }
        }
        // Case 2: hex public key
        else if (IsHex(ks))
        {
            vchPubKey = CPubKey(ParseHex(ks));
        }

        if (!vchPubKey.IsValid() || !pubkeys[i].SetPubKey(vchPubKey))
        {
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY,
                               string("Invalid public key: ") + ks);
        }
    }

    // Construct using pay-to-script-hash:
    CScript inner;
    inner.SetMultisig(nRequired, pubkeys);
    if (!CheckWalletError(pService->ImportScript(strWallet,strAccount,inner)))
    {
        throw JSONRPCError(RPC_WALLET_ERROR,"Failed to add script");
    }

    return CLoMoAddress(inner.GetID()).ToString();
}

Value CRPCMod::RPCAddColdMintingAddress(const Array& params,bool fHelp)
{
    if (fHelp || params.size() < 2 || params.size() > 3)
    {
        throw runtime_error(
            "addcoldmintingaddress <minting address> <spending address> [account]\n"
            "Add a cold minting address to the wallet.\n"
            "The coins sent to this address will be mintable only with the minting private key.\n"
            "And they will be spendable only with the spending private key.\n"
            "If [account] is specified, assign address to [account].");
    }

    CLoMoAddress mintingAddress(params[0].get_str());
    CLoMoAddress spendingAddress(params[1].get_str());
    if (!mintingAddress.IsValid())
    {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid minting address");
    }
    if (!spendingAddress.IsValid())
    {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid spending address");
    }

    string strAccount = GetAccount(params,2);

    CKeyID mintingKeyID;
    if (!mintingAddress.GetKeyID(mintingKeyID))
    {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY,"Minting address does not refer to a key");
    }

    // Construct using pay-to-script-hash:
    string strWallet = GetWalletName(strURL);
    CScript inner;
    if (spendingAddress.IsScript())
    {
        CScriptID spendingScriptID;
        if (!spendingAddress.GetScriptID(spendingScriptID))
        {
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Spending address does not refer to script");
        }
        CScript spendingScript;
        if (!CheckWalletError(pService->ExportScript(strWallet,spendingScriptID, spendingScript)))
        {
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Spending address does not refer to redeem script");
        }
        txnouttype txType;
        vector<vector<unsigned char> > vSolutions;
        if (!Solver(spendingScript, txType, vSolutions)
            || txType != TX_MULTISIG)
        {
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Spending address does not refer to multisig script");
        }
        inner.SetMultisigColdMinting(mintingKeyID, spendingScript);
    }
    else
    {
        CKeyID spendingKeyID;
        if (!spendingAddress.GetKeyID(spendingKeyID))
        {
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Spending address does not refer to a key");
        }
        inner.SetColdMinting(mintingKeyID, spendingKeyID);
    }

    if (!CheckWalletError(pService->ImportScript(strWallet,strAccount,inner)))
    {
        throw JSONRPCError(RPC_WALLET_ERROR,"Failed to add script");
    }

    return CLoMoAddress(inner.GetID()).ToString();
}

Value CRPCMod::RPCSetTxFee(const Array& params,bool fHelp)
{
    if (fHelp || params.size() != 1 || AmountFromValue(params[0]) < MIN_TX_FEE)
    {
        throw runtime_error(
            "settxfee <amount>\n"
            "<amount> is a real and is rounded to 0.01 (cent)\n"
            "Minimum and default transaction fee per KB is 1 cent");
    }
    return CheckWalletError(pService->SetTxFee(GetWalletName(strURL),AmountFromValue(params[0])));
}

Value CRPCMod::RPCEncryptWallet(const Array& params,bool fHelp)
{
    if (fHelp || params.size() != 1)
    {
        throw runtime_error(
            "encryptwallet <passphrase>\n"
            "Encrypts the wallet with <passphrase>.");
    }
    SecureString strWalletPass;
    strWalletPass.reserve(100);
    strWalletPass = params[0].get_str().c_str();
    if (strWalletPass.length() < 1)
    {
        throw JSONRPCError(RPC_WALLET_INVALID_PASSPHRASE,"Invalid passphrase");
    }

    if (!CheckWalletError(pService->EncryptWallet(GetWalletName(strURL),strWalletPass)))
    {
        throw JSONRPCError(RPC_WALLET_ENCRYPTION_FAILED,"Failed to encrypt the wallet.");
    }
    return Value::null;
}

Value CRPCMod::RPCWalletLock(const Array& params,bool fHelp)
{
    if (fHelp || params.size() != 0)
    {
        throw runtime_error(
            "walletlock\n"
            "Removes the wallet encryption key from memory, locking the wallet.\n"
            "After calling this method, you will need to call walletpassphrase again\n"
            "before being able to call any methods which require the wallet to be unlocked.");
    }
    CheckWalletError(pService->LockWallet(GetWalletName(strURL)));
    return Value::null;
}

Value CRPCMod::RPCWalletPassphrase(const Array& params,bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
    {
        throw runtime_error(
            "walletpassphrase <passphrase> [timeout=0]\n"
            "If [timeout] > 0,stores the wallet decryption key in memory for [timeout] seconds.\n");
    }

    SecureString strWalletPass;
    strWalletPass.reserve(100);
    strWalletPass = params[0].get_str().c_str();
    if (strWalletPass.length() < 1)
    {
        throw JSONRPCError(RPC_WALLET_INVALID_PASSPHRASE,"Invalid passphrase");
    }
    int64 nTimeout = 0;
    if (params.size() > 1)
    {
         nTimeout = params[1].get_int64();
    }

    if (!CheckWalletError(pService->UnlockWallet(GetWalletName(strURL),strWalletPass,nTimeout)))
    {
        throw JSONRPCError(RPC_WALLET_PASSPHRASE_INCORRECT,"The wallet passphrase entered was incorrect.");
    }
    return Value::null;
}

Value CRPCMod::RPCWalletPassphraseChange(const Array& params,bool fHelp)
{
    if (fHelp || params.size() != 2)
    {
        throw runtime_error(
            "walletpassphrasechange <oldpassphrase> <newpassphrase>\n"
            "Changes the wallet passphrase from <oldpassphrase> to <newpassphrase>.");
    }

    SecureString strOldWalletPass;
    strOldWalletPass.reserve(100);
    strOldWalletPass = params[0].get_str().c_str();

    SecureString strNewWalletPass;
    strNewWalletPass.reserve(100);
    strNewWalletPass = params[1].get_str().c_str();
    if (strOldWalletPass.length() < 1 || strNewWalletPass.length() < 1)
    {
        throw JSONRPCError(RPC_WALLET_INVALID_PASSPHRASE,"Invalid passphrase");
    }

    if (!CheckWalletError(pService->ChangeWalletPassphrase(GetWalletName(strURL),strOldWalletPass,strNewWalletPass)))
    {
        throw JSONRPCError(RPC_WALLET_PASSPHRASE_INCORRECT,"The wallet passphrase entered was incorrect.");
    }
    return Value::null;
}

Value CRPCMod::RPCWalletExport(const Array& params,bool fHelp)
{
    if (fHelp || params.size() != 2)
    {
        throw runtime_error(
            "walletexport <destination> <passphrase>\n"
            "Export the wallet to <destination>.\n"
            "For encrypted wallet, enter <passphrase> same as the wallet passphrase.\n"
            "Otherwise, <passphrase> is new passphrase to import only.\n");
    }

    boost::filesystem::path pathDest(boost::filesystem::system_complete(params[0].get_str()));

    SecureString strWalletPass;
    strWalletPass.reserve(100);
    strWalletPass = params[1].get_str().c_str();

    if (strWalletPass.length() < 1)
    {
        throw JSONRPCError(RPC_WALLET_INVALID_PASSPHRASE,"Invalid passphrase");
    }

    if (!CheckWalletError(pService->ExportWallet(GetWalletName(strURL),pathDest,strWalletPass)))
    {
        throw JSONRPCError(RPC_WALLET_ERROR,"Failed to export wallet");
    }
    return Value::null;
}

Value CRPCMod::RPCWalletImport(const Array& params,bool fHelp)
{
    if (fHelp || params.size() != 2)
    {
        throw runtime_error(
            "walletimport <filepath> <passphrase>\n"
            "Import the wallet from <filepath>.\n");
    }
    boost::filesystem::path pathFrom(boost::filesystem::system_complete(params[0].get_str()));

    SecureString strWalletPass;
    strWalletPass.reserve(100);
    strWalletPass = params[1].get_str().c_str();

    if (strWalletPass.length() < 1)
    {
        throw JSONRPCError(RPC_WALLET_INVALID_PASSPHRASE,"Invalid passphrase");
    }

    if (!CheckWalletError(pService->ExportWallet(GetWalletName(strURL),pathFrom,strWalletPass)))
    {
        throw JSONRPCError(RPC_WALLET_ERROR,"Failed to import wallet");
    }
    return Value::null;
}

Value CRPCMod::RPCGetInfo(const Array& params,bool fHelp)
{
    if (fHelp || params.size() != 0)
    {
        throw runtime_error(
            "getinfo\n"
            "Returns an object containing various state info.");
    }

    CLoMoBlockChainStatus statusBlockChain;
    CLoMoWalletStatus statusWallet;
    
    pService->GetBlockChainStatus(statusBlockChain);
    if (!pService->GetWalletStatus(GetWalletName(strURL),statusWallet))
    {
        throw JSONRPCError(RPC_INVALID_REQUEST,"Missing wallet");
    }
    CWalletTxStatus& txStatus = statusWallet.txStatus;

    Object obj;
    /* Version */
    obj.push_back(Pair("version",       FormatFullVersion()));
    obj.push_back(Pair("protocolversion",(int)PROTOCOL_VERSION));
    obj.push_back(Pair("walletversion", (int)WALLET_VERSION));
    /* Wallet */
    obj.push_back(Pair("balance",       ValueFromAmount(txStatus.nBalance)));
    obj.push_back(Pair("frozen",        ValueFromAmount(txStatus.nFrozen)));
    obj.push_back(Pair("mintingonly",   ValueFromAmount(txStatus.nMintingOnly)));
    obj.push_back(Pair("newmint",       ValueFromAmount(txStatus.nNewMint)));
    obj.push_back(Pair("stake",         ValueFromAmount(txStatus.nStake)));
    obj.push_back(Pair("transactions",  (int)txStatus.nTransactions));
    obj.push_back(Pair("uncomfirmed",   (int)txStatus.nUnconfirmed));
    obj.push_back(Pair("keypoolsize",   (int)statusWallet.nKeyPoolSize));
    obj.push_back(Pair("paytxfee",      ValueFromAmount(statusWallet.nTransactionFee)));
    obj.push_back(Pair("islocked",      statusWallet.fLocked));
    if (!statusWallet.fLocked && statusWallet.nUnlockTime > 0)
    {
        obj.push_back(Pair("unlocked_until", (boost::int64_t)statusWallet.nUnlockTime));
    }
    /* Blockchain */
    obj.push_back(Pair("blocks",        statusBlockChain.nBestChainHeight));
    obj.push_back(Pair("moneysupply",   ValueFromAmount(pService->GetMoneySupply())));
    /* Config */
    obj.push_back(Pair("testnet",       WalleveConfig()->fTestNet));
    /* Network */
    obj.push_back(Pair("connections",   pService->GetPeerCount()));
//    obj.push_back(Pair("proxy",         (fUseProxy ? addrProxy.ToStringIPPort() : string())));
    obj.push_back(Pair("ip",            pService->GetExtIP()));
    /* Error */
    obj.push_back(Pair("errors",        WalleveGetWarnings()));
    return obj;
}


Value CRPCMod::RPCGetAddressInfo(const Array& params,bool fHelp)
{
    if (fHelp || params.size() != 1)
    {
        throw runtime_error(
            "getaddressinfo <address>\n"
            "Returns an object containing various state info for given address.");
    }
    CLoMoAddress address(params[0].get_str());
    if (!address.IsValid())
    {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, string("Invalid LoMoCoin address: ")+params[0].get_str());
    }

    CWalletAddressInfo info;
    CheckWalletError(pService->GetAddressInfo(GetWalletName(strURL),address.Get(),info));

    Object obj;
    obj.push_back(Pair("balance",       ValueFromAmount(info.nBalance)));
    obj.push_back(Pair("frozen",        ValueFromAmount(info.nFrozen)));
    obj.push_back(Pair("mintingonly",   ValueFromAmount(info.nMintingOnly)));
    obj.push_back(Pair("stake",         ValueFromAmount(info.nStake)));
    return obj;
}

Value CRPCMod::RPCSetAddressBook(const Array& params,bool fHelp)
{
    if (fHelp || params.size() != 2)
    {
        throw runtime_error(
            "setaddressbook <label> <lomocoinaddress>\n"
            "Sets the addressbook associated with the given address.");
    }
    string strLabel = params[0].get_str();
    string strAddress = params[1].get_str();
    if (!CheckWalletError(pService->SetAddressBook(GetWalletName(strURL),strLabel,strAddress)))
    {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY,"Invalid lomocoin address");
    }
    return Value::null;
}

Value CRPCMod::RPCGetAddressBook(const Array& params,bool fHelp)
{
    if (fHelp || params.size() != 1)
    {
        throw runtime_error(
            "getaddressbook <label>\n"
            "Returns the current lomocoin address for this label.");
    }
    string strLabel = params[0].get_str();
    string strAddress;
    if (!CheckWalletError(pService->GetAddressBook(GetWalletName(strURL),strLabel,strAddress)))
    {
        throw JSONRPCError(RPC_WALLET_INVALID_LABEL_NAME,"Invalid label name");
    }
    return strAddress;
}

Value CRPCMod::RPCDelAddressBook(const Array& params,bool fHelp)
{
    if (fHelp || params.size() != 1)
    {
        throw runtime_error(
            "deladdressbook <label>\n"
            "Removes label from addressbook.");
    }
    string strLabel = params[0].get_str();
    CheckWalletError(pService->DelAddressBook(GetWalletName(strURL),strLabel));
    return Value::null;
}

Value CRPCMod::RPCListAddressBook(const Array& params,bool fHelp)
{
    if (fHelp || params.size() != 0)
    {
        throw runtime_error(
            "listaddressbook\n"
            "Returns Object that has label names as keys, associated address as values.");
    }
    map<string,string> mapAddressBook;
    CheckWalletError(pService->ListAddressBook(GetWalletName(strURL),mapAddressBook));
    Object ret;
    BOOST_FOREACH(const PAIRTYPE(string,string)& addrbook,mapAddressBook)
    {
        ret.push_back(Pair(addrbook.first, addrbook.second));
    }
    return ret;
}

Value CRPCMod::RPCListAccounts(const Array& params,bool fHelp)
{
    if (fHelp || params.size() > 1)
    {
        throw runtime_error(
            "listaccounts [minconf=1]\n"
            "Returns Object that has account names as keys, account balances as values.");
    }
    int nMinConf = GetInt(params,0,1);
    vector<pair<string,int64> > vecAccountBalance;
    CheckWalletError(pService->ListAccountBalance(GetWalletName(strURL),nMinConf,vecAccountBalance));
    Object ret;
    BOOST_FOREACH(const PAIRTYPE(string, int64)& accountBalance, vecAccountBalance)
    {
        ret.push_back(Pair(accountBalance.first, ValueFromAmount(accountBalance.second)));
    }
    return ret;
}

Value CRPCMod::RPCGetNewAddress(const Array& params,bool fHelp)
{
    if (fHelp || params.size() > 1)
    {
        throw runtime_error(
            "getnewaddress [account]\n"
            "Returns a new lomocoin address for receiving payments.  "
            "If [account] is specified (recommended), it is added to the address book "
            "so payments received with the address will be credited to [account].");
    }
    // Parse the account first so we don't generate a key if there's an error
    string strAccount = GetAccount(params,0);
    CKeyID keyID;
    CheckWalletError(pService->CreateNewKey(GetWalletName(strURL),strAccount,keyID));
    return CLoMoAddress(keyID).ToString();
}

Value CRPCMod::RPCGetAccountAddress(const Array& params,bool fHelp)
{
    if (fHelp || params.size() > 1)
    {
        throw runtime_error(
            "getaccountaddress <account>\n"
            "Returns the current lomocoin address for receiving payments to this account.");
    }

    string strWallet = GetWalletName(strURL);
    string strAccount = GetAccount(params,0);

    vector<CTxDestination> vecAddress;
    CheckWalletError(pService->GetAccountAddress(strWallet,strAccount,vecAddress));

    if (!vecAddress.empty())
    {
        return CLoMoAddress(vecAddress[0]).ToString();
    }

    CKeyID keyID;
    CheckWalletError(pService->CreateNewKey(strWallet,strAccount,keyID));
    return CLoMoAddress(keyID).ToString();
}

Value CRPCMod::RPCSetAccount(const Array& params,bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
    {
        throw runtime_error(
            "setaccount <lomocoinaddress> <account>\n"
            "Sets the account associated with the given address.");
    }

    string strAccount = GetAccount(params,1);
    if (!CheckWalletError(pService->SetAccount(GetWalletName(strURL),params[0].get_str(),strAccount)))
    {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY,"Invalid lomocoin address");
    }
    return Value::null;
}

Value CRPCMod::RPCGetAccount(const Array& params,bool fHelp)
{
    if (fHelp || params.size() != 1)
    {
        throw runtime_error(
            "getaccount <lomocoinaddress>\n"
            "Returns the account associated with the given address.");
    }

    string strAccount;
    if (!CheckWalletError(pService->GetAccount(GetWalletName(strURL),params[0].get_str(),strAccount)))
    {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY,"Unknown address");
    }
    return strAccount;
}

Value CRPCMod::RPCGetAddressesByAccount(const Array& params,bool fHelp)
{
    if (fHelp || params.size() > 1)
    {
        throw runtime_error(
            "getaddressesbyaccount <account>\n"
            "Returns the list of addresses for the given account.");
    }
    string strAccount = GetAccount(params,0);

    // Find all addresses that have the given account
    vector<CTxDestination> vecAddress;
    CheckWalletError(pService->GetAccountAddress(GetWalletName(strURL),strAccount,vecAddress));
    Array ret;
    BOOST_FOREACH(const CTxDestination& address,vecAddress)
    {
        ret.push_back(CLoMoAddress(address).ToString());
    }
    return ret;
}

Value CRPCMod::RPCValidateAddress(const Array& params,bool fHelp)
{
    if (fHelp || params.size() != 1)
    {
        throw runtime_error(
            "validateaddress <lomocoinaddress>\n"
            "Return information about <lomocoinaddress>.");
    }

    CLoMoAddress address(params[0].get_str());
    bool isValid = address.IsValid();
    string strWallet = GetWalletName(strURL);
    Object ret;
    ret.push_back(Pair("isvalid", isValid));
    if (isValid)
    {
        CTxDestination dest = address.Get();
        string currentAddress = address.ToString();
        ret.push_back(Pair("address", currentAddress));
        Object obj;
        bool isMine = false;
        bool isScript = address.IsScript();
        obj.push_back(Pair("isscript",isScript));
        if (!isScript)
        {
            CPubKey vchPubKey;
            isMine = CheckWalletError(pService->ExportPubKey(strWallet,boost::get<CKeyID>(dest),vchPubKey));
            if (isMine)
            {
                obj.push_back(Pair("pubkey", HexStr(vchPubKey.Raw())));
                obj.push_back(Pair("iscompressed", vchPubKey.IsCompressed()));
            }
        }         
        else
        {
            CScript subscript;
            isMine = CheckWalletError(pService->ExportScript(strWallet,boost::get<CScriptID>(dest),subscript));
            if (isMine)
            {
                vector<CTxDestination> addresses;
                txnouttype whichType;
                int nRequired;
                ExtractDestinations(subscript, whichType, addresses, nRequired);
                obj.push_back(Pair("script", GetTxnOutputType(whichType)));
                obj.push_back(Pair("hex",HexStr(subscript.begin(), subscript.end())));
                Array a;
                BOOST_FOREACH(const CTxDestination& addr, addresses)
                {
                    a.push_back(CLoMoAddress(addr).ToString());
                }
                obj.push_back(Pair("addresses", a));
                if (whichType == TX_MULTISIG || whichType == TX_MULTISIGCOLDMINTING)
                {
                    obj.push_back(Pair("sigsrequired", nRequired));
                }
            }
        }

        ret.push_back(Pair("ismine", isMine));
        if (isMine)
        {
            ret.insert(ret.end(), obj.begin(), obj.end());
            string strAccount;
            if (CheckWalletError(pService->GetAccount(strWallet,currentAddress,strAccount)))
            {
                ret.push_back(Pair("account",strAccount));
            }
        }
    }
    return ret;
}

Value CRPCMod::RPCGetBalance(const Array& params,bool fHelp)
{
    if (fHelp || params.size() > 2)
    {
        throw runtime_error(
            "getbalance [account] [minconf=1]\n"
            "If [account] is not specified, returns the wallet's total available balance.\n"
            "If [account] is specified, returns the balance in the account.");
    }

    string strAccount = GetAccount(params,0,true);
    int nMinConf = (params.size() > 1 ? params[1].get_int() : 1);
    int64 nBalance = 0;
    CheckWalletError(pService->GetBalance(GetWalletName(strURL),strAccount,nMinConf,nBalance));
    return ValueFromAmount(nBalance);
}

Value CRPCMod::RPCGetTransaction(const Array& params,bool fHelp)
{
    if (fHelp || params.size() != 1)
    {
        throw runtime_error(
            "gettransaction <txid>\n"
            "Get detailed information about <txid>");
    }

    uint256 hash;
    hash.SetHex(params[0].get_str());
    Object entry;

    CWalletTx wtx;
    list<pair<CTxDestination, int64> > listSent,listReceived;
    if (!CheckWalletError(pService->GetWalletTx(GetWalletName(strURL),hash,wtx,listSent,listReceived)))
    {
        throw JSONRPCError(RPC_WALLET_ERROR, "Invalid or non-wallet transaction id");
    }

    int nBestChainHeight = pService->GetBestChainHeight();

    int64 nDebit = wtx.GetDebit();
    int64 nCredit = 0;
    BOOST_FOREACH(const PAIRTYPE(CTxDestination, int64)& r,listReceived)
    {
        nCredit += r.second;
    }

    int64 nFee = 0;
    if (wtx.IsFromMe() && !wtx.IsCoinBase() && !wtx.IsCoinStake())
    {
        nFee = wtx.GetValueOut() - wtx.GetValueIn();
    }

    entry.push_back(Pair("amount", ValueFromAmount(nCredit - nDebit - nFee)));
    if (wtx.IsFromMe())
    {
        entry.push_back(Pair("fee", ValueFromAmount(nFee)));
    }
    WalletTxToJSON(wtx,nBestChainHeight,entry);

    Array details;
    WalletTxDetailsToJSON(wtx,pCoreProtocol->GetCoinbaseMaturity(),
                              nBestChainHeight,listSent,listReceived,false,details);
    entry.push_back(Pair("details", details));

    return entry;
}

Value CRPCMod::RPCListTransactions(const Array& params,bool fHelp)
{
    if (fHelp || params.size() > 3)
    {
        throw runtime_error(
            "listtransactions [account] [count=10] [from=0]\n"
            "Returns up to [count] most recent transactions skipping the first [from] transactions for account [account].");
    }

    string strAccount = GetAccount(params,0,true);
    int nCount = GetInt(params,1,10);
    int nFrom = GetInt(params,2,0);
    if (nCount < 0)
    {
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Negative count");
    }
    if (nFrom < 0)
    {
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Negative from");
    }

    string strWallet = GetWalletName(strURL);
    int nBestChainHeight = pService->GetBestChainHeight();

    set<CTxDestination> setAddress;
    GetAccountAddress(strWallet,strAccount,setAddress);

    vector<uint256> vTxid;
    CheckWalletError(pService->ListTransactions(strWallet,0,vTxid));

    Array ret;
    BOOST_REVERSE_FOREACH(const uint256& txid,vTxid)
    {
        CWalletTx wtx;
        list<pair<CTxDestination, int64> > listSent,listReceived;
        if (!CheckWalletError(pService->GetWalletTx(strWallet,txid,wtx,listSent,listReceived)))
        {
            continue;
        }
        SelectOutputByAddress(listSent,setAddress);
        SelectOutputByAddress(listReceived,setAddress);
        WalletTxDetailsToJSON(wtx,pCoreProtocol->GetCoinbaseMaturity(),
                                  nBestChainHeight,listSent,listReceived,true,ret);
        if (ret.size() >= (nCount+nFrom))
        {
            break;
        }
    }

    if (nFrom > (int)ret.size())
    {
        nFrom = ret.size();
    }
    if ((nFrom + nCount) > (int)ret.size())
    {
        nCount = ret.size() - nFrom;
    }

    Array::iterator first = ret.begin();
    std::advance(first, nFrom);
    Array::iterator last = ret.begin();
    std::advance(last, nFrom+nCount);

    if (last != ret.end()) ret.erase(last, ret.end());
    if (first != ret.begin()) ret.erase(ret.begin(), first);

    std::reverse(ret.begin(), ret.end()); // Return oldest to newest

    return ret;

}

Value CRPCMod::RPCListUnspent(const Array& params,bool fHelp)
{
    if (fHelp || params.size() > 3)
    {
        throw runtime_error(
            "listunspent [minconf=1] [maxconf=9999999]  [\"address\",...]\n"
            "Returns array of unspent transaction outputs\n"
            "with between minconf and maxconf (inclusive) confirmations.\n"
            "Optionally filtered to only include txouts paid to specified addresses.\n"
            "Results are an array of Objects, each of which has:\n"
            "{txid, vout, scriptPubKey, amount, confirmations}");
    }

    int nMinDepth = GetInt(params,0,1);
    int nMaxDepth = GetInt(params,1,-1);

    vector<CTxDestination> vAddress;
    if (params.size() > 2)
    {
        set<CLoMoAddress> setAddress;
        Array inputs = params[2].get_array();
        BOOST_FOREACH(Value& input, inputs)
        {
            CLoMoAddress address(input.get_str());
            if (!address.IsValid())
            {
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY,
                                   string("Invalid LoMoCoin address: ")+input.get_str());
            }
            if (setAddress.count(address))
            {
                throw JSONRPCError(RPC_INVALID_PARAMETER,
                                   string("Invalid parameter, duplicated address: ")+input.get_str());
            }
            setAddress.insert(address);
            vAddress.push_back(address.Get());
        }
    }
    vector<COutput> vOutput;
    
    CheckWalletError(pService->ListUnspents(GetWalletName(strURL),vAddress,nMaxDepth,nMinDepth,vOutput));

    Array results;
    BOOST_FOREACH(const COutput& out, vOutput)
    {
        const CScript& pk = out.txout.scriptPubKey;
        Object entry;
        entry.push_back(Pair("txid",out.txid.GetHex()));
        entry.push_back(Pair("vout", (int)out.n));
        CTxDestination address;
        if (ExtractDestination(pk,address))
        {
            entry.push_back(Pair("address", CLoMoAddress(address).ToString()));
        }
        entry.push_back(Pair("scriptPubKey", HexStr(pk.begin(), pk.end())));
        entry.push_back(Pair("amount",ValueFromAmount(out.txout.nValue)));
        entry.push_back(Pair("confirmations",out.nDepth));
        results.push_back(entry);
    }
    return results;
}

Value CRPCMod::RPCListSinceBlock(const Array& params,bool fHelp)
{
    if (fHelp)
    {
        throw runtime_error(
            "listsinceblock [blockhash] [target-confirmations]\n"
            "Get all transactions in blocks since block [blockhash], or all transactions if omitted");
    }
    uint256 hashStartBlock = 0;
    uint256 hashLastBlock = 0;

    if (params.size() > 0)
    {
        hashStartBlock.SetHex(params[0].get_str());
    }

    int target_confirms = GetInt(params,1,1);
    if (target_confirms < 1)
    {
        throw JSONRPCError(RPC_INVALID_PARAMETER,"Invalid parameter");
    }
   
    int nMinHeight = 0; 
    if (hashStartBlock != 0)
    {
        CAssembledBlock block;
        if (!pService->GetBlock(hashStartBlock,block))
        {
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Block not found in main chain");
        }
        nMinHeight = block.nHeight;
    }

    string strWallet = GetWalletName(strURL);
    int nBestChainHeight = pService->GetBestChainHeight();
    int target_height = nBestChainHeight + 1 - target_confirms;

    pService->GetBlockHash(target_height,hashLastBlock);
    
    vector<uint256> vTxid;
    CheckWalletError(pService->ListTransactions(strWallet,target_confirms,vTxid));
    Array transactions;
    BOOST_FOREACH(const uint256& txid,vTxid)
    {
        CWalletTx wtx;
        list<pair<CTxDestination, int64> > listSent,listReceived;
        if (CheckWalletError(pService->GetWalletTx(strWallet,txid,wtx,listSent,listReceived))
            && wtx.nBlockHeight >= nMinHeight)
        {
            WalletTxDetailsToJSON(wtx,pCoreProtocol->GetCoinbaseMaturity(),
                                      nBestChainHeight,listSent,listReceived,true,transactions);
        }
    }

    Object ret;
    ret.push_back(Pair("transactions", transactions));
    ret.push_back(Pair("lastblock", hashLastBlock.GetHex()));

    return ret;
}

Value CRPCMod::RPCListReceivedByAddress(const Array& params,bool fHelp)
{
    if (fHelp || params.size() > 2)
    {
        throw runtime_error(
            "listreceivedbyaddress [minconf=1] [includeempty=false]\n"
            "[minconf] is the minimum number of confirmations before payments are included.\n"
            "[includeempty] whether to include addresses that haven't received any payments.\n"
            "Returns an array of objects containing:\n"
            "  \"address\" : receiving address\n"
            "  \"account\" : the account of the receiving address\n"
            "  \"amount\" : total amount received by the address\n"
            "  \"confirmations\" : number of confirmations of the most recent transaction included");
    }

    // Minimum confirmations
    int nMinDepth = GetInt(params,0,1);

    // Whether to include empty accounts
    bool fIncludeEmpty = false;
    if (params.size() > 1)
    {
        fIncludeEmpty = params[1].get_bool();
    }

    string strWallet = GetWalletName(strURL);

    map<CTxDestination,pair<int64,int> > mapReceived;
    ListReceived(strWallet,nMinDepth,mapReceived);

    multimap<string,CTxDestination> mapAccount;
    CheckWalletError(pService->ListAccountAddress(strWallet,mapAccount));

    Array ret;

    for (multimap<string,CTxDestination>::iterator it = mapAccount.begin();
         it != mapAccount.end(); ++it)
    {
        CTxDestination& address = (*it).second;
        int64 nAmount = 0;
        int nConf = 0;
        map<CTxDestination,pair<int64,int> >::iterator mi = mapReceived.find(address);
        if (mi != mapReceived.end())
        {
            nAmount = (*mi).second.first;
            nConf = (*mi).second.second;
        }
        else if (!fIncludeEmpty)
        {
            continue;
        }
        Object obj;
        obj.push_back(Pair("address",       CLoMoAddress(address).ToString()));
        obj.push_back(Pair("account",       (*it).first));
        obj.push_back(Pair("amount",        ValueFromAmount(nAmount)));
        obj.push_back(Pair("confirmations", nConf));
        ret.push_back(obj);
    }

    return ret;
}

Value CRPCMod::RPCListReceivedByAccount(const Array& params,bool fHelp)
{
    if (fHelp || params.size() > 2)
    {
        throw runtime_error(
            "listreceivedbyaccount [minconf=1] [includeempty=false]\n"
            "[minconf] is the minimum number of confirmations before payments are included.\n"
            "[includeempty] whether to include accounts that haven't received any payments.\n"
            "Returns an array of objects containing:\n"
            "  \"account\" : the account of the receiving addresses\n"
            "  \"amount\" : total amount received by addresses with this account\n"
            "  \"confirmations\" : number of confirmations of the most recent transaction included");
    }
    // Minimum confirmations
    int nMinDepth = GetInt(params,0,1);

    // Whether to include empty accounts
    bool fIncludeEmpty = false;
    if (params.size() > 1)
    {
        fIncludeEmpty = params[1].get_bool();
    }

    string strWallet = GetWalletName(strURL);

    map<CTxDestination,pair<int64,int> > mapReceived;
    ListReceived(strWallet,nMinDepth,mapReceived);

    multimap<string,CTxDestination> mapAccount;
    CheckWalletError(pService->ListAccountAddress(strWallet,mapAccount));

    map<string,pair<int64,int> > mapAccountReceived;
    for (multimap<string,CTxDestination>::iterator it = mapAccount.begin();
         it != mapAccount.end(); ++it)
    {
        CTxDestination& address = (*it).second;
        map<CTxDestination,pair<int64,int> >::iterator mi = mapReceived.find(address);
        if (mi != mapReceived.end())
        {
            map<string,pair<int64,int> >::iterator i;
            i = mapAccountReceived.insert(make_pair((*it).first,
                                                    make_pair(0,numeric_limits<int>::max()))).first;
            pair<int64,int>& p = (*i).second;
            p.first += (*mi).second.first;
            if (p.second > (*mi).second.second)
            {
                p.second = (*mi).second.second;
            }
        }
        else if (fIncludeEmpty)
        {
            mapAccountReceived.insert(make_pair((*it).first,make_pair(0,numeric_limits<int>::max())));
        }
    }

    Array ret;
    for (map<string,pair<int64,int> >::iterator it = mapAccountReceived.begin();
         it != mapAccountReceived.end();++it)
    {
        int64 nAmount = (*it).second.first;
        int nConf = (*it).second.second;
        Object obj;
        obj.push_back(Pair("account",       (*it).first));
        obj.push_back(Pair("amount",        ValueFromAmount(nAmount)));
        obj.push_back(Pair("confirmations", (nConf == numeric_limits<int>::max() ? 0 : nConf)));
        ret.push_back(obj);
    }
    return ret;
}

Value CRPCMod::RPCGetReceivedByAddress(const Array& params,bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
    {
        throw runtime_error(
            "getreceivedbyaddress <lomocoinaddress> [minconf=1]\n"
            "Returns the total amount received by <lomocoinaddress> in transactions with at least [minconf] confirmations.");
    }

    set<CTxDestination> setAddress;
    CLoMoAddress address(params[0].get_str());
    if (!address.IsValid())
    {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid LoMoCoin address");
    }
    setAddress.insert(address.Get());

    // Minimum confirmations
    int nMinDepth = GetInt(params,1,1);

    return ValueFromAmount(GetReceived(GetWalletName(strURL),setAddress,nMinDepth));
}

Value CRPCMod::RPCGetReceivedByAccount(const Array& params,bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
    {
        throw runtime_error(
            "getreceivedbyaccount <account> [minconf=1]\n"
            "Returns the total amount received by addresses with <account> in transactions with at least [minconf] confirmations.");
    }

    string strAccount = GetAccount(params,0);
    string strWallet = GetWalletName(strURL);
    set<CTxDestination> setAddress;
    GetAccountAddress(strWallet,strAccount,setAddress);

    // Minimum confirmations
    int nMinDepth = GetInt(params,1,1);

    return ValueFromAmount(GetReceived(strWallet,setAddress,nMinDepth));
}

Value CRPCMod::RPCSendToAddress(const Array& params,bool fHelp)
{
    if (fHelp ||  params.size() != 2)
    {
        throw runtime_error(
            "sendtoaddress <lomocoinaddress> <amount>\n"
            "<amount> is a real and is rounded to the nearest 0.000001");
    }

    CLoMoAddress address(params[0].get_str());
    if (!address.IsValid())
    {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid LoMoCoin address");
    }

    // Amount
    int64 nAmount = AmountFromValue(params[1]);
    if (nAmount < MIN_TXOUT_AMOUNT)
    {
        throw JSONRPCError(RPC_INVALID_PARAMETER,"Send amount too small");
    }

    CScript scriptPubKey;
    scriptPubKey.SetDestination(address.Get());
    vector<CTxOut> vecSend;
    vecSend.push_back(CTxOut(nAmount,scriptPubKey));

    CTransaction tx;
    LMCErr err = pService->SendMoney(GetWalletName(strURL),string("*"),1,vecSend,nAmount,tx);
    if (!CheckWalletError(err))
    {
        throw JSONRPCError(RPC_WALLET_ERROR,string("Failed to send : ") + LMCErrString(err));
    }

    return tx.GetHash().GetHex();
}

Value CRPCMod::RPCSendFrom(const Array& params,bool fHelp)
{
    if (fHelp || params.size() < 3 || params.size() > 4)
    {
        throw runtime_error(
            "sendfrom <fromaddress/fromaccount> <tolomocoinaddress> <amount> [minconf=1]\n"
            "<amount> is a real and is rounded to the nearest 0.000001");
    }

    string strFrom = params[0].get_str();

    CLoMoAddress address(params[1].get_str());
    if (!address.IsValid())
    {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid LoMoCoin address");
    }

    // Amount
    int64 nAmount = AmountFromValue(params[2]);
    if (nAmount < MIN_TXOUT_AMOUNT)
    {
        throw JSONRPCError(RPC_INVALID_PARAMETER,"Send amount too small");
    }

    int nMinDepth = GetInt(params,3,1);

    CScript scriptPubKey;
    scriptPubKey.SetDestination(address.Get());
    vector<CTxOut> vecSend;
    vecSend.push_back(CTxOut(nAmount,scriptPubKey));

    CTransaction tx;
    LMCErr err = pService->SendMoney(GetWalletName(strURL),strFrom,nMinDepth,vecSend,nAmount,tx);
    if (!CheckWalletError(err))
    {
        throw JSONRPCError(RPC_WALLET_ERROR,string("Failed to send : ") + LMCErrString(err));
    }

    return tx.GetHash().GetHex();
}

Value CRPCMod::RPCSendMany(const Array& params,bool fHelp)
{
    if (fHelp || params.size() < 2 || params.size() > 3)
    {
        throw runtime_error(
            "sendmany <fromaddress/fromaccount> {address:amount,data:hex,...} [minconf=1]\n"
            "amounts are double-precision floating point numbers");
    }
    string strFrom = params[0].get_str();
    Object sendTo = params[1].get_obj();
    vector<CTxOut> vecSend;
    int64 nValueOut = GetSendToList(sendTo,vecSend);
    int nMinDepth = GetInt(params,3,1);

    CTransaction tx;
    LMCErr err = pService->SendMoney(GetWalletName(strURL),strFrom,nMinDepth,vecSend,nValueOut,tx);
    if (!CheckWalletError(err))
    {
        throw JSONRPCError(RPC_WALLET_ERROR,string("Failed to send : ") + LMCErrString(err));
    }

    return tx.GetHash().GetHex();
}

Value CRPCMod::RPCCreateSendFromAddress(const Array& params,bool fHelp)
{
    if (fHelp || params.size() < 2 || params.size() > 3)
    {
        throw runtime_error(
            "createsendfromaddress <fromaddress> {address:amount,data:hex,...} [minconf=1]\n"
            "Create a transaction spending from given address,\n"
            "sending to given address(es).\n"
            "Returns hex-encoded raw transaction.\n"
            "Note that the transaction's inputs are not signed, and\n"
            "it is not stored in the wallet or transmitted to the network.");
    }
    CLoMoAddress addressFrom(params[0].get_str());
    if (!addressFrom.IsValid())
    {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid LoMoCoin address");
    }
    int nMinDepth = GetInt(params,3,1);

    string strWallet = GetWalletName(strURL);

    CTransaction tx;
    tx.nTime = WalleveGetNetTime();
    Object sendTo = params[1].get_obj();
    int64 nValueOut = GetSendToList(sendTo,tx.vout);
    int64 nValueIn = 0;
    if (!CheckWalletError(pService->SelectUnspent(strWallet,addressFrom.Get(),nValueOut,tx.nTime,
                                                            nMinDepth,tx.vin,nValueIn)))
    {
        throw JSONRPCError(RPC_WALLET_INSUFFICIENT_FUNDS,"Insufficient funds");
    }
    int64 nTxFee;
    CheckWalletError(pService->GetTxFee(strWallet,nTxFee));
    int64 nChange = nValueIn - nValueOut - nTxFee;
    if (nChange < 0)
    {
        throw JSONRPCError(RPC_WALLET_ERROR,"Invalid amount");
    }
    else if (nChange >= MIN_TXOUT_AMOUNT)
    {
        CScript scriptChange;
        scriptChange.SetDestination(addressFrom.Get());
        vector<CTxOut>::iterator position = tx.vout.begin()+GetRandInt(tx.vout.size());
        tx.vout.insert(position, CTxOut(nChange, scriptChange));
    }

    CWalleveBufStream ss;
    ss << tx;
    return HexStr(ss.GetData(),ss.GetData() + ss.GetSize());
}

Value CRPCMod::RPCImportPrivKey(const Array& params,bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
    {
        throw runtime_error(
            "importprivkey <lomocoinprivkey> [account]\n"
            "Adds a private key (as returned by dumpprivkey) to your wallet.");
    }
    string strSecret = params[0].get_str();
    string strAccount = GetAccount(params,1);

    CLoMoSecret vchSecret;
    if (!vchSecret.SetString(strSecret))
    {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY,"Invalid private key");
    }
    CKeyID keyID;
    if (!CheckWalletError(pService->ImportPrivKey(GetWalletName(strURL),strAccount,vchSecret,keyID)))
    {
        throw JSONRPCError(RPC_WALLET_ERROR,"Error adding key to wallet");
    }

    return Value::null;
}

Value CRPCMod::RPCDumpPrivKey(const Array& params,bool fHelp)
{
    if (fHelp || params.size() != 1)
    {
        throw runtime_error(
            "dumpprivkey <lomocoinaddress>\n"
            "Reveals the private key corresponding to <lomocoinaddress>.");
    }

    string strAddress = params[0].get_str();
    CLoMoAddress address;
    if (!address.SetString(strAddress))
    {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid LoMoCoin address");
    }
    CKeyID keyID;
    if (!address.GetKeyID(keyID))
    {
        throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to a key");
    }

    CLoMoSecret vchSecret;
    if (!CheckWalletError(pService->ExportPrivKey(GetWalletName(strURL),keyID,vchSecret)))
    {
        throw JSONRPCError(RPC_WALLET_ERROR,"Private key for address " + strAddress + " is not known");
    }
    return vchSecret.ToString();
}

Value CRPCMod::RPCSignMessage(const Array& params,bool fHelp)
{
    if (fHelp || params.size() != 2)
    {
        throw runtime_error(
            "signmessage <lomocoinaddress> <message>\n"
            "Sign a message with the private key of an address");
    }

    const string strMessageMagic = "LoMoCoin Signed Message:\n";
    string strAddress = params[0].get_str();
    string strMessage = params[1].get_str();

    CLoMoAddress addr(strAddress);
    if (!addr.IsValid())
    {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid address");
    }
    CKeyID keyID;
    if (!addr.GetKeyID(keyID))
    {
        throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to a key");
    }

    CWalleveBufStream ss;
    ss << strMessageMagic;
    ss << strMessage;

    vector<unsigned char> vchSig;
    if (!CheckWalletError(pService->SignSignature(GetWalletName(strURL),keyID,GetHash(ss), vchSig)))
    {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Sign failed");
    }

    return EncodeBase64(&vchSig[0], vchSig.size());
}

Value CRPCMod::RPCVerifyMessage(const Array& params,bool fHelp)
{
    if (fHelp || params.size() != 3)
    {
        throw runtime_error(
            "verifymessage <lomocoinaddress> <signature> <message>\n"
            "Verify a signed message");
    }

    const string strMessageMagic = "LoMoCoin Signed Message:\n";
    string strAddress  = params[0].get_str();
    string strSign     = params[1].get_str();
    string strMessage  = params[2].get_str();

    CLoMoAddress addr(strAddress);
    if (!addr.IsValid())
    {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid address");
    }
    CKeyID keyID;
    if (!addr.GetKeyID(keyID))
    {
        throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to a key");
    }

    bool fInvalid = false;
    vector<unsigned char> vchSig = DecodeBase64(strSign.c_str(), &fInvalid);

    if (fInvalid)
    {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Malformed base64 encoding");
    }

    CWalleveBufStream ss;
    ss << strMessageMagic;
    ss << strMessage;

    CKey key;
    if (!key.SetCompactSignature(GetHash(ss), vchSig))
    {
        return false;
    }

    return (key.GetPubKey().GetID() == keyID);
}

Value CRPCMod::RPCCreateRawTransaction(const Array& params,bool fHelp)
{
    if (fHelp || params.size() < 2 || params.size() > 3)
    {
        throw runtime_error(
            "createrawtransaction [{\"txid\":txid,\"vout\":n},...] {address:amount,data:hex,...} [\"frozentime\"]\n"
            "Create a transaction spending given inputs\n"
            "(array of objects containing transaction id and output number),\n"
            "sending to given address(es).\n"
            "the format of frozentime is year-month-day hour:minute:second\n"
            "Returns hex-encoded raw transaction.\n"
            "Note that the transaction's inputs are not signed, and\n"
            "it is not stored in the wallet or transmitted to the network.");
    }

    Array inputs = params[0].get_array();
    Object sendTo = params[1].get_obj();

    CTransaction rawTx;
    rawTx.nTime = WalleveGetNetTime();

    if (params.size() == 3)
    {
        struct tm tm = DateTimeFromStr(params[2].get_str());
        rawTx.nLockTime = mktime(&tm);
    }

    BOOST_FOREACH(Value& input, inputs)
    {
        const Object& o = input.get_obj();

        const Value& txid_v = find_value(o, "txid");
        if (txid_v.type() != str_type)
        {
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, missing txid key");
        }
        string txid = txid_v.get_str();
        if (!IsHex(txid))
        {
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, expected hex txid");
        }
        const Value& vout_v = find_value(o, "vout");
        if (vout_v.type() != int_type)
        {
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, missing vout key");
        }
        int nOutput = vout_v.get_int();
        if (nOutput < 0)
        {
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, vout must be positive");
        }
        CTxIn in(COutPoint(uint256(txid), nOutput));
        rawTx.vin.push_back(in);
    }

    GetSendToList(sendTo,rawTx.vout);

    CWalleveBufStream ss;
    ss << rawTx;
    return HexStr(ss.GetData(),ss.GetData() + ss.GetSize());
}

Value CRPCMod::RPCDecodeRawTransaction(const Array& params,bool fHelp)
{
    if (fHelp || params.size() != 1)
    {
        throw runtime_error(
            "decoderawtransaction <hex string>\n"
            "Return a JSON object representing the serialized, hex-encoded transaction.");
    }
    vector<unsigned char> txData(ParseHex(params[0].get_str()));
    CWalleveBufStream ss;
    ss.Write((char *)&txData[0],txData.size());
    CTransaction rawTx;
    try
    {
        ss >> rawTx;
    }
    catch (std::exception &e)
    {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "TX decode failed");
    }

    Object result;
    TxToJSON(CAssembledTx(rawTx),result);

    return result;
}

Value CRPCMod::RPCSignRawTransaction(const Array& params,bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 4)
    {
        throw runtime_error(
            "signrawtransaction <hex string> [{\"txid\":txid,\"vout\":n,\"scriptPubKey\":hex,\"redeemScript\":hex},...] [<privatekey1>,...] [sighashtype=\"ALL\"]\n"
            "Sign inputs for raw transaction (serialized, hex-encoded).\n"
            "Second optional argument (may be null) is an array of previous transaction outputs that\n"
            "this transaction depends on but may not yet be in the blockchain.\n"
            "Third optional argument (may be null) is an array of base58-encoded private\n"
            "keys that, if given, will be the only keys used to sign the transaction.\n"
            "Fourth optional argument is a string that is one of six values; ALL, NONE, SINGLE or\n"
            "ALL|ANYONECANPAY, NONE|ANYONECANPAY, SINGLE|ANYONECANPAY.\n"
            "Returns json object with keys:\n"
            "  hex : raw transaction with signature(s) (hex-encoded string)\n"
            "  complete : 1 if transaction has a complete set of signature (0 if not)\n");
    }

    vector<unsigned char> txData(ParseHex(params[0].get_str()));
    CWalleveBufStream ss;
    ss.Write((char *)&txData[0],txData.size());
    vector<CTransaction> txVariants;
    while(ss.GetSize())
    {
        try
        {
            CTransaction tx;
            ss >> tx;
            txVariants.push_back(tx);
        }
        catch (std::exception &e)
        {
            throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "TX decode failed");
        }
    }

    map<COutPoint,CScript> mapPrev;
    CBasicKeyStore tempKeystore;
    if (params.size() > 1 && params[1].type() != null_type)
    {
        Array prevTxs = params[1].get_array();
        BOOST_FOREACH(Value& p, prevTxs)
        {
            if (p.type() != obj_type)
            {
                throw JSONRPCError(RPC_INVALID_PARAMETER,
                                   "expected object with {\"txid'\",\"vout\",\"scriptPubKey\"}");
            }

            Object prevOut = p.get_obj();
            string txidHex = find_value(prevOut, "txid").get_str();
            if (!IsHex(txidHex))
            {
                throw JSONRPCError(RPC_INVALID_PARAMETER, "txid must be hexadecimal");
            }
            uint256 txid;
            txid.SetHex(txidHex);
            int nOut = find_value(prevOut, "vout").get_int();
            if (nOut < 0)
            {
                throw JSONRPCError(RPC_INVALID_PARAMETER, "vout must be positive");
            }
            string pkHex = find_value(prevOut, "scriptPubKey").get_str();
            if (!IsHex(pkHex))
            {
                throw JSONRPCError(RPC_INVALID_PARAMETER, "scriptPubKey must be hexadecimal");
            }
            vector<unsigned char> pkData(ParseHex(pkHex));
            CScript scriptPubKey(pkData.begin(), pkData.end());
            
            mapPrev[COutPoint(txid,nOut)] = scriptPubKey;

            if (scriptPubKey.IsPayToScriptHash())
            {
                Value v = find_value(prevOut, "redeemScript");
                if (!(v == Value::null))
                {
                    vector<unsigned char> rsData(ParseHex(v.get_str()));
                    CScript redeemScript(rsData.begin(), rsData.end());
                    tempKeystore.AddCScript(redeemScript);
                }
            }
        }
    }

    bool fGivenKeys = false;
    if (params.size() > 2 && params[2].type() != null_type)
    {
        fGivenKeys = true;
        Array keys = params[2].get_array();
        BOOST_FOREACH(Value k, keys)
        {
            CLoMoSecret vchSecret;
            if (!vchSecret.SetString(k.get_str()))
            {
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY,"Invalid private key");
            }
            CKey key;
            bool fCompressed;
            CSecret secret = vchSecret.GetSecret(fCompressed);
            key.SetSecret(secret, fCompressed);
            tempKeystore.AddKey(key);
        }
    }

    int nHashType = SIGHASH_ALL;
    if (params.size() > 3 && params[3].type() != null_type)
    {
        static map<string, int> mapSigHashValues =
            boost::assign::map_list_of
            (string("ALL"), int(SIGHASH_ALL))
            (string("ALL|ANYONECANPAY"), int(SIGHASH_ALL|SIGHASH_ANYONECANPAY))
            (string("NONE"), int(SIGHASH_NONE))
            (string("NONE|ANYONECANPAY"), int(SIGHASH_NONE|SIGHASH_ANYONECANPAY))
            (string("SINGLE"), int(SIGHASH_SINGLE))
            (string("SINGLE|ANYONECANPAY"), int(SIGHASH_SINGLE|SIGHASH_ANYONECANPAY))
            ;
        string strHashType = params[3].get_str();
        if (mapSigHashValues.count(strHashType))
        {
            nHashType = mapSigHashValues[strHashType];
        }
        else
        {
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid sighash param");
        }
    }

    CTransaction mergedTx;
    bool fComplete = true;

    // Sign what we can:
    
    LMCErr err = fGivenKeys ? pService->SignTransaction(tempKeystore,txVariants,mapPrev,nHashType,mergedTx,fComplete)
                            : pService->SignTransaction(GetWalletName(strURL),txVariants,mapPrev,nHashType,mergedTx,fComplete);
    if (!CheckWalletError(err))
    {
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Failed to sign transaction");
    }

    Object result;
    CWalleveBufStream ssTx;
    ssTx << mergedTx;
    result.push_back(Pair("hex", HexStr(ssTx.GetData(),ssTx.GetData() + ssTx.GetSize())));
    result.push_back(Pair("complete", fComplete));

    return result;
}

Value CRPCMod::RPCSendRawTransaction(const Array& params,bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
    {
        throw runtime_error(
            "sendrawtransaction <hex string> [checkinputs=1]\n"
            "Submits raw transaction (serialized, hex-encoded) to local node and network.\n"
            "If checkinputs is non-zero, checks the validity of the inputs of the transaction before sending it.");
    }
    vector<unsigned char> txData(ParseHex(params[0].get_str()));
    CWalleveBufStream ss;
    ss.Write((char *)&txData[0],txData.size());
    CTransaction rawTx;
    try
    {
        ss >> rawTx;
    }
    catch (std::exception &e)
    {
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "TX decode failed");
    }

    LMCErr err = pService->SendTransaction(rawTx);
    if (err != LMC_OK)
    {
        throw JSONRPCError(RPC_TRANSACTION_REJECTED,string("Tx rejected : ")
                                                    + LMCErrString(err));
    }

    return rawTx.GetHash().GetHex();
}

#endif
