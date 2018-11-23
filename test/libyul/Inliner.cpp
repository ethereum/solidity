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
 * @date 2017
 * Unit tests for the Yul function inliner.
 */

#include <test/libyul/Common.h>

#include <libyul/optimiser/ExpressionInliner.h>
#include <libyul/optimiser/InlinableExpressionFunctionFinder.h>
#include <libyul/optimiser/FullInliner.h>
#include <libyul/optimiser/FunctionHoister.h>
#include <libyul/optimiser/FunctionGrouper.h>
#include <libyul/AsmPrinter.h>

#include <boost/test/unit_test.hpp>

#include <boost/range/adaptors.hpp>
#include <boost/algorithm/string/join.hpp>

using namespace std;
using namespace dev;
using namespace dev::yul;
using namespace dev::yul::test;
using namespace dev::solidity;

namespace
{
string inlinableFunctions(string const& _source)
{
	auto ast = disambiguate(_source);

	InlinableExpressionFunctionFinder funFinder;
	funFinder(ast);

	vector<string> functionNames;
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
	BOOST_CHECK_EQUAL(inlinableFunctions("{ function f() -> x:u256 { x := 2:u256 } }"), "f");
	BOOST_CHECK_EQUAL(inlinableFunctions("{"
		"function g(a:u256) -> b:u256 { b := a }"
		"function f() -> x:u256 { x := g(2:u256) }"
	"}"), "g,f");
}

BOOST_AUTO_TEST_CASE(simple_inside_structures)
{
	BOOST_CHECK_EQUAL(inlinableFunctions("{"
		"switch 2:u256 "
		"case 2:u256 {"
			"function g(a:u256) -> b:u256 { b := a }"
			"function f() -> x:u256 { x := g(2:u256) }"
		"}"
	"}"), "g,f");
	BOOST_CHECK_EQUAL(inlinableFunctions("{"
		"for {"
			"function g(a:u256) -> b:u256 { b := a }"
		"} 1:u256 {"
			"function f() -> x:u256 { x := g(2:u256) }"
		"}"
		"{"
			"function h() -> y:u256 { y := 2:u256 }"
		"}"
	"}"), "h,g,f");
}

BOOST_AUTO_TEST_CASE(negative)
{
	BOOST_CHECK_EQUAL(inlinableFunctions("{ function f() -> x:u256 { } }"), "");
	BOOST_CHECK_EQUAL(inlinableFunctions("{ function f() -> x:u256 { x := 2:u256 {} } }"), "");
	BOOST_CHECK_EQUAL(inlinableFunctions("{ function f() -> x:u256 { x := f() } }"), "");
	BOOST_CHECK_EQUAL(inlinableFunctions("{ function f() -> x:u256 { x := x } }"), "");
	BOOST_CHECK_EQUAL(inlinableFunctions("{ function f() -> x:u256, y:u256 { x := 2:u256 } }"), "");
  BOOST_CHECK_EQUAL(inlinableFunctions(
    "{ function g() -> x:u256, y:u256 {} function f(y:u256) -> x:u256 { x,y := g() } }"), "");
	BOOST_CHECK_EQUAL(inlinableFunctions("{ function f(y:u256) -> x:u256 { y := 2:u256 } }"), "");
}


BOOST_AUTO_TEST_SUITE_END()
