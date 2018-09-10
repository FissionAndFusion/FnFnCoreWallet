// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_MODE_BASIC_CONFIG_H
#define MULTIVERSE_MODE_BASIC_CONFIG_H

#include <exception>
#include <string>

#include <boost/program_options.hpp>

#include "walleve/walleve.h"

namespace multiverse
{
/**
 * Dynamic cast walleve::CWalleveConfig* to it's derived class.
 * T is a pointer type.
 * If occured error, throw runtime_error.
 */
template <typename T>
typename std::enable_if<std::is_pointer<T>::value, T>::type
CastConfigPtr(walleve::CWalleveConfig* ptr)
{
    if (!ptr) return NULL;

    T p = dynamic_cast<T>(ptr);
    if (!p)
    {
        throw std::runtime_error(
            std::string("bad_cast: walleve::CWalleveConfig* to ") +
            walleve::TypeName(typeid(T)));
    }
    return p;
}

/**
 * basic config on business.
 */
class CMvBasicConfig : virtual public walleve::CWalleveConfig
{
public:
    CMvBasicConfig();
    virtual ~CMvBasicConfig();
    virtual bool PostLoad();
    virtual std::string ListConfig() const;

public:
    unsigned int nMagicNum;
    bool fTestNet;

protected:
    template <typename T>
    inline void AddOpt(boost::program_options::options_description& desc,
                       const std::string& name, T& var)
    {
        desc.add_options()(name.c_str(),
                           boost::program_options::value<T>(&var));
    }
    template <typename T>
    inline void AddOpt(boost::program_options::options_description& desc,
                       const std::string& name, T& var, const T& def)
    {
        desc.add_options()(
            name.c_str(),
            boost::program_options::value<T>(&var)->default_value(def));
    }
};

}  // namespace multiverse

#endif  // MULTIVERSE_MODE_BASIC_CONFIG_H
