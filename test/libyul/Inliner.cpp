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
 * @date 2017
 * Unit tests for the Yul function inliner.
 */

#include <test/libyul/Common.h>

#include <libyul/optimiser/ExpressionInliner.h>
#include <libyul/optimiser/InlinableExpressionFunctionFinder.h>
#include <libyul/optimiser/FunctionGrouper.h>
#include <libyul/AST.h>

#include <boost/test/unit_test.hpp>

#include <boost/algorithm/string/join.hpp>

using namespace solidity;
using namespace solidity::util;
using namespace solidity::yul;
using namespace solidity::yul::test;

namespace
{
std::string inlinableFunctions(std::string const& _source)
{
	auto ast = disambiguate(_source);

	InlinableExpressionFunctionFinder funFinder;
	funFinder(ast);

	std::vector<std::string> functionNames;
	for (auto const& f: funFinder.inlinableFunctions())
		functionNames.emplace_back(f.first.str());
	return boost::algorithm::join(functionNames, ",");
}

}


BOOST_AUTO_TEST_SUITE(YulInlinableFunctionFilter)

BOOST_AUTO_TEST_CASE(smoke_test)
{
	BOOST_CHECK_EQUAL(inlinableFunctions("{ }"), "");
}

BOOST_AUTO_TEST_CASE(simple)
{
	BOOST_CHECK_EQUAL(inlinableFunctions("{ function f() -> x { x := 2 } }"), "f");
	BOOST_CHECK_EQUAL(inlinableFunctions("{"
		"function g(a) -> b { b := a }"
		"function f() -> x { x := g(2) }"
	"}"), "g,f");
}

BOOST_AUTO_TEST_CASE(simple_inside_structures)
{
	BOOST_CHECK_EQUAL(inlinableFunctions("{"
		"switch 2 "
		"case 2 {"
			"function g(a) -> b { b := a }"
			"function f() -> x { x := g(2) }"
		"}"
	"}"), "g,f");
	BOOST_CHECK_EQUAL(inlinableFunctions("{"
		"function g(a) -> b { b := a }"
		"for {"
		"} true {"
			"function f() -> x { x := g(2) }"
		"}"
		"{"
			"function h() -> y { y := 2 }"
		"}"
	"}"), "h,g,f");
}

BOOST_AUTO_TEST_CASE(negative)
{
	BOOST_CHECK_EQUAL(inlinableFunctions("{ function f() -> x { } }"), "");
	BOOST_CHECK_EQUAL(inlinableFunctions("{ function f() -> x { x := 2 {} } }"), "");
	BOOST_CHECK_EQUAL(inlinableFunctions("{ function f() -> x { x := f() } }"), "");
	BOOST_CHECK_EQUAL(inlinableFunctions("{ function f() -> x { x := x } }"), "");
	BOOST_CHECK_EQUAL(inlinableFunctions("{ function f() -> x, y { x := 2 } }"), "");
  BOOST_CHECK_EQUAL(inlinableFunctions(
    "{ function g() -> x, y {} function f(y) -> x { x,y := g() } }"), "");
	BOOST_CHECK_EQUAL(inlinableFunctions("{ function f(y) -> x { y := 2 } }"), "");
}


BOOST_AUTO_TEST_SUITE_END()
