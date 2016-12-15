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
 * @author Alex Beregszaszi
 * @date 2016
 * Unit tests for the LLL parser.
 */

#include <string>
#include <memory>
#include <boost/test/unit_test.hpp>
#include <liblll/Compiler.h>

using namespace std;

namespace dev
{
namespace lll
{
namespace test
{

namespace
{

bool successParse(std::string const& _source)
{
	std::string ret = eth::parseLLL(_source);
	return ret.size() != 0;
}

std::string parse(std::string const& _source)
{
	return eth::parseLLL(_source);
}

}

BOOST_AUTO_TEST_SUITE(LLLParser)

BOOST_AUTO_TEST_CASE(smoke_test)
{
	char const* text = "1";
	BOOST_CHECK(successParse(text));
}

BOOST_AUTO_TEST_CASE(string)
{
	char const* text = "\"string\"";
	BOOST_CHECK(successParse(text));
	BOOST_CHECK_EQUAL(parse(text), R"("string")");
}

BOOST_AUTO_TEST_CASE(symbol)
{
	char const* text = "symbol";
	BOOST_CHECK(successParse(text));
	BOOST_CHECK_EQUAL(parse(text), R"(symbol)");

	BOOST_CHECK(successParse("'symbol"));
	BOOST_CHECK_EQUAL(parse(text), R"(symbol)");
}

BOOST_AUTO_TEST_CASE(decimals)
{
	char const* text = "1234";
	BOOST_CHECK(successParse(text));
	BOOST_CHECK_EQUAL(parse(text), R"(1234)");
}

BOOST_AUTO_TEST_CASE(hexadecimals)
{
	char const* text = "0x1234";
	BOOST_CHECK(successParse(text));
	BOOST_CHECK_EQUAL(parse(text), R"(4660)");

	BOOST_CHECK(!successParse("0x"));
}

BOOST_AUTO_TEST_CASE(sequence)
{
	char const* text = "{ 1234 }";
	BOOST_CHECK(successParse(text));
	BOOST_CHECK_EQUAL(parse(text), R"({ 1234 })");
}

BOOST_AUTO_TEST_CASE(empty_sequence)
{
	char const* text = "{}";
	BOOST_CHECK(successParse(text));
	BOOST_CHECK_EQUAL(parse(text), R"({ })");
}

BOOST_AUTO_TEST_CASE(mload)
{
	char const* text = "@0";
	BOOST_CHECK(successParse(text));
	BOOST_CHECK_EQUAL(parse(text), R"(@ 0)");

	BOOST_CHECK(successParse("@0x0"));
	BOOST_CHECK(successParse("@symbol"));
	BOOST_CHECK(!successParse("@"));
}

BOOST_AUTO_TEST_CASE(sload)
{
	char const* text = "@@0";
	BOOST_CHECK(successParse(text));
	BOOST_CHECK_EQUAL(parse(text), R"(@@ 0)");

	BOOST_CHECK(successParse("@@0x0"));
	BOOST_CHECK(successParse("@@symbol"));
	BOOST_CHECK(!successParse("@@"));
}

BOOST_AUTO_TEST_CASE(mstore)
{
	char const* text = "[0]:0";
	BOOST_CHECK(successParse(text));
	BOOST_CHECK_EQUAL(parse(text), R"([ 0 ] 0)");

	BOOST_CHECK(successParse("[0] 0"));
	BOOST_CHECK(successParse("[0x0]:0x0"));
	BOOST_CHECK(successParse("[symbol]:symbol"));
	BOOST_CHECK(!successParse("[]"));
	BOOST_CHECK(!successParse("[0]"));
}

BOOST_AUTO_TEST_CASE(sstore)
{
	char const* text = "[[0]]:0";
	BOOST_CHECK(successParse(text));
	BOOST_CHECK_EQUAL(parse(text), R"([[ 0 ]] 0)");

	BOOST_CHECK(successParse("[[0]] 0"));
	BOOST_CHECK(successParse("[[0x0]]:0x0"));
	BOOST_CHECK(successParse("[[symbol]]:symbol"));
	BOOST_CHECK(!successParse("[[]]"));
	BOOST_CHECK(!successParse("[[0x0]]"));
}

BOOST_AUTO_TEST_CASE(calldataload)
{
	char const* text = "$0";
	BOOST_CHECK(successParse(text));
	BOOST_CHECK_EQUAL(parse(text), R"($ 0)");

	BOOST_CHECK(successParse("$0x0"));
	BOOST_CHECK(successParse("$symbol"));
	BOOST_CHECK(!successParse("$"));
}

BOOST_AUTO_TEST_CASE(list)
{
	char const* text = "( 1234 )";
	BOOST_CHECK(successParse(text));
	BOOST_CHECK_EQUAL(parse(text), R"(( 1234 ))");

	BOOST_CHECK(successParse("( 1234 5467 )"));
	BOOST_CHECK(!successParse("()"));
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
