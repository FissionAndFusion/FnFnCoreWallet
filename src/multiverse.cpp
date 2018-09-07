// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "entry.h"
#include <iostream>
#include <exception>
#include <walleve/walleve.h>

using namespace multiverse;

void MvShutdown()
{
    CMvEntry::GetInstance().Stop();
}

int main(int argc,char **argv)
{
    CMvEntry& mvEntry = CMvEntry::GetInstance();
    try
    {
        if (mvEntry.Initialize(argc,argv))
        {
            mvEntry.Run();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Multiverse exception caught : " << e.what() << "\n";
    }
    catch (...)
    {
        std::cerr << "Multiverse exception caught : unknown\n";
    }

    mvEntry.Exit();

    return 0;
}
