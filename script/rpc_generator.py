#!/usr/bin/env python
# -*-conding:utf-8-*-

import sys
sys.dont_write_bytecode = True

import os
import json
from collections import OrderedDict
from tool import *

root = os.path.dirname(__file__)
json_root = os.path.abspath(os.path.join(root, 'template'))
cpp_src_root = os.path.abspath(os.path.join(root, '../src'))

rpc_json = os.path.join(json_root, 'rpc.json')
mode_json = os.path.join(json_root, 'mode.json')
rpc_h = os.path.join(cpp_src_root, 'rpc/auto_rpc.h')
rpc_cpp = os.path.join(cpp_src_root, 'rpc/auto_rpc.cpp')

modes = OrderedDict()
param_class = OrderedDict()
result_class = OrderedDict()
config_class = OrderedDict()
introduction = OrderedDict()


def CreateCRPCParam_h(w):
    w.write('// Create family class shared ptr of CRPCParam\n')
    w.write('CRPCParamPtr CreateCRPCParam(const std::string& cmd, const json_spirit::Value& valParam);\n')


def CreateCRPCParam_cpp(w):
    w.write('CRPCParamPtr CreateCRPCParam(const std::string& cmd, const json_spirit::Value& valParam)\n')
    indent = brace_begin(w)

    if len(param_class) == 0:
        w.write(
            indent + 'throw CRPCException(RPC_METHOD_NOT_FOUND, cmd + " not found!");\n')
    else:
        w.write(indent)
        for cmd, name in param_class.items():
            w.write('if (cmd == "' + cmd + '")\n')
            indent = brace_begin(w, indent)
            w.write(indent + 'auto ptr = Make' + name + 'Ptr();\n')
            w.write(indent + 'ptr->FromJSON(valParam);\n')
            w.write(indent + 'return ptr;\n')
            indent = brace_end(w, indent)
            w.write(indent + 'else ')

        empty_line(w)
        indent = brace_begin(w, indent)
        w.write(indent + 'throw CRPCException(RPC_METHOD_NOT_FOUND, cmd + " not found!");\n')
        indent = brace_end(w, indent)

    brace_end(w, indent)


def CreateCRPCResult_h(w):
    w.write('// Create family class shared ptr of CRPCResult\n')
    w.write('CRPCResultPtr CreateCRPCResult(const std::string& cmd, const json_spirit::Value& valResult);\n')


def CreateCRPCResult_cpp(w):
    w.write('CRPCResultPtr CreateCRPCResult(const std::string& cmd, const json_spirit::Value& valResult)\n')
    indent = brace_begin(w)

    if len(result_class) == 0:
        w.write(indent + 'return std::make_shared<CRPCCommonResult>();\n')
    else:
        w.write(indent)
        for cmd, name in result_class.items():
            if result_class:
                w.write('if (cmd == "' + cmd + '")\n')
                indent = brace_begin(w, indent)
                w.write(indent + 'auto ptr = Make' + name + 'Ptr();\n')
                w.write(indent + 'ptr->FromJSON(valResult);\n')
                w.write(indent + 'return ptr;\n')
                indent = brace_end(w, indent)
                w.write(indent + 'else ')

        empty_line(w)
        indent = brace_begin(w, indent)
        w.write(indent + 'auto ptr = std::make_shared<CRPCCommonResult>();\n')
        w.write(indent + 'ptr->FromJSON(valResult);\n')
        w.write(indent + 'return ptr;\n')
        indent = brace_end(w, indent)

    brace_end(w, indent)


def Help_h(w):
    w.write('// Return help info by mode type and sub command\n')
    w.write('std::string Help(EModeType type, const std::string& subCmd, const std::string& options = "");\n')


def Help_cpp(w):
    # write "Options"
    def write_options(w, indent):
        w.write(indent + 'oss << "Options:\\n";\n')
        w.write(indent + 'oss << options << "\\n";\n')
    
    # write 'commands'
    def write_introduction(cmd, introduction, w, indent):
        introduction_list = split(introduction, max_len = introduction_max_line_len)
        introduction_code = terminal_str_code(indent, 'oss << ', cmd + space(cmd), introduction_list)
        w.write(introduction_code)

    w.write('std::string Help(EModeType type, const std::string& subCmd, const std::string& options)\n')
    indent = brace_begin(w)

    w.write(indent + 'std::ostringstream oss;\n')
    w.write(indent)
    for mode in modes:
        usage, desc = modes[mode]
        w.write('if (type == ' + mode + ')\n')
        indent = brace_begin(w, indent)
        if mode != 'EModeType::CONSOLE':
            write_usage(usage, w, indent)
            write_desc(desc, w, indent)
            write_options(w, indent)
        else:
            # sub command is empty, output help
            w.write(indent + 'if (subCmd.empty())\n')
            indent = brace_begin(w, indent)

            # options is not empty, output options. It's multiverse-cli -help
            w.write(indent + 'if (!options.empty())\n')
            indent = brace_begin(w, indent)
            write_usage(usage, w, indent)
            write_desc(desc, w, indent)
            write_options(w, indent)
            indent = brace_end(w, indent)

            # output command list
            w.write(indent + 'oss << "Commands:\\n";\n')
            for cmd, intro in introduction.items():
                write_introduction(commands_indent + cmd, intro, w, indent)

            indent = brace_end(w, indent)

            # output one command help. If subCmd == "all", show all command help
            for cmd, class_name in config_class.items():
                w.write(indent + 'if (subCmd == "all" || subCmd == "' + cmd + '")\n')
                indent = brace_begin(w, indent)
                w.write(indent + 'oss << ' + class_name + '().Help();\n')
                indent = brace_end(w, indent)

        indent = brace_end(w, indent)
        w.write(indent + 'else ')

    empty_line(w)
    indent = brace_begin(w, indent)
    indent = brace_end(w, indent)

    w.write(indent + 'return oss.str();\n')
    brace_end(w, indent)


