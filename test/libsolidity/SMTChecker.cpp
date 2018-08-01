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
