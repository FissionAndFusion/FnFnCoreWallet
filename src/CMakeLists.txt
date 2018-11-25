#------------------------------------------------------------------------------
# CMake file for Multiverse
#
# Copyright (c) 2016 The Multiverse developers
# Distributed under the MIT/X11 software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
#------------------------------------------------------------------------------

# auto generate files
set(auto_files
	${CMAKE_CURRENT_BINARY_DIR}/rpc/auto_protocol.cpp
	${CMAKE_CURRENT_BINARY_DIR}/rpc/auto_protocol.h
	${CMAKE_CURRENT_BINARY_DIR}/rpc/auto_rpc.cpp
	${CMAKE_CURRENT_BINARY_DIR}/rpc/auto_rpc.h
	${CMAKE_CURRENT_BINARY_DIR}/mode/auto_options.h
)
add_custom_command(OUTPUT ${auto_files}
	COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/../script/generator.py" "'${CMAKE_CURRENT_BINARY_DIR}'"
	DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/../script/template/rpc.json"
			"${CMAKE_CURRENT_SOURCE_DIR}/../script/template/options.json"
			"${CMAKE_CURRENT_SOURCE_DIR}/../script/template/mode.json"
)

set(src
	multiverse.cpp
	address.cpp address.h
	config.cpp config.h
	core.cpp core.h
	dispatcher.cpp dispatcher.h
	entry.cpp entry.h
	error.cpp error.h
	miner.cpp miner.h
	netchn.cpp netchn.h
	delegatedchn.cpp delegatedchn.h
	network.cpp network.h
	rpcclient.cpp rpcclient.h
	rpcjson.cpp rpcjson.h
	rpcmod.cpp rpcmod.h
	consensus.cpp consensus.h
	blockmaker.cpp blockmaker.h
	rpcclient.cpp rpcclient.h
	miner.cpp miner.h

	dbpservice.cpp dbpservice.h
	dbptype.h
	dbpserver.h
	dbpserver.cpp
	dbputils.h
	dnseed.cpp dnseed.h
	schedule.cpp schedule.h
	service.cpp service.h
	txpool.cpp txpool.h
	wallet.cpp wallet.h
	worldline.cpp worldline.h
	event.h
	mvbase.h
	mvtype.h
	param.h
	storage.h
	version.h
)
set(mode_src
	mode/basic_config.cpp mode/basic_config.h
	mode/fork_config.cpp mode/fork_config.h
	mode/mint_config.cpp mode/mint_config.h
	mode/network_config.cpp mode/network_config.h
	mode/rpc_config.cpp mode/rpc_config.h
	mode/dbp_config.cpp mode/dbp_config.h
	mode/storage_config.cpp mode/storage_config.h
	mode/config_macro.h
	mode/config_type.h
	mode/mode.h
	mode/mode_impl.h
	mode/mode_type.h
	mode/module_type.h
)
set(rpc_src
	rpc/rpc.cpp rpc/rpc.h
	rpc/rpc_error.cpp rpc/rpc_error.h
	rpc/rpc_req.cpp rpc/rpc_req.h
	rpc/rpc_resp.cpp rpc/rpc_resp.h
	rpc/rpc_type.h
)

set(sources
	${auto_files}
	${src}
	${mode_src}
	${rpc_src}
)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS 
dbp-proto/dbp.proto 
dbp-proto/lws.proto)

add_executable(multiverse ${sources} ${PROTO_SRCS} ${PROTO_HDRS})

include_directories(../walleve ../crypto ../common ../storage ../network ../delegate ../mpvss ./)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${Protobuf_INCLUDE_DIRS})

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
	${Protobuf_LIBRARIES}
	delegate	
	${Readline_LIBRARY}
)

# create symbol link
set(exec_link multiverse-cli multiverse-server multiverse-miner multiverse-dnseed)
foreach(link ${exec_link})
	add_custom_command(TARGET multiverse POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E create_symlink multiverse ${link}
	)
endforeach()

# install to path
foreach(link multiverse ${exec_link})
	install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/${link}"
		DESTINATION bin
	)
endforeach()