def CreateConfig_h(w):
    w.write('// Dynamic create combin config with rpc command\n')
    w.write('template <typename... T>\n')
    w.write('CMvBasicConfig* CreateConfig(const std::string& cmd)\n')
    indent = brace_begin(w)

    if len(config_class) == 0:
        w.write(indent + 'return new mode_impl::CCombinConfig<T...>();\n')
    else:
        w.write(indent)
        for cmd, name in config_class.items():
            w.write('if (cmd == "' + cmd + '")\n')
            indent = brace_begin(w, indent)
            w.write(indent + 'return new mode_impl::CCombinConfig<' +
                    name + ', T...>;\n')
            indent = brace_end(w, indent)
            w.write(indent + 'else ')

        empty_line(w)
        indent = brace_begin(w, indent)
        w.write(indent + 'return new mode_impl::CCombinConfig<T...>();\n')
        indent = brace_end(w, indent)

    brace_end(w, indent)


def RPCCmdList_h(w):
    w.write('// All rpc command list\n')
    w.write('const std::vector<std::string>& RPCCmdList();\n')


def RPCCmdList_cpp(w):
    w.write('const std::vector<std::string>& RPCCmdList()\n')
    indent = brace_begin(w)

    w.write(indent + 'static std::vector<std::string> list = \n')
    w.write(indent + '{\n')
    for cmd in param_class:
        w.write(indent + '\t' + quote(cmd) + ',\n')
    w.write(indent + '};\n')

    w.write(indent + 'return list;\n')

    brace_end(w, indent)


def parse():
    # parse mode.json
    with open(mode_json, 'r') as r:
        content = json.loads(r.read(), object_pairs_hook=OrderedDict)

        for mode, detail in content.items():
            usage = get_json_value(mode, detail, 'usage', unicode, u'')
            desc = get_desc(mode, detail)
            modes[mode] = (usage, desc)

    # parse rpc.json
    with open(rpc_json, 'r') as r:
        content = json.loads(r.read(), object_pairs_hook=OrderedDict)

        for cmd, detail in content.items():
            type = get_json_value(cmd, detail, 'type', unicode, u'command')
            if type == 'command':
                name = get_json_value(cmd, detail, 'name', unicode, default=cmd.title())
                if 'request' in detail:
                    param_class[cmd] = param_class_name(name)
                    config_class[cmd] = config_class_name(name)
                if 'response' in detail:
                    result_class[cmd] = result_class_name(name)

                intro = get_multiple_text(cmd, detail, 'introduction')
                if len(intro) == 0:
                    intro = get_desc(cmd, detail)
                introduction[cmd] = intro


def generate_h():
    with open(rpc_h, 'w') as w:
        # copyright
        w.write(copyright)
        # file top
        w.write(
            '''
#ifndef MULTIVERSE_RPC_AUTO_RPC_H
#define MULTIVERSE_RPC_AUTO_RPC_H

#include <tuple>
#include "mode/mode_type.h"
#include "mode/mode_impl.h"
#include "rpc/rpc_error.h"
#include "rpc/rpc_req.h"
#include "rpc/rpc_resp.h"
#include "rpc/auto_protocol.h"

namespace multiverse
{
namespace rpc
{
''')
        # function CreateCRPCParam
        CreateCRPCParam_h(w)
        empty_line(w)

        # function CreateCRPCResult
        CreateCRPCResult_h(w)
        empty_line(w)

        # function Help
        Help_h(w)
        empty_line(w)

        # function CreateConfig
        CreateConfig_h(w)
        empty_line(w)

        # function RPCCmdList
        RPCCmdList_h(w)
        empty_line(w)

        # help tips
        w.write('// help tips used when error occured or help\n')
        w.write('static const string strHelpTips = "\\n\\nRun \'help COMMAND\' for more information on a command.\\n";\n')

        # file bottom
        w.write(
            '''
}  // namespace rpc

}  // namespace multiverse

#endif  // MULTIVERSE_RPC_AUTO_RPC_H
''')


def generate_cpp():
    with open(rpc_cpp, 'w') as w:
        # file top
        w.write(
            '''\
#include "auto_rpc.h"

#include "rpc/rpc_error.h"

namespace multiverse
{
namespace rpc
{
''')

        # function CreateCRPCParam
        CreateCRPCParam_cpp(w)
        empty_line(w)

        # function CreateCRPCResult
        CreateCRPCResult_cpp(w)
        empty_line(w)

        # function Help
        Help_cpp(w)

        # function RPCCmdList
        RPCCmdList_cpp(w)

        # file bottom
        w.write(
            '''
}  // namespace rpc

}  // namespace multiverse
''')

# entry


def generate_rpc(mode_json_path = None, rpc_json_path = None, 
                 rpc_h_path = None, rpc_cpp_path = None):
    global mode_json, rpc_json, rpc_h, rpc_cpp
    if mode_json_path:
        mode_json = mode_json_path
    if rpc_json_path:
        rpc_json = rpc_json_path
    if rpc_h_path:
        rpc_h = rpc_h_path
    if rpc_cpp_path:
        rpc_cpp = rpc_cpp_path

    # parse json file
    parse()

    # generate template.h
    generate_h()

    # generate template.cpp
    generate_cpp()


if __name__ == '__main__':
    generate_rpc()