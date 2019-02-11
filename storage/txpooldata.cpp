// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
    
#include "txpooldata.h"

using namespace std;
using namespace boost::filesystem;
using namespace walleve;
using namespace multiverse::storage;
    
//////////////////////////////
// CTxPoolData

CTxPoolData::CTxPoolData()
{   
}   
    
CTxPoolData::~CTxPoolData()
{   
}

bool CTxPoolData::Initialize(const path& pathData)
{
    path pathTxPool = pathData / "txpool";

    if (!exists(pathTxPool))
    {
        create_directories(pathTxPool);
    }

    if (!is_directory(pathTxPool))
    {
        return false;
    }

    pathTxPoolFile = pathTxPool / "txpool.dat";

    if (exists(pathTxPoolFile) && !is_regular_file(pathTxPoolFile))
    {
        return false;
    }

    return true;
}

bool CTxPoolData::Remove()
{
    if (is_regular_file(pathTxPoolFile))
    {
        return remove(pathTxPoolFile);
    }
    return true;
}

bool CTxPoolData::Save(const vector<pair<uint256,pair<uint256,CAssembledTx> > >& vTx)
{
    FILE * fp = fopen(pathTxPoolFile.c_str(),"w");
    if (fp == NULL)
    {
        return false;
    }
    fclose(fp);

    if (!is_regular_file(pathTxPoolFile))
    {
        return false;
    }

    try
    {
        CWalleveFileStream fs(pathTxPoolFile.c_str());
        fs << vTx;
    }
    catch (std::exception& e)
    {
        StdError(__PRETTY_FUNCTION__, e.what());
        return false;
    }
    
    return true;
}

bool CTxPoolData::Load(vector<pair<uint256,pair<uint256,CAssembledTx> > >& vTx)
{
    vTx.clear();

    if (!is_regular_file(pathTxPoolFile))
    {
        return true;
    }

    try
    {
        CWalleveFileStream fs(pathTxPoolFile.c_str());
        fs >> vTx;
    }
    catch (std::exception& e)
    {
        StdError(__PRETTY_FUNCTION__, e.what());
        return false;
    }
    
    remove(pathTxPoolFile);

    return true;
}

