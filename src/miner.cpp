// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "miner.h"

#include "json/json_spirit_reader_template.h"
#include "json/json_spirit_writer_template.h"
#include "json/json_spirit_utils.h"

#include "rpc/rpc.h"

using namespace std;
using namespace multiverse;
using namespace walleve;
using namespace json_spirit;
using boost::asio::ip::tcp;
using namespace multiverse::rpc;

extern void MvShutdown();

///////////////////////////////
// CMiner

CMiner::CMiner(const vector<string>& vArgsIn)
: IIOModule("miner"),
  thrFetcher("fetcher",boost::bind(&CMiner::LaunchFetcher,this)),
  thrMiner("miner",boost::bind(&CMiner::LaunchMiner,this))
{
    nNonceGetWork = 1;
    nNonceSubmitWork = 2;
    pHttpGet = NULL;
    if (vArgsIn.size() >= 3)
    {
        strAddrSpent = vArgsIn[1];
        strMintKey = vArgsIn[2];
    } 
}

CMiner::~CMiner()
{
}

bool CMiner::WalleveHandleInitialize()
{
    if (!WalleveGetObject("httpget",pHttpGet))
    {
        cerr << "Failed to request httpget\n";
        return false;
    }
    return true;
}

void CMiner::WalleveHandleDeinitialize()
{
    pHttpGet = NULL;
}

bool CMiner::WalleveHandleInvoke()
{
    if (strAddrSpent.empty())
    {
        cerr << "Invalid spent address\n";
        return false;
    }
    if (strMintKey.empty())
    {
        cerr << "Invalid mint key\n";
        return false;
    }
    if (!WalleveThreadDelayStart(thrFetcher))
    {
        return false;
    }
    if (!WalleveThreadDelayStart(thrMiner))
    {
        return false;
    }
    nMinerStatus = MINER_RUN;
    return IIOModule::WalleveHandleInvoke();
}

void CMiner::WalleveHandleHalt()
{
    IIOModule::WalleveHandleHalt();

    {
        boost::unique_lock<boost::mutex> lock(mutex);
        nMinerStatus = MINER_EXIT;
    }
    condFetcher.notify_all();
    condMiner.notify_all();
   
    if (thrFetcher.IsRunning())
    {
        CancelRPC();
        thrFetcher.Interrupt();
    }
    thrFetcher.Exit();

    if (thrMiner.IsRunning())
    {
        thrMiner.Interrupt();
    }
    thrMiner.Exit();
}

const CMvRPCClientConfig * CMiner::WalleveConfig()
{
    return dynamic_cast<const CMvRPCClientConfig *>(IWalleveBase::WalleveConfig());
}

bool CMiner::HandleEvent(CWalleveEventHttpGetRsp& event)
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
        if (event.nNonce == nNonceGetWork) 
        {
            auto spResp = DeserializeCRPCResp<CGetWorkResult>(rsp.strContent);
            if (spResp->IsError())
            {
                // Error
                cerr << spResp->spError->Serialize(true) << endl;
                cerr << strHelpTips << endl;
            }
            else if (spResp->IsSuccessful())
            {
                auto spResult = CastResultPtr<CGetWorkResult>(spResp->spResult);
                cout << spResp->spResult->Method() << endl;
                if (spResult->fResult.IsValid())
                {
                    if (!spResult->fResult)
                    {
                        {
                            boost::unique_lock<boost::mutex> lock(mutex);

                            workCurrent.SetNull();
                            nMinerStatus = MINER_HOLD;
                        }
                        condMiner.notify_all(); 
                    }
                }
                else if (spResult->work.IsValid())
                {
                    {
                        boost::unique_lock<boost::mutex> lock(mutex);

                        workCurrent.hashPrev.SetHex(spResult->work.strPrevblockhash);
                        workCurrent.nPrevTime = spResult->work.nPrevblocktime;
                        workCurrent.nAlgo = spResult->work.nAlgo;
                        workCurrent.nBits = spResult->work.nBits;
                        workCurrent.vchWorkData = ParseHexString(spResult->work.strData);

                        nMinerStatus = MINER_RESET;
                    }
                    condMiner.notify_all(); 
                }
            }
            else
            {
                cerr << "server error: neither error nor result. resp: " << spResp->Serialize(true) << endl;
            }
        } 
        else if (event.nNonce == nNonceSubmitWork)
        {
            auto spResp = DeserializeCRPCResp<CSubmitWorkResult>(rsp.strContent);
            if (spResp->IsError())
            {
                // Error
                cerr << spResp->spError->Serialize(true) << endl;
                cerr << strHelpTips << endl;
            }
            else if (spResp->IsSuccessful())
            {
                auto spResult = CastResultPtr<CSubmitWorkResult>(spResp->spResult);
                if (spResult->strHash.IsValid())
                {
                    cout << "Submited new block : " << spResult->strHash << "\n";
                }
            }
            else
            {
                cerr << "server error: neither error nor result. resp: " << spResp->Serialize(true) << endl;
            }
        }
        return true;
    }
    catch (const std::exception& e)
    {
        cerr << "error: " << e.what() << "\n";
    }
    catch (...)
    {
        cerr << "unknown exception\n";
    }
    return true;
}

