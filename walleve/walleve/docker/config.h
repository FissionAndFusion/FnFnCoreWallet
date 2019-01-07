// Copyright (c) 2016-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  WALLEVE_CONFIG_H
#define  WALLEVE_CONFIG_H

#include <string>
#include <vector>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

namespace walleve
{

class CWalleveConfig
{
public:
    // Only parse command
    static std::vector<std::string> ParseCmd(int argc,char *argv[]);
public:
    CWalleveConfig();
    virtual ~CWalleveConfig();
    bool Load(int argc,char *argv[],const boost::filesystem::path& pathDefault,
              const std::string& strConfile);
    virtual bool PostLoad();
    virtual std::string ListConfig() const;
    virtual std::string Help() const;

    void SetIgnoreCmd(int number);
protected:
    static std::pair<std::string,std::string> ExtraParser(const std::string& s);
    void AddOptions(boost::program_options::options_description& desc);
    bool TokenizeConfile(const char *pzConfile,std::vector<std::string>& tokens);

public:
    bool fDebug;
    bool fHelp;
    bool fDaemon;
    boost::filesystem::path pathRoot;
    boost::filesystem::path pathData;
    std::vector<std::string> vecCommand;
protected:
    boost::program_options::options_description defaultDesc;
    int ignoreCmd;
    boost::program_options::variables_map vm;
};

} // namespace walleve

#endif //WALLEVE_CONFIG_H

