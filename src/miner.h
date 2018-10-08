// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_MINER_H
#define MULTIVERSE_MINER_H

#include "mvbase.h"
#include "walleve/walleve.h"
#include "json/json_spirit_value.h"

#include <string>
#include <vector>

namespace multiverse
{

class CMinerWork
{
  public:
    std::vector<unsigned char> vchWorkData;
    uint256 hashPrev;
    int64 nPrevTime;
    int nAlgo;
    int nBits;

  public:
    CMinerWork() { SetNull(); }
    void SetNull()
    {
        vchWorkData.clear();
        hashPrev = 0;
        nPrevTime = 0;
        nAlgo = 0;
        nBits = 0;
    }
    bool IsNull() const { return (nAlgo == 0); }
};

class CMiner : public walleve::IIOModule, virtual public walleve::CWalleveHttpEventListener
{
  public:
    CMiner(const std::vector<std::string> &vArgsIn);
    ~CMiner();

  protected:
    bool WalleveHandleInitialize() override;
    void WalleveHandleDeinitialize() override;
    bool WalleveHandleInvoke() override;
    void WalleveHandleHalt() override;
    const CMvRPCClientConfig *WalleveConfig();
    bool Interrupted() { return (nMinerStatus != MINER_RUN); }
    bool HandleEvent(walleve::CWalleveEventHttpGetRsp &event) override;
    bool SendRequest(uint64 nNonce, const string &content);
    bool GetWork();
    bool SubmitWork(const std::vector<unsigned char> &vchWorkData);
    void CancelRPC();
    uint256 GetHashTarget(const CMinerWork &work, int64 nTime);

  private:
    enum
    {
        MINER_RUN = 0,
        MINER_RESET = 1,
        MINER_EXIT = 2,
        MINER_HOLD = 3
    };
    void LaunchFetcher();
    void LaunchMiner();

  protected:
    walleve::IIOProc *pHttpGet;
    walleve::CWalleveThread thrFetcher;
    walleve::CWalleveThread thrMiner;
    boost::mutex mutex;
    boost::condition_variable condFetcher;
    boost::condition_variable condMiner;
    std::string strAddrSpent;
    std::string strMintKey;
    int nMinerStatus;
    CMinerWork workCurrent;
    uint64 nNonceGetWork;
    uint64 nNonceSubmitWork;
};

} // namespace multiverse
#endif //MULTIVERSE_MINER_H
