// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "mode/dbp_config.h"

#include <boost/algorithm/algorithm.hpp>
#include "mode/config_macro.h"

namespace multiverse
{
namespace po = boost::program_options;
namespace fs = boost::filesystem;
using tcp = boost::asio::ip::tcp;

/////////////////////////////////////////////////////////////////////
// CMvDbpBasicConfig

CMvDbpBasicConfig::CMvDbpBasicConfig()
{
    po::options_description desc("DbpBasic");

    CMvDbpBasicConfigOption::AddOptionsImpl(desc);

    AddOptions(desc);
}

CMvDbpBasicConfig::~CMvDbpBasicConfig() {}

bool CMvDbpBasicConfig::PostLoad()
{
    if (fHelp)
    {
        return true;
    }

    if (nDbpPortInt <= 0 || nDbpPortInt > 0xFFFF)
    {
        nDbpPort = (fTestNet ? DEFAULT_TESTNET_DBPPORT : DEFAULT_DBPPORT);
    }
    else
    {
        nDbpPort = (unsigned short)nDbpPortInt;
    }

    if (!fs::path(strDbpCAFile).is_complete())
    {
        strDbpCAFile = (pathRoot / strDbpCAFile).string();
    }

    if (!fs::path(strDbpCertFile).is_complete())
    {
        strDbpCertFile = (pathRoot / strDbpCertFile).string();
    }

    if (!fs::path(strDbpPKFile).is_complete())
    {
        strDbpPKFile = (pathRoot / strDbpPKFile).string();
    }

    return true;
}

std::string CMvDbpBasicConfig::ListConfig() const
{
    std::ostringstream oss;
    oss << CMvDbpBasicConfigOption::ListConfigImpl();
    oss << "DbpPort: " << nDbpPort << "\n";
    return oss.str();
}

std::string CMvDbpBasicConfig::Help() const
{
    return CMvDbpBasicConfigOption::HelpImpl();
}

/////////////////////////////////////////////////////////////////////
// CMvRPCServerConfig

CMvDbpServerConfig::CMvDbpServerConfig()
{
    po::options_description desc("DbpServer");

    CMvDbpServerConfigOption::AddOptionsImpl(desc);

    AddOptions(desc);
}

CMvDbpServerConfig::~CMvDbpServerConfig() {}

bool CMvDbpServerConfig::PostLoad()
{
    if (fHelp)
    {
        return true;
    }

    CMvDbpBasicConfig::PostLoad();

    if(fDbpListen4 || (!fDbpListen4 && !fDbpListen6))
    {
        epDbp = tcp::endpoint(!vDbpAllowIP.empty()
                              ? boost::asio::ip::address_v4::any()
                              : boost::asio::ip::address_v4::loopback(),
                          nDbpPort);
    }

    if(fDbpListen6)
    {
        epDbp = tcp::endpoint(!vDbpAllowIP.empty()
                              ? boost::asio::ip::address_v6::any()
                              : boost::asio::ip::address_v6::loopback(),
                          nDbpPort);
    }

    
    return true;
}

std::string CMvDbpServerConfig::ListConfig() const
{
    std::ostringstream oss;
    oss << CMvDbpServerConfigOption::ListConfigImpl();
    oss << "epDbp: " << epDbp << "\n";
    return CMvDbpBasicConfig::ListConfig() + oss.str();
}

std::string CMvDbpServerConfig::Help() const
{
    return CMvDbpBasicConfig::Help() + CMvDbpServerConfigOption::HelpImpl();
}

CMvDbpClientConfig::CMvDbpClientConfig()
{
    po::options_description desc("DbpClient");

    CMvDbpClientConfigOption::AddOptionsImpl(desc);

    AddOptions(desc);
}

CMvDbpClientConfig::~CMvDbpClientConfig()
{

}
    
bool CMvDbpClientConfig::PostLoad()
{
    if (fHelp)
    {
        return true;
    }

    CMvDbpBasicConfig::PostLoad();
    return true;
}

std::string CMvDbpClientConfig::ListConfig() const
{
    std::ostringstream oss;
    oss << CMvDbpClientConfigOption::ListConfigImpl();
    oss << "epParentDbp: " << epParentHost << "\n";
    return oss.str();
}

std::string CMvDbpClientConfig::Help() const
{
    return CMvDbpClientConfigOption::HelpImpl();
}

} // namepspace multiverse
