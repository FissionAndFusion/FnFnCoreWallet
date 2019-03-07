// Copyright (c) 2017-2019 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MULTIVERSE_TEMPLATE_ID_H
#define MULTIVERSE_TEMPLATE_ID_H

#include "uint256.h"

/**
 * hash of template data + template type
 */
class CTemplateId : public uint256
{
public:
    CTemplateId();
    CTemplateId(const uint256& data);
    CTemplateId(const uint16 type, const uint256& hash);

    uint16 GetType() const;
    CTemplateId& operator=(uint64 b);
};

#endif //MULTIVERSE_TEMPLATE_ID_H