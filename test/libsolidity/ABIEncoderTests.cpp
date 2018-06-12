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
 * Unit tests for Solidity's ABI encoder.
 */

#include <functional>
#include <string>
#include <tuple>
#include <boost/test/unit_test.hpp>
#include <libsolidity/interface/Exceptions.h>
#include <test/libsolidity/SolidityExecutionFramework.h>

#include <test/libsolidity/ABITestsCommon.h>

using namespace std;
using namespace std::placeholders;
using namespace dev::test;

namespace dev
{
namespace solidity
{
namespace test
{

#define REQUIRE_LOG_DATA(DATA) do { \
	BOOST_REQUIRE_EQUAL(m_logs.size(), 1); \
	BOOST_CHECK_EQUAL(m_logs[0].address, m_contractAddress); \
	BOOST_CHECK_EQUAL(toHex(m_logs[0].data), toHex(DATA)); \
} while (false)

BOOST_FIXTURE_TEST_SUITE(ABIEncoderTest, SolidityExecutionFramework)

BOOST_AUTO_TEST_CASE(both_encoders_macro)
{
	// This tests that the "both encoders macro" at least runs twice and
	// modifies the source.
	string sourceCode;
	int runs = 0;
	BOTH_ENCODERS(runs++;)
	BOOST_CHECK(sourceCode == NewEncoderPragma);
	BOOST_CHECK_EQUAL(runs, 2);
}

BOOST_AUTO_TEST_CASE(value_types)
{
	string sourceCode = R"(
		contract C {
			event E(uint a, uint16 b, uint24 c, int24 d, bytes3 x, bool, C);
			function f() public {
				bytes6 x = hex"1bababababa2";
				bool b;
				assembly { b := 7 }
				C c;
				assembly { c := sub(0, 5) }
				E(10, uint16(uint256(-2)), uint24(0x12121212), int24(int256(-1)), bytes3(x), b, c);
			}
		}
	)";
	BOTH_ENCODERS(
		compileAndRun(sourceCode);
		callContractFunction("f()");
		REQUIRE_LOG_DATA(encodeArgs(
			10, u256(65534), u256(0x121212), u256(-1), string("\x1b\xab\xab"), true, u160(u256(-5))
		));
	)
}

BOOST_AUTO_TEST_CASE(string_literal)
{
	string sourceCode = R"(
		contract C {
			event E(string, bytes20, string);
			function f() public {
				E("abcdef", "abcde", "abcdefabcdefgehabcabcasdfjklabcdefabcedefghabcabcasdfjklabcdefabcdefghabcabcasdfjklabcdeefabcdefghabcabcasdefjklabcdefabcdefghabcabcasdfjkl");
			}
		}
	)";
	BOTH_ENCODERS(
		compileAndRun(sourceCode);
		callContractFunction("f()");
		REQUIRE_LOG_DATA(encodeArgs(
			0x60, string("abcde"), 0xa0,
			6, string("abcdef"),
			0x8b, string("abcdefabcdefgehabcabcasdfjklabcdefabcedefghabcabcasdfjklabcdefabcdefghabcabcasdfjklabcdeefabcdefghabcabcasdefjklabcdefabcdefghabcabcasdfjkl")
		));
	)
}


BOOST_AUTO_TEST_CASE(enum_type_cleanup)
{
	string sourceCode = R"(
		contract C {
			enum E { A, B }
			function f(uint x) public returns (E en) {
				assembly { en := x }
			}
		}
	)";
	BOTH_ENCODERS(
		compileAndRun(sourceCode);
		BOOST_CHECK(callContractFunction("f(uint256)", 0) == encodeArgs(0));
		BOOST_CHECK(callContractFunction("f(uint256)", 1) == encodeArgs(1));
		BOOST_CHECK(callContractFunction("f(uint256)", 2) == encodeArgs());
	)
}

