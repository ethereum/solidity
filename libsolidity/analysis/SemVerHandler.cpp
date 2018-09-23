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

#include <libsolidity/analysis/SemVerHandler.h>
#include <functional>

using namespace std;
using namespace dev;
using namespace dev::solidity;

SemVerVersion::SemVerVersion(string const& _versionString)
{
	auto i = _versionString.begin();
	auto end = _versionString.end();

	for (unsigned level = 0; level < 3; ++level)
	{
		unsigned v = 0;
		for (; i != end && '0' <= *i && *i <= '9'; ++i)
			v = v * 10 + (*i - '0');
		numbers[level] = v;
		if (level < 2)
		{
			if (i == end || *i != '.')
				throw SemVerError();
			else
				++i;
		}
	}
	if (i != end && *i == '-')
	{
		auto prereleaseStart = ++i;
		while (i != end && *i != '+') ++i;
		prerelease = string(prereleaseStart, i);
	}
	if (i != end && *i == '+')
	{
		auto buildStart = ++i;
		while (i != end) ++i;
		build = string(buildStart, i);
	}
	if (i != end)
		throw SemVerError();
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

		if (comp.version.numbers[0] == 0)
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
			if (version.numbers[i] != unsigned(-1))
			{
				didCompare = true;
				cmp = _version.numbers[i] - version.numbers[i];
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

	try
	{
		while (true)
		{
			parseMatchExpression();
			if (m_pos >= m_tokens.size())
				break;
			if (currentToken() != Token::Or)
				throw SemVerError();
			nextToken();
		}
	}
	catch (SemVerError const&)
	{
		reset();
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
		range.components[0].prefix = Token::GreaterThanOrEqual;
		nextToken();
		range.components.push_back(parseMatchComponent());
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
	Token::Value token = currentToken();

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
		unsigned v = c - '0';
		// If we skip to the next token, the current number is terminated.
		while (m_pos == startPos && '0' <= currentChar() && currentChar() <= '9')
		{
			c = currentChar();
			if (v * 10 < v || v * 10 + (c - '0') < v * 10)
				throw SemVerError();
			v = v * 10 + c - '0';
			nextChar();
		}
		return v;
	}
	else
		throw SemVerError();
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

Token::Value SemVerMatchExpressionParser::currentToken() const
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
