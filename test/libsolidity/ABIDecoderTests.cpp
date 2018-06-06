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
 * Unit tests for Solidity's ABI decoder.
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

BOOST_FIXTURE_TEST_SUITE(ABIDecoderTest, SolidityExecutionFramework)

BOOST_AUTO_TEST_CASE(both_encoders_macro)
{
	// This tests that the "both decoders macro" at least runs twice and
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
			function f(uint a, uint16 b, uint24 c, int24 d, bytes3 x, bool e, C g) public returns (uint) {
				if (a != 1) return 1;
				if (b != 2) return 2;
				if (c != 3) return 3;
				if (d != 4) return 4;
				if (x != "abc") return 5;
				if (e != true) return 6;
				if (g != this) return 7;
				return 20;
			}
		}
	)";
	BOTH_ENCODERS(
		compileAndRun(sourceCode);
		ABI_CHECK(callContractFunction(
			"f(uint256,uint16,uint24,int24,bytes3,bool,address)",
			1, 2, 3, 4, string("abc"), true, u160(m_contractAddress)
		), encodeArgs(u256(20)));
	)
}

BOOST_AUTO_TEST_CASE(enums)
{
	string sourceCode = R"(
		contract C {
			enum E { A, B }
			function f(E e) public pure returns (uint x) {
				assembly { x := e }
			}
		}
	)";
	bool newDecoder = false;
	BOTH_ENCODERS(
		compileAndRun(sourceCode);
		ABI_CHECK(callContractFunction("f(uint8)", 0), encodeArgs(u256(0)));
		ABI_CHECK(callContractFunction("f(uint8)", 1), encodeArgs(u256(1)));
		// The old decoder was not as strict about enums
		ABI_CHECK(callContractFunction("f(uint8)", 2), (newDecoder ? encodeArgs() : encodeArgs(2)));
		ABI_CHECK(callContractFunction("f(uint8)", u256(-1)), (newDecoder? encodeArgs() : encodeArgs(u256(0xff))));
		newDecoder = true;
	)
}

BOOST_AUTO_TEST_CASE(cleanup)
{
	string sourceCode = R"(
		contract C {
			function f(uint16 a, int16 b, address c, bytes3 d, bool e)
					public pure returns (uint v, uint w, uint x, uint y, uint z) {
				assembly { v := a  w := b x := c y := d z := e}
			}
		}
	)";
	BOTH_ENCODERS(
		compileAndRun(sourceCode);
		ABI_CHECK(
			callContractFunction("f(uint16,int16,address,bytes3,bool)", 1, 2, 3, "a", true),
			encodeArgs(u256(1), u256(2), u256(3), string("a"), true)
		);
		ABI_CHECK(
			callContractFunction(
				"f(uint16,int16,address,bytes3,bool)",
				u256(0xffffff), u256(0x1ffff), u256(-1), string("abcd"), u256(4)
			),
			encodeArgs(u256(0xffff), u256(-1), (u256(1) << 160) - 1, string("abc"), true)
		);
	)
}

BOOST_AUTO_TEST_CASE(fixed_arrays)
{
	string sourceCode = R"(
		contract C {
			function f(uint16[3] a, uint16[2][3] b, uint i, uint j, uint k)
					public pure returns (uint, uint) {
				return (a[i], b[j][k]);
			}
		}
	)";
	BOTH_ENCODERS(
		compileAndRun(sourceCode);
		bytes args = encodeArgs(
			1, 2, 3,
			11, 12,
			21, 22,
			31, 32,
			1, 2, 1
		);
		ABI_CHECK(
			callContractFunction("f(uint16[3],uint16[2][3],uint256,uint256,uint256)", args),
			encodeArgs(u256(2), u256(32))
		);
	)
}

BOOST_AUTO_TEST_CASE(dynamic_arrays)
{
	string sourceCode = R"(
		contract C {
			function f(uint a, uint16[] b, uint c)
					public pure returns (uint, uint, uint) {
				return (b.length, b[a], c);
			}
		}
	)";
	BOTH_ENCODERS(
		compileAndRun(sourceCode);
		bytes args = encodeArgs(
			6, 0x60, 9,
			7,
			11, 12, 13, 14, 15, 16, 17
		);
		ABI_CHECK(
			callContractFunction("f(uint256,uint16[],uint256)", args),
			encodeArgs(u256(7), u256(17), u256(9))
		);
	)
}

