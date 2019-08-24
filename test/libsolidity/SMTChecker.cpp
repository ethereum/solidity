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
 * Unit tests for the SMT checker.
 */

#include <test/libsolidity/AnalysisFramework.h>
#include <test/Options.h>

#include <boost/test/unit_test.hpp>

#include <string>

using namespace std;
using namespace langutil;

namespace dev
{
namespace solidity
{
namespace test
{

class SMTCheckerFramework: public AnalysisFramework
{
protected:
	virtual std::pair<SourceUnit const*, ErrorList>
	parseAnalyseAndReturnError(
		std::string const& _source,
		bool _reportWarnings = false,
		bool _insertVersionPragma = true,
		bool _allowMultipleErrors = false,
		bool _allowRecoveryErrors = false
	)
	{
		return AnalysisFramework::parseAnalyseAndReturnError(
			"pragma experimental SMTChecker;\n" + _source,
			_reportWarnings,
			_insertVersionPragma,
			_allowMultipleErrors,
			_allowRecoveryErrors
		);
	}
};

BOOST_FIXTURE_TEST_SUITE(SMTChecker, SMTCheckerFramework)

BOOST_AUTO_TEST_CASE(division)
{
	string text = R"(
		contract C {
			function f(uint x, uint y) public pure returns (uint) {
				return x / y;
			}
		}
	)";
	CHECK_WARNING(text, "Division by zero");
	text = R"(
		contract C {
			function f(uint x, uint y) public pure returns (uint) {
				require(y != 0);
				return x / y;
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
	text = R"(
		contract C {
			function f(int x, int y) public pure returns (int) {
				require(y != 0);
				return x / y;
			}
		}
	)";
	CHECK_WARNING(text, "Overflow");
	text = R"(
		contract C {
			function f(int x, int y) public pure returns (int) {
				require(y != 0);
				require(y != -1);
				return x / y;
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
	text = R"(
		contract C {
			function mul(uint256 a, uint256 b) internal pure returns (uint256) {
				uint256 c;
				if (a != 0) {
					c = a * b;
					require(c / a == b);
				}
				return c;
			}
		}
	)";
	CHECK_SUCCESS_OR_WARNING(text, "might happen");
	text = R"(
		contract C {
			function mul(uint256 a, uint256 b) internal pure returns (uint256) {
				if (a == 0) {
					return 0;
				}
				// TODO remove when SMTChecker sees that this code is the `else` of the `return`.
				require(a != 0);
				uint256 c = a * b;
				require(c / a == b);
				return c;
			}
		}
	)";
	CHECK_SUCCESS_OR_WARNING(text, "might happen");
	text = R"(
		contract C {
			function div(uint256 a, uint256 b) internal pure returns (uint256) {
				require(b > 0);
				uint256 c = a / b;
				return c;
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);


}

BOOST_AUTO_TEST_CASE(division_truncates_correctly)
{
	string text = R"(
		contract C {
			function f(uint x, uint y) public pure {
				x = 7;
				y = 2;
				assert(x / y == 3);
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
	text = R"(
		contract C {
			function f(int x, int y) public pure {
				x = 7;
				y = 2;
				assert(x / y == 3);
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
	text = R"(
		contract C {
			function f(int x, int y) public pure {
				x = -7;
				y = 2;
				assert(x / y == -3);
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
	text = R"(
		contract C {
			function f(int x, int y) public pure {
				x = 7;
				y = -2;
				assert(x / y == -3);
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
	text = R"(
		contract C {
			function f(int x, int y) public pure {
				x = -7;
				y = -2;
				assert(x / y == 3);
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(compound_assignment_division)
{
	string text = R"(
		contract C {
			function f(uint x) public pure {
				require(x == 2);
				uint y = 10;
				y /= y / x;
				assert(y == x);
				assert(y == 0);
			}
		}
	)";
	CHECK_WARNING(text, "Assertion violation");
	text = R"(
		contract C {
			uint[] array;
			function f(uint x, uint p) public {
				require(x == 2);
				require(array[p] == 10);
				array[p] /= array[p] / x;
				assert(array[p] == x);
				assert(array[p] == 0);
			}
		}
	)";
	CHECK_WARNING(text, "Assertion violation");
	text = R"(
		contract C {
			mapping (uint => uint) map;
			function f(uint x, uint p) public {
				require(x == 2);
				require(map[p] == 10);
				map[p] /= map[p] / x;
				assert(map[p] == x);
				assert(map[p] == 0);
			}
		}
	)";
	CHECK_WARNING(text, "Assertion violation");
}

BOOST_AUTO_TEST_CASE(mod)
{
	string text = R"(
		contract C {
			function f(int x, int y) public pure {
				require(y == -10);
				require(x == 100);
				int z1 = x % y;
				int z2 = x % -y;
				assert(z1 == z2);
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(import_base)
{
	CompilerStack c;
	c.setSources({
	{"base", R"(
		pragma solidity >=0.0;
		contract Base {
			uint x;
			address a;
			function f() internal returns (uint) {
				a = address(this);
				++x;
				return 2;
			}
		}
	)"},
	{"der", R"(
		pragma solidity >=0.0;
		pragma experimental SMTChecker;
		import "base";
		contract Der is Base {
			function g(uint y) public {
				x += f();
				assert(y > x);
			}
		}
	)"}
	});
	c.setEVMVersion(dev::test::Options::get().evmVersion());
	BOOST_CHECK(c.compile());

	unsigned asserts = 0;
	for (auto const& e: c.errors())
	{
		string const* msg = e->comment();
		BOOST_REQUIRE(msg);
		if (msg->find("Assertion violation") != string::npos)
			++asserts;
	}
	BOOST_CHECK_EQUAL(asserts, 1);
}

BOOST_AUTO_TEST_CASE(import_library)
{
	CompilerStack c;
	c.setSources({
	{"lib", R"(
		pragma solidity >=0.0;
		library L {
			uint constant one = 1;
			function f() internal pure returns (uint) {
				return one;
			}
		}
	)"},
	{"c", R"(
		pragma solidity >=0.0;
		pragma experimental SMTChecker;
		import "lib";
		contract C {
			function g(uint x) public pure {
				uint y = L.f();
				assert(x > y);
			}
		}
	)"}
	});
	c.setEVMVersion(dev::test::Options::get().evmVersion());
	BOOST_CHECK(c.compile());

	unsigned asserts = 0;
	for (auto const& e: c.errors())
	{
		string const* msg = e->comment();
		BOOST_REQUIRE(msg);
		if (msg->find("Assertion violation") != string::npos)
			++asserts;
	}
	BOOST_CHECK_EQUAL(asserts, 1);

}


BOOST_AUTO_TEST_SUITE_END()

}
}
}
