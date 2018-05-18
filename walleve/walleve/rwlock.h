// Copyright (c) 2016-2018 The LoMoCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  WALLEVE_RWLOCK_H
#define  WALLEVE_RWLOCK_H

#include <boost/noncopyable.hpp>
#include <boost/thread/thread.hpp>
#include <boost/atomic.hpp>

namespace walleve
{

class CWalleveRWAccess : public boost::noncopyable
{
public:
    CWalleveRWAccess() : nRead(0),nWrite(0),nWriting(0) {}

    void ReadLock()
    {
        boost::shared_lock<boost::shared_mutex> lock(mutex);
        while (nWrite)
        {
            condRead.wait(lock);
        }
        nRead.fetch_add(1);
    }
    void ReadUnlock()
    {
        bool fNotifyWrite = false;
        {
            boost::shared_lock<boost::shared_mutex> lock(mutex);
            fNotifyWrite = (nRead.fetch_sub(1) == 1 && nWrite);
        }
        if (fNotifyWrite)
        {
            condWrite.notify_one();
        }
    }
    void WriteLock()
    {
        boost::unique_lock<boost::shared_mutex> lock(mutex);
        if (nRead || nWrite.fetch_add(1))
        {
            do { condWrite.wait(lock); } while (nRead || nWriting);
        }
        nWriting.fetch_add(1);
    }
    void WriteUnlock()
    {
        bool fNotifyWrite = false;
        {
            boost::unique_lock<boost::shared_mutex> lock(mutex);
            fNotifyWrite = (nWrite.fetch_sub(1) != 1);
            nWriting.fetch_sub(1);
        }
        if (fNotifyWrite)
        {
            condWrite.notify_one();
        }
        else
        {
            condRead.notify_all();
        }
    }
protected:
    boost::atomic<int> nRead;
    boost::atomic<int> nWrite;
    boost::atomic<int> nWriting;
    boost::shared_mutex mutex;
    boost::condition_variable_any condRead;
    boost::condition_variable_any condWrite;
};

class CWalleveReadLock
{
public:
    CWalleveReadLock(CWalleveRWAccess& access) 
    : _access(access)
    {
        _access.ReadLock();
    }
    ~CWalleveReadLock()
    {
        _access.ReadUnlock();
    }
protected:
    CWalleveRWAccess& _access;
};

class CWalleveWriteLock
{
public:
    CWalleveWriteLock(CWalleveRWAccess& access) 
    : _access(access)
    {
        _access.WriteLock();
    }
    ~CWalleveWriteLock()
    {
        _access.WriteUnlock();
    }
protected:
    CWalleveRWAccess& _access;
};

} // namespace walleve

#endif //WALLEVE_RWLOCK_H

