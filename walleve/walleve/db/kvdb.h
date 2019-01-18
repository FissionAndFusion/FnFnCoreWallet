// Copyright (c) 2016-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  WALLEVE_KVDB_H
#define  WALLEVE_KVDB_H

#include "walleve/util.h"
#include "walleve/stream/stream.h"
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <transaction.h>

namespace walleve
{

class CKVDBEngine
{
public:    
    virtual ~CKVDBEngine() {}  

    virtual bool Open() = 0;
    virtual void Close() = 0;
    virtual bool TxnBegin() = 0;
    virtual bool TxnCommit() = 0;
    virtual void TxnAbort() = 0;
    virtual bool Get(CWalleveBufStream& ssKey,CWalleveBufStream& ssValue) = 0;
    virtual bool Put(CWalleveBufStream& ssKey,CWalleveBufStream& ssValue, bool fOverwrite) = 0;
    virtual bool Remove(CWalleveBufStream& ssKey) = 0;
    virtual bool RemoveAll() = 0;
    virtual bool MoveFirst() = 0;
    virtual bool MoveNext(CWalleveBufStream& ssKey,CWalleveBufStream& ssValue) = 0;
};

class CKVDB
{
public:
    typedef boost::function<bool(CWalleveBufStream&,CWalleveBufStream&)> WalkerFunc;

    CKVDB() : dbEngine(NULL) {}
    CKVDB(CKVDBEngine * engine) : dbEngine(engine)
    {
        boost::recursive_mutex::scoped_lock lock(mtx);
        if (dbEngine != NULL)
        {
            if (!dbEngine->Open())
            {
                delete dbEngine;
                dbEngine = NULL;
            }
        }
    }

    virtual ~CKVDB() 
    {
        boost::recursive_mutex::scoped_lock lock(mtx);
        if (dbEngine != NULL)
        {
            dbEngine->Close();
            delete dbEngine;
        }
    }

    bool Open(CKVDBEngine * engine)
    {
        if (dbEngine == NULL && engine != NULL && engine->Open())
        {
            boost::recursive_mutex::scoped_lock lock(mtx);
            dbEngine = engine;
            return true;
        }
        
        return false;    
    }    

    void Close()
    {
        boost::recursive_mutex::scoped_lock lock(mtx);
        if (dbEngine != NULL)
        {
            dbEngine->Close();
            delete dbEngine;
            dbEngine = NULL;
        }
    }

    bool TxnBegin()
    {
        boost::recursive_mutex::scoped_lock lock(mtx);
        if (dbEngine != NULL)
        {
            return dbEngine->TxnBegin();
        }
        return false;
    }

    bool TxnCommit()
    {
        boost::recursive_mutex::scoped_lock lock(mtx);
        if (dbEngine != NULL)
        {
            return dbEngine->TxnCommit();
        }
        return false;
    }

    void TxnAbort()
    {
        boost::recursive_mutex::scoped_lock lock(mtx);
        if (dbEngine != NULL)
        {
            dbEngine->TxnAbort();
        }
    }

    bool RemoveAll()
    {
        boost::recursive_mutex::scoped_lock lock(mtx);
        if (dbEngine != NULL)
        {
            if (dbEngine->RemoveAll())
            {
                return true;
            }
            Close();
        }
        return false; 
    } 

    bool IsValid() const 
    {
        return (dbEngine != NULL);
    }

protected:
    virtual bool DBWalker(CWalleveBufStream& ssKey, CWalleveBufStream& ssValue) = 0;

    template<typename K, typename T>
    bool Read(const K& key, T& value) 
    {
        CWalleveBufStream ssKey,ssValue;
        ssKey << key;

        try
        {
            boost::recursive_mutex::scoped_lock lock(mtx);

            if (dbEngine == NULL)
                return false;

            if (dbEngine->Get(ssKey,ssValue))
            {
                ssValue >> value;
                return true;
            }
        }
        catch (const boost::thread_interrupted&) 
        {
            throw;
        }
        catch (std::exception& e)
        {
            StdError(__PRETTY_FUNCTION__, e.what());
        }

        return false;
    }

