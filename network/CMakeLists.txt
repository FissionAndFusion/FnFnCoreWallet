#------------------------------------------------------------------------------
# CMake file for Multiverse
#
# Copyright (c) 2016 The Multiverse developers
# Distributed under the MIT/X11 software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
#------------------------------------------------------------------------------

set(sources
	mvproto.cpp mvproto.h
	mvpeerevent.h
	mvpeer.cpp mvpeer.h
	mvpeernet.cpp mvpeernet.h
)

add_library(network ${sources})

include_directories(../common ../walleve ../crypto ../storage)

target_link_libraries(network
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
	crypto
	storage
)
