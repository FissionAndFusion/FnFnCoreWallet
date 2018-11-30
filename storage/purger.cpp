// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "purger.h"
#include "blockdb.h"    
#include "txpooldb.h"
#include "walletdb.h"

using namespace std;
using namespace boost::filesystem;
using namespace multiverse::storage;
using namespace walleve;
    
//////////////////////////////
// CPurger

bool CPurger::ResetDB(const CMvDBConfig& dbConfig) const
{
    {
        CBlockDB dbBlock;
        if (dbBlock.DBPoolInitialize(dbConfig,1) && dbBlock.Initialize())
        {
            if (!dbBlock.RemoveAll())
            {
                return false;
            }
            dbBlock.Deinitialize();
        }
    }

    {
        CTxPoolDB dbTxPool;
        if (dbTxPool.Initialize(dbConfig))
        {
            if (!dbTxPool.RemoveAll())
            {
                return false;
            }
            dbTxPool.Deinitialize();
        }
    }

    {
        CWalletDB dbWallet;
        if (dbWallet.Initialize(dbConfig))
        {
            if (!dbWallet.ClearTx())
            {
                return false;
            }
            dbWallet.Deinitialize();
        }
    }

    return true;
}

bool CPurger::RemoveBlockFile(const path& pathDataLocation) const
{
    try
    {
        path pathBlock = pathDataLocation / "block";
        if (exists(pathBlock))
        {
            remove_all(pathBlock);
        }
    }
    catch (exception& e)
    {
        StdError(__PRETTY_FUNCTION__, e.what());
        return false;
    }
    return true;
}

