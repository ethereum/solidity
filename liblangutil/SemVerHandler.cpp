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
 * @author Christian <chris@ethereum.org>
 * @date 2016
 * Utilities to handle semantic versioning.
 */

#include <liblangutil/SemVerHandler.h>

#include <liblangutil/Exceptions.h>

#include <functional>
#include <limits>
#include <fmt/format.h>

using namespace std::string_literals;
using namespace solidity;
using namespace solidity::langutil;
using namespace solidity::util;

SemVerMatchExpressionParser::SemVerMatchExpressionParser(std::vector<Token> _tokens, std::vector<std::string> _literals):
	m_tokens(std::move(_tokens)), m_literals(std::move(_literals))
{
	solAssert(m_tokens.size() == m_literals.size(), "");
}

SemVerVersion::SemVerVersion(std::string const& _versionString)
{
	auto i = _versionString.begin();
	auto end = _versionString.end();

	for (unsigned level = 0; level < 3; ++level)
	{
		unsigned v = 0;
		for (; i != end && '0' <= *i && *i <= '9'; ++i)
			v = v * 10 + unsigned(*i - '0');
		numbers[level] = v;
		if (level < 2)
		{
			if (i == end || *i != '.')
				solThrow(SemVerError, "Invalid versionString: "s + _versionString);
			else
				++i;
		}
	}
	if (i != end && *i == '-')
	{
		auto prereleaseStart = ++i;
		while (i != end && *i != '+') ++i;
		prerelease = std::string(prereleaseStart, i);
	}
	if (i != end && *i == '+')
	{
		auto buildStart = ++i;
		while (i != end) ++i;
		build = std::string(buildStart, i);
	}
	if (i != end)
		solThrow(SemVerError, "Invalid versionString "s + _versionString);
}

bool SemVerMatchExpression::MatchComponent::matches(SemVerVersion const& _version) const
{
	if (prefix == Token::BitNot)
	{
		MatchComponent comp = *this;

		comp.prefix = Token::GreaterThanOrEqual;
		if (!comp.matches(_version))
			return false;

		if (levelsPresent >= 2)
			comp.levelsPresent = 2;
		else
			comp.levelsPresent = 1;
		comp.prefix = Token::LessThanOrEqual;
		return comp.matches(_version);
	}
	else if (prefix == Token::BitXor)
	{
		MatchComponent comp = *this;

		comp.prefix = Token::GreaterThanOrEqual;
		if (!comp.matches(_version))
			return false;

		if (comp.version.numbers[0] == 0 && comp.levelsPresent != 1)
			comp.levelsPresent = 2;
		else
			comp.levelsPresent = 1;
		comp.prefix = Token::LessThanOrEqual;
		return comp.matches(_version);
	}
	else
	{
		int cmp = 0;
		bool didCompare = false;
		for (unsigned i = 0; i < levelsPresent && cmp == 0; i++)
			if (version.numbers[i] != std::numeric_limits<unsigned>::max())
			{
				didCompare = true;
				cmp = static_cast<int>(_version.numbers[i]) - static_cast<int>(version.numbers[i]);
			}

		if (cmp == 0 && !_version.prerelease.empty() && didCompare)
			cmp = -1;

		switch (prefix)
		{
		case Token::Assign:
			return cmp == 0;
		case Token::LessThan:
			return cmp < 0;
		case Token::LessThanOrEqual:
			return cmp <= 0;
		case Token::GreaterThan:
			return cmp > 0;
		case Token::GreaterThanOrEqual:
			return cmp >= 0;
		default:
			solAssert(false, "Invalid SemVer expression");
		}
		return false;
	}
}

bool SemVerMatchExpression::Conjunction::matches(SemVerVersion const& _version) const
{
	for (auto const& component: components)
		if (!component.matches(_version))
			return false;
	return true;
}

bool SemVerMatchExpression::matches(SemVerVersion const& _version) const
{
	if (!isValid())
		return false;
	for (auto const& range: m_disjunction)
		if (range.matches(_version))
			return true;
	return false;
}

