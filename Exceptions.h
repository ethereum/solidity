/*
    This file is part of cpp-ethereum.

    cpp-ethereum is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    cpp-ethereum is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Solidity exception hierarchy.
 */

#pragma once

#include <string>
#include <libdevcore/Exceptions.h>
#include <libsolidity/BaseTypes.h>

namespace dev
{
namespace solidity
{

struct ParserError: virtual Exception {};
struct TypeError: virtual Exception {};
struct DeclarationError: virtual Exception {};
struct CompilerError: virtual Exception {};
struct InternalCompilerError: virtual Exception {};
struct DocstringParsingError: virtual Exception {};

using errinfo_sourceLocation = boost::error_info<struct tag_sourceLocation, Location>;

}
}