BOOST_AUTO_TEST_CASE(dynamic_nested_arrays)
{
	string sourceCode = R"(
		contract C {
			function f(uint a, uint16[][] b, uint[2][][3] c, uint d)
					public pure returns (uint, uint, uint, uint, uint, uint, uint) {
				return (a, b.length, b[1].length, b[1][1], c[1].length, c[1][1][1], d);
			}
			function test() view returns (uint, uint, uint, uint, uint, uint, uint) {
				uint16[][] memory b = new uint16[][](3);
				b[0] = new uint16[](2);
				b[0][0] = 0x55;
				b[0][1] = 0x56;
				b[1] = new uint16[](4);
				b[1][0] = 0x65;
				b[1][1] = 0x66;
				b[1][2] = 0x67;
				b[1][3] = 0x68;

				uint[2][][3] memory c;
				c[0] = new uint[2][](1);
				c[0][0][1] = 0x75;
				c[1] = new uint[2][](5);
				c[1][1][1] = 0x85;

				return this.f(0x12, b, c, 0x13);
			}
		}
	)";
	NEW_ENCODER(
		compileAndRun(sourceCode);
		bytes args = encodeArgs(
			0x12, 4 * 0x20, 17 * 0x20, 0x13,
			// b
			3, 3 * 0x20, 6 * 0x20, 11 * 0x20,
			2, 85, 86,
			4, 101, 102, 103, 104,
			0,
			// c
			3 * 0x20, 6 * 0x20, 17 * 0x20,
			1, 0, 117,
			5, 0, 0, 0, 133, 0, 0, 0, 0, 0, 0,
			0
		);

		bytes expectation = encodeArgs(0x12, 3, 4, 0x66, 5, 0x85, 0x13);
		ABI_CHECK(callContractFunction("test()"), expectation);
		ABI_CHECK(callContractFunction("f(uint256,uint16[][],uint256[2][][3],uint256)", args), expectation);
	)
}

BOOST_AUTO_TEST_CASE(byte_arrays)
{
	string sourceCode = R"(
		contract C {
			function f(uint a, bytes b, uint c)
					public pure returns (uint, uint, byte, uint) {
				return (a, b.length, b[3], c);
			}

			function f_external(uint a, bytes b, uint c)
					external pure returns (uint, uint, byte, uint) {
				return (a, b.length, b[3], c);
			}
		}
	)";
	BOTH_ENCODERS(
		compileAndRun(sourceCode);
		bytes args = encodeArgs(
			6, 0x60, 9,
			7, "abcdefg"
		);
		ABI_CHECK(
			callContractFunction("f(uint256,bytes,uint256)", args),
			encodeArgs(u256(6), u256(7), "d", 9)
		);
		ABI_CHECK(
			callContractFunction("f_external(uint256,bytes,uint256)", args),
			encodeArgs(u256(6), u256(7), "d", 9)
		);
	)
}

BOOST_AUTO_TEST_CASE(calldata_arrays_too_large)
{
	string sourceCode = R"(
		contract C {
			function f(uint a, uint[] b, uint c) external pure returns (uint) {
				return 7;
			}
		}
	)";
	bool newEncoder = false;
	BOTH_ENCODERS(
		compileAndRun(sourceCode);
		bytes args = encodeArgs(
			6, 0x60, 9,
			(u256(1) << 255) + 2, 1, 2
		);
		ABI_CHECK(
			callContractFunction("f(uint256,uint256[],uint256)", args),
			newEncoder ? encodeArgs() : encodeArgs(7)
		);
		newEncoder = true;
	)
}

