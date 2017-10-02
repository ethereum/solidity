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
 * @date 2017
 * Unit tests for the LLL compiler.
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

bool successCompile(std::string const& _sourceCode)
{
	std::vector<std::string> errors;
	bytes bytecode = eth::compileLLL(_sourceCode, false, &errors);
	if (!errors.empty())
		return false;
	if (bytecode.empty())
		return false;
	return true;
}

}

BOOST_AUTO_TEST_SUITE(LLLCompiler)

BOOST_AUTO_TEST_CASE(smoke_test)
{
	char const* sourceCode = "1";
	BOOST_CHECK(successCompile(sourceCode));
}

BOOST_AUTO_TEST_CASE(switch_valid)
{
	char const* sourceCode = R"(
		(switch (origin))
	)";
	BOOST_CHECK(successCompile(sourceCode));
	sourceCode = R"(
		(switch
			1 (panic)
			2 (panic))
	)";
	BOOST_CHECK(successCompile(sourceCode));
	sourceCode = R"(
		(switch
			1 (panic)
			2 (panic)
			(panic))
	)";
	BOOST_CHECK(successCompile(sourceCode));
	sourceCode = R"(
		(switch
			1 (origin)
			2 (origin)
			(origin))
	)";
	BOOST_CHECK(successCompile(sourceCode));
}

BOOST_AUTO_TEST_CASE(switch_invalid_arg_count)
{
	char const* sourceCode = R"(
		(switch)
	)";
	BOOST_CHECK(!successCompile(sourceCode));
}

BOOST_AUTO_TEST_CASE(switch_inconsistent_return_count)
{
	// cannot return stack items if the default case is not present
	char const* sourceCode = R"(
		(switch
			1 (origin)
			2 (origin)
	)";
	BOOST_CHECK(!successCompile(sourceCode));
	// return count mismatch
	sourceCode = R"(
		(switch
			1 (origin)
			2 (origin)
			(panic))
	)";
	BOOST_CHECK(!successCompile(sourceCode));
	// return count mismatch
	sourceCode = R"(
		(switch
			1 (panic)
			2 (panic)
			(origin))
	)";
	BOOST_CHECK(!successCompile(sourceCode));
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
