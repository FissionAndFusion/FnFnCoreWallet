// Copyright (c) 2017-2018 The Multiverse developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

/*
 * Curve 25519 C++ implementation
 * Derived from public domain C code by Daniel J. Bernstein <djb@cr.yp.to>
 * More information can be found : http://cr.yp.to/ecdh.html
 */

#ifndef CURVE25519_H
#define CURVE25519_H

#include "base25519.h"
#include "fp25519.h"
#include "sc25519.h"
#include "ed25519.h"

using CFP25519 = curve25519::CFP25519;
using CSC25519 = curve25519::CSC25519;
using CEdwards25519 = curve25519::CEdwards25519;

#endif //CURVE25519_H
