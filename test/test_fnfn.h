// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TEST_FNFN_COREWALLET_H
#define TEST_FNFN_COREWALLET_H

#include <string>
#include <iostream>

struct BasicUtfSetup
{
    explicit BasicUtfSetup(const std::string& strMsg = "BasicUtfSetup");
    ~BasicUtfSetup();
};

struct UtfSetup: public BasicUtfSetup
{
    explicit UtfSetup(const std::string& strMsg = "UtfSetup");
    ~UtfSetup();
};

struct UtfWorldline100Setup : public UtfSetup
{
    UtfWorldline100Setup();
    ~UtfWorldline100Setup();
};

#endif
