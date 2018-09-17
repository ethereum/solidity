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

#include <boost/test/unit_test.hpp>

#include <string>

using namespace std;

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
		bool _allowMultipleErrors = false
	)
	{
		return AnalysisFramework::parseAnalyseAndReturnError(
			"pragma experimental SMTChecker;\n" + _source,
			_reportWarnings,
			_insertVersionPragma,
			_allowMultipleErrors
		);
	}
};

BOOST_FIXTURE_TEST_SUITE(SMTChecker, SMTCheckerFramework)

BOOST_AUTO_TEST_CASE(smoke_test)
{
	string text = R"(
		contract C { }
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(simple_overflow)
{
	string text = R"(
		contract C {
			function f(uint a, uint b) public pure returns (uint) { return a + b; }
		}
	)";
	CHECK_WARNING(text, "Overflow (resulting value larger than");
}

BOOST_AUTO_TEST_CASE(warn_on_typecast)
{
	string text = R"(
		contract C {
			function f() public pure returns (uint) {
				return uint8(1);
			}
		}
	)";
	CHECK_WARNING(text, "Assertion checker does not yet implement this expression.");
}

BOOST_AUTO_TEST_CASE(warn_on_struct)
{
	string text = R"(
		pragma experimental ABIEncoderV2;
		contract C {
			struct A { uint a; uint b; }
			function f() public pure returns (A memory) {
				return A({ a: 1, b: 2 });
			}
		}
	)";
	CHECK_WARNING_ALLOW_MULTI(text, (vector<string>{
		"Experimental feature",
		"Assertion checker does not yet implement this expression.",
		"Assertion checker does not yet support the type of this variable."
	}));
}

BOOST_AUTO_TEST_CASE(simple_assert)
{
	string text = R"(
		contract C {
			function f(uint a) public pure { assert(a == 2); }
		}
	)";
	CHECK_WARNING(text, "Assertion violation happens here");
}

