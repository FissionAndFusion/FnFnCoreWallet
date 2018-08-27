
#------------------------------------------------------------------------------
# CMake file for Mutiverse Tests
#
# Copyright (c) 2016 The Multiverse developers
# Distributed under the MIT/X11 software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

set(sources
main.cpp
)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS 
../../walleve/walleve/dbp/dbp-proto/dbp.proto 
../../walleve/walleve/dbp/dbp-proto/lws.proto)

add_executable(dbpclient ${sources}  ${PROTO_SRCS} ${PROTO_HDRS})

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${Protobuf_INCLUDE_DIRS})

target_link_libraries(dbpclient
Boost::system
Boost::filesystem
Boost::program_options
Boost::thread
Boost::date_time
Boost::regex
    OpenSSL::SSL
    OpenSSL::Crypto
${Protobuf_LIBRARIES}
)