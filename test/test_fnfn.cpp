// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "test_fnfn.h"

BasicUtfSetup::BasicUtfSetup(const std::string& strMsg)
{
    std::cout << strMsg << (" - basic fixture loaded!\n");
}

BasicUtfSetup::~BasicUtfSetup()
{
    std::cout << ("basic fixture unloaded!\n");
}

UtfSetup::UtfSetup(const std::string& strMsg) : BasicUtfSetup(strMsg)
{
    std::cout << strMsg << (" - UTF fixture loaded!\n");
}

UtfSetup::~UtfSetup()
{
    std::cout << ("UTF fixture unloaded!\n");
}

UtfWorldline100Setup::UtfWorldline100Setup()
        : UtfSetup("UtfWorldline100Setup")
{
    std::cout << ("UtfWorldline100Setup - specific fixture loaded!\n");
}

UtfWorldline100Setup::~UtfWorldline100Setup()
{
    std::cout << ("UtfWorldline100Setup - specific fixture unloaded!\n");
}
