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
 * @author Christian <chris@ethereum.org>
 * @date 2016
 * Utilities to handle semantic versioning.
 */

#pragma once

#include <liblangutil/Token.h>
#include <string>
#include <vector>

namespace solidity::langutil
{

class SemVerError: util::Exception
{
};

#undef major
#undef minor

struct SemVerVersion
{
	unsigned numbers[3];
	std::string prerelease;
	std::string build;

	[[nodiscard]] unsigned major() const { return numbers[0]; }
	[[nodiscard]] unsigned minor() const { return numbers[1]; }
	[[nodiscard]] unsigned patch() const { return numbers[2]; }

	[[nodiscard]] bool isPrerelease() const { return !prerelease.empty(); }

	explicit SemVerVersion(std::string const& _versionString = "0.0.0");
};

struct SemVerMatchExpression
{
	[[nodiscard]] bool matches(SemVerVersion const& _version) const;

	[[nodiscard]] bool isValid() const { return !m_disjunction.empty(); }

	struct MatchComponent
	{
		/// Prefix from < > <= >= ~ ^
		Token prefix = Token::Illegal;
		/// Version, where unsigned(-1) in major, minor or patch denotes '*', 'x' or 'X'
		SemVerVersion version;
		/// Whether we have 1, 1.2 or 1.2.4
		unsigned levelsPresent = 1;
		[[nodiscard]] bool matches(SemVerVersion const& _version) const;
	};

	struct Conjunction
	{
		std::vector<MatchComponent> components;
		[[nodiscard]] bool matches(SemVerVersion const& _version) const;
	};

	std::vector<Conjunction> m_disjunction;
};

class SemVerMatchExpressionParser
{
public:
	SemVerMatchExpressionParser(std::vector<Token> const& _tokens, std::vector<std::string> const& _literals):
		m_tokens(_tokens), m_literals(_literals)
	{}
	SemVerMatchExpression parse();

private:
	void reset();

	void parseMatchExpression();
	SemVerMatchExpression::MatchComponent parseMatchComponent();
	unsigned parseVersionPart();

	[[nodiscard]] char currentChar() const;
	char nextChar();
	[[nodiscard]] Token currentToken() const;
	void nextToken();

	std::vector<Token> m_tokens;
	std::vector<std::string> m_literals;

	unsigned m_pos = 0;
	unsigned m_posInside = 0;

	SemVerMatchExpression m_expression;
};

}