BOOST_AUTO_TEST_CASE(conversion)
{
	string sourceCode = R"(
		contract C {
			event E(bytes4, bytes4, uint16, uint8, int16, int8);
			function f() public {
				bytes2 x; assembly { x := 0xf1f2f3f400000000000000000000000000000000000000000000000000000000 }
				uint8 a;
				uint16 b = 0x1ff;
				int8 c;
				int16 d;
				assembly { a := sub(0, 1) c := 0x0101ff d := 0xff01 }
				E(10, x, a, uint8(b), c, int8(d));
			}
		}
	)";
	BOTH_ENCODERS(
		compileAndRun(sourceCode);
		callContractFunction("f()");
		REQUIRE_LOG_DATA(encodeArgs(
			string(3, 0) + string("\x0a"), string("\xf1\xf2"),
			0xff, 0xff, u256(-1), u256(1)
		));
	)
}

BOOST_AUTO_TEST_CASE(memory_array_one_dim)
{
	string sourceCode = R"(
		contract C {
			event E(uint a, int16[] b, uint c);
			function f() public {
				int16[] memory x = new int16[](3);
				assembly {
					for { let i := 0 } lt(i, 3) { i := add(i, 1) } {
						mstore(add(x, mul(add(i, 1), 0x20)), add(0xfffffffe, i))
					}
				}
				E(10, x, 11);
			}
		}
	)";

	compileAndRun(sourceCode);
	callContractFunction("f()");
	// The old encoder does not clean array elements.
	REQUIRE_LOG_DATA(encodeArgs(10, 0x60, 11, 3, u256("0xfffffffe"), u256("0xffffffff"), u256("0x100000000")));

	compileAndRun(NewEncoderPragma + sourceCode);
	callContractFunction("f()");
	REQUIRE_LOG_DATA(encodeArgs(10, 0x60, 11, 3, u256(-2), u256(-1), u256(0)));
}

BOOST_AUTO_TEST_CASE(memory_array_two_dim)
{
	string sourceCode = R"(
		contract C {
			event E(uint a, int16[][2] b, uint c);
			function f() public {
				int16[][2] memory x;
				x[0] = new int16[](3);
				x[1] = new int16[](2);
				x[0][0] = 7;
				x[0][1] = int16(0x010203040506);
				x[0][2] = -1;
				x[1][0] = 4;
				x[1][1] = 5;
				E(10, x, 11);
			}
		}
	)";
	NEW_ENCODER(
		compileAndRun(sourceCode);
		callContractFunction("f()");
		REQUIRE_LOG_DATA(encodeArgs(10, 0x60, 11, 0x40, 0xc0, 3, 7, 0x0506, u256(-1), 2, 4, 5));
	)
}

BOOST_AUTO_TEST_CASE(memory_byte_array)
{
	string sourceCode = R"(
		contract C {
			event E(uint a, bytes[] b, uint c);
			function f() public {
				bytes[] memory x = new bytes[](2);
				x[0] = "abcabcdefghjklmnopqrsuvwabcdefgijklmnopqrstuwabcdefgijklmnoprstuvw";
				x[1] = "abcdefghijklmnopqrtuvwabcfghijklmnopqstuvwabcdeghijklmopqrstuvw";
				E(10, x, 11);
			}
		}
	)";
	NEW_ENCODER(
		compileAndRun(sourceCode);
		callContractFunction("f()");
		REQUIRE_LOG_DATA(encodeArgs(
			10, 0x60, 11,
			2, 0x40, 0xc0,
			66, string("abcabcdefghjklmnopqrsuvwabcdefgijklmnopqrstuwabcdefgijklmnoprstuvw"),
			63, string("abcdefghijklmnopqrtuvwabcfghijklmnopqstuvwabcdeghijklmopqrstuvw")
		));
	)
}

BOOST_AUTO_TEST_CASE(storage_byte_array)
{
	string sourceCode = R"(
		contract C {
			bytes short;
			bytes long;
			event E(bytes s, bytes l);
			function f() public {
				short = "123456789012345678901234567890a";
				long = "ffff123456789012345678901234567890afffffffff123456789012345678901234567890a";
				E(short, long);
			}
		}
	)";
	BOTH_ENCODERS(
		compileAndRun(sourceCode);
		callContractFunction("f()");
		REQUIRE_LOG_DATA(encodeArgs(
			0x40, 0x80,
			31, string("123456789012345678901234567890a"),
			75, string("ffff123456789012345678901234567890afffffffff123456789012345678901234567890a")
		));
	)
}