    template<typename K, typename T>
    bool Write(const K& key, const T& value, bool fOverwrite=true) 
    {
        CWalleveBufStream ssKey,ssValue;
        ssKey << key;
        ssValue << value;
        
        try
        {
            boost::recursive_mutex::scoped_lock lock(mtx);

            if (dbEngine == NULL)
                return false;
            return dbEngine->Put(ssKey,ssValue,fOverwrite);
        }
        catch (const boost::thread_interrupted&) 
        {
            throw;
        }
        catch (std::exception& e)
        {
            StdError(__PRETTY_FUNCTION__, e.what());
        }

        return false;
    }

    template<typename K>
    bool Erase(const K& key) 
    {
        CWalleveBufStream ssKey;
        ssKey << key;
        
        try
        {
            boost::recursive_mutex::scoped_lock lock(mtx);

            if (dbEngine == NULL)
                return false;
            return dbEngine->Remove(ssKey);
        }
        catch (const boost::thread_interrupted&) 
        {
            throw;
        }
        catch (std::exception& e)
        {
            StdError(__PRETTY_FUNCTION__, e.what());
        }

        return false;
    }
   
    bool WalkThrough()
    {
        try
        {
            boost::recursive_mutex::scoped_lock lock(mtx);

            if (dbEngine == NULL)
                return false;

            if (!dbEngine->MoveFirst())
                return false;
            
            for (;;)
            { 
                CWalleveBufStream ssKey,ssValue;
                if (!dbEngine->MoveNext(ssKey,ssValue))
                    break;

                if (!DBWalker(ssKey,ssValue))
                    break;                               
            }
            return true;
        }
        catch (std::exception& e)
        {
            StdError(__PRETTY_FUNCTION__, e.what());
        }

        return false;
    }  

    bool WalkThrough(WalkerFunc fnWalker)
    {
        try
        {
            boost::recursive_mutex::scoped_lock lock(mtx);

            if (dbEngine == NULL)
                return false;

            if (!dbEngine->MoveFirst())
                return false;

            std::ofstream all_unspent("all_unspent.txt", std::ios::out | std::ios::trunc | std::ios::ate);
            if(!all_unspent)
            {
                assert(0);
            }
            for (;;)
            {
                CWalleveBufStream ssKey,ssValue;
                if (!dbEngine->MoveNext(ssKey,ssValue))
                    break;

                //dump all unspent to file
                CTxOutPoint outpoint;
                CTxOutput output;
                ssKey >> outpoint;
                ssValue >> output;
                all_unspent << std::setw(64) << outpoint.hash.ToString() << " "
                           << std::setw(2) << std::to_string(outpoint.n) << " "
                           << std::setw(57) << output.destTo.GetHex() << " "
                           << std::setw(20) << std::to_string(output.nAmount) << " "
                           << std::setw(4) << std::to_string(output.nLockUntil)
                           << std::endl;
                //CMvAddress addr(output.destTo);
                if(output.destTo.IsPubKey())
                {
                    //std::cout << "IsPubKey" << std::endl;
                    //all_unspent << "output.destTo<IsPubKey>:" << output.destTo.GetHex() << std::endl;
                } else if(output.destTo.IsTemplate())
                {
                    //std::cout << "IsTemplate" << std::endl;
                }
                //break;
/*                if (!fnWalker(ssKey,ssValue))
                    break;*/
            }
            return true;
        }
        catch (std::exception& e)
        {
            StdError(__PRETTY_FUNCTION__, e.what());
        }

        return false;
    }  
protected:
    boost::recursive_mutex mtx;
    CKVDBEngine * dbEngine;
};


} // namespace walleve

#endif //WALLEVE_KVDB_H

