// Copyright (c) 2016-2017 The LoMoCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "console.h"
#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <readline/readline.h>
#include <readline/history.h>

#ifdef WIN32
#include <io.h>
#endif

using namespace std;
using namespace walleve;

///////////////////////////////
// CConsole

CConsole* CConsole::pCurrentConsole = NULL;
boost::mutex CConsole::mutexConsole;

CConsole::CConsole(const string& walleveOwnKeyIn,const string& strPromptIn)
: IWalleveBase(walleveOwnKeyIn),
  thrConsole(walleveOwnKeyIn,boost::bind(&CConsole::ConsoleThreadFunc,this)),strPrompt(strPromptIn),
  ioStrand(ioService),
#ifdef WIN32
  inStream(ioService, GetStdHandle(STD_INPUT_HANDLE))
#else  
  inStream(ioService,::dup(STDIN_FILENO))
#endif
{
}

CConsole::~CConsole()
{
    UninstallReadline();
}

bool CConsole::DispatchEvent(CWalleveEvent* pEvent)
{
    bool fResult = false;
    CIOCompletion complt;
    try
    {
        ioStrand.dispatch(boost::bind(&CConsole::ConsoleHandleEvent,this,pEvent,boost::ref(complt)));
        complt.WaitForComplete(fResult);
    }
    catch (exception& e)
    {
        StdError(__PRETTY_FUNCTION__, e.what());
        return false;
    }
    return fResult;
}

void CConsole::DispatchLine(const string& strLine)
{
    ioStrand.dispatch(boost::bind(&CConsole::ConsoleHandleLine,this,strLine));
}

void CConsole::DispatchOutput(const std::string& strOutput)
{
    ioStrand.dispatch(boost::bind(&CConsole::ConsoleHandleOutput,this,strOutput));
}

bool CConsole::WalleveHandleInvoke()
{
    if (!InstallReadline(strPrompt))
    {
        WalleveError("Failed to setup readline\n");
        return false;
    }

    strLastHistory.clear();

    if (!WalleveThreadDelayStart(thrConsole))
    {
        WalleveError("Failed to start console thread\n");
        return false;
    }
    return true;
}

void CConsole::WalleveHandleHalt()
{
    if (!ioService.stopped())
    {
        ioService.stop();
    }
    WalleveThreadExit(thrConsole);
    UninstallReadline();
}

bool CConsole::InstallReadline(const string& strPrompt)
{
    boost::unique_lock<boost::mutex> lock(CConsole::mutexConsole);
    if (pCurrentConsole != NULL && pCurrentConsole != this)
    {
        return false;
    }

    pCurrentConsole = this;
    rl_callback_handler_install(strPrompt.c_str(),CConsole::ReadlineCallback);
    return true;
}

void CConsole::UninstallReadline()
{
    boost::unique_lock<boost::mutex> lock(CConsole::mutexConsole);
    if (pCurrentConsole == this)
    {
        pCurrentConsole = NULL;
        rl_callback_handler_remove();
        cout << endl;
    }
}

void CConsole::EnterLoop()
{
}

void CConsole::LeaveLoop()
{
}

bool CConsole::HandleLine(const string& strLine)
{
    return true;
}

void CConsole::ReadlineCallback(char *line)
{
    boost::unique_lock<boost::mutex> lock(CConsole::mutexConsole);
    if (pCurrentConsole != NULL)
    {
        pCurrentConsole->DispatchLine(line); 
    }
}

void CConsole::WaitForChars()
{
#ifdef WIN32
    // TODO: Just to be compilable. It works incorrect.
    inStream.async_read_some(boost::asio::mutable_buffer(),boost::bind(&CConsole::HandleRead,this,
                                                     boost::asio::placeholders::error,
                                                     boost::asio::placeholders::bytes_transferred));
#else
    inStream.async_read_some(bufReadNull,boost::bind(&CConsole::HandleRead,this,
                                                     boost::asio::placeholders::error,
                                                     boost::asio::placeholders::bytes_transferred));
#endif
}

void CConsole::HandleRead(boost::system::error_code const& err,size_t nTransferred)
{
    if (err == boost::system::errc::success)
    {
        rl_callback_read_char();
        WaitForChars();
    }
}

void CConsole::ConsoleThreadFunc()
{
    ioService.reset();

    EnterLoop();

    WaitForChars();
    ioService.run();

    LeaveLoop();    
}

void CConsole::ConsoleHandleEvent(CWalleveEvent* pEvent,CIOCompletion& compltHandle)
{
    compltHandle.Completed(pEvent->Handle(*this));
}

void CConsole::ConsoleHandleLine(const string& strLine)
{
    if (HandleLine(strLine))
    {
        if (!strLine.empty() && strLastHistory != strLine)
        {
            add_history(strLine.c_str());
            strLastHistory = strLine;
        }
    }
}

void CConsole::ConsoleHandleOutput(const string& strOutput)
{
    boost::unique_lock<boost::mutex> lock(CConsole::mutexConsole);
    if (pCurrentConsole == this)
    {
        std::cout << '\n' << strOutput << std::flush << std::endl;
        rl_on_new_line();
        rl_redisplay();
    }
}

