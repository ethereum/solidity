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

#include <vector>
#include <liblangutil/Token.h>

namespace dev
{
namespace solidity
{

class SemVerError: dev::Exception
{
};

#undef major
#undef minor

struct SemVerVersion
{
	unsigned numbers[3];
	std::string prerelease;
	std::string build;

	unsigned major() const { return numbers[0]; }
	unsigned minor() const { return numbers[1]; }
	unsigned patch() const { return numbers[2]; }

	bool isPrerelease() const { return !prerelease.empty(); }

	explicit SemVerVersion(std::string const& _versionString = "0.0.0");
};

struct SemVerMatchExpression
{
	bool matches(SemVerVersion const& _version) const;

	bool isValid() const { return !m_disjunction.empty(); }

	struct MatchComponent
	{
		/// Prefix from < > <= >= ~ ^
		Token prefix = Token::Illegal;
		/// Version, where unsigned(-1) in major, minor or patch denotes '*', 'x' or 'X'
		SemVerVersion version;
		/// Whether we have 1, 1.2 or 1.2.4
		unsigned levelsPresent = 1;
		bool matches(SemVerVersion const& _version) const;
	};

	struct Conjunction
	{
		std::vector<MatchComponent> components;
		bool matches(SemVerVersion const& _version) const;
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

	char currentChar() const;
	char nextChar();
	Token currentToken() const;
	void nextToken();

	std::vector<Token> m_tokens;
	std::vector<std::string> m_literals;

	unsigned m_pos = 0;
	unsigned m_posInside = 0;

	SemVerMatchExpression m_expression;
};

}
}
