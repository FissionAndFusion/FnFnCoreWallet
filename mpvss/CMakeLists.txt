#------------------------------------------------------------------------------
# CMake file for Multiverse
#
# Copyright (c) 2016 The Multiverse developers
# Distributed under the MIT/X11 software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
#------------------------------------------------------------------------------

# if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
#     add_definitions(-D__GLIBCXX_TYPE_INT_N_0=__int128 -D__GLIBCXX_BITSIZE_INT_N_0=128)
# endif()

set(curve25519
    curve25519/base25519.cpp curve25519/base25519.h
    curve25519/fp25519.cpp curve25519/fp25519.h
    curve25519/sc25519.cpp curve25519/sc25519.h
    curve25519/ed25519.cpp curve25519/ed25519.h
    curve25519/curve25519.h
)

set(sources
    ${curve25519}
    parallel.h
    mpbox.cpp mpbox.h
    mpinterpolation.cpp mpinterpolation.h
    mpvss.cpp mpvss.h
)

add_library(mpvss ${sources})

include_directories(../walleve ../crypto ./)

target_link_libraries(mpvss
    walleve
)