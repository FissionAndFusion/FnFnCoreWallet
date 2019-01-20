// Copyright (c) 2016-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  WALLEVE_RWLOCK_H
#define  WALLEVE_RWLOCK_H

#include <boost/noncopyable.hpp>
#include <boost/thread/thread.hpp>

namespace walleve
{

class CWalleveRWAccess : public boost::noncopyable
{
public:
    CWalleveRWAccess() : nRead(0),nWrite(0),fExclusive(false),fUpgraded(false) {}

    void ReadLock()
    {
        boost::unique_lock<boost::mutex> lock(mutex);
        while (nWrite || fUpgraded)
        {
            condRead.wait(lock);
        }
        ++nRead;
    }
    void ReadUnlock()
    {
        bool fNotifyWrite   = false;
        bool fNotifyUpgrade = false;
        {
            boost::unique_lock<boost::mutex> lock(mutex);
            
            if (--nRead == 0)
            {
                fNotifyWrite   = (nWrite != 0);
                fNotifyUpgrade = fExclusive;
            }
        }
        if (fNotifyUpgrade)
        {
            condUpgrade.notify_one();
        }
        else if (fNotifyWrite)
        {
            condWrite.notify_one();
        }
    }
    void WriteLock()
    {
        boost::unique_lock<boost::mutex> lock(mutex);

        ++nWrite;

        while (nRead || fExclusive)
        {
            condWrite.wait(lock);
        }

        fExclusive = true;
    }
    void WriteUnlock()
    {
        bool fNotifyWrite = false;
        {
            boost::unique_lock<boost::mutex> lock(mutex);
            fNotifyWrite = (--nWrite != 0);
            fExclusive = false;
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
    void UpgradeLock()
    {
        boost::unique_lock<boost::mutex> lock(mutex);

        while (fExclusive)
        {
            condRead.wait(lock);
        }

        fExclusive = true;
    }
    void UpgradeToWriteLock()
    {
        boost::unique_lock<boost::mutex> lock(mutex);

        fUpgraded = true;

        while (nRead)
        {
            condUpgrade.wait(lock);
        }
    }
    void UpgradeUnlock()
    {
        bool fNotifyWrite = false;
        {
            boost::unique_lock<boost::mutex> lock(mutex);

            fNotifyWrite = (nWrite != 0);

            if (!fUpgraded)
            {
                fNotifyWrite = (fNotifyWrite && nRead == 0);
            }

            fExclusive = false;
            fUpgraded = false;
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
    int nRead;
    int nWrite;
    bool fExclusive;
    bool fUpgraded;
    boost::mutex mutex;
    boost::condition_variable_any condRead;
    boost::condition_variable_any condWrite;
    boost::condition_variable_any condUpgrade;
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

class CWalleveUpgradeLock
{
public:
    CWalleveUpgradeLock(CWalleveRWAccess& access) 
    : _access(access)
    {
        _access.UpgradeLock();
    }
    ~CWalleveUpgradeLock()
    {
        _access.UpgradeUnlock();
    }
    void Upgrade()
    {
        _access.UpgradeToWriteLock();
    }
protected:
    CWalleveRWAccess& _access;
};
} // namespace walleve

#endif //WALLEVE_RWLOCK_H

