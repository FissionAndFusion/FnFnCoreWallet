#------------------------------------------------------------------------------
# CMake file for Multiverse
#
# Copyright (c) 2016 The Multiverse developers
# Distributed under the MIT/X11 software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
#------------------------------------------------------------------------------

set(sources
	timeseries.cpp timeseries.h
	blockdb.cpp blockdb.h
	blockbase.cpp blockbase.h
	blockindexdb.cpp blockindexdb.h
	walletdb.cpp walletdb.h
	txpooldb.cpp txpooldb.h
	unspentdb.cpp unspentdb.h
	delegatedb.cpp delegatedb.h
	forkdb.cpp forkdb.h
	purger.cpp purger.h
	leveldbeng.cpp leveldbeng.h
        txindexdb.cpp txindexdb.h
        ctsdb.cpp ctsdb.h
)

add_library(storage ${sources})

include_directories(../walleve ../crypto ../common ../leveldb/include ../snappy)
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
        leveldb	
        snappy	
)