BOOST_AUTO_TEST_CASE(simple_assert_with_require)
{
	string text = R"(
		contract C {
			function f(uint a) public pure { require(a < 10); assert(a < 20); }
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(assignment_in_declaration)
{
	string text = R"(
		contract C {
			function f() public pure { uint a = 2; assert(a == 2); }
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(function_call_does_not_clear_local_vars)
{
	string text = R"(
		contract C {
			function f() public {
				uint a = 3;
				this.f();
				assert(a == 3);
				f();
				assert(a == 3);
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(branches_merge_variables)
{
	// Branch does not touch variable a
	string text = R"(
		contract C {
			function f(uint x) public pure {
				uint a = 3;
				if (x > 10) {
				}
				assert(a == 3);
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
	// Positive branch touches variable a, but assertion should still hold.
	text = R"(
	contract C {
			function f(uint x) public pure {
				uint a = 3;
				if (x > 10) {
					a = 3;
				}
				assert(a == 3);
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
	// Negative branch touches variable a, but assertion should still hold.
	text = R"(
		contract C {
			function f(uint x) public pure {
				uint a = 3;
				if (x > 10) {
				} else {
					a = 3;
				}
				assert(a == 3);
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
	// Variable is not merged, if it is only read.
	text = R"(
		contract C {
			function f(uint x) public pure {
				uint a = 3;
				if (x > 10) {
					assert(a == 3);
				} else {
					assert(a == 3);
				}
				assert(a == 3);
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
	// Variable is reset in both branches
	text = R"(
		contract C {
			function f(uint x) public pure {
				uint a = 2;
				if (x > 10) {
					a = 3;
				} else {
					a = 3;
				}
				assert(a == 3);
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
	// Variable is reset in both branches
	text = R"(
		contract C {
			function f(uint x) public pure {
				uint a = 2;
				if (x > 10) {
					a = 3;
				} else {
					a = 4;
				}
				assert(a >= 3);
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(branches_assert_condition)
{
	string text = R"(
		contract C {
			function f(uint x) public pure {
				if (x > 10) {
					assert(x > 9);
				}
				else
				{
					assert(x < 11);
				}
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
	text = R"(
		contract C {
			function f(uint x) public pure {
				if (x > 10) {
					assert(x > 9);
				}
				else if (x > 2)
				{
					assert(x <= 10 && x > 2);
				}
				else
				{
				   assert(0 <= x && x <= 2);
				}
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(ways_to_merge_variables)
{
	string text = R"(
		contract C {
			function f(uint x) public pure {
				uint a = 3;
				if (x > 10) {
					a++;
				}
				assert(a == 3);
			}
		}
	)";
	CHECK_WARNING(text, "Assertion violation happens here");
	text = R"(
		contract C {
			function f(uint x) public pure {
				uint a = 3;
				if (x > 10) {
					++a;
				}
				assert(a == 3);
			}
		}
	)";
	CHECK_WARNING(text, "Assertion violation happens here");
	text = R"(
		contract C {
			function f(uint x) public pure {
				uint a = 3;
				if (x > 10) {
					a = 5;
				}
				assert(a == 3);
			}
		}
	)";
	CHECK_WARNING(text, "Assertion violation happens here");
}

BOOST_AUTO_TEST_CASE(bool_simple)
{
	string text = R"(
		contract C {
			function f(bool x) public pure {
				assert(x);
			}
		}
	)";
	CHECK_WARNING(text, "Assertion violation happens here");
	text = R"(
		contract C {
			function f(bool x, bool y) public pure {
				assert(x == y);
			}
		}
	)";
	CHECK_WARNING(text, "Assertion violation happens here");
	text = R"(
		contract C {
			function f(bool x, bool y) public pure {
				bool z = x || y;
				assert(!(x && y) || z);
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
	text = R"(
		contract C {
			function f(bool x) public pure {
				if (x) {
					assert(x);
				} else {
					assert(!x);
				}
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
	text = R"(
		contract C {
			function f(bool x) public pure {
				bool y = x;
				assert(x == y);
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
	text = R"(
		contract C {
			function f(bool x) public pure {
				require(x);
				bool y;
				y = false;
				assert(x || y);
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(bool_int_mixed)
{
	string text = R"(
		contract C {
			function f(bool x) public pure {
				uint a;
				if (x)
					a = 1;
				assert(!x || a > 0);
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
	text = R"(
		contract C {
			function f(bool x, uint a) public pure {
				require(!x || a > 0);
				uint b = a;
				assert(!x || b > 0);
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
	text = R"(
		contract C {
			function f(bool x, bool y) public pure {
				uint a;
				if (x) {
					if (y) {
						a = 0;
					} else {
						a = 1;
					}
				} else {
					if (y) {
						a = 1;
					} else {
						a = 0;
					}
				}
				bool xor_x_y = (x && !y) || (!x && y);
				assert(!xor_x_y || a > 0);
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(storage_value_vars)
{
	string text = R"(
		contract C
		{
			address a;
			bool b;
			uint c;
			function f(uint x) public {
				if (x == 0)
				{
					a = 0x0000000000000000000000000000000000000100;
					b = true;
				}
				else
				{
					a = 0x0000000000000000000000000000000000000200;
					b = false;
				}
				assert(a > 0x0000000000000000000000000000000000000000 && b);
			}
		}
	)";
	CHECK_WARNING(text, "Assertion violation happens here");
	text = R"(
		contract C
		{
			address a;
			bool b;
			uint c;
			function f() public view {
				assert(c > 0);
			}
		}
	)";
	CHECK_WARNING(text, "Assertion violation happens here");
	text = R"(
		contract C
		{
			function f(uint x) public {
				if (x == 0)
				{
					a = 0x0000000000000000000000000000000000000100;
					b = true;
				}
				else
				{
					a = 0x0000000000000000000000000000000000000200;
					b = false;
				}
				assert(b == (a < 0x0000000000000000000000000000000000000200));
			}

			function g() public view {
				require(a < 0x0000000000000000000000000000000000000100);
				assert(c >= 0);
			}
			address a;
			bool b;
			uint c;
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
	text = R"(
		contract C
		{
			function f() public view {
				assert(c > 0);
			}
			uint c;
		}
	)";
	CHECK_WARNING(text, "Assertion violation happens here");

}

BOOST_AUTO_TEST_CASE(while_loop_simple)
{
	// Check that variables are cleared
	string text = R"(
		contract C {
			function f(uint x) public pure {
				x = 2;
				while (x > 1) {
					x = 2;
				}
				assert(x == 2);
			}
		}
	)";
	CHECK_WARNING(text, "Assertion violation happens here");
	// Check that condition is assumed.
	text = R"(
		contract C {
			function f(uint x) public pure {
				while (x == 2) {
					assert(x == 2);
				}
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
	// Check that condition is not assumed after the body anymore
	text = R"(
		contract C {
			function f(uint x) public pure {
				while (x == 2) {
				}
				assert(x == 2);
			}
		}
	)";
	CHECK_WARNING(text, "Assertion violation happens here");
	// Check that negation of condition is not assumed after the body anymore
	text = R"(
		contract C {
			function f(uint x) public pure {
				while (x == 2) {
				}
				assert(x != 2);
			}
		}
	)";
	CHECK_WARNING(text, "Assertion violation happens here");
	// Check that side-effects of condition are taken into account
	text = R"(
		contract C {
			function f(uint x, uint y) public pure {
				x = 7;
				while ((x = y) > 0) {
				}
				assert(x == 7);
			}
		}
	)";
	CHECK_WARNING(text, "Assertion violation happens here");
}

BOOST_AUTO_TEST_CASE(constant_condition)
{
	string text = R"(
		contract C {
			function f(uint x) public pure {
				if (x >= 0) { revert(); }
			}
		}
	)";
	CHECK_WARNING(text, "Condition is always true");
	text = R"(
		contract C {
			function f(uint x) public pure {
				if (x >= 10) { if (x < 10) { revert(); } }
			}
		}
	)";
	CHECK_WARNING(text, "Condition is always false");
	// a plain literal constant is fine
	text = R"(
		contract C {
			function f(uint) public pure {
				if (true) { revert(); }
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}


BOOST_AUTO_TEST_CASE(for_loop)
{
	string text = R"(
		contract C {
			function f(uint x) public pure {
				require(x == 2);
				for (;;) {}
				assert(x == 2);
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
	text = R"(
		contract C {
			function f(uint x) public pure {
				for (; x == 2; ) {
					assert(x == 2);
				}
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
	text = R"(
		contract C {
			function f(uint x) public pure {
				for (uint y = 2; x < 10; ) {
					assert(y == 2);
				}
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
	text = R"(
		contract C {
			function f(uint x) public pure {
				for (uint y = 2; x < 10; y = 3) {
					assert(y == 2);
				}
			}
		}
	)";
	CHECK_WARNING(text, "Assertion violation");
	text = R"(
		contract C {
			function f(uint x) public pure {
				uint y;
				for (y = 2; x < 10; ) {
					y = 3;
				}
				assert(y == 3);
			}
		}
	)";
	CHECK_WARNING(text, "Assertion violation");
	text = R"(
		contract C {
			function f(uint x) public pure {
				uint y;
				for (y = 2; x < 10; ) {
					y = 3;
				}
				assert(y == 2);
			}
		}
	)";
	CHECK_WARNING(text, "Assertion violation");
}

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

BOOST_AUTO_TEST_SUITE_END()

}
}
}
