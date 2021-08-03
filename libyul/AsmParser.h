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

#include <libyul/AST.h>
#include <libyul/ASTForward.h>
#include <libyul/Dialect.h>

#include <liblangutil/SourceLocation.h>
#include <liblangutil/Scanner.h>
#include <liblangutil/ParserBase.h>

#include <map>
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

	enum class UseSourceLocationFrom
	{
		Scanner, LocationOverride, Comments,
	};

	explicit Parser(
		langutil::ErrorReporter& _errorReporter,
		Dialect const& _dialect,
		std::optional<langutil::SourceLocation> _locationOverride = {}
	):
		ParserBase(_errorReporter),
		m_dialect(_dialect),
		m_locationOverride{_locationOverride ? *_locationOverride : langutil::SourceLocation{}},
		m_debugDataOverride{},
		m_useSourceLocationFrom{
			_locationOverride ?
			UseSourceLocationFrom::LocationOverride :
			UseSourceLocationFrom::Scanner
		}
	{}

	/// Constructs a Yul parser that is using the source locations
	/// from the comments (via @src).
	explicit Parser(
		langutil::ErrorReporter& _errorReporter,
		Dialect const& _dialect,
		std::optional<std::map<unsigned, std::shared_ptr<std::string const>>> _sourceNames
	):
		ParserBase(_errorReporter),
		m_dialect(_dialect),
		m_sourceNames{std::move(_sourceNames)},
		m_debugDataOverride{DebugData::create()},
		m_useSourceLocationFrom{
			m_sourceNames.has_value() ?
			UseSourceLocationFrom::Comments :
			UseSourceLocationFrom::Scanner
		}
	{}

	/// Parses an inline assembly block starting with `{` and ending with `}`.
	/// @returns an empty shared pointer on error.
	std::unique_ptr<Block> parseInline(std::shared_ptr<langutil::Scanner> const& _scanner);

	/// Parses an assembly block starting with `{` and ending with `}`
	/// and expects end of input after the '}'.
	/// @returns an empty shared pointer on error.
	std::unique_ptr<Block> parse(langutil::CharStream& _charStream);

protected:
	langutil::SourceLocation currentLocation() const override
	{
		if (m_useSourceLocationFrom == UseSourceLocationFrom::Scanner)
			return ParserBase::currentLocation();
		return m_locationOverride;
	}

	langutil::Token advance() override;

	void fetchSourceLocationFromComment();

	/// Creates a DebugData object with the correct source location set.
	std::shared_ptr<DebugData const> createDebugData() const;

	/// Creates an inline assembly node with the current source location.
	template <class T> T createWithLocation() const
	{
		T r;
		r.debugData = createDebugData();
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

	std::optional<std::map<unsigned, std::shared_ptr<std::string const>>> m_sourceNames;
	langutil::SourceLocation m_locationOverride;
	std::shared_ptr<DebugData const> m_debugDataOverride;
	UseSourceLocationFrom m_useSourceLocationFrom = UseSourceLocationFrom::Scanner;
	ForLoopComponent m_currentForLoopComponent = ForLoopComponent::None;
	bool m_insideFunction = false;
};

}
