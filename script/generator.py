#!/usr/bin/env python
#-*-conding:utf-8-*-
#
# 1. option.json
# Parse option json to generate config class
#
# Explanation:
# CMvRPCBasicConfigOption": [             // Class name, content is array
#   {                                   // Each object of array is one parameter
#       "access": "protected",          // (optional, default="public"). The access modifier of c++ class
#       "name": "nRPCPortInt",          // (required) parameter name
#       "type": "int",                  // (required) parameter type
#       "opt": "rpcport",               // (required) option of command-line
#       "default": 0,                   // (optional) default value of parameter
#       "format": "-rpcport=port",      // (required) show formatting when help
#       "desc": "Listen for JSON-RPC"   // (optional) show description when help
#   },
#   ...     // next parameters
# ]
#
# 2. mode.json
# Set usage and description of mode 
#
# Explanation:
# "EModeType::SERVER": {                        // Mode type, defiend in src/mode/mode_type.h
#     "usage": "multiverse-server [OPTIONS]",   // Usage
#     "desc": "Run multiverse server"           // description
# },
#
# 3. rpc.json
# Define rpc protocol
#
import sys
sys.dont_write_bytecode = True

import os
import options_generator as options
import rpc_protocol_generator as protocol
import rpc_generator as rpc
from tool import *

root = os.path.dirname(__file__)
json_root = os.path.abspath(os.path.join(root, 'template'))
cpp_src_root = os.path.abspath(os.path.join(root, '../src'))

options_json = os.path.join(json_root, 'options.json')
rpc_json = os.path.join(json_root, 'rpc.json')
mode_json = os.path.join(json_root, 'mode.json')

options_h = os.path.join(cpp_src_root, 'mode/auto_options.h')
rpc_h = os.path.join(cpp_src_root, 'rpc/auto_rpc.h')
rpc_cpp = os.path.join(cpp_src_root, 'rpc/auto_rpc.cpp')
rpc_protocol_h = os.path.join(cpp_src_root, 'rpc/auto_protocol.h')
rpc_protocol_cpp = os.path.join(cpp_src_root, 'rpc/auto_protocol.cpp')


def generate():
    # generate auto_options.h
    options.generate_options(options_json, options_h)
    
    # generate auto_rpc.h auto_rpc.cpp
    protocol.generate_protocol(rpc_json, rpc_protocol_h, rpc_protocol_cpp)

    # generate auto_rpc.h auto_rpc.cpp
    rpc.generate_rpc(mode_json, rpc_json, rpc_h, rpc_cpp)


if __name__ == '__main__':
    generate()