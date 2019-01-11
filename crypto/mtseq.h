// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_MTSEQ_H
#define  MULTIVERSE_MTSEQ_H

#include "uint256.h"

#include <stdexcept>
#include <memory>
#include <string>
#include <vector>

namespace multiverse
{
namespace crypto
{

class CMTSeq
{
public:
    CMTSeq(const uint64 nSeed);
    uint64 Get(std::size_t nDistance);
protected:
    void MakeSeq(const uint64 nSeed);
     
protected:
    enum {NN=312,MM=156};
    uint64 nMaxtrix[NN];
    std::vector<uint64> vMt;
};

} // namespace crypto
} // namespace multiverse

#endif //MULTIVERSE_MTSEQ_H
