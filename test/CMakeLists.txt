#------------------------------------------------------------------------------
# CMake file for Multiverse
#
# Copyright (c) 2018 The Multiverse developers
# Distributed under the MIT/X11 software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
#------------------------------------------------------------------------------

set(sources
	test_fnfn_main.cpp
	test_fnfn.h test_fnfn.cpp
	uint256_tests.cpp
	rpc_tests.cpp
	version_tests.cpp
	mpvss_tests.cpp
	crypto_tests.cpp
	ipv6_tests.cpp
)

add_executable(test_fnfn ${sources})

include_directories(../src ../walleve ../crypto ../common ../storage ../network ../mpvss)

target_link_libraries(test_fnfn
	Boost::unit_test_framework
	Boost::system
	Boost::thread
	mpvss
	crypto
)

add_executable(test_ctsdb test_fnfn_main.cpp test_fnfn.h test_fnfn.cpp ctsdb_test.cpp)
target_link_libraries(test_ctsdb
        Boost::unit_test_framework
        Boost::system
        Boost::thread
        storage
)
