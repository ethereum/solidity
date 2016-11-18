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

#include <functional>
#include <libsolidity/interface/Exceptions.h>

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
struct Identifier;

class CodeGenerator
{
public:
	enum class IdentifierContext { LValue, RValue };
	/// Function type that is called for external identifiers. Such a function should search for
	/// the identifier and append appropriate assembly items to the assembly. If in lvalue context,
	/// the value to assign is assumed to be on the stack and an assignment is to be performed.
	/// If in rvalue context, the function is assumed to append instructions to
	/// push the value of the identifier onto the stack. On error, the function should return false.
	using IdentifierAccess = std::function<bool(assembly::Identifier const&, eth::Assembly&, IdentifierContext)>;
	CodeGenerator(Block const& _parsedData, ErrorList& _errors):
		m_parsedData(_parsedData), m_errors(_errors) {}
	/// Performs type checks and @returns false on error.
	/// Actually runs the full code generation but discards the result.
	bool typeCheck(IdentifierAccess const& _identifierAccess = IdentifierAccess());
	/// Performs code generation and @returns the result.
	eth::Assembly assemble(IdentifierAccess const& _identifierAccess = IdentifierAccess());
	/// Performs code generation and appends generated to to _assembly.
	void assemble(eth::Assembly& _assembly, IdentifierAccess const& _identifierAccess = IdentifierAccess());

private:
	Block const& m_parsedData;
	ErrorList& m_errors;
};

}
}
}
