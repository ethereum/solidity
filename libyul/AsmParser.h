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

#include <libyul/AsmData.h>
#include <libyul/Dialect.h>

#include <liblangutil/SourceLocation.h>
#include <liblangutil/Scanner.h>
#include <liblangutil/ParserBase.h>

#include <memory>
#include <variant>
#include <vector>

namespace yul
{

class Parser: public langutil::ParserBase
{
public:
	enum class ForLoopComponent
	{
		None, ForLoopPre, ForLoopPost, ForLoopBody
	};

	explicit Parser(langutil::ErrorReporter& _errorReporter, Dialect const& _dialect):
		ParserBase(_errorReporter), m_dialect(_dialect) {}

	/// Parses an inline assembly block starting with `{` and ending with `}`.
	/// @param _reuseScanner if true, do check for end of input after the `}`.
	/// @returns an empty shared pointer on error.
	std::shared_ptr<Block> parse(std::shared_ptr<langutil::Scanner> const& _scanner, bool _reuseScanner);

	/// @returns a map of all EVM instructions available to assembly.
	static std::map<std::string, dev::eth::Instruction> const& instructions();

protected:
	using ElementaryOperation = std::variant<Instruction, Literal, Identifier, FunctionCall>;

	/// Creates an inline assembly node with the given source location.
	template <class T> T createWithLocation(langutil::SourceLocation const& _loc = {}) const
	{
		T r;
		r.location = _loc;
		if (r.location.isEmpty())
		{
			r.location.start = position();
			r.location.end = endPosition();
		}
		if (!r.location.source)
			r.location.source = m_scanner->charStream();
		return r;
	}
	langutil::SourceLocation location() const { return {position(), endPosition(), m_scanner->charStream()}; }

	Block parseBlock();
	Statement parseStatement();
	Case parseCase();
	ForLoop parseForLoop();
	/// Parses a functional expression that has to push exactly one stack element
	Expression parseExpression();
	static std::map<dev::eth::Instruction, std::string> const& instructionNames();
	/// Parses an elementary operation, i.e. a literal, identifier, instruction or
	/// builtin functian call (only the name).
	ElementaryOperation parseElementaryOperation();
	VariableDeclaration parseVariableDeclaration();
	FunctionDefinition parseFunctionDefinition();
	Expression parseCall(ElementaryOperation&& _initialOp);
	TypedName parseTypedName();
	YulString expectAsmIdentifier();

	/// Reports an error if we are currently not inside the body part of a for loop.
	void checkBreakContinuePosition(std::string const& _which);

	static bool isValidNumberLiteral(std::string const& _literal);

private:
	Dialect const& m_dialect;
	ForLoopComponent m_currentForLoopComponent = ForLoopComponent::None;
};

}
