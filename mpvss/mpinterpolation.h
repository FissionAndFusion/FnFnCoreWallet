// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MPLAGRANGE_H
#define  MPLAGRANGE_H

#include "uint256.h"
#include <vector>

const uint256 MPLagrange(std::vector<std::pair<uint32_t,uint256> >& vShare);

const uint256 MPNewton(std::vector<std::pair<uint32_t,uint256> >& vShare);

#endif //MPLAGRANGE_H
