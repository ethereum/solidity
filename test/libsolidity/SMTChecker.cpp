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
public:
	SMTCheckerFramework()
	{
		m_warningsToFilter.push_back("Experimental features are turned on.");
	}

protected:
	virtual std::pair<SourceUnit const*, std::shared_ptr<Error const>>
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
		contract C {
			struct A { uint a; uint b; }
			function f() public pure returns (A) {
				return A({ a: 1, b: 2 });
			}
		}
	)";
	/// Multiple warnings, should check for: Assertion checker does not yet implement this expression.
	CHECK_WARNING_ALLOW_MULTI(text, "");
}

BOOST_AUTO_TEST_CASE(simple_assert)
{
	string text = R"(
		contract C {
			function f(uint a) public pure { assert(a == 2); }
		}
	)";
	CHECK_WARNING(text, "Assertion violation happens here for");
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

BOOST_AUTO_TEST_CASE(use_before_declaration)
{
	string text = R"(
		contract C {
			function f() public pure { a = 3; uint a = 2; assert(a == 2); }
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
	text = R"(
		contract C {
			function f() public pure { assert(a == 0); uint a = 2; assert(a == 2); }
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

BOOST_AUTO_TEST_CASE(branches_clear_variables)
{
	// Only clears accessed variables
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
	// It is just a plain clear and will not combine branches.
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
	CHECK_WARNING(text, "Assertion violation happens here");
	// Clear also works on the else branch
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
	CHECK_WARNING(text, "Assertion violation happens here");
	// Variable is not cleared, if it is only read.
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
}

BOOST_AUTO_TEST_CASE(ways_to_clear_variables)
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
			function f(uint x) public pure {
				x = 7;
				while ((x = 5) > 0) {
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

BOOST_AUTO_TEST_SUITE_END()

}
}
}
