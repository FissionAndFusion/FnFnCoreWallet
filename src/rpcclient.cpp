// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "rpcclient.h"

#include "json/json_spirit_reader_template.h"
#include "json/json_spirit_writer_template.h"
#include "json/json_spirit_utils.h"

#include <readline/readline.h>
#include <readline/history.h>

using namespace std;
using namespace multiverse;
using namespace walleve;
using namespace json_spirit;
using boost::asio::ip::tcp;

static void Initialize_ReadLine();
extern void MvShutdown();
///////////////////////////////
// CRPCDispatch

CRPCDispatch::CRPCDispatch()
: IIOModule("rpcdispatch"),
  thrDispatch("rpcconsole",boost::bind(&CRPCDispatch::LaunchConsole,this))
{
    nLastNonce = 0;
    pHttpGet = NULL;
}

CRPCDispatch::CRPCDispatch(const vector<string>& vArgsIn)
: IIOModule("rpcdispatch"),
  thrDispatch("rpccommand",boost::bind(&CRPCDispatch::LaunchCommand,this)),
  vArgs(vArgsIn)
{
    nLastNonce = 0;
    pHttpGet = NULL;
}

CRPCDispatch::~CRPCDispatch()
{
}

bool CRPCDispatch::WalleveHandleInitialize()
{
    if (!WalleveGetObject("httpget",pHttpGet))
    {
        cerr << "Failed to request httpget\n";
        return false;
    }
    return true;
}

void CRPCDispatch::WalleveHandleDeinitialize()
{
    pHttpGet = NULL;
}

bool CRPCDispatch::WalleveHandleInvoke()
{
    if (!WalleveThreadDelayStart(thrDispatch))
    {
        return false;
    }
    return IIOModule::WalleveHandleInvoke();
}

void CRPCDispatch::WalleveHandleHalt()
{
    IIOModule::WalleveHandleHalt();

    if (thrDispatch.IsRunning())
    {
        CancelCommand();
        thrDispatch.Interrupt();
    }
    thrDispatch.Exit();
}

const CMvRPCConfig * CRPCDispatch::WalleveConfig()
{
    return dynamic_cast<const CMvRPCConfig *>(IWalleveBase::WalleveConfig());
}

bool CRPCDispatch::HandleEvent(CWalleveEventHttpGetRsp& event)
{
    try
    {
        CWalleveHttpRsp& rsp = event.data;

        if (rsp.nStatusCode < 0)
        {
            
            const char * strErr[] = {"","connect failed","invalid nonce","activate failed",
                                     "disconnected","no response","resolve failed",
                                     "internal failure","aborted"};
            throw runtime_error(rsp.nStatusCode >= HTTPGET_ABORTED ? 
                                strErr[-rsp.nStatusCode] : "unknown error");
        }
        if (rsp.nStatusCode == 401)
        {
            throw runtime_error("incorrect rpcuser or rpcpassword (authorization failed)");
        }
        else if (rsp.nStatusCode > 400 && rsp.nStatusCode != 404 && rsp.nStatusCode != 500)
        {
            ostringstream oss;
            oss << "server returned HTTP error " << rsp.nStatusCode;
            throw runtime_error(oss.str());
        }
        else if (rsp.strContent.empty())
        {
            throw runtime_error("no response from server");
        }

        // Parse reply
        Value valReply;
        if (!read_string(rsp.strContent, valReply))
        {
            throw runtime_error("couldn't parse reply from server");
        }
        const Object& reply = valReply.get_obj();
        if (reply.empty())
        {
            throw runtime_error("expected reply to have result, error and id properties");
        }

        // Parse reply
        const Value& result = find_value(reply, "result");
        const Value& error  = find_value(reply, "error");

        if (error.type() == null_type)
        {
            // Result
            if (result.type() == str_type)
            {
                cout << result.get_str() << "\n";
            }
            else if (result.type() != null_type)
            {
                cout << write_string(result, true) << "\n";
            }
            ioComplt.Completed(true);
            return true;
        }
        else
        {
            // Error
            cerr << "error: " << write_string(error, false) << "\n";
        }
    }
    catch (const std::exception& e)
    {
        cerr << "error: " << e.what() << "\n";
    }
    catch (...)
    {
        cerr << "unknown exception\n";
    }
    ioComplt.Completed(false);
    return true;
}

