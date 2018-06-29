#------------------------------------------------------------------------------
# CMake file for Multiverse
#
# Copyright (c) 2016 The Multiverse developers
# Distributed under the MIT/X11 software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
#------------------------------------------------------------------------------

set(sources
	uint256.h
	crc24q.cpp crc24q.h
	base32.cpp base32.h
	hash.cpp hash.h
	crypto.cpp crypto.h
	key.cpp key.h
	keystore.cpp keystore.h
)

add_library(crypto ${sources})

include_directories(../walleve ${sodium_INCLUDE_DIR})

target_link_libraries(crypto
	Boost::system
	Boost::filesystem
	Boost::program_options
	Boost::thread
	Boost::date_time
	Boost::regex
        OpenSSL::SSL
        OpenSSL::Crypto
	${sodium_LIBRARY_RELEASE}
	walleve
)
