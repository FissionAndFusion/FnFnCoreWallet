// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_DELEGATE_COMM_H
#define  MULTIVERSE_DELEGATE_COMM_H

#include "mpvss.h"
#include "uint256.h"
#include "destination.h"

namespace multiverse 
{
namespace delegate 
{

#define MV_CONSENSUS_DISTRIBUTE_INTERVAL	15
#define MV_CONSENSUS_ENROLL_INTERVAL		30
#define MV_CONSENSUS_INTERVAL			(MV_CONSENSUS_DISTRIBUTE_INTERVAL + MV_CONSENSUS_ENROLL_INTERVAL + 1)

/* 
   Consensus phase:

   T : target height (consensus index)
   B : begin of enrolling
   E : end of enrolling
   P : previous block height

   B                                    E                   P       T
   |____________________________________|___________________|_______|
                 enroll                       distribute     publish
   
   P = T - 1
   E = P - MV_CONSENSUS_DISTRIBUTE_INTERVAL
   B = E - MV_CONSENSUS_ENROLL_INTERVAL

   state:
   h = current block height
   x = consensus index
   Tx = x, 
   Px = x - 1,
   Ex = x - MV_CONSENSUS_DISTRIBUTE_INTERVAL - 1
   Bx = x - MV_CONSENSUS_INTERVAL 

   init       : 
       h == Bx
       x == h + MV_CONSENSUS_INTERVAL
   enroll     :
       h <= Ex && h > Bx
       x > h + MV_CONSENSUS_DISTRIBUTE_INTERVAL && x < h + MV_CONSENSUS_INTERVAL
   distribute : 
       h < Px && h >= Ex
       x > h + 1 && x <= h + MV_CONSENSUS_DISTRIBUTE_INTERVAL + 1
   publish     :
       h == Px
       x == h + 1
   stall      :
       h >= Px + 1
       x <= h
*/

inline const CDestination DestFromIdentUInt256(const uint256& nIdent)
{
    return CDestination(CTemplateId(nIdent));
}

inline const uint256 DestToIdentUInt256(const CDestination& dest)
{
    return dest.GetTemplateId();
}

} // namespace delegate
} // namespace multiverse

#endif //MULTIVERSE_DELEGATE_COMM_H