BOOST_AUTO_TEST_CASE(storage_array)
{
	string sourceCode = R"(
		contract C {
			address[3] addr;
			event E(address[3] a);
			function f() public {
				assembly {
					sstore(0, sub(0, 1))
					sstore(1, sub(0, 2))
					sstore(2, sub(0, 3))
				}
				E(addr);
			}
		}
	)";
	BOTH_ENCODERS(
		compileAndRun(sourceCode);
		callContractFunction("f()");
		REQUIRE_LOG_DATA(encodeArgs(u160(-1), u160(-2), u160(-3)));
	)
}

BOOST_AUTO_TEST_CASE(storage_array_dyn)
{
	string sourceCode = R"(
		contract C {
			address[] addr;
			event E(address[] a);
			function f() public {
				addr.push(1);
				addr.push(2);
				addr.push(3);
				E(addr);
			}
		}
	)";
	BOTH_ENCODERS(
		compileAndRun(sourceCode);
		callContractFunction("f()");
		REQUIRE_LOG_DATA(encodeArgs(0x20, 3, u160(1), u160(2), u160(3)));
	)
}

BOOST_AUTO_TEST_CASE(storage_array_compact)
{
	string sourceCode = R"(
		contract C {
			int72[] x;
			event E(int72[]);
			function f() public {
				x.push(-1);
				x.push(2);
				x.push(-3);
				x.push(4);
				x.push(-5);
				x.push(6);
				x.push(-7);
				x.push(8);
				E(x);
			}
		}
	)";
	BOTH_ENCODERS(
		compileAndRun(sourceCode);
		callContractFunction("f()");
		REQUIRE_LOG_DATA(encodeArgs(
			0x20, 8, u256(-1), 2, u256(-3), 4, u256(-5), 6, u256(-7), 8
		));
	)
}

BOOST_AUTO_TEST_CASE(external_function)
{
	string sourceCode = R"(
		contract C {
			event E(function(uint) external returns (uint), function(uint) external returns (uint));
			function(uint) external returns (uint) g;
			function f(uint) public returns (uint) {
				g = this.f;
				E(this.f, g);
			}
		}
	)";
	BOTH_ENCODERS(
		compileAndRun(sourceCode);
		callContractFunction("f(uint256)", u256(0));
		string functionIdF = asString(m_contractAddress.ref()) + asString(FixedHash<4>(dev::keccak256("f(uint256)")).ref());
		REQUIRE_LOG_DATA(encodeArgs(functionIdF, functionIdF));
	)
}

BOOST_AUTO_TEST_CASE(external_function_cleanup)
{
	string sourceCode = R"(
		contract C {
			event E(function(uint) external returns (uint), function(uint) external returns (uint));
			// This test relies on the fact that g is stored in slot zero.
			function(uint) external returns (uint) g;
			function f(uint) public returns (uint) {
				function(uint) external returns (uint)[1] memory h;
				assembly { sstore(0, sub(0, 1)) mstore(h, sub(0, 1)) }
				E(h[0], g);
			}
		}
	)";
	BOTH_ENCODERS(
		compileAndRun(sourceCode);
		callContractFunction("f(uint256)", u256(0));
		REQUIRE_LOG_DATA(encodeArgs(string(24, char(-1)), string(24, char(-1))));
	)
}

BOOST_AUTO_TEST_CASE(calldata)
{
	string sourceCode = R"(
		contract C {
			event E(bytes);
			function f(bytes a) external {
				E(a);
			}
		}
	)";
	string s("abcdef");
	string t("abcdefgggggggggggggggggggggggggggggggggggggggghhheeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeggg");
	BOTH_ENCODERS(
		compileAndRun(sourceCode);
		callContractFunction("f(bytes)", 0x20, s.size(), s);
		REQUIRE_LOG_DATA(encodeArgs(0x20, s.size(), s));
		callContractFunction("f(bytes)", 0x20, t.size(), t);
		REQUIRE_LOG_DATA(encodeArgs(0x20, t.size(), t));
	)
}