BOOST_AUTO_TEST_CASE(decode_from_memory_simple)
{
	string sourceCode = R"(
		contract C {
			uint public _a;
			uint[] public _b;
			function C(uint a, uint[] b) {
				_a = a;
				_b = b;
			}
		}
	)";
	BOTH_ENCODERS(
		compileAndRun(sourceCode, 0, "C", encodeArgs(
			7, 0x40,
			// b
			3, 0x21, 0x22, 0x23
		));
		ABI_CHECK(callContractFunction("_a()"), encodeArgs(7));
		ABI_CHECK(callContractFunction("_b(uint256)", 0), encodeArgs(0x21));
		ABI_CHECK(callContractFunction("_b(uint256)", 1), encodeArgs(0x22));
		ABI_CHECK(callContractFunction("_b(uint256)", 2), encodeArgs(0x23));
		ABI_CHECK(callContractFunction("_b(uint256)", 3), encodeArgs());
	)
}

BOOST_AUTO_TEST_CASE(decode_function_type)
{
	string sourceCode = R"(
		contract D {
			function () external returns (uint) public _a;
			function D(function () external returns (uint) a) {
				_a = a;
			}
		}
		contract C {
			function f() returns (uint) {
				return 3;
			}
			function g(function () external returns (uint) _f) returns (uint) {
				return _f();
			}
			// uses "decode from memory"
			function test1() returns (uint) {
				D d = new D(this.f);
				return d._a()();
			}
			// uses "decode from calldata"
			function test2() returns (uint) {
				return this.g(this.f);
			}
		}
	)";
	BOTH_ENCODERS(
		compileAndRun(sourceCode, 0, "C");
		ABI_CHECK(callContractFunction("test1()"), encodeArgs(3));
		ABI_CHECK(callContractFunction("test2()"), encodeArgs(3));
	)
}

BOOST_AUTO_TEST_CASE(decode_function_type_array)
{
	string sourceCode = R"(
		contract D {
			function () external returns (uint)[] public _a;
			function D(function () external returns (uint)[] a) {
				_a = a;
			}
		}
		contract E {
			function () external returns (uint)[3] public _a;
			function E(function () external returns (uint)[3] a) {
				_a = a;
			}
		}
		contract C {
			function f1() public returns (uint) {
				return 1;
			}
			function f2() public returns (uint) {
				return 2;
			}
			function f3() public returns (uint) {
				return 3;
			}
			function g(function () external returns (uint)[] _f, uint i) public returns (uint) {
				return _f[i]();
			}
			function h(function () external returns (uint)[3] _f, uint i) public returns (uint) {
				return _f[i]();
			}
			// uses "decode from memory"
			function test1_dynamic() public returns (uint) {
				var x = new function() external returns (uint)[](3);
				x[0] = this.f1;
				x[1] = this.f2;
				x[2] = this.f3;
				D d = new D(x);
				return d._a(2)();
			}
			function test1_static() public returns (uint) {
				E e = new E([this.f1, this.f2, this.f3]);
				return e._a(2)();
			}
			// uses "decode from calldata"
			function test2_dynamic() public returns (uint) {
				var x = new function() external returns (uint)[](3);
				x[0] = this.f1;
				x[1] = this.f2;
				x[2] = this.f3;
				return this.g(x, 0);
			}
			function test2_static() public returns (uint) {
				return this.h([this.f1, this.f2, this.f3], 0);
			}
		}
	)";
	BOTH_ENCODERS(
		compileAndRun(sourceCode, 0, "C");
		ABI_CHECK(callContractFunction("test1_static()"), encodeArgs(3));
		ABI_CHECK(callContractFunction("test1_dynamic()"), encodeArgs(3));
		ABI_CHECK(callContractFunction("test2_static()"), encodeArgs(1));
		ABI_CHECK(callContractFunction("test2_dynamic()"), encodeArgs(1));
	)
}

