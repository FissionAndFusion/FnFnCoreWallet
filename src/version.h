// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_VERSION_H
#define  MULTIVERSE_VERSION_H

#include <string>
#include <sstream>

#define MV_VERSION_NAME        "Multiverse"

#define MV_VERSION_MAJOR       0
#define MV_VERSION_MINOR       1
#define MV_VERSION_REVISION    0

static const int MV_VERSION =
                           10000 * MV_VERSION_MAJOR
                         +   100 * MV_VERSION_MINOR
                         +     1 * MV_VERSION_REVISION;

static const int PROTO_VERSION = 100;
static const int MIN_PROTO_VERSION = 100;

inline std::string FormatVersion(int nVersion)
{
    std::ostringstream ss;
    ss << (nVersion / 10000) << "." << ((nVersion / 100) % 100) << "." << (nVersion % 100);
    return ss.str();
}

inline std::string FormatSubVersion()
{
    std::ostringstream ss;
    ss << "/" << MV_VERSION_NAME << ":" << FormatVersion(MV_VERSION);
    ss << "/Protocol:" << FormatVersion(PROTO_VERSION) << "/";
    return ss.str();
}

#endif //MULTIVERSE_VERSION_H
