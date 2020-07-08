// SPDX-License-Identifier: GPL-3.0
/**
 * @author Christian <chris@ethereum.org>
 * @date 2016
 * Utilities to handle semantic versioning.
 */

#pragma once

#include <liblangutil/Token.h>

#include <string>
#include <utility>
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
	SemVerMatchExpressionParser(std::vector<Token>  _tokens, std::vector<std::string>  _literals):
		m_tokens(std::move(_tokens)), m_literals(std::move(_literals))
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
