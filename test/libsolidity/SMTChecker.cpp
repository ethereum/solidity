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

BOOST_AUTO_TEST_SUITE_END()

}
}
}
