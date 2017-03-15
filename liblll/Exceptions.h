/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file Exceptions.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <libdevcore/Exceptions.h>

namespace dev
{
namespace eth
{

/// Compile a Low-level Lisp-like Language program into EVM-code.
class CompilerException: public dev::Exception {};
class InvalidOperation: public CompilerException {};
class IntegerOutOfRange: public CompilerException {};
class EmptyList: public CompilerException {};
class DataNotExecutable: public CompilerException {};
class IncorrectParameterCount: public CompilerException {};
class InvalidName: public CompilerException {};
class InvalidMacroArgs: public CompilerException {};
class InvalidLiteral: public CompilerException {};
class BareSymbol: public CompilerException {};
class ParserException: public CompilerException {};

}
}
