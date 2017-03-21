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
/**
 * @author Christian <c@ethdev.com>
 * @date 2016
 * Code-generating part of inline assembly.
 */

#pragma once

#include <libsolidity/interface/Exceptions.h>
#include <libsolidity/inlineasm/AsmStack.h>

#include <functional>

namespace dev
{
namespace eth
{
class Assembly;
}
namespace solidity
{
namespace assembly
{
struct Block;

class CodeGenerator
{
public:
	CodeGenerator(Block const& _parsedData, ErrorList& _errors):
		m_parsedData(_parsedData), m_errors(_errors) {}
	/// Performs code generation and @returns the result.
	eth::Assembly assemble(ExternalIdentifierAccess const& _identifierAccess = ExternalIdentifierAccess());
	/// Performs code generation and appends generated to to _assembly.
	void assemble(eth::Assembly& _assembly, ExternalIdentifierAccess const& _identifierAccess = ExternalIdentifierAccess());

private:
	Block const& m_parsedData;
	ErrorList& m_errors;
};

}
}
}