BOOST_AUTO_TEST_CASE(decode_from_memory_complex)
{
	string sourceCode = R"(
		contract C {
			uint public _a;
			uint[] public _b;
			bytes[2] public _c;
			function C(uint a, uint[] b, bytes[2] c) {
				_a = a;
				_b = b;
				_c = c;
			}
		}
	)";
	NEW_ENCODER(
		compileAndRun(sourceCode, 0, "C", encodeArgs(
			7, 0x60, 7 * 0x20,
			// b
			3, 0x21, 0x22, 0x23,
			// c
			0x40, 0x80,
			8, string("abcdefgh"),
			52, string("ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ")
		));
		ABI_CHECK(callContractFunction("_a()"), encodeArgs(7));
		ABI_CHECK(callContractFunction("_b(uint256)", 0), encodeArgs(0x21));
		ABI_CHECK(callContractFunction("_b(uint256)", 1), encodeArgs(0x22));
		ABI_CHECK(callContractFunction("_b(uint256)", 2), encodeArgs(0x23));
		ABI_CHECK(callContractFunction("_b(uint256)", 3), encodeArgs());
		ABI_CHECK(callContractFunction("_c(uint256)", 0), encodeArgs(0x20, 8, string("abcdefgh")));
		ABI_CHECK(callContractFunction("_c(uint256)", 1), encodeArgs(0x20, 52, string("ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ")));
		ABI_CHECK(callContractFunction("_c(uint256)", 2), encodeArgs());
	)
}

BOOST_AUTO_TEST_CASE(short_input_value_type)
{
	string sourceCode = R"(
		contract C {
			function f(uint a, uint b) public pure returns (uint) { return a; }
		}
	)";
	bool newDecoder = false;
	BOTH_ENCODERS(
		compileAndRun(sourceCode);
		ABI_CHECK(callContractFunction("f(uint256,uint256)", 1, 2), encodeArgs(1));
		ABI_CHECK(callContractFunctionNoEncoding("f(uint256,uint256)", bytes(64, 0)), encodeArgs(0));
		ABI_CHECK(callContractFunctionNoEncoding("f(uint256,uint256)", bytes(63, 0)), newDecoder ? encodeArgs() : encodeArgs(0));
		newDecoder = true;
	)
}

BOOST_AUTO_TEST_CASE(short_input_array)
{
	string sourceCode = R"(
		contract C {
			function f(uint[] a) public pure returns (uint) { return 7; }
		}
	)";
	bool newDecoder = false;
	BOTH_ENCODERS(
		compileAndRun(sourceCode);
		ABI_CHECK(callContractFunctionNoEncoding("f(uint256[])", encodeArgs(0x20, 0)), encodeArgs(7));
		ABI_CHECK(callContractFunctionNoEncoding("f(uint256[])", encodeArgs(0x20, 1)), newDecoder ? encodeArgs() : encodeArgs(7));
		ABI_CHECK(callContractFunctionNoEncoding("f(uint256[])", encodeArgs(0x20, 1) + bytes(31, 0)), newDecoder ? encodeArgs() : encodeArgs(7));
		ABI_CHECK(callContractFunctionNoEncoding("f(uint256[])", encodeArgs(0x20, 1) + bytes(32, 0)), encodeArgs(7));
		ABI_CHECK(callContractFunctionNoEncoding("f(uint256[])", encodeArgs(0x20, 2, 5, 6)), encodeArgs(7));
		newDecoder = true;
	)
}

BOOST_AUTO_TEST_CASE(short_dynamic_input_array)
{
	string sourceCode = R"(
		contract C {
			function f(bytes[1] a) public pure returns (uint) { return 7; }
		}
	)";
	NEW_ENCODER(
		compileAndRun(sourceCode);
		ABI_CHECK(callContractFunctionNoEncoding("f(bytes[1])", encodeArgs(0x20)), encodeArgs());
	)
}

