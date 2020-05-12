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
#include <test/Common.h>

#include <boost/test/unit_test.hpp>

#include <string>

using namespace std;
using namespace solidity::langutil;

namespace solidity::frontend::test
{

class SMTCheckerFramework: public AnalysisFramework
{
protected:
	std::pair<SourceUnit const*, ErrorList>
	parseAnalyseAndReturnError(
		std::string const& _source,
		bool _reportWarnings = false,
		bool _insertLicenseAndVersionPragma = true,
		bool _allowMultipleErrors = false,
		bool _allowRecoveryErrors = false
	) override
	{
		return AnalysisFramework::parseAnalyseAndReturnError(
			"pragma experimental SMTChecker;\n" + _source,
			_reportWarnings,
			_insertLicenseAndVersionPragma,
			_allowMultipleErrors,
			_allowRecoveryErrors
		);
	}
};

BOOST_FIXTURE_TEST_SUITE(SMTChecker, SMTCheckerFramework)

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
	c.setEVMVersion(solidity::test::CommonOptions::get().evmVersion());
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
	c.setEVMVersion(solidity::test::CommonOptions::get().evmVersion());
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
