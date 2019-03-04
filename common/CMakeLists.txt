#------------------------------------------------------------------------------
# CMake file for Multiverse
#
# Copyright (c) 2016 The Multiverse developers
# Distributed under the MIT/X11 software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
#------------------------------------------------------------------------------

set(template
	template/templateid.h template/templateid.cpp 
	template/template.h template/template.cpp
	template/weighted.h template/weighted.cpp 
	template/multisig.h template/multisig.cpp 
	template/fork.h template/fork.cpp 
	template/mint.h template/mint.cpp 
	template/proof.h template/proof.cpp 
	template/delegate.h template/delegate.cpp 
)

set(sources
	destination.h destination.cpp 
	transaction.h 
	wallettx.h 
	proof.h
	profile.h profile.cpp
	block.h
	forkcontext.h
	${template}
)

add_library(common ${sources})

include_directories(../walleve ../crypto ../jsonrpc ./)
include_directories(${CMAKE_BINARY_DIR}/jsonrpc)
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
