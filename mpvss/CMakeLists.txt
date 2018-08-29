#------------------------------------------------------------------------------
# CMake file for Multiverse
#
# Copyright (c) 2016 The Multiverse developers
# Distributed under the MIT/X11 software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
#------------------------------------------------------------------------------

set(sources
	curve25519.cpp curve25519.h
        mpu256.h
        mpbox.cpp mpbox.h
        mplagrange.cpp mplagrange.h
        mpvss.cpp mpvss.h
)

add_library(mpvss ${sources})

include_directories(../walleve)

target_link_libraries(mpvss
	walleve
)

