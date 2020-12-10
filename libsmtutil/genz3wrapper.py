#!/usr/bin/env python3
# ------------------------------------------------------------------------------
# This file is part of solidity.
#
# solidity is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# solidity is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with solidity.  If not, see <http://www.gnu.org/licenses/>
#------------------------------------------------------------------------------
#
# Script that generates a dlsym-wrapper for Z3 from the header files.
# Expects all Z3 headers as arguments and outputs the wrapper code to stdout.

import sys
import re

# Patterns to match Z3 API entry point definitions.
def_pat = re.compile(" *def_API(.*)")
extradef_pat = re.compile(" *extra_API(.*)")
# Pattern to extract name and arguments from the above.
def_args_pat = re.compile("\('([^']*)'[^\(\)]*\((.*)\)\s*\)")
# Pattern to extract a list of arguments from the above.
arg_list_pat = re.compile("[^\(]*\([^\)]*\)[, ]*")

def generateEntryPoint(line, args):
    m = def_args_pat.match(args)
    if not m:
        raise Exception('Could not parse entry point definition: ' + line)
    name = m.group(1)
    num_args = len(arg_list_pat.findall(m.group(2)))
    arglist = ', '.join(f"_{i}" for i in range(num_args))
    paramlist = ', '.join(f"ArgType<&{name}, {i}> _{i}" for i in range(num_args))
    print(f'ResultType<&{name}> Z3_API {name}({paramlist})')
    print('{')
    print(f'\tstatic auto sym = reinterpret_cast<decltype(&{name})>(Z3Loader::get().loadSymbol(\"{name}\"));')
    print(f'\treturn sym({arglist});')
    print('}')


print(r"""// This file is auto-generated from genz3wrapper.py
#include <libsmtutil/Z3Loader.h>
#include <tuple>
#include <z3.h>

namespace
{

template<typename T>
struct FunctionTrait;

template<typename R, typename... Args>
struct FunctionTrait<R(*)(Args...)>
{
    using ResultType = R;
    template<unsigned N>
    using ArgType = std::tuple_element_t<N, std::tuple<Args...>>;
};

template<auto F>
using ResultType = typename FunctionTrait<decltype(F)>::ResultType;

template<auto F, unsigned N>
using ArgType = typename FunctionTrait<decltype(F)>::template ArgType<N>;

}

using namespace solidity;
using namespace solidity::smtutil;

extern "C"
{

void Z3_API Z3_set_error_handler(Z3_context c, Z3_error_handler h)
{
    static auto sym = reinterpret_cast<decltype(&Z3_set_error_handler)>(Z3Loader::get().loadSymbol("Z3_set_error_handler"));
    sym(c, h);
}
""")

for header in sys.argv[1:]:
    with open(header, 'r') as f:
        for line in f:
            line = line.strip('\r\n\t ')
            m = def_pat.match(line)
            if m:
                generateEntryPoint(line, m.group(1).strip('\r\n\t '))
            m = extradef_pat.match(line)
            if m:
                generateEntryPoint(line, m.group(1).strip('\r\n\t '))

print('}')
