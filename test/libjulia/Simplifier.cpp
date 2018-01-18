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
 * Unit tests for the expression simplifier optimizer stage.
 */

#include <test/libjulia/Common.h>

#include <libjulia/optimiser/ExpressionSimplifier.h>

#include <libsolidity/inlineasm/AsmPrinter.h>

#include <boost/test/unit_test.hpp>

#include <boost/range/adaptors.hpp>
#include <boost/algorithm/string/join.hpp>

using namespace std;
using namespace dev;
using namespace dev::julia;
using namespace dev::julia::test;
using namespace dev::solidity;


#define CHECK(_original, _expectation)\
do\
{\
	assembly::AsmPrinter p;\
	Block b = *(parse(_original, false).first);\
	(ExpressionSimplifier{})(b);\
	string result = p(b);\
	BOOST_CHECK_EQUAL(result, format(_expectation, false));\
}\
while(false)

BOOST_AUTO_TEST_SUITE(IuliaSimplifier)

BOOST_AUTO_TEST_CASE(smoke_test)
{
	CHECK("{ }", "{ }");
}

BOOST_AUTO_TEST_CASE(constants)
{
	CHECK(
		"{ let a := add(1, mul(3, 4)) }",
		"{ let a := 13 }"
	);
}

BOOST_AUTO_TEST_SUITE_END()