SemVerMatchExpression SemVerMatchExpressionParser::parse()
{
	reset();

	if (m_tokens.empty())
		solThrow(SemVerError, "Empty version pragma.");

	try
	{
		while (true)
		{
			parseMatchExpression();
			if (m_pos >= m_tokens.size())
				break;
			if (currentToken() != Token::Or)
			{
				solThrow(
					SemVerError,
					"You can only combine version ranges using the || operator."
				);
			}
			nextToken();
		}
	}
	catch (SemVerError const& e)
	{
		reset();
		throw e;
	}

	return m_expression;
}


void SemVerMatchExpressionParser::reset()
{
	m_expression = SemVerMatchExpression();
	m_pos = 0;
	m_posInside = 0;
}

void SemVerMatchExpressionParser::parseMatchExpression()
{
	// component - component (range)
	// or component component* (conjunction)

	SemVerMatchExpression::Conjunction range;
	range.components.push_back(parseMatchComponent());
	if (currentToken() == Token::Sub)
	{
		nextToken();
		range.components.push_back(parseMatchComponent());

		if (TokenTraits::isPragmaOp(range.components[0].prefix) || TokenTraits::isPragmaOp(range.components[1].prefix))
		{
			solThrow(
				SemVerError,
				"You cannot use operators (<, <=, >=, >, ^) with version ranges (-)."
			);
		}

		range.components[0].prefix = Token::GreaterThanOrEqual;
		range.components[1].prefix = Token::LessThanOrEqual;
	}
	else
		while (currentToken() != Token::Or && currentToken() != Token::Illegal)
			range.components.push_back(parseMatchComponent());
	m_expression.m_disjunction.push_back(range);
}

SemVerMatchExpression::MatchComponent SemVerMatchExpressionParser::parseMatchComponent()
{
	SemVerMatchExpression::MatchComponent component;
	Token token = currentToken();

	switch (token)
	{
	case Token::BitXor:
	case Token::BitNot:
	case Token::LessThan:
	case Token::LessThanOrEqual:
	case Token::GreaterThan:
	case Token::GreaterThanOrEqual:
	case Token::Assign:
		component.prefix = token;
		nextToken();
		break;
	default:
		component.prefix = Token::Assign;
	}

	component.levelsPresent = 0;
	while (component.levelsPresent < 3)
	{
		component.version.numbers[component.levelsPresent] = parseVersionPart();
		component.levelsPresent++;
		if (currentChar() == '.')
			nextChar();
		else
			break;
	}
	// TODO we do not support pre and build version qualifiers for now in match expressions
	// (but we do support them in the actual versions)
	return component;
}

unsigned SemVerMatchExpressionParser::parseVersionPart()
{
	auto startPos = m_pos;
	char c = currentChar();
	nextChar();
	if (c == 'x' || c == 'X' || c == '*')
		return unsigned(-1);
	else if (c == '0')
		return 0;
	else if ('1' <= c && c <= '9')
	{
		auto v = static_cast<unsigned>(c - '0');
		// If we skip to the next token, the current number is terminated.
		while (m_pos == startPos && '0' <= currentChar() && currentChar() <= '9')
		{
			c = currentChar();
			if (v * 10 < v || v * 10 + static_cast<unsigned>(c - '0') < v * 10)
				solThrow(SemVerError, "Integer too large to be used in a version number.");
			v = v * 10 + static_cast<unsigned>(c - '0');
			nextChar();
		}
		return v;
	}
	else if (c == char(-1))
		solThrow(SemVerError, "Expected version number but reached end of pragma.");
	else
		solThrow(
			SemVerError, fmt::format(
				"Expected the start of a version number but instead found character '{}'. "
				"Version number is invalid or the pragma is not terminated with a semicolon.",
				c
			)
		);
}

char SemVerMatchExpressionParser::currentChar() const
{
	if (m_pos >= m_literals.size())
		return char(-1);
	if (m_posInside >= m_literals[m_pos].size())
		return char(-1);
	return m_literals[m_pos][m_posInside];
}

char SemVerMatchExpressionParser::nextChar()
{
	if (m_pos < m_literals.size())
	{
		if (m_posInside + 1 >= m_literals[m_pos].size())
			nextToken();
		else
			++m_posInside;
	}
	return currentChar();
}

Token SemVerMatchExpressionParser::currentToken() const
{
	if (m_pos < m_tokens.size())
		return m_tokens[m_pos];
	else
		return Token::Illegal;
}

void SemVerMatchExpressionParser::nextToken()
{
	++m_pos;
	m_posInside = 0;
}
