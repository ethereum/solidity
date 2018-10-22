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
 * Unit tests for the semantic versioning matcher.
 */

#include <string>
#include <vector>
#include <tuple>
#include <libsolidity/parsing/Scanner.h>
#include <libsolidity/analysis/SemVerHandler.h>
#include <test/Options.h>

using namespace std;

namespace dev
{
namespace solidity
{
namespace test
{

BOOST_AUTO_TEST_SUITE(SemVerMatcher)

SemVerMatchExpression parseExpression(string const& _input)
{
	Scanner scanner{CharStream(_input)};
	vector<string> literals;
	vector<Token> tokens;
	while (scanner.currentToken() != Token::EOS)
	{
		auto token = scanner.currentToken();
		string literal = scanner.currentLiteral();
		if (literal.empty() && TokenTraits::toString(token))
			literal = TokenTraits::toString(token);
		literals.push_back(literal);
		tokens.push_back(token);
		scanner.next();
	}

	auto expression = SemVerMatchExpressionParser(tokens, literals).parse();
	BOOST_CHECK_MESSAGE(
		expression.isValid(),
		"Expression \"" + _input + "\" did not parse properly."
	);
	return expression;
}

BOOST_AUTO_TEST_CASE(positive_range)
{
	// Positive range tests
	vector<pair<string, string>> tests = {
		{"*", "1.2.3-foo"},
		{"1.0.0 - 2.0.0", "1.2.3"},
		{"1.0.0", "1.0.0"},
		{">=*", "0.2.4"},
		{"*", "1.2.3"},
		{">=1.0.0", "1.0.0"},
		{">=1.0.0", "1.0.1"},
		{">=1.0.0", "1.1.0"},
		{">1.0.0", "1.0.1"},
		{">1.0.0", "1.1.0"},
		{"<=2.0.0", "2.0.0"},
		{"<=2.0.0", "1.9999.9999"},
		{"<=2.0.0", "0.2.9"},
		{"<2.0.0", "1.9999.9999"},
		{"<2.0.0", "0.2.9"},
		{">= 1.0.0", "1.0.0"},
		{">=  1.0.0", "1.0.1"},
		{">=   1.0.0", "1.1.0"},
		{"> 1.0.0", "1.0.1"},
		{">  1.0.0", "1.1.0"},
		{"<=   2.0.0", "2.0.0"},
		{"<= 2.0.0", "1.9999.9999"},
		{"<=  2.0.0", "0.2.9"},
		{"<    2.0.0", "1.9999.9999"},
		{"<\t2.0.0", "0.2.9"},
		{">=0.1.97", "0.1.97"},
		{"0.1.20 || 1.2.4", "1.2.4"},
		{">=0.2.3 || <0.0.1", "0.0.0"},
		{">=0.2.3 || <0.0.1", "0.2.3"},
		{">=0.2.3 || <0.0.1", "0.2.4"},
		{"\"2.x.x\"", "2.1.3"},
		{"1.2.x", "1.2.3"},
		{"\"1.2.x\" || \"2.x\"", "2.1.3"},
		{"\"1.2.x\" || \"2.x\"", "1.2.3"},
		{"x", "1.2.3"},
		{"2.*.*", "2.1.3"},
		{"1.2.*", "1.2.3"},
		{"1.2.* || 2.*", "2.1.3"},
		{"1.2.* || 2.*", "1.2.3"},
		{"*", "1.2.3"},
		{"2", "2.1.2"},
		{"2.3", "2.3.1"},
		{"~2.4", "2.4.0"}, // >=2.4.0 <2.5.0
		{"~2.4", "2.4.5"},
		{"~1", "1.2.3"}, // >=1.0.0 <2.0.0
		{"~1.0", "1.0.2"}, // >=1.0.0 <1.1.0,
		{"~ 1.0", "1.0.2"},
		{"~ 1.0.3", "1.0.12"},
		{">=1", "1.0.0"},
		{">= 1", "1.0.0"},
		{"<1.2", "1.1.1"},
		{"< 1.2", "1.1.1"},
		{"=0.7.x", "0.7.2"},
		{"<=0.7.x", "0.7.2"},
		{">=0.7.x", "0.7.2"},
		{"<=0.7.x", "0.6.2"},
		{"~1.2.1 >=1.2.3", "1.2.3"},
		{"~1.2.1 =1.2.3", "1.2.3"},
		{"~1.2.1 1.2.3", "1.2.3"},
		{"~1.2.1 >=1.2.3 1.2.3", "1.2.3"},
		{"~1.2.1 1.2.3 >=1.2.3", "1.2.3"},
		{">=\"1.2.1\" 1.2.3", "1.2.3"},
		{"1.2.3 >=1.2.1", "1.2.3"},
		{">=1.2.3 >=1.2.1", "1.2.3"},
		{">=1.2.1 >=1.2.3", "1.2.3"},
		{">=1.2", "1.2.8"},
		{"^1.2.3", "1.8.1"},
		{"^0.1.2", "0.1.2"},
		{"^0.1", "0.1.2"},
		{"^1.2", "1.4.2"},
		{"<=1.2.3", "1.2.3-beta"},
		{">1.2", "1.3.0-beta"},
		{"<1.2.3", "1.2.3-beta"},
		{"^1.2 ^1", "1.4.2"}
	};
	for (auto const& t: tests)
	{
		SemVerVersion version(t.second);
		SemVerMatchExpression expression = parseExpression(t.first);
		BOOST_CHECK_MESSAGE(
			expression.matches(version),
			"Version \"" + t.second + "\" did not satisfy expression \"" + t.first + "\""
		);
	}
}

BOOST_AUTO_TEST_CASE(negative_range)
{
	// Positive range tests
	vector<pair<string, string>> tests = {
		{"1.0.0 - 2.0.0", "2.2.3"},
		{"^1.2.3", "1.2.3-pre"},
		{"^1.2", "1.2.0-pre"},
		{"^1.2.3", "1.2.3-beta"},
		{"=0.7.x", "0.7.0-asdf"},
		{">=0.7.x", "0.7.0-asdf"},
		{"1.0.0", "1.0.1"},
		{">=1.0.0", "0.0.0"},
		{">=1.0.0", "0.0.1"},
		{">=1.0.0", "0.1.0"},
		{">1.0.0", "0.0.1"},
		{">1.0.0", "0.1.0"},
		{"<=2.0.0", "3.0.0"},
		{"<=2.0.0", "2.9999.9999"},
		{"<=2.0.0", "2.2.9"},
		{"<2.0.0", "2.9999.9999"},
		{"<2.0.0", "2.2.9"},
		{">=0.1.97", "0.1.93"},
		{"0.1.20 || 1.2.4", "1.2.3"},
		{">=0.2.3 || <0.0.1", "0.0.3"},
		{">=0.2.3 || <0.0.1", "0.2.2"},
		{"\"2.x.x\"", "1.1.3"},
		{"\"2.x.x\"", "3.1.3"},
		{"1.2.x", "1.3.3"},
		{"\"1.2.x\" || \"2.x\"", "3.1.3"},
		{"\"1.2.x\" || \"2.x\"", "1.1.3"},
		{"2.*.*", "1.1.3"},
		{"2.*.*", "3.1.3"},
		{"1.2.*", "1.3.3"},
		{"1.2.* || 2.*", "3.1.3"},
		{"1.2.* || 2.*", "1.1.3"},
		{"2", "1.1.2"},
		{"2.3", "2.4.1"},
		{"~2.4", "2.5.0"}, // >=2.4.0 <2.5.0
		{"~2.4", "2.3.9"},
		{"~1", "0.2.3"}, // >=1.0.0 <2.0.0
		{"~1.0", "1.1.0"}, // >=1.0.0 <1.1.0
		{"<1", "1.0.0"},
		{">=1.2", "1.1.1"},
		{"=0.7.x", "0.8.2"},
		{">=0.7.x", "0.6.2"},
		{"<0.7.x", "0.7.2"},
		{"=1.2.3", "1.2.3-beta"},
		{">1.2", "1.2.8"},
		{"^1.2.3", "2.0.0-alpha"},
		{"^1.2.3", "1.2.2"},
		{"^1.2", "1.1.9"}
	};
	for (auto const& t: tests)
	{
		SemVerVersion version(t.second);
		SemVerMatchExpression expression = parseExpression(t.first);
		BOOST_CHECK_MESSAGE(
			!expression.matches(version),
			"Version \"" + t.second + "\" did satisfy expression \"" + t.first + "\" " +
			"(although it should not)"
		);
	}
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
