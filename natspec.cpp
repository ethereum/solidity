/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
 */
/** @file natspec.cpp
 * @author Marek Kotewicz <marek@ethdev.com>
 * @date 2015
 */

#if !ETH_HEADLESS

#include <boost/test/unit_test.hpp>
#include <libdevcore/Log.h>
#include <libnatspec/NatspecExpressionEvaluator.h>

using namespace std;

BOOST_AUTO_TEST_SUITE(natspec)

BOOST_AUTO_TEST_CASE(natspec_eval_function_exists)
{
	// given
	NatspecExpressionEvaluator e;
	// when
	string result = e.evalExpression("`typeof evaluateExpression`").toStdString();
	// then
	BOOST_CHECK_EQUAL(result, "function");
}

BOOST_AUTO_TEST_CASE(natspec_js_eval)
{
	// given
	NatspecExpressionEvaluator e;
	// when
	string result = e.evalExpression("`1 + 2`").toStdString();
	// then
	BOOST_CHECK_EQUAL(result, "3");
}

BOOST_AUTO_TEST_CASE(natspec_create_custom_function)
{
	// given
	NatspecExpressionEvaluator e;
	// when
	auto x = e.evalExpression("`test = function (x) { return x + 'ok'; }`"); // ommit var, make it global
	string result = e.evalExpression("`test(5)`").toStdString();
	string result2 = e.evalExpression("`typeof test`").toStdString();
	// then
	BOOST_CHECK_EQUAL(result, "5ok");
	BOOST_CHECK_EQUAL(result2, "function");
}

BOOST_AUTO_TEST_CASE(natspec_js_eval_separated_expressions)
{
	// given
	NatspecExpressionEvaluator e;
	// when
	string result = e.evalExpression("`x = 1` + `y = 2` will be equal `x + y`").toStdString();
	// then
	BOOST_CHECK_EQUAL(result, "1 + 2 will be equal 3");
}

BOOST_AUTO_TEST_CASE(natspec_js_eval_input_params)
{
	// given
	char const* abi = R"([
	{
		"name": "f",
		"constant": false,
		"type": "function",
		"inputs": [
		{
			"name": "a",
			"type": "uint256"
		}
		],
		"outputs": [
		{
			"name": "d",
			"type": "uint256"
		}
		]
	}
	])";
	NatspecExpressionEvaluator e(abi, "'f'", "[4]");
	// when
	string result = e.evalExpression("Will multiply `a` by 7 and return `a * 7`.").toStdString();
	// then
	BOOST_CHECK_EQUAL(result, "Will multiply 4 by 7 and return 28.");
}

BOOST_AUTO_TEST_CASE(natspec_js_eval_error)
{
	// given
	NatspecExpressionEvaluator e;
	// when
	string result = e.evalExpression("`test(`").toStdString();
	// then
	BOOST_CHECK_EQUAL(result, "`test(`");
}

BOOST_AUTO_TEST_SUITE_END()

#endif
