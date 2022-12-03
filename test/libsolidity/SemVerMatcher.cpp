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
 * Unit tests for the semantic versioning matcher.
 */

#include <string>
#include <vector>
#include <tuple>
#include <liblangutil/Scanner.h>
#include <liblangutil/SemVerHandler.h>
#include <test/Common.h>
#include <test/libsolidity/util/SoltestErrors.h>

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace solidity::langutil;

namespace solidity::frontend::test
{

BOOST_AUTO_TEST_SUITE(SemVerMatcher)

namespace
{

SemVerMatchExpression parseExpression(string const& _input)
{
	CharStream stream(_input, "");
	Scanner scanner{stream};
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

	try
	{
		auto matchExpression = SemVerMatchExpressionParser(tokens, literals).parse();

		BOOST_CHECK_MESSAGE(
			matchExpression.isValid(),
			"Expression \"" + _input + "\" did not parse properly."
		);

		return matchExpression;
	}
	catch (SemVerError const&)
	{
		// Ignored, since a test case should have a parsable version
		soltestAssert(false);
	}

	// FIXME: Workaround for spurious GCC 12.1 warning (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105794)
	util::unreachable();
}

}

BOOST_AUTO_TEST_CASE(exception_on_invalid_version_in_semverversion_constructor)
{
	BOOST_CHECK_EXCEPTION(
		SemVerVersion version("1.2"),
		SemVerError,
		[&](auto const& _exception) { BOOST_TEST(_exception.what() == "Invalid versionString: 1.2"); return true; }
	);

	BOOST_CHECK_EXCEPTION(
		SemVerVersion version("-1.2.0"),
		SemVerError,
		[&](auto const& _exception) { BOOST_TEST(_exception.what() == "Invalid versionString: -1.2.0"); return true; }
	);
}

BOOST_AUTO_TEST_CASE(positive_range)
{
	// Positive range tests
	vector<pair<string, string>> tests = {
		{"*", "1.2.3-foo"},
		{"1.0.0 - 2.0.0", "1.2.3"},
		{"1.0.0", "1.0.0"},
		{"1.0", "1.0.0"},
		{"1", "1.0.0"},
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
		{"<1.0", "1.0.0-pre"},
		{"<1", "1.0.0-pre"},
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
		{"^1.2", "1.2.0"},
		{"^1", "1.2.0"},
		{"<=1.2.3", "1.2.3-beta"},
		{">1.2", "1.3.0-beta"},
		{"<1.2.3", "1.2.3-beta"},
		{"^1.2 ^1", "1.4.2"},
		{"^0", "0.5.1"},
		{"^0", "0.1.1"},
	};
	for (auto const& t: tests)
	{
		SemVerVersion version(t.second);
		SemVerMatchExpression matchExpression = parseExpression(t.first);
		BOOST_CHECK_MESSAGE(
			matchExpression.matches(version),
			"Version \"" + t.second + "\" did not satisfy expression \"" + t.first + "\""
		);
	}
}

BOOST_AUTO_TEST_CASE(negative_range)
{
	// Negative range tests
	vector<pair<string, string>> tests = {
		{"^0^1", "0.0.0"},
		{"^0^1", "1.0.0"},
		{"1.0.0 - 2.0.0", "2.2.3"},
		{"1.0", "1.0.0-pre"},
		{"1", "1.0.0-pre"},
		{"^1.2.3", "1.2.3-pre"},
		{"^1.2", "1.2.0-pre"},
		{"^1.2", "1.2.1-pre"},
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
		{"^0.6", "0.6.2-alpha"},
		{"^0.6", "0.6.0-alpha"},
		{"^1.2", "1.2.1-pre"},
		{"^1.2.3", "1.2.2"},
		{"^1", "1.2.0-pre"},
		{"^1", "1.2.0-pre"},
		{"^1.2", "1.1.9"},
		{"^0", "0.5.1-pre"},
		{"^0", "0.0.0-pre"},
		{"^0", "1.0.0"},
	};
	for (auto const& t: tests)
	{
		SemVerVersion version(t.second);
		auto matchExpression = parseExpression(t.first);
		BOOST_CHECK_MESSAGE(
			!matchExpression.matches(version),
			"Version \"" + t.second + "\" did satisfy expression \"" + t.first + "\" " +
			"(although it should not)"
		);
	}
}

BOOST_AUTO_TEST_SUITE_END()

} // end namespaces