bool CRPCDispatch::GetResponse(uint64 nNonce,const string& strWallet,Object& jsonReq)
{
    CWalleveEventHttpGet eventHttpGet(nNonce);
    CWalleveHttpGet& httpGet = eventHttpGet.data;
    httpGet.strIOModule = WalleveGetOwnKey();
    httpGet.nTimeout = WalleveConfig()->nRPCConnectTimeout;;
    if (WalleveConfig()->fRPCSSLEnable)
    {
        httpGet.strProtocol = "https";
        httpGet.fVerifyPeer = true;
        httpGet.strPathCA   = WalleveConfig()->strRPCCAFile;
        httpGet.strPathCert = WalleveConfig()->strRPCCertFile;
        httpGet.strPathPK   = WalleveConfig()->strRPCPKFile;
    }
    else
    {
        httpGet.strProtocol = "http";
    }

    CNetHost host(WalleveConfig()->strRPCConnect,WalleveConfig()->nRPCPort);
    httpGet.mapHeader["host"] = host.ToString();
    httpGet.mapHeader["url"] = string("/") + strWallet;
    httpGet.mapHeader["method"] = "POST";
    httpGet.mapHeader["accept"] = "application/json";
    httpGet.mapHeader["content-type"] = "application/json";
    httpGet.mapHeader["user-agent"] = string("multivers-json-rpc/");
    httpGet.mapHeader["connection"] = "Keep-Alive";
    if (!WalleveConfig()->strRPCPass.empty() || !WalleveConfig()->strRPCUser.empty())
    {
        string strAuth;
        CHttpUtil().Base64Encode(WalleveConfig()->strRPCUser + ":" + WalleveConfig()->strRPCPass,strAuth);
        httpGet.mapHeader["authorization"] = string("Basic ") + strAuth;
    }

    httpGet.strContent = write_string(Value(jsonReq), false) + "\n";
    
    ioComplt.Reset();

    if (!pHttpGet->DispatchEvent(&eventHttpGet))
    {
        throw runtime_error("failed to send json request");
    }
    bool fResult = false;
    return (ioComplt.WaitForComplete(fResult) && fResult);
}

bool CRPCDispatch::CallRPC(const vector<string>& vCommand,const string& strWallet,int nReqId)
{
    string strMethod;
    Array params;
    try
    {
        vector<string>::const_iterator it = vCommand.begin();
        strMethod = (*it);
        while (++it != vCommand.end())
        {
            params.push_back(GetParamValue((*it)));
        }

        Object request;
        request.push_back(Pair("method", strMethod));
        request.push_back(Pair("params", params));
        request.push_back(Pair("id", nReqId));
        return GetResponse(1,strWallet,request);
    }
    catch (const std::exception& e)
    {
        cerr << "error: " << e.what() << "\n";
    }
    catch (...)
    {
        cerr << "unknown exception\n";
    }
    return false;
}

