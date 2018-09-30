# -*-conding:utf-8-*-
import sys
sys.dont_write_bytecode = True

import json
import os
from collections import OrderedDict
from tool import *

root = os.path.dirname(__file__)
json_root = os.path.abspath(os.path.join(root, 'template'))
cpp_src_root = os.path.abspath(os.path.join(root, '../src'))

options_json = os.path.join(json_root, 'options.json')
options_h = os.path.join(cpp_src_root, 'mode/auto_options.h')

class Param:
    def __init__(self, p_name, p_type, p_opt, p_format, p_desc, p_default):
        self.name = p_name
        self.type = p_type
        self.opt = p_opt
        self.format = p_format
        self.default = p_default
        self.desc = p_desc


def generate_options(options_json_path = None, options_h_path = None):
    if options_json_path:
        options_json = options_json_path
    if options_h_path:
        options_h = options_h_path

    with open(options_json, 'r') as r:
        content = json.loads(r.read(), object_pairs_hook=OrderedDict)
        check_value_type(options_json, content, dict)

        with open(options_h, 'w') as w:
            # copyright
            w.write(copyright)
            # file top
            w.write(
                '''
#ifndef MULTIVERSE_AUTO_OPTIONS_H
#define MULTIVERSE_AUTO_OPTIONS_H

#include <string>
#include <vector>
#include <sstream>

#include "mode/config_macro.h"

using std::string;
using std::vector;
using std::ostringstream;

namespace multiverse
{
''')
            # classes
            for class_name, detail in content.items():
                check_value_type(class_name, detail, list)

                params_list = []
                params_public_list = []
                params_protected_list = []
                params_private_list = []

                # parse params
                for param in detail:
                    name = get_json_value(class_name, param, 'name', type=unicode)
                    type = get_json_value(class_name, param, 'type', type=unicode)
                    opt = get_json_value(class_name, param, 'opt', type=unicode)
                    fmt = get_json_value(class_name, param, 'format', type=unicode)
                    desc = get_desc(class_name, param)

                    default = get_json_value(class_name, param, 'default', required=False)
                    if default != None:
                        default = quote(default, type)

                    p = Param(name, type, opt, fmt, desc, default)
                    params_list.append(p)

                    access = get_json_value(class_name, param, 'format', type=unicode, default='public')
                    if access == "protected":
                        params_protected_list.append(p)
                    elif access == "private":
                        params_private_list.append(p)
                    else:
                        params_public_list.append(p)

                # class top
                w.write('class ' + class_name + '\n')
                indent = brace_begin(w)

                # declare param
                w.write('public:\n')
                for p in params_public_list:
                    w.write(indent + p.type + ' ' + p.name + ';\n')

                w.write('protected:\n')
                for p in params_protected_list:
                    w.write(indent + p.type + ' ' + p.name + ';\n')

                # function HelpImpl()
                w.write('protected:\n')
                w.write(indent + 'string HelpImpl() const\n')
                indent = brace_begin(w, indent)
                w.write(indent + 'ostringstream oss;\n')

                for p in params_list:
                    fmt_indent = options_indent + p.format
                    fmt_indent = fmt_indent + space(fmt_indent)
                    desc_list = split(p.desc)
                    w.write(terminal_str_code(indent, 'oss << ', fmt_indent, desc_list))

                w.write(indent + 'return oss.str();\n')
                indent = brace_end(w, indent)

                # function AddOptionsImpl()
                w.write(indent + 'void AddOptionsImpl(boost::program_options::options_description& desc)\n')
                indent = brace_begin(w, indent)
                for p in params_list:
                    w.write(indent + 'AddOpt(desc, "' + p.opt + '", ' + p.name)
                    if p.default:
                        type = p.type
                        if p.type.find(' ') >= 0:
                            type = '(' + p.type + ')'
                        w.write(', ' + type + '(' + p.default + '));\n')
                    else:
                        w.write(');\n')

                indent = brace_end(w, indent)

                # function AddOptionsImpl()
                w.write(indent + 'string ListConfigImpl() const\n')
                indent = brace_begin(w, indent)
                w.write(indent + 'ostringstream oss;\n')
                for p in params_list:
                    if p.type.find('vector') >= 0:
                        w.write(indent + 'oss << ' + quote(' -' + p.opt + ': ') + ';\n')
                        w.write(indent + 'for (auto& s: ' + p.name + ')\n')
                        indent = brace_begin(w, indent)
                        w.write(indent + 'oss << s << " ";\n')
                        indent = brace_end(w, indent)
                        w.write(indent + 'oss << "\\n";\n')
                    elif p.type == 'bool':
                        w.write(indent + 'oss << ' + quote(' -' + p.opt + ': ') + ' << (' + p.name + ' ? "Y" : "N") << "\\n";\n')
                    else:
                        w.write(indent + 'oss << ' + quote(' -' + p.opt + ': ') + ' << ' + p.name + ' << "\\n";\n')

                w.write(indent + 'return oss.str();\n')
                indent = brace_end(w, indent)

                # class bottom
                w.write('};\n')

            # file bottom
            w.write(
                '''\
} // namespace multiverse

#endif // MULTIVERSE_AUTO_OPTIONS_H
''')


if __name__ == '__main__':
    generate_options()