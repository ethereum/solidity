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
// SPDX-License-Identifier: GPL-3.0
/**
 * @author Christian <c@ethdev.com>
 * @date 2016
 * Solidity inline assembly parser.
 */

#pragma once

#include <libyul/ASTForward.h>
#include <libyul/Dialect.h>

#include <liblangutil/SourceLocation.h>
#include <liblangutil/Scanner.h>
#include <liblangutil/ParserBase.h>

#include <memory>
#include <variant>
#include <vector>

namespace solidity::yul
{

class Parser: public langutil::ParserBase
{
public:
	enum class ForLoopComponent
	{
		None, ForLoopPre, ForLoopPost, ForLoopBody
	};

	explicit Parser(
		langutil::ErrorReporter& _errorReporter,
		Dialect const& _dialect,
		std::optional<langutil::SourceLocation> _locationOverride = {}
	):
		ParserBase(_errorReporter),
		m_dialect(_dialect),
		m_locationOverride(std::move(_locationOverride))
	{}

	/// Parses an inline assembly block starting with `{` and ending with `}`.
	/// @param _reuseScanner if true, do check for end of input after the `}`.
	/// @returns an empty shared pointer on error.
	std::unique_ptr<Block> parse(std::shared_ptr<langutil::Scanner> const& _scanner, bool _reuseScanner);

protected:
	langutil::SourceLocation currentLocation() const override
	{
		return m_locationOverride ? *m_locationOverride : ParserBase::currentLocation();
	}

	/// Creates an inline assembly node with the current source location.
	template <class T> T createWithLocation() const
	{
		T r;
		r.location = currentLocation();
		return r;
	}

	Block parseBlock();
	Statement parseStatement();
	Case parseCase();
	ForLoop parseForLoop();
	/// Parses a functional expression that has to push exactly one stack element
	Expression parseExpression();
	/// Parses an elementary operation, i.e. a literal, identifier, instruction or
	/// builtin functian call (only the name).
	std::variant<Literal, Identifier> parseLiteralOrIdentifier();
	VariableDeclaration parseVariableDeclaration();
	FunctionDefinition parseFunctionDefinition();
	FunctionCall parseCall(std::variant<Literal, Identifier>&& _initialOp);
	TypedName parseTypedName();
	YulString expectAsmIdentifier();

	/// Reports an error if we are currently not inside the body part of a for loop.
	void checkBreakContinuePosition(std::string const& _which);

	static bool isValidNumberLiteral(std::string const& _literal);

private:
	Dialect const& m_dialect;
	std::optional<langutil::SourceLocation> m_locationOverride;
	ForLoopComponent m_currentForLoopComponent = ForLoopComponent::None;
	bool m_insideFunction = false;
};

}
