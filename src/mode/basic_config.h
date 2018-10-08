// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_BASIC_CONFIG_H
#define MULTIVERSE_BASIC_CONFIG_H

#include <exception>
#include <string>

#include "walleve/walleve.h"
#include "mode/auto_options.h"

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
class CMvBasicConfig : virtual public walleve::CWalleveConfig,
                       virtual public CMvBasicConfigOption
{
public:
    CMvBasicConfig();
    virtual ~CMvBasicConfig();
    virtual bool PostLoad();
    virtual std::string ListConfig() const;
    virtual std::string Help() const;

public:
    unsigned int nMagicNum;
};

}  // namespace multiverse

#endif  // MULTIVERSE_BASIC_CONFIG_H
