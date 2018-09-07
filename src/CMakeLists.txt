#------------------------------------------------------------------------------
# CMake file for Multiverse
#
# Copyright (c) 2016 The Multiverse developers
# Distributed under the MIT/X11 software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
#------------------------------------------------------------------------------

aux_source_directory(mode mode_src)
set(sources
	multiverse.cpp
	usage.cpp
	version.h
	address.cpp address.h
	error.cpp error.h
	config.cpp config.h
	entry.cpp entry.h        
	mvbase.h
	mvtype.h
	event.h
	core.cpp core.h
	worldline.cpp worldline.h
	txpool.cpp txpool.h
	dispatcher.cpp dispatcher.h
	network.cpp network.h
	netchn.cpp netchn.h
	schedule.cpp schedule.h
	wallet.cpp wallet.h
	service.cpp service.h
	rpcjson.cpp rpcjson.h
	rpcmod.cpp rpcmod.h
	blockmaker.cpp blockmaker.h
	rpcclient.cpp rpcclient.h
	miner.cpp miner.h
	${mode_src}
)

add_executable(multiverse ${sources})

include_directories(../walleve ../crypto ../common ../storage ../network ./)

target_link_libraries(multiverse
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
	storage	
	network	
	${WS2_32_LIB}
	${WSSOCK32_LIB}
	${Readline_LIBRARY}
)
