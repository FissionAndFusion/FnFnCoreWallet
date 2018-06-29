// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef  MULTIVERSE_BASE32_H
#define  MULTIVERSE_BASE32_H

#include <string>

namespace multiverse
{
namespace crypto
{

// Crockford's Base32 without symbols check
// http://www.crockford.com/wrmg/base32.html
// alphabet : 0123456789abcdefghjkmnpqrstvwxyz, upper and lower case letters are accepted.
// only handle 32bytes data encode & decode, includes crc24 checksum

void Base32Encode(const unsigned char* md32,std::string& strBase32);
bool Base32Decode(const std::string& strBase32,unsigned char* md32);

} // namespace crypto
} // namespace multiverse

#endif //MULTIVERSE_BASE32_H