BOOST_AUTO_TEST_CASE(short_input_bytes)
{
	string sourceCode = R"(
		contract C {
			function e(bytes a) public pure returns (uint) { return 7; }
			function f(bytes[] a) public pure returns (uint) { return 7; }
		}
	)";
	NEW_ENCODER(
		compileAndRun(sourceCode);
		ABI_CHECK(callContractFunctionNoEncoding("e(bytes)", encodeArgs(0x20, 7) + bytes(5, 0)), encodeArgs());
		ABI_CHECK(callContractFunctionNoEncoding("e(bytes)", encodeArgs(0x20, 7) + bytes(6, 0)), encodeArgs());
		ABI_CHECK(callContractFunctionNoEncoding("e(bytes)", encodeArgs(0x20, 7) + bytes(7, 0)), encodeArgs(7));
		ABI_CHECK(callContractFunctionNoEncoding("e(bytes)", encodeArgs(0x20, 7) + bytes(8, 0)), encodeArgs(7));
		ABI_CHECK(callContractFunctionNoEncoding("f(bytes[])", encodeArgs(0x20, 1, 0x20, 7) + bytes(5, 0)), encodeArgs());
		ABI_CHECK(callContractFunctionNoEncoding("f(bytes[])", encodeArgs(0x20, 1, 0x20, 7) + bytes(6, 0)), encodeArgs());
		ABI_CHECK(callContractFunctionNoEncoding("f(bytes[])", encodeArgs(0x20, 1, 0x20, 7) + bytes(7, 0)), encodeArgs(7));
		ABI_CHECK(callContractFunctionNoEncoding("f(bytes[])", encodeArgs(0x20, 1, 0x20, 7) + bytes(8, 0)), encodeArgs(7));
	)
}

BOOST_AUTO_TEST_CASE(cleanup_int_inside_arrays)
{
	string sourceCode = R"(
		contract C {
			enum E { A, B }
			function f(uint16[] a) public pure returns (uint r) { assembly { r := mload(add(a, 0x20)) } }
			function g(int16[] a) public pure returns (uint r) { assembly { r := mload(add(a, 0x20)) } }
			function h(E[] a) public pure returns (uint r) { assembly { r := mload(add(a, 0x20)) } }
		}
	)";
	NEW_ENCODER(
		compileAndRun(sourceCode);
		ABI_CHECK(callContractFunction("f(uint16[])", 0x20, 1, 7), encodeArgs(7));
		ABI_CHECK(callContractFunction("g(int16[])", 0x20, 1, 7), encodeArgs(7));
		ABI_CHECK(callContractFunction("f(uint16[])", 0x20, 1, u256("0xffff")), encodeArgs(u256("0xffff")));
		ABI_CHECK(callContractFunction("g(int16[])", 0x20, 1, u256("0xffff")), encodeArgs(u256(-1)));
		ABI_CHECK(callContractFunction("f(uint16[])", 0x20, 1, u256("0x1ffff")), encodeArgs(u256("0xffff")));
		ABI_CHECK(callContractFunction("g(int16[])", 0x20, 1, u256("0x10fff")), encodeArgs(u256("0x0fff")));
		ABI_CHECK(callContractFunction("h(uint8[])", 0x20, 1, 0), encodeArgs(u256(0)));
		ABI_CHECK(callContractFunction("h(uint8[])", 0x20, 1, 1), encodeArgs(u256(1)));
		ABI_CHECK(callContractFunction("h(uint8[])", 0x20, 1, 2), encodeArgs());
	)
}

BOOST_AUTO_TEST_CASE(storage_ptr)
{
	string sourceCode = R"(
		library L {
			struct S { uint x; uint y; }
			function f(uint[] storage r, S storage s) public returns (uint, uint, uint, uint) {
				r[2] = 8;
				s.x = 7;
				return (r[0], r[1], s.x, s.y);
			}
		}
		contract C {
			uint8 x = 3;
			L.S s;
			uint[] r;
			function f() public returns (uint, uint, uint, uint, uint, uint) {
				r.length = 6;
				r[0] = 1;
				r[1] = 2;
				r[2] = 3;
				s.x = 11;
				s.y = 12;
				var (a, b, c, d) = L.f(r, s);
				return (r[2], s.x, a, b, c, d);
			}
		}
	)";
	BOTH_ENCODERS(
		compileAndRun(sourceCode, 0, "L");
		compileAndRun(sourceCode, 0, "C", bytes(), map<string, Address>{{"L", m_contractAddress}});
		ABI_CHECK(callContractFunction("f()"), encodeArgs(8, 7, 1, 2, 7, 12));
	)
}

BOOST_AUTO_TEST_CASE(struct_simple)
{
	string sourceCode = R"(
		contract C {
			struct S { uint a; uint8 b; uint8 c; bytes2 d; }
			function f(S s) public pure returns (uint a, uint b, uint c, uint d) {
				a = s.a;
				b = s.b;
				c = s.c;
				d = uint16(s.d);
			}
		}
	)";
	NEW_ENCODER(
		compileAndRun(sourceCode, 0, "C");
		ABI_CHECK(callContractFunction("f((uint256,uint8,uint8,bytes2))", 1, 2, 3, "ab"), encodeArgs(1, 2, 3, 'a' * 0x100 + 'b'));
	)
}

