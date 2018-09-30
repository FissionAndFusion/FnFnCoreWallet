// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "rpcclient.h"

#include "json/json_spirit_reader_template.h"
#include "json/json_spirit_writer_template.h"
#include "json/json_spirit_utils.h"

#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <readline/readline.h>
#include <readline/history.h>


using namespace std;
using namespace multiverse;
using namespace walleve;
using namespace json_spirit;
using namespace multiverse::rpc;
using boost::asio::ip::tcp;

static void Initialize_ReadLine();
extern void MvShutdown();

static string LocalCommandUsage(string command = "")
{
    ostringstream oss;
    if (command == "")
    {
        oss << "Local Command:\n";
    }
    if (command == "" || command == "getwallet")
    {
        oss << "  getwallet\t\t\tReturns current wallet name.\n";
    }
    if (command == "" || command == "setwallet")
    {
        oss << "  setwallet [\"walletname\"=\"\"]\tSets current wallet.\n";
    }
    if (command == "" || command == "quit")
    {
        oss << "  quit\t\t\t\tQuits this console.(CTRL-C) or (CTRL-D)\n";
    }
    return oss.str();
}

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

const CMvRPCClientConfig * CRPCDispatch::WalleveConfig()
{
    return dynamic_cast<const CMvRPCClientConfig *>(IWalleveBase::WalleveConfig());
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
        if (WalleveConfig()->fDebug)
        {
            cout << "response: " << rsp.strContent;
        }

        auto spResp = DeserializeCRPCResp("", rsp.strContent);
        if (spResp->IsError())
        {
            // Error
            cerr << spResp->spError->Serialize() << endl;
            cerr << strHelpTips << endl;
        }
        else if (spResp->IsSuccessful())
        {
            cout << spResp->spResult->Serialize() << endl;
        }
        else
        {
            cerr << "server error: neigher error nor result. resp: " << spResp->Serialize() << endl;
        }
    }
    catch (exception& e)
    {
        cerr << "error: " << e.what() << "\n";
    }

    ioComplt.Completed(false);
    return true;
}

bool CRPCDispatch::GetResponse(uint64 nNonce,const string& strWallet,const std::string& content)
{
    if (WalleveConfig()->fDebug)
    {
        cout << "request: " << content << endl;
    }

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
    httpGet.mapHeader["user-agent"] = string("multiverse-json-rpc/");
    httpGet.mapHeader["connection"] = "Keep-Alive";
    if (!WalleveConfig()->strRPCPass.empty() || !WalleveConfig()->strRPCUser.empty())
    {
        string strAuth;
        CHttpUtil().Base64Encode(WalleveConfig()->strRPCUser + ":" + WalleveConfig()->strRPCPass,strAuth);
        httpGet.mapHeader["authorization"] = string("Basic ") + strAuth;
    }

    httpGet.strContent = content + "\n";
    
    ioComplt.Reset();

    if (!pHttpGet->DispatchEvent(&eventHttpGet))
    {
        throw runtime_error("failed to send json request");
    }
    bool fResult = false;
    return (ioComplt.WaitForComplete(fResult) && fResult);
}

bool CRPCDispatch::CallRPC(CRPCParamPtr spParam, const string& strWallet,int nReqId)
{
    try
    {
        CRPCReqPtr spReq = MakeCRPCReqPtr(nReqId, spParam->Method(), spParam);
        return GetResponse(1, strWallet, spReq->Serialize());
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
            cout << LocalCommandUsage() << endl;
            return false;
        }
        else
        {
            string usage = LocalCommandUsage(vCommand[1]);
            if (usage.empty())
            {
                return false;
            }
            cout << usage << endl;
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
    const char* prompt = "multiverse> ";
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

        // parse command line input
        // part 1: Parse (blank,',")
        // part 2: If part 1 is blank, part 2 is any charactor besides (blank,',"). 
        //         if part 1 is (' or "), part 2 is any charactor besides 0 or even \ + (' or "), \ is escape character
        // part 3: consume the tail of match.
        boost::regex e("[ \t]*((?<quote>['\"])|[ \t]*)"
            "((?(<quote>).*?(?=((?<none>[^\\\\])|(?<even>([\\\\]{2})+))\\k<quote>)(\\k<none>|\\k<even>)|[^ \t'\\\"]+))"
            "(?(<quote>)\\k<quote>|([ \t]+|$))", boost::regex::perl);
        boost::sregex_iterator it(strLine.begin(), strLine.end(), e);
        boost::sregex_iterator end;
        for (; it != end; it++)
        {
            string str = (*it)[3];
            boost::replace_all(str, "\\\"", "\"");
            boost::replace_all(str, "\\'", "'");
            vCommand.push_back(str);
        }

        if (WalleveConfig()->fDebug)
        {
            cout << "command : " ;
            for (auto& x: vCommand)
            {
                cout << x << ',';
            }
            cout << endl;
        }

        if (!vCommand.empty())
        {
            add_history(line);

            if (!CallConsoleCommand(vCommand,strWallet))
            {
                try
                {
                    CMvConfig config;

                    char* argv[vCommand.size() + 1];
                    argv[0] = const_cast<char*>("multiverse-cli");
                    for (int i = 0; i < vCommand.size(); ++i)
                    {
                        argv[i + 1] = const_cast<char*>(vCommand[i].c_str());
                    }

                    if (!config.Load(vCommand.size() + 1, argv, "", "") || !config.PostLoad())
                    {
                        continue;
                    }

                    // help
                    if (config.GetConfig()->fHelp)
                    {
                        cout << config.Help() << endl;
                        continue;
                    }

                    // call rpc
                    CRPCParam* param = dynamic_cast<CRPCParam*>(config.GetConfig());
                    if (param != NULL)
                    {
                        // avoid delete global point
                        CRPCParamPtr spParam(param, [](CRPCParam* p) {});
                        CallRPC(spParam, WalleveConfig()->strRPCWallet, ++nLastNonce);
                    }
                    else
                    {
                        cerr << "Unknown command" << endl;
                    }
                }
                catch (CRPCException& e)
                {
                    cerr << e.strMessage + strHelpTips << endl;
                }
                catch (exception& e)
                {
                    cerr << e.what() << endl;
                }
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
    const CRPCParam* param = dynamic_cast<const CRPCParam*>(IWalleveBase::WalleveConfig());
    if (param != NULL)
    {
        // avoid delete global point
        CRPCParamPtr spParam(const_cast<CRPCParam*>(param), [](CRPCParam* p) {});
        CallRPC(spParam, WalleveConfig()->strRPCWallet, nLastNonce);
    }
    else
    {
        cerr << "Unknown command" << endl;
    }
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

static char * RPCCommand_Generator(const char *text,int state)
{
    static int listIndex,len;
    if (!state)
    {
        listIndex = 0;
        len = strlen(text);
    }

    auto& list = RPCCmdList();
    for (; listIndex < list.size(); )
    {
        const char* cmd = list[listIndex].c_str();
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
    (void)end;
    char **matches = NULL;
    if (start == 0)
    {
        matches = rl_completion_matches(text,RPCCommand_Generator);
    }
    return matches;
}

static void Initialize_ReadLine()
{
    rl_catch_signals = 0;
    rl_attempted_completion_function = RPCCommand_Completion;
}
