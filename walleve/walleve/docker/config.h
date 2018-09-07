// Copyright (c) 2016-2018 The LoMoCoin developers
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

    void SetIgnoreCmd(int number);
protected:
    static std::pair<std::string,std::string> ExtraParser(const std::string& s);
    void AddOptions(boost::program_options::options_description& desc);
    bool TokenizeConfile(const char *pzConfile,std::vector<std::string>& tokens);
    void AddExtraDescription(const std::string& command, const boost::program_options::options_description& desc);
    void AddExtraDescription(const std::map<std::string, const boost::program_options::options_description&>& desc);

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
    std::map<std::string, const boost::program_options::options_description&> extraDesc;
};

} // namespace walleve

#endif //WALLEVE_CONFIG_H

