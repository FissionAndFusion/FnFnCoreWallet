#------------------------------------------------------------------------------
# CMake file for Multiverse
#
# Copyright (c) 2016 The Multiverse developers
# Distributed under the MIT/X11 software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
#------------------------------------------------------------------------------

set(sources
	timeseries.cpp timeseries.h
	dbconn.cpp dbconn.h
	dbpool.cpp dbpool.h
	blockdb.cpp blockdb.h
	blockbase.cpp blockbase.h
	walletdb.cpp walletdb.h
	txpooldb.cpp txpooldb.h
)

add_library(storage ${sources})

include_directories(../walleve ../crypto ../common)
target_link_libraries(storage
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
	common
	${MYSQL_LIB}	
)