BOOST_AUTO_TEST_CASE(function_name_collision)
{
	// This tests a collision between a function name used by inline assembly
	// and by the ABI encoder
	string sourceCode = R"(
		contract C {
			function f(uint x) public returns (uint) {
				assembly {
					function abi_encode_t_uint256_to_t_uint256() {
						mstore(0, 7)
						return(0, 0x20)
					}
					switch x
					case 0 { abi_encode_t_uint256_to_t_uint256() }
				}
				return 1;
			}
		}
	)";
	BOTH_ENCODERS(
		compileAndRun(sourceCode);
		BOOST_CHECK(callContractFunction("f(uint256)", encodeArgs(0)) == encodeArgs(7));
		BOOST_CHECK(callContractFunction("f(uint256)", encodeArgs(1)) == encodeArgs(1));
	)
}

BOOST_AUTO_TEST_CASE(structs)
{
	string sourceCode = R"(
		contract C {
			struct S { uint16 a; uint16 b; T[] sub; uint16 c; }
			struct T { uint64[2] x; }
			S s;
			event e(uint16, S);
			function f() public returns (uint, S) {
				uint16 x = 7;
				s.a = 8;
				s.b = 9;
				s.c = 10;
				s.sub.length = 3;
				s.sub[0].x[0] = 11;
				s.sub[1].x[0] = 12;
				s.sub[2].x[1] = 13;
				e(x, s);
				return (x, s);
			}
		}
	)";

	NEW_ENCODER(
		compileAndRun(sourceCode, 0, "C");
		bytes encoded = encodeArgs(
			u256(7), 0x40,
			8, 9, 0x80, 10,
			3,
			11, 0,
			12, 0,
			0, 13
		);
		BOOST_CHECK(callContractFunction("f()") == encoded);
		REQUIRE_LOG_DATA(encoded);
	)
}

BOOST_AUTO_TEST_CASE(empty_struct)
{
	string sourceCode = R"(
		contract C {
			struct S { }
			S s;
			event e(uint16, S, uint16);
			function f() returns (uint, S, uint) {
				e(7, s, 8);
				return (7, s, 8);
			}
		}
	)";

	NEW_ENCODER(
		compileAndRun(sourceCode, 0, "C");
		bytes encoded = encodeArgs(7, 8);
		BOOST_CHECK(callContractFunction("f()") == encoded);
		REQUIRE_LOG_DATA(encoded);
	)
}

BOOST_AUTO_TEST_CASE(structs2)
{
	string sourceCode = R"(
		contract C {
			enum E {A, B, C}
			struct T { uint x; E e; uint8 y; }
			struct S { C c; T[] t;}
			function f() public returns (uint a, S[2] s1, S[] s2, uint b) {
				a = 7;
				b = 8;
				s1[0].c = this;
				s1[0].t = new T[](1);
				s1[0].t[0].x = 0x11;
				s1[0].t[0].e = E.B;
				s1[0].t[0].y = 0x12;
				s2 = new S[](2);
				s2[1].c = C(0x1234);
				s2[1].t = new T[](3);
				s2[1].t[1].x = 0x21;
				s2[1].t[1].e = E.C;
				s2[1].t[1].y = 0x22;
			}
		}
	)";

	NEW_ENCODER(
		compileAndRun(sourceCode, 0, "C");
		ABI_CHECK(callContractFunction("f()"), encodeArgs(
			7, 0x80, 0x1e0, 8,
			// S[2] s1
			0x40,
			0x100,
			// S s1[0]
			u256(u160(m_contractAddress)),
			0x40,
			// T s1[0].t
			1, // length
			// s1[0].t[0]
			0x11, 1, 0x12,
			// S s1[1]
			0, 0x40,
			// T s1[1].t
			0,
			// S[] s2 (0x1e0)
			2, // length
			0x40, 0xa0,
			// S s2[0]
			0, 0x40, 0,
			// S s2[1]
			0x1234, 0x40,
			// s2[1].t
			3, // length
			0, 0, 0,
			0x21, 2, 0x22,
			0, 0, 0
		));
	)
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