bool CMiner::SendRequest(uint64 nNonce, const string& content)
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
    httpGet.mapHeader["url"] = string("/");
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
    
    return pHttpGet->DispatchEvent(&eventHttpGet);
}

bool CMiner::GetWork()
{
    try
    {
    //    nNonceGetWork += 2;
        auto spParam = MakeCGetWorkParamPtr(workCurrent.hashPrev.GetHex());
        CRPCReqPtr spReq = MakeCRPCReqPtr(nNonceGetWork, spParam);
        return SendRequest(nNonceGetWork, spReq->Serialize());
    }
    catch (...)
    {
        cerr << "getwork exception\n";
    }
    return false;
}

bool CMiner::SubmitWork(const vector<unsigned char>& vchWorkData)
{
    Array params;
    try
    {
    //    nNonceSubmitWork += 2;
        auto spParam = MakeCSubmitWorkParamPtr();
        spParam->strData = ToHexString(vchWorkData);
        spParam->strSpent = strAddrSpent;
        spParam->strPrivkey = strMintKey;

        CRPCReqPtr spReq = MakeCRPCReqPtr(nNonceSubmitWork, spParam);
        return SendRequest(nNonceSubmitWork, spReq->Serialize());
    }
    catch (...)
    {
        cerr << "submitwork exception\n";
    }
    return false;
}

void CMiner::CancelRPC()
{
    if (pHttpGet)
    {
        CWalleveEventHttpAbort eventAbort(0);
        CWalleveHttpAbort& httpAbort = eventAbort.data;
        httpAbort.strIOModule = WalleveGetOwnKey();
        httpAbort.vNonce.push_back(nNonceGetWork);
        httpAbort.vNonce.push_back(nNonceSubmitWork);
        pHttpGet->DispatchEvent(&eventAbort);
    }
}

uint256 CMiner::GetHashTarget(const CMinerWork& work,int64 nTime)
{
    int64 nPrevTime = work.nPrevTime;
    int nBits = work.nBits;

    if (nTime - nPrevTime < BLOCK_TARGET_SPACING)
    {
        return (nBits + 1);
    }

    nBits -= (nTime - nPrevTime - BLOCK_TARGET_SPACING) / PROOF_OF_WORK_DECAY_STEP;
    if (nBits < 16)
    {
        nBits = 16;
    } 
    return ((~uint256(0) >> nBits));
}

void CMiner::LaunchFetcher()
{
    while (nMinerStatus != MINER_EXIT) 
    {
        if (!GetWork())
        {
            cerr << "Failed to getwork\n";
        }
        boost::system_time const timeout = boost::get_system_time() + boost::posix_time::seconds(30);
        {
            boost::unique_lock<boost::mutex> lock(mutex);
            while(nMinerStatus != MINER_EXIT)
            {
                if (!condFetcher.timed_wait(lock,timeout) || nMinerStatus == MINER_HOLD)
                {
                    break;
                }
            }
        }
    }    
}

void CMiner::LaunchMiner()
{
    while (nMinerStatus != MINER_EXIT) 
    {
        CMinerWork work; 
        {
            boost::unique_lock<boost::mutex> lock(mutex);
            while (nMinerStatus == MINER_HOLD)
            {
                condMiner.wait(lock);
            }
            if (nMinerStatus == MINER_EXIT)
            {
                break;
            }
            if (workCurrent.IsNull() || workCurrent.nAlgo != CM_BLAKE512)
            {
                nMinerStatus = MINER_HOLD;
                continue;
            }
            work = workCurrent;
            nMinerStatus = MINER_RUN;
        }

        uint32& nTime = *((uint32*)&work.vchWorkData[4]);
        uint256& nNonce = *((uint256*)&work.vchWorkData[work.vchWorkData.size() - 32]);

        if (work.nAlgo == CM_BLAKE512)
        {
            cout << "Get blake512 work,prev block hash : " << work.hashPrev.GetHex() << "\n";
            uint256 hashTarget = GetHashTarget(work,nTime); 
            while (nMinerStatus == MINER_RUN)
            {
                int64 t = GetTime();
                if (t > nTime)
                {
                    nTime = t;
                    hashTarget = GetHashTarget(work,t);
                }
                for (int i = 0; i < 100 * 1024;i++,nNonce++)
                {
                    uint256 hash = crypto::CryptoHash(&work.vchWorkData[0],work.vchWorkData.size());
                    if (hash <= hashTarget)
                    {
                        cout << "Proof-of-work found\n hash : " << hash.GetHex() << "\ntarget : " << hashTarget.GetHex() << "\n";
                        if (!SubmitWork(work.vchWorkData))
                        {
                            cerr << "Failed to submit work\n";
                        }
                        boost::unique_lock<boost::mutex> lock(mutex);
                        if (nMinerStatus == MINER_RUN)
                        {
                            nMinerStatus = MINER_HOLD;
                        }
                        break;
                    }
                }
            }
            condFetcher.notify_all();
        }
    }
}

