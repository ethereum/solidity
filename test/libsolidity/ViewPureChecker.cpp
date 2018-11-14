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
 * Unit tests for the view and pure checker.
 */

#include <test/libsolidity/AnalysisFramework.h>

#include <test/Options.h>

#include <boost/test/unit_test.hpp>

#include <string>
#include <tuple>

using namespace std;
using namespace langutil;

namespace dev
{
namespace solidity
{
namespace test
{

BOOST_FIXTURE_TEST_SUITE(ViewPureChecker, AnalysisFramework)

BOOST_AUTO_TEST_CASE(environment_access)
{
	vector<string> view{
		"block.coinbase",
		"block.timestamp",
		"block.difficulty",
		"block.number",
		"block.gaslimit",
		"blockhash(7)",
		"gasleft()",
		"msg.value",
		"msg.sender",
		"tx.origin",
		"tx.gasprice",
		"this",
		"address(1).balance",
	};
	if (dev::test::Options::get().evmVersion().hasStaticCall())
		view.emplace_back("address(0x4242).staticcall(\"\")");

	// ``block.blockhash`` and ``blockhash`` are tested separately below because their usage will
	// produce warnings that can't be handled in a generic way.
	vector<string> pure{
		"msg.data",
		"msg.data[0]",
		"msg.sig",
		"msg",
		"block",
		"tx"
	};
	for (string const& x: view)
	{
		CHECK_ERROR(
			"contract C { function f() pure public { " + x + "; } }",
			TypeError,
			"Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires \"view\""
		);
	}
	for (string const& x: pure)
	{
		CHECK_WARNING(
			"contract C { function f() view public { " + x + "; } }",
			"Function state mutability can be restricted to pure"
		);
	}

	CHECK_WARNING_ALLOW_MULTI(
		"contract C { function f() view public { blockhash; } }",
		(std::vector<std::string>{
			"Function state mutability can be restricted to pure",
			"Statement has no effect."
	}));

	CHECK_ERROR(
		"contract C { function f() view public { block.blockhash; } }",
		TypeError,
		"\"block.blockhash()\" has been deprecated in favor of \"blockhash()\""
	);
}

BOOST_AUTO_TEST_CASE(address_staticcall)
{
	string text = R"(
		contract C {
			function i() view public returns (bool) {
				(bool success,) = address(0x4242).staticcall("");
				return success;
			}
		}
	)";
	if (!dev::test::Options::get().evmVersion().hasStaticCall())
		CHECK_ERROR(text, TypeError, "\"staticcall\" is not supported by the VM version.");
	else
		CHECK_SUCCESS_NO_WARNINGS(text);
}


BOOST_AUTO_TEST_CASE(assembly_staticcall)
{
	string text = R"(
		contract C {
			function i() view public {
				assembly { pop(staticcall(gas, 1, 2, 3, 4, 5)) }
			}
		}
	)";
	if (!dev::test::Options::get().evmVersion().hasStaticCall())
		CHECK_WARNING(text, "\"staticcall\" instruction is only available for Byzantium-compatible");
	else
		CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_SUITE_END()

}
}
}