bool CRPCDispatch::CallConsoleCommand(const vector<std::string>& vCommand,string& strWallet)
{
    if (vCommand[0] == "getwallet")
    {
        if (strWallet.empty())
        {
            cout << "main\n";
        }
        else
        {
            cout << strWallet << "\n";
        }
    }
    else if (vCommand[0] == "setwallet")
    {
        strWallet = vCommand.size() > 1 && vCommand[1] != "main" ? vCommand[1] : "";
    }
    else if (vCommand[0] == "help")
    {
        if (vCommand.size() == 1)
        {
            cout << "getwallet\n" << "setwallet [\"walletname\"=\"\"]\n" << "quit\n";
            CallRPC(vCommand,strWallet,++nLastNonce);
        }
        else if (vCommand[1] == "getwallet")
        {
            cout << "getwallet\nReturns current wallet name.\n";
        }
        else if (vCommand[1] == "setwallet")
        {
            cout << "setwallet [\"walletname\"=\"\"]\nSets current wallet.\n";
        }
        else if (vCommand[1] == "quit")
        {
            cout << "quit\n Quits this console.(CTRL-D)\n";
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
    return true;
}

void CRPCDispatch::LaunchConsole()
{
    const char* prompt = "multivers> ";
    char *line = NULL;
    Initialize_ReadLine();
    string strWallet = WalleveConfig()->strRPCWallet;

    while ((line = readline(prompt)))
    {
        if (strncmp(line,"quit",4) == 0)
        {
            break;
        }
        vector<string> vCommand;
        string strLine(line);
        size_t start,stop = 0;
        while ((start = strLine.find_first_not_of(" ",stop)) != string::npos)
        {
            stop = strLine.find(" ",start);
            if (stop == string::npos)
            {
                vCommand.push_back(strLine.substr(start));
                break;
            }
            else
            {
                vCommand.push_back(strLine.substr(start,stop - start));
            }
        }
        if (!vCommand.empty())
        {
            add_history(line);

            if (!CallConsoleCommand(vCommand,strWallet))
            {
                CallRPC(vCommand,strWallet,++nLastNonce);
            }
        }
    }
    if (!line)
    {
       cout << "\n";
    }
    MvShutdown();
}

void CRPCDispatch::LaunchCommand()
{
    CallRPC(vArgs,WalleveConfig()->strRPCWallet);
    MvShutdown();
}

void CRPCDispatch::CancelCommand()
{
    if (pHttpGet)
    {
        CWalleveEventHttpAbort eventAbort(0);
        CWalleveHttpAbort& httpAbort = eventAbort.data;
        httpAbort.strIOModule = WalleveGetOwnKey();
        httpAbort.vNonce.push_back(1);
        pHttpGet->DispatchEvent(&eventAbort);
    }
}

Value CRPCDispatch::GetParamValue(const string& strParam)
{
    Value v,value(strParam);
    if (!read_string(value.get_str(), v))
    {
        return value;
    }

    if (strParam[0] == '[')
    {
        return v.get_array();
    }
    else if (strParam[0] == '{')
    {
        return v.get_obj();
    }
    else if (strcasecmp(strParam.c_str(),"true") == 0
             || strcasecmp(strParam.c_str(),"false") == 0)
    {
        return v.get_value<bool>();
    }
    else if (strParam.find_first_not_of("0123456789.-+") == string::npos)
    {
        size_t pos = strParam.find_first_of('.');
        if (pos == string::npos)
        {
            return v.get_value<boost::int64_t>();
        }
        else if (pos == strParam.find_last_of('.'))
        {
            return v.get_value<double>();
        }
    }

    if (value.type() != str_type)
    {
        throw runtime_error("type mismatch");
    }
    return value;
}



///////////////////////////////
// readline
static const char *rpccmd[] =
{
    "addcoldmintingaddress",
    "addnewwallet",
    "addnode",
    "addmultisigaddress",
    "createrawtransaction",
    "createsendfromaddress",
    "decoderawtransaction",
    "deladdressbook",
    "dumpprivkey",
    "encryptwallet",
    "getaccount",
    "getaccountaddress",
    "getaddressbook",
    "getaddressesbyaccount",
    "getaddressinfo",
    "getbalance",
    "getblock",
    "getblockcount",
    "getblockhash",
    "getconnectioncount",
    "getinfo",
    "getmininginfo",
    "getnewaddress",
    "getpeerinfo",
    "getrawmempool",
    "getrawtransaction",
    "getreceivedbyaccount",
    "getreceivedbyaddress",
    "gettransaction",
    "help",
    "importprivkey",
    "listaccounts",
    "listaddressbook",
    "listreceivedbyaccount",
    "listreceivedbyaddress",
    "listsinceblock",
    "listtransactions",
    "listunspent",
    "listwallet",
    "makekeypair",
    "sendfrom",
    "sendmany",
    "sendrawtransaction",
    "sendtoaddress",
    "setaccount",
    "setaddressbook",
    "settxfee",
    "signmessage",
    "signrawtransaction",
    "stop",
    "validateaddress",
    "verifymessage",
    "walletexport",
    "walletimport",
    "walletlock",
    "walletpassphrase",
    "walletpassphrasechange",
    "getwallet",
    "setwallet",
    NULL,
};

static char * RPCCommand_Generator(const char *text,int state)
{
    static int listIndex,len;
    if (!state)
    {
        listIndex = 0;
        len = strlen(text);
    }

    const char *cmd;
    while((cmd = rpccmd[listIndex]))
    {
        listIndex++;
        if (strncmp(cmd,text,len) == 0)
        {
            char *r = (char*)malloc(strlen(cmd) + 1);
            strcpy(r,cmd);
            return r;
        }
    }
    return NULL;
}

static char ** RPCCommand_Completion(const char *text,int start,int end)
{
    char **matches = NULL;
    if (start == 0)
    {
        matches = rl_completion_matches(text,RPCCommand_Generator);
    }
    return matches;
}

static void Initialize_ReadLine()
{
    rl_attempted_completion_function = RPCCommand_Completion;
}
