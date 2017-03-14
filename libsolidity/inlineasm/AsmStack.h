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
 * Full-stack Solidity inline assember.
 */

#pragma once

#include <libsolidity/interface/Exceptions.h>

#include <string>
#include <functional>

namespace dev
{
namespace eth
{
class Assembly;
}
namespace solidity
{
class Scanner;
namespace assembly
{
struct Block;
struct Identifier;

enum class IdentifierContext { LValue, RValue };

/// Object that is used to resolve references and generate code for access to identifiers external
/// to inline assembly (not used in standalone assembly mode).
struct ExternalIdentifierAccess
{
	/// Resolve a an external reference given by the identifier in the given context.
	/// @returns the size of the value (number of stack slots) or size_t(-1) if not found.
	std::function<size_t(assembly::Identifier const&, IdentifierContext)> resolve;
	/// Generate code for retrieving the value (rvalue context) or storing the value (lvalue context)
	/// of an identifier. The code should be appended to the assembly. In rvalue context, the value is supposed
	/// to be put onto the stack, in lvalue context, the value is assumed to be at the top of the stack.
	std::function<void(assembly::Identifier const&, IdentifierContext, eth::Assembly&)> generateCode;
};

class InlineAssemblyStack
{
public:
	/// Parse the given inline assembly chunk starting with `{` and ending with the corresponding `}`.
	/// @return false or error.
	bool parse(std::shared_ptr<Scanner> const& _scanner);
	/// Converts the parser result back into a string form (not necessarily the same form
	/// as the source form, but it should parse into the same parsed form again).
	std::string toString();

	eth::Assembly assemble();

	/// Parse and assemble a string in one run - for use in Solidity code generation itself.
	bool parseAndAssemble(
		std::string const& _input,
		eth::Assembly& _assembly,
		ExternalIdentifierAccess const& _identifierAccess = ExternalIdentifierAccess()
	);

	ErrorList const& errors() const { return m_errors; }

private:
	std::shared_ptr<Block> m_parserResult;
	ErrorList m_errors;
};

}
}
}