BOOST_AUTO_TEST_CASE(struct_cleanup)
{
	string sourceCode = R"(
		contract C {
			struct S { int16 a; uint8 b; bytes2 c; }
			function f(S s) public pure returns (uint a, uint b, uint c) {
				assembly {
					a := mload(s)
					b := mload(add(s, 0x20))
					c := mload(add(s, 0x40))
				}
			}
		}
	)";
	NEW_ENCODER(
		compileAndRun(sourceCode, 0, "C");
		ABI_CHECK(
			callContractFunction("f((int16,uint8,bytes2))", 0xff010, 0xff0002, "abcd"),
			encodeArgs(u256("0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff010"), 2, "ab")
		);
	)
}

BOOST_AUTO_TEST_CASE(struct_short)
{
	string sourceCode = R"(
		contract C {
			struct S { int a; uint b; bytes16 c; }
			function f(S s) public pure returns (S q) {
				q = s;
			}
		}
	)";
	NEW_ENCODER(
		compileAndRun(sourceCode, 0, "C");
		ABI_CHECK(
			callContractFunction("f((int256,uint256,bytes16))", 0xff010, 0xff0002, "abcd"),
			encodeArgs(0xff010, 0xff0002, "abcd")
		);
		ABI_CHECK(
			callContractFunctionNoEncoding("f((int256,uint256,bytes16))", encodeArgs(0xff010, 0xff0002) + bytes(32, 0)),
			encodeArgs(0xff010, 0xff0002, 0)
		);
		ABI_CHECK(
			callContractFunctionNoEncoding("f((int256,uint256,bytes16))", encodeArgs(0xff010, 0xff0002) + bytes(31, 0)),
			encodeArgs()
		);
	)
}

BOOST_AUTO_TEST_CASE(struct_function)
{
	string sourceCode = R"(
		contract C {
			struct S { function () external returns (uint) f; uint b; }
			function f(S s) public returns (uint, uint) {
				return (s.f(), s.b);
			}
			function test() public returns (uint, uint) {
				return this.f(S(this.g, 3));
			}
			function g() public returns (uint) { return 7; }
		}
	)";
	NEW_ENCODER(
		compileAndRun(sourceCode, 0, "C");
		ABI_CHECK(callContractFunction("test()"), encodeArgs(7, 3));
	)
}

BOOST_AUTO_TEST_CASE(mediocre_struct)
{
	string sourceCode = R"(
		contract C {
			struct S { C c; }
			function f(uint a, S[2] s1, uint b) public returns (uint r1, C r2, uint r3) {
				r1 = a;
				r2 = s1[0].c;
				r3 = b;
			}
		}
	)";
	NEW_ENCODER(
		compileAndRun(sourceCode, 0, "C");
		string sig = "f(uint256,(address)[2],uint256)";
		ABI_CHECK(callContractFunction(sig,
			7, u256(u160(m_contractAddress)), 0, 8
		), encodeArgs(7, u256(u160(m_contractAddress)), 8));
	)
}

BOOST_AUTO_TEST_CASE(mediocre2_struct)
{
	string sourceCode = R"(
		contract C {
			struct S { C c; uint[] x; }
			function f(uint a, S[2] s1, uint b) public returns (uint r1, C r2, uint r3) {
				r1 = a;
				r2 = s1[0].c;
				r3 = b;
			}
		}
	)";
	NEW_ENCODER(
		compileAndRun(sourceCode, 0, "C");
		string sig = "f(uint256,(address,uint256[])[2],uint256)";
		ABI_CHECK(callContractFunction(sig,
			7, 0x60, 8,
			0x40, 7 * 0x20,
			u256(u160(m_contractAddress)), 0x40,
			2, 0x11, 0x12,
			0x99, 0x40,
			4, 0x31, 0x32, 0x34, 0x35
		), encodeArgs(7, u256(u160(m_contractAddress)), 8));
	)
}

