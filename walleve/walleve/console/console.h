// Copyright (c) 2016-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  WALLEVE_CONSOLE_H
#define  WALLEVE_CONSOLE_H

#include "walleve/base/base.h"
#include "walleve/netio/ioproc.h"
#include <string>
#include <boost/asio.hpp>

#ifdef WIN32
    typedef boost::asio::windows::stream_handle stream_desc;
#else
    typedef boost::asio::posix::stream_descriptor stream_desc;
#endif

namespace walleve
{

class CConsole : public IWalleveBase
{
public:
    CConsole(const std::string& walleveOwnKeyIn,const std::string& strPromptIn);
    virtual ~CConsole();
    bool DispatchEvent(CWalleveEvent* pEvent) override;
    void DispatchLine(const std::string& strLine); 
    void DispatchOutput(const std::string& strOutput); 
    
protected:
    bool WalleveHandleInvoke() override;
    void WalleveHandleHalt() override;
    bool InstallReadline(const std::string& strPrompt);
    void UninstallReadline();
    static void ReadlineCallback(char *line); 
    virtual void EnterLoop();
    virtual void LeaveLoop();
    virtual bool HandleLine(const std::string& strLine);

private:
    void WaitForChars();
    void HandleRead(boost::system::error_code const& err,size_t nTransferred);
    void ConsoleThreadFunc();
    void ConsoleHandleEvent(CWalleveEvent* pEvent,CIOCompletion& compltHandle);
    void ConsoleHandleLine(const std::string& strLine);
    void ConsoleHandleOutput(const std::string& strOutput);
public:
    static CConsole* pCurrentConsole;
    static std::mutex mutexConsole;
private:
    CWalleveThread thrConsole;
    std::string strPrompt;
    std::string strLastHistory;
    boost::asio::io_service ioService;
    boost::asio::io_service::strand ioStrand;
    stream_desc inStream;
    boost::asio::null_buffers bufReadNull;
};

} // namespace walleve

#endif //WALLEVE_CONSOLE_H


