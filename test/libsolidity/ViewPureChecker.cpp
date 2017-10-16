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

#include <boost/test/unit_test.hpp>

#include <string>

using namespace std;

namespace dev
{
namespace solidity
{
namespace test
{

BOOST_FIXTURE_TEST_SUITE(ViewPureChecker, AnalysisFramework)

BOOST_AUTO_TEST_CASE(smoke_test)
{
	char const* text = R"(
		contract C {
			uint x;
			function g() pure public {}
			function f() view public returns (uint) { return now; }
			function h() public { x = 2; }
			function i() payable public { x = 2; }
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(call_internal_functions_success)
{
	char const* text = R"(
		contract C {
			function g() pure public { g(); }
			function f() view public returns (uint) { f(); g(); }
			function h() public { h(); g(); f(); }
			function i() payable public { i(); h(); g(); f(); }
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(suggest_pure)
{
	char const* text = R"(
		contract C {
			function g() view public { }
		}
	)";
	CHECK_WARNING(text, "can be restricted to pure");
}

BOOST_AUTO_TEST_CASE(suggest_view)
{
	char const* text = R"(
		contract C {
			uint x;
			function g() public returns (uint) { return x; }
		}
	)";
	CHECK_WARNING(text, "can be restricted to view");
}

BOOST_AUTO_TEST_CASE(call_internal_functions_fail)
{
	CHECK_ERROR(
		"contract C{ function f() pure public { g(); } function g() view public {} }",
		TypeError,
		"Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires \"view\""
	);
}

BOOST_AUTO_TEST_CASE(write_storage_fail)
{
	CHECK_WARNING(
		"contract C{ uint x; function f() view public { x = 2; } }",
		"Function declared as view, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable."
	);
}

BOOST_AUTO_TEST_CASE(environment_access)
{
	vector<string> view{
		"block.coinbase",
		"block.timestamp",
		"block.blockhash(7)",
		"block.difficulty",
		"block.number",
		"block.gaslimit",
		"msg.gas",
		"msg.value",
		"msg.sender",
		"tx.origin",
		"tx.gasprice",
		"this",
		"address(1).balance"
	};
	vector<string> pure{
		"msg.data",
		"msg.data[0]",
		"msg.sig",
		"block.blockhash", // Not evaluating the function
		"msg",
		"block",
		"tx"
	};
	for (string const& x: view)
	{
		CHECK_ERROR(
			"contract C { function f() pure public { var x = " + x + "; x; } }",
			TypeError,
			"Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires \"view\""
		);
	}
	for (string const& x: pure)
	{
		CHECK_WARNING(
			"contract C { function f() view public { var x = " + x + "; x; } }",
			"restricted to pure"
		);
	}
}

BOOST_AUTO_TEST_CASE(view_error_for_050)
{
	CHECK_ERROR(
		"pragma experimental \"v0.5.0\"; contract C { uint x; function f() view { x = 2; } }",
		TypeError,
		"Function declared as view, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable."
	);

}

BOOST_AUTO_TEST_CASE(modifiers)
{
	string text = R"(
		contract D {
			uint x;
			modifier purem(uint) { _; }
			modifier viewm(uint) { uint a = x; _; a; }
			modifier nonpayablem(uint) { x = 2; _; }
		}
		contract C is D {
			function f() purem(0) pure public {}
			function g() viewm(0) view public {}
			function h() nonpayablem(0) public {}
			function i() purem(x) view public {}
			function j() viewm(x) view public {}
			function k() nonpayablem(x) public {}
			function l() purem(x = 2) public {}
			function m() viewm(x = 2) public {}
			function n() nonpayablem(x = 2) public {}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(interface)
{
	string text = R"(
		interface D {
			function f() view public;
		}
		contract C is D {
			function f() view public {}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(overriding)
{
	string text = R"(
		contract D {
			uint x;
			function f() public { x = 2; }
		}
		contract C is D {
			function f() public {}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(returning_structs)
{
	string text = R"(
		contract C {
			struct S { uint x; }
			S s;
			function f() view internal returns (S storage) {
				return s;
			}
			function g() public {
				f().x = 2;
			}
			function h() view public {
				f();
				f().x;
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(mappings)
{
	string text = R"(
		contract C {
			mapping(uint => uint) a;
			function f() view public {
				a;
			}
			function g() view public {
				a[2];
			}
			function h() public {
				a[2] = 3;
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(local_storage_variables)
{
	string text = R"(
		contract C {
			struct S { uint a; }
			S s;
			function f() view public {
				S storage x = s;
				x;
			}
			function g() view public {
				S storage x = s;
				x = s;
			}
			function i() public {
				s.a = 2;
			}
			function h() public {
				S storage x = s;
				x.a = 2;
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(builtin_functions)
{
	string text = R"(
		contract C {
			function f() public {
				this.transfer(1);
				require(this.send(2));
				selfdestruct(this);
				require(this.delegatecall());
				require(this.call());
			}
			function g() pure public {
				var x = keccak256("abc");
				var y = sha256("abc");
				var z = ecrecover(1, 2, 3, 4);
				require(true);
				assert(true);
				x; y; z;
			}
			function() payable public {}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(function_types)
{
	string text = R"(
		contract C {
			function f() pure public {
				function () external nonpayFun;
				function () external view viewFun;
				function () external pure pureFun;

				nonpayFun;
				viewFun;
				pureFun;
				pureFun();
			}
			function g() view public {
				function () external view viewFun;

				viewFun();
			}
			function h() public {
				function () external nonpayFun;

				nonpayFun();
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(creation)
{
	string text = R"(
		contract D {}
		contract C {
			function f() public { new D(); }
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_CASE(assembly)
{
	string text = R"(
		contract C {
			struct S { uint x; }
			S s;
			function e() pure public {
				assembly { mstore(keccak256(0, 20), mul(s_slot, 2)) }
			}
			function f() pure public {
				uint x;
				assembly { x := 7 }
			}
			function g() view public {
				assembly { for {} 1 { pop(sload(0)) } { } pop(gas) }
			}
			function h() view public {
				assembly { function g() { pop(blockhash(20)) } }
			}
			function j() public {
				assembly { pop(call(0, 1, 2, 3, 4, 5, 6)) }
			}
			function k() public {
				assembly { pop(call(gas, 1, 2, 3, 4, 5, 6)) }
			}
		}
	)";
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
	CHECK_WARNING(text, "only available after the Metropolis");
}

BOOST_AUTO_TEST_CASE(assembly_jump)
{
	string text = R"(
		contract C {
			function k() public {
				assembly { jump(2) }
			}
		}
	)";
	CHECK_WARNING(text, "low-level EVM features");
}

BOOST_AUTO_TEST_CASE(constant)
{
	string text = R"(
		contract C {
			uint constant x = 2;
			function k() pure public returns (uint) {
				return x;
			}
		}
	)";
	CHECK_SUCCESS_NO_WARNINGS(text);
}

BOOST_AUTO_TEST_SUITE_END()

}
}
}