BOOST_AUTO_TEST_CASE(complex_struct)
{
	string sourceCode = R"(
		contract C {
			enum E {A, B, C}
			struct T { uint x; E e; uint8 y; }
			struct S { C c; T[] t;}
			function f(uint a, S[2] s1, S[] s2, uint b) public returns
					(uint r1, C r2, uint r3, uint r4, C r5, uint r6, E r7, uint8 r8) {
				r1 = a;
				r2 = s1[0].c;
				r3 = b;
				r4 = s2.length;
				r5 = s2[1].c;
				r6 = s2[1].t.length;
				r7 = s2[1].t[1].e;
				r8 = s2[1].t[1].y;
			}
		}
	)";
	NEW_ENCODER(
		compileAndRun(sourceCode, 0, "C");
		string sig = "f(uint256,(address,(uint256,uint8,uint8)[])[2],(address,(uint256,uint8,uint8)[])[],uint256)";
		bytes args = encodeArgs(
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
		);
		ABI_CHECK(callContractFunction(sig, args), encodeArgs(7, u256(u160(m_contractAddress)), 8, 2, 0x1234, 3, 2, 0x22));
		// invalid enum value
		args.data()[0x20 * 28] = 3;
		ABI_CHECK(callContractFunction(sig, args), encodeArgs());
	)
}


BOOST_AUTO_TEST_CASE(return_dynamic_types_cross_call_simple)
{
	if (m_evmVersion == EVMVersion::homestead())
		return;

	string sourceCode = R"(
		contract C {
			function dyn() public returns (bytes) {
				return "1234567890123456789012345678901234567890";
			}
			function f() public returns (bytes) {
				return this.dyn();
			}
		}
	)";
	BOTH_ENCODERS(
		compileAndRun(sourceCode, 0, "C");
		ABI_CHECK(callContractFunction("f()"), encodeArgs(0x20, 40, string("1234567890123456789012345678901234567890")));
	)
}

BOOST_AUTO_TEST_CASE(return_dynamic_types_cross_call_advanced)
{
	if (m_evmVersion == EVMVersion::homestead())
		return;

	string sourceCode = R"(
		contract C {
			function dyn() public returns (bytes a, uint b, bytes20[] c, uint d) {
				a = "1234567890123456789012345678901234567890";
				b = uint(-1);
				c = new bytes20[](4);
				c[0] = bytes20(1234);
				c[3] = bytes20(6789);
				d = 0x1234;
			}
			function f() public returns (bytes, uint, bytes20[], uint) {
				return this.dyn();
			}
		}
	)";
	BOTH_ENCODERS(
		compileAndRun(sourceCode, 0, "C");
		ABI_CHECK(callContractFunction("f()"), encodeArgs(
			0x80, u256(-1), 0xe0, 0x1234,
			40, string("1234567890123456789012345678901234567890"),
			4, u256(1234) << (8 * (32 - 20)), 0, 0, u256(6789) << (8 * (32 - 20))
		));
	)
}

BOOST_AUTO_TEST_CASE(return_dynamic_types_cross_call_out_of_range)
{
	string sourceCode = R"(
		contract C {
			function dyn(uint x) public returns (bytes a) {
				assembly {
					mstore(0, 0x20)
					mstore(0x20, 0x21)
					return(0, x)
				}
			}
			function f(uint x) public returns (bool) {
				this.dyn(x);
				return true;
			}
		}
	)";
	BOTH_ENCODERS(
		compileAndRun(sourceCode, 0, "C");
		if (m_evmVersion == EVMVersion::homestead())
		{
			ABI_CHECK(callContractFunction("f(uint256)", 0x60), encodeArgs(true));
			ABI_CHECK(callContractFunction("f(uint256)", 0x7f), encodeArgs(true));
		}
		else
		{
			ABI_CHECK(callContractFunction("f(uint256)", 0x60), encodeArgs());
			ABI_CHECK(callContractFunction("f(uint256)", 0x61), encodeArgs(true));
		}
		ABI_CHECK(callContractFunction("f(uint256)", 0x80), encodeArgs(true));
	)
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
