// Copyright (c) 2016-2018 The LoMoCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "config.h"
#include <sstream>
#include <fstream>
#include <boost/algorithm/string/trim.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

using namespace std;
using namespace walleve;

namespace po = boost::program_options;
namespace fs = boost::filesystem;

//////////////////////////////
// CWalleveConfig

CWalleveConfig::CWalleveConfig()
: optDesc("WalleveOptions") 
{
}

CWalleveConfig::~CWalleveConfig()
{
}

bool CWalleveConfig::Load(int argc,char *argv[],const fs::path& pathDefault,const string& strConfile)
{
    const int cmdStyle = po::command_line_style::allow_long 
                         | po::command_line_style::long_allow_adjacent
                         | po::command_line_style::allow_long_disguise;
    fs::path pathConfile;
    try
    { 
        optDesc.add_options()
        ("cmd", po::value<vector<string> >(&vecCommand))
        ("debug", po::value<bool>(&fDebug)->default_value(false))
        ("daemon", po::value<bool>(&fDaemon)->default_value(false))
        ("help", po::value<bool>(&fHelp)->default_value(false))
        ("datadir", po::value<fs::path>(&pathRoot)->default_value(pathDefault))
        ("conf", po::value<fs::path>(&pathConfile)->default_value(fs::path(strConfile)));

        po::positional_options_description posDesc;
        posDesc.add("cmd",-1);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(optDesc).style(cmdStyle)
                  .allow_unregistered().extra_parser(CWalleveConfig::ExtraParser)
                  .positional(posDesc).run(),vm);
        po::notify(vm);

        if (fHelp)
        {
            return true;
        }

        if (!pathConfile.is_complete())
        {
            pathConfile = pathRoot / pathConfile;
        }

        vector<string> confToken;
        if (TokenizeConfile(pathConfile.string().c_str(),confToken))
        {
            po::store(po::command_line_parser(confToken).options(optDesc).style(cmdStyle)
                      .allow_unregistered().extra_parser(CWalleveConfig::ExtraParser).run(),vm);
            po::notify(vm);
        }
    }
    catch (...)
    {
        return false;
    }
    return true;
}

bool CWalleveConfig::PostLoad()
{
    pathData = pathRoot; 
    return true;
}

string CWalleveConfig::ListConfig()
{
    ostringstream oss;
    oss << "Debug : " << (fDebug ? "Y" : "N") << "\n"
        << "Data Path : " << pathData << "\n";
    return oss.str();
}

pair<string,string> CWalleveConfig::ExtraParser(const string& s)
{
    if (s[0] == '-')
    {
        bool fRev = (s.substr(1,2) == "no");
        size_t eq = s.find('=');
        if (eq == string::npos)
        {
            if (fRev)
            {   
                return make_pair(s.substr(3),string("0"));
            }
            else
            {   
                return make_pair(s.substr(1),string("1"));
            }
        }
        else if (fRev)
        {
            int v = atoi(s.substr(eq+1).c_str());
            return make_pair(s.substr(3,eq-3),string(v != 0 ? "0" : "1"));
        }
    }
    return make_pair(string(), string());
}

void CWalleveConfig::AddOptions(boost::program_options::options_description& desc)
{
    optDesc.add(desc);
}

bool CWalleveConfig::TokenizeConfile(const char *pzConfile,vector<string>& tokens)
{
    ifstream ifs(pzConfile);
    if (!ifs)
    {
        return false;
    }
    string line;
    while(!getline(ifs,line).eof())
    {
        string s = line.substr(0,line.find('#'));
        boost::trim(s);
        if (!s.empty())
        {
            tokens.push_back(string("-") + s);
        }
    }

    return true;
}


