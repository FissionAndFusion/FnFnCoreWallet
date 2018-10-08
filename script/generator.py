#!/usr/bin/env python
#-*-conding:utf-8-*-

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