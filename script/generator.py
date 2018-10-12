#!/usr/bin/env python
# -*-conding:utf-8-*-

import sys
sys.dont_write_bytecode = True

import os
import options_generator as options
import rpc_protocol_generator as protocol
import rpc_generator as rpc
from tool import *


def create_dir(dir):
    if not os.path.exists(dir):
        os.makedirs(dir)


def delete_file(file):
    if os.path.exists(file):
        os.remove(file)


# script dictionary
root = os.path.dirname(__file__)
# json files dictionary
json_root = os.path.abspath(os.path.join(root, 'template'))
# cpp files dictionary
cpp_src_root = os.path.abspath(os.path.join(root, '../build/src'))
if len(sys.argv) > 1:
    cpp_src_root = sys.argv[1]
    if not os.path.isabs(cpp_src_root):
        cpp_src_root = os.path.join(os.getcwd(), cpp_src_root)

create_dir(os.path.join(cpp_src_root, 'mode'))
create_dir(os.path.join(cpp_src_root, 'rpc'))

# json file name
json_files = {
    'options_json': 'options.json',
    'rpc_json': 'rpc.json',
    'mode_json': 'mode.json',
}

# cpp file name
cpp_files = {
    'options_h': 'mode/auto_options.h',
    'rpc_h': 'rpc/auto_rpc.h',
    'rpc_cpp': 'rpc/auto_rpc.cpp',
    'rpc_protocol_h': 'rpc/auto_protocol.h',
    'rpc_protocol_cpp': 'rpc/auto_protocol.cpp',
}


def generate():
    # json files path
    for k, v in json_files.items():
        json_files[k] = os.path.join(json_root, v)

    # cpp files path
    for k, v in cpp_files.items():
        # new cpp file path
        cpp_files[k] = os.path.join(cpp_src_root, v)

    # generate auto_options.h
    options.generate_options(json_files['options_json'], cpp_files['options_h'])

    # generate auto_rpc.h auto_rpc.cpp
    protocol.generate_protocol(json_files['rpc_json'], cpp_files['rpc_protocol_h'], cpp_files['rpc_protocol_cpp'])

    # generate auto_rpc.h auto_rpc.cpp
    rpc.generate_rpc(json_files['mode_json'], json_files['rpc_json'], cpp_files['rpc_h'], cpp_files['rpc_cpp'])


if __name__ == '__main__':
    generate()
