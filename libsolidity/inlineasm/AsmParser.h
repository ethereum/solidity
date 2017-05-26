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
 * Solidity inline assembly parser.
 */

#pragma once

#include <memory>
#include <vector>
#include <libsolidity/inlineasm/AsmData.h>
#include <libsolidity/parsing/ParserBase.h>

namespace dev
{
namespace solidity
{
namespace assembly
{

class Parser: public ParserBase
{
public:
	explicit Parser(ErrorList& _errors, bool _julia = false): ParserBase(_errors), m_julia(_julia) {}

	/// Parses an inline assembly block starting with `{` and ending with `}`.
	/// @returns an empty shared pointer on error.
	std::shared_ptr<Block> parse(std::shared_ptr<Scanner> const& _scanner);

protected:
	/// Creates an inline assembly node with the given source location.
	template <class T> T createWithLocation(SourceLocation const& _loc = SourceLocation())
	{
		T r;
		r.location = _loc;
		if (r.location.isEmpty())
		{
			r.location.start = position();
			r.location.end = endPosition();
		}
		if (!r.location.sourceName)
			r.location.sourceName = sourceName();
		return r;
	}
	SourceLocation location() const { return SourceLocation(position(), endPosition(), sourceName()); }

	Block parseBlock();
	Statement parseStatement();
	Case parseCase();
	/// Parses a functional expression that has to push exactly one stack element
	Statement parseExpression();
	std::map<std::string, dev::solidity::Instruction> const& instructions();
	Statement parseElementaryOperation(bool _onlySinglePusher = false);
	VariableDeclaration parseVariableDeclaration();
	FunctionDefinition parseFunctionDefinition();
	Statement parseCall(Statement&& _instruction);
	TypedName parseTypedName();
	std::string expectAsmIdentifier();

private:
	bool m_julia = false;
};

}
}
}
