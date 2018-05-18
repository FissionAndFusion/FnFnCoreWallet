// Copyright (c) 2016-2018 The LoMoCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  WALLEVE_ENTRY_H
#define  WALLEVE_ENTRY_H

#include <string>
#include <boost/asio.hpp>
#include <boost/interprocess/sync/file_lock.hpp>

namespace walleve
{

class CWalleveEntry
{
public:
    CWalleveEntry();
    ~CWalleveEntry();
    bool TryLockFile(const std::string& strLockFile);
    virtual bool Run();
    virtual void Stop();
protected:
    void HandleSignal(const boost::system::error_code& error,int signal_number);
protected:
    boost::asio::io_service ioService;
    boost::asio::signal_set ipcSignals;
    boost::interprocess::file_lock lockFile;
};


} // namespace walleve

#endif //WALLEVE_ENTRY_H

