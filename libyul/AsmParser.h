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
#include <string_view>

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
		m_useSourceLocationFrom{
			_locationOverride ?
			UseSourceLocationFrom::LocationOverride :
			UseSourceLocationFrom::Scanner
		}
	{}

	/// Constructs a Yul parser that is using the debug data
	/// from the comments (via @src and other tags).
	explicit Parser(
		langutil::ErrorReporter& _errorReporter,
		Dialect const& _dialect,
		std::optional<std::map<unsigned, std::shared_ptr<std::string const>>> _sourceNames
	):
		ParserBase(_errorReporter),
		m_dialect(_dialect),
		m_sourceNames{std::move(_sourceNames)},
		m_useSourceLocationFrom{
			m_sourceNames.has_value() ?
			UseSourceLocationFrom::Comments :
			UseSourceLocationFrom::Scanner
		}
	{}

	/// Parses an inline assembly block starting with `{` and ending with `}`.
	/// @returns an empty shared pointer on error.
	std::unique_ptr<AST> parseInline(std::shared_ptr<langutil::Scanner> const& _scanner);

	/// Parses an assembly block starting with `{` and ending with `}`
	/// and expects end of input after the '}'.
	/// @returns an empty shared pointer on error.
	std::unique_ptr<AST> parse(langutil::CharStream& _charStream);

protected:
	langutil::SourceLocation currentLocation() const override
	{
		if (m_useSourceLocationFrom == UseSourceLocationFrom::LocationOverride)
			return m_locationOverride;

		return ParserBase::currentLocation();
	}

	langutil::Token advance() override;

	void fetchDebugDataFromComment();

	std::optional<std::pair<std::string_view, langutil::SourceLocation>> parseSrcComment(
		std::string_view _arguments,
		langutil::SourceLocation const& _commentLocation
	);

	std::optional<std::pair<std::string_view, std::optional<int>>> parseASTIDComment(
		std::string_view _arguments,
		langutil::SourceLocation const& _commentLocation
	);

	/// Creates a DebugData object with the correct source location set.
	langutil::DebugData::ConstPtr createDebugData() const;

	void updateLocationEndFrom(
		langutil::DebugData::ConstPtr& _debugData,
		langutil::SourceLocation const& _location
	) const;

	/// Creates an inline assembly node with the current debug data.
	template <class T> T createWithDebugData() const
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
	Expression parseExpression(bool _unlimitedLiteralArgument = false);
	/// Parses an elementary operation, i.e. a literal, identifier, instruction or
	/// builtin function call (only the name).
	std::variant<Literal, Identifier> parseLiteralOrIdentifier(bool _unlimitedLiteralArgument = false);
	VariableDeclaration parseVariableDeclaration();
	FunctionDefinition parseFunctionDefinition();
	FunctionCall parseCall(std::variant<Literal, Identifier>&& _initialOp);
	NameWithDebugData parseNameWithDebugData();
	YulName expectAsmIdentifier();
	void raiseUnsupportedTypesError(langutil::SourceLocation const& _location) const;

	/// Reports an error if we are currently not inside the body part of a for loop.
	void checkBreakContinuePosition(std::string const& _which);

	static bool isValidNumberLiteral(std::string const& _literal);

private:
	Dialect const& m_dialect;

	std::optional<std::map<unsigned, std::shared_ptr<std::string const>>> m_sourceNames;
	langutil::SourceLocation m_locationOverride;
	langutil::SourceLocation m_locationFromComment;
	std::optional<int64_t> m_astIDFromComment;
	UseSourceLocationFrom m_useSourceLocationFrom = UseSourceLocationFrom::Scanner;
	ForLoopComponent m_currentForLoopComponent = ForLoopComponent::None;
	bool m_insideFunction = false;
};

}
