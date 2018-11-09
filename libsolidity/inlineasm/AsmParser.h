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
#include <liblangutil/SourceLocation.h>
#include <liblangutil/Scanner.h>
#include <liblangutil/ParserBase.h>

namespace dev
{
namespace solidity
{
namespace assembly
{

class Parser: public langutil::ParserBase
{
public:
	explicit Parser(langutil::ErrorReporter& _errorReporter, AsmFlavour _flavour = AsmFlavour::Loose):
		ParserBase(_errorReporter), m_flavour(_flavour) {}

	/// Parses an inline assembly block starting with `{` and ending with `}`.
	/// @param _reuseScanner if true, do check for end of input after the `}`.
	/// @returns an empty shared pointer on error.
	std::shared_ptr<Block> parse(std::shared_ptr<langutil::Scanner> const& _scanner, bool _reuseScanner);

protected:
	using ElementaryOperation = boost::variant<assembly::Instruction, assembly::Literal, assembly::Identifier>;

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
		if (!r.location.sourceName)
			r.location.sourceName = sourceName();
		return r;
	}
	langutil::SourceLocation location() const { return {position(), endPosition(), sourceName()}; }

	Block parseBlock();
	Statement parseStatement();
	Case parseCase();
	ForLoop parseForLoop();
	/// Parses a functional expression that has to push exactly one stack element
	assembly::Expression parseExpression();
	static std::map<std::string, dev::solidity::Instruction> const& instructions();
	static std::map<dev::solidity::Instruction, std::string> const& instructionNames();
	/// Parses an elementary operation, i.e. a literal, identifier or instruction.
	/// This will parse instructions even in strict mode as part of the full parser
	/// for FunctionalInstruction.
	ElementaryOperation parseElementaryOperation();
	VariableDeclaration parseVariableDeclaration();
	FunctionDefinition parseFunctionDefinition();
	assembly::Expression parseCall(ElementaryOperation&& _initialOp);
	TypedName parseTypedName();
	std::string expectAsmIdentifier();

	static bool isValidNumberLiteral(std::string const& _literal);

private:
	AsmFlavour m_flavour = AsmFlavour::Loose;
};

}
}
}
