#------------------------------------------------------------------------------
# CMake file for Multiverse
#
# Copyright (c) 2016 The Multiverse developers
# Distributed under the MIT/X11 software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
#------------------------------------------------------------------------------

set(sources
	destination.h 
	transaction.h 
	block.h
)

add_library(common ${sources})

include_directories(../walleve ../crypto)
target_link_libraries(common
	Boost::system
	Boost::filesystem
	Boost::program_options
	Boost::thread
	Boost::date_time
	Boost::regex
        OpenSSL::SSL
        OpenSSL::Crypto
	walleve
	crypto
)
