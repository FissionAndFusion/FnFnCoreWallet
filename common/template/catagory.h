// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_TEMPLATE_CATAGORY_H
#define MULTIVERSE_TEMPLATE_CATAGORY_H

#include <vector>

#include "walleve/util.h"
#include "walleve/stream/datastream.h"

#include "destination.h"

/**
 * This template is spendable
 */
class CSpendableTemplate
{
};

/**
 * If transfer to this template, must record destIn in signature
 */
class CDestInRecordedTemplate
{
public:
    static void RecordDestIn(const CDestination& destIn, const std::vector<uint8>& vchPreSig, std::vector<uint8>& vchSig)
    {
        std::vector<uint8> vchTemp;
        walleve::CWalleveODataStream os(vchTemp, CDestination::DESTINATION_SIZE + vchPreSig.size());
        os << destIn;
        vchTemp.insert(vchTemp.end(), vchPreSig.begin(), vchPreSig.end());
        vchSig = std::move(vchTemp);
    }

    static bool ParseDestIn(const std::vector<uint8>& vchSig, CDestination& destIn, std::vector<uint8>& vchSubSig)
    {
        walleve::CWalleveIDataStream is(vchSig);
        try
        {
            is >> destIn;
            vchSubSig.assign(vchSig.begin() + CDestination::DESTINATION_SIZE, vchSig.end());
        }
        catch (std::exception& e)
        {
            walleve::StdError(__PRETTY_FUNCTION__, e.what());
            return false;
        }

        return true;
    }
};

class CLockedCoinTemplate
{
public:
    virtual int64 LockedCoin(const CDestination& destTo, const int32 nForkHeight) const = 0;
};

#endif // MULTIVERSE_TEMPLATE_CATAGORY_H