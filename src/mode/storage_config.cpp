// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "mode/storage_config.h"

namespace po = boost::program_options;
using namespace multiverse;

#define DEFAULT_DB_CONNECTION 8

CMvStorageConfig::CMvStorageConfig()
{
    po::options_description desc("MvStorage");

    AddOpt<std::string>(desc, "dbhost", strDBHost, "localhost");
    AddOpt<std::string>(desc, "dbname", strDBName, "multiverse");
    AddOpt<std::string>(desc, "dbuser", strDBUser, "multiverse");
    AddOpt<std::string>(desc, "dbpass", strDBPass, "multiverse");
    AddOpt<int>(desc, "dbport", nDBPort, 0);
    AddOpt<int>(desc, "dbconn", nDBConn, DEFAULT_DB_CONNECTION);
    AddOpt<std::string>(desc, "dnseeddbname", strDNSeedDBName, "dnseed");

    AddOptions(desc);
}

CMvStorageConfig::~CMvStorageConfig() {}

bool CMvStorageConfig::PostLoad()
{
    if (fTestNet)
    {
        strDBName += "-testnet";
    }
    if (nDBPort < 0 || nDBPort > 0xFFFF)
    {
        nDBPort = 0;
    }
    return true;
}

std::string CMvStorageConfig::ListConfig() const { return "storage"; }