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
 * @author Christian <c@ethdev.com>
 * @author Gav Wood <g@ethdev.com>
 * @date 2014
 * Unit tests for the solidity expression compiler, testing the behaviour of the code.
 */

#include <test/libsolidity/SolidityExecutionFramework.h>

#include <test/Options.h>

#include <liblangutil/Exceptions.h>
#include <liblangutil/EVMVersion.h>

#include <libevmasm/Assembly.h>

#include <boost/test/unit_test.hpp>

#include <functional>
#include <string>
#include <tuple>

using namespace std;
using namespace std::placeholders;
using namespace dev::test;

namespace dev
{
namespace solidity
{
namespace test
{

BOOST_FIXTURE_TEST_SUITE(SolidityEndToEndTest, SolidityExecutionFramework)

BOOST_AUTO_TEST_CASE(transaction_status)
{
	char const* sourceCode = R"(
		contract test {
			function f() public { }
			function g() public { revert(); }
			function h() public { assert(false); }
		}
	)";
	compileAndRun(sourceCode);
	callContractFunction("f()");
	BOOST_CHECK(m_transactionSuccessful);
	callContractFunction("g()");
	BOOST_CHECK(!m_transactionSuccessful);
	callContractFunction("h()");
	BOOST_CHECK(!m_transactionSuccessful);
}


BOOST_AUTO_TEST_CASE(smoke_test)
{
	char const* sourceCode = R"(
		contract test {
			function f(uint a) public returns(uint d) { return a * 7; }
		}
	)";
	compileAndRun(sourceCode);
	testContractAgainstCppOnRange("f(uint256)", [](u256 const& a) -> u256 { return a * 7; }, 0, 100);
}

BOOST_AUTO_TEST_CASE(empty_contract)
{
	char const* sourceCode = R"(
		contract test { }
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callContractFunction("i_am_not_there()", bytes()).empty());
}

BOOST_AUTO_TEST_CASE(exp_operator)
{
	char const* sourceCode = R"(
		contract test {
			function f(uint a) public returns(uint d) { return 2 ** a; }
		}
	)";
	compileAndRun(sourceCode);
	testContractAgainstCppOnRange("f(uint256)", [](u256 const& a) -> u256 { return u256(1 << a.convert_to<int>()); }, 0, 16);
}

BOOST_AUTO_TEST_CASE(exp_operator_const)
{
	char const* sourceCode = R"(
		contract test {
			function f() public returns(uint d) { return 2 ** 3; }
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f()", bytes()), toBigEndian(u256(8)));
}

BOOST_AUTO_TEST_CASE(exp_operator_const_signed)
{
	char const* sourceCode = R"(
		contract test {
			function f() public returns(int d) { return (-2) ** 3; }
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f()", bytes()), toBigEndian(u256(-8)));
}

BOOST_AUTO_TEST_CASE(exp_zero)
{
	char const* sourceCode = R"(
		contract test {
			function f(uint a) public returns(uint d) { return a ** 0; }
		}
	)";
	compileAndRun(sourceCode);
	testContractAgainstCppOnRange("f(uint256)", [](u256 const&) -> u256 { return u256(1); }, 0, 16);
}

BOOST_AUTO_TEST_CASE(exp_zero_literal)
{
	char const* sourceCode = R"(
		contract test {
			function f() public returns(uint d) { return 0 ** 0; }
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f()", bytes()), toBigEndian(u256(1)));
}


BOOST_AUTO_TEST_CASE(conditional_expression_true_literal)
{
	char const* sourceCode = R"(
		contract test {
			function f() public returns(uint d) {
				return true ? 5 : 10;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f()", bytes()), toBigEndian(u256(5)));
}

BOOST_AUTO_TEST_CASE(conditional_expression_false_literal)
{
	char const* sourceCode = R"(
		contract test {
			function f() public returns(uint d) {
				return false ? 5 : 10;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f()", bytes()), toBigEndian(u256(10)));
}

BOOST_AUTO_TEST_CASE(conditional_expression_multiple)
{
	char const* sourceCode = R"(
		contract test {
			function f(uint x) public returns(uint d) {
				return x > 100 ?
							x > 1000 ? 1000 : 100
							:
							x > 50 ? 50 : 10;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f(uint256)", u256(1001)), toBigEndian(u256(1000)));
	ABI_CHECK(callContractFunction("f(uint256)", u256(500)), toBigEndian(u256(100)));
	ABI_CHECK(callContractFunction("f(uint256)", u256(80)), toBigEndian(u256(50)));
	ABI_CHECK(callContractFunction("f(uint256)", u256(40)), toBigEndian(u256(10)));
}

BOOST_AUTO_TEST_CASE(conditional_expression_with_return_values)
{
	char const* sourceCode = R"(
		contract test {
			function f(bool cond, uint v) public returns (uint a, uint b) {
				cond ? a = v : b = v;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f(bool,uint256)", true, u256(20)), encodeArgs(u256(20), u256(0)));
	ABI_CHECK(callContractFunction("f(bool,uint256)", false, u256(20)), encodeArgs(u256(0), u256(20)));
}

BOOST_AUTO_TEST_CASE(conditional_expression_storage_memory_1)
{
	char const* sourceCode = R"(
		contract test {
			bytes2[2] data1;
			function f(bool cond) public returns (uint) {
				bytes2[2] memory x;
				x[0] = "aa";
				bytes2[2] memory y;
				y[0] = "bb";

				data1 = cond ? x : y;

				uint ret = 0;
				if (data1[0] == "aa")
				{
					ret = 1;
				}

				if (data1[0] == "bb")
				{
					ret = 2;
				}

				return ret;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f(bool)", true), encodeArgs(u256(1)));
	ABI_CHECK(callContractFunction("f(bool)", false), encodeArgs(u256(2)));
}

BOOST_AUTO_TEST_CASE(conditional_expression_storage_memory_2)
{
	char const* sourceCode = R"(
		contract test {
			bytes2[2] data1;
			function f(bool cond) public returns (uint) {
				data1[0] = "cc";

				bytes2[2] memory x;
				bytes2[2] memory y;
				y[0] = "bb";

				x = cond ? y : data1;

				uint ret = 0;
				if (x[0] == "bb")
				{
					ret = 1;
				}

				if (x[0] == "cc")
				{
					ret = 2;
				}

				return ret;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f(bool)", true), encodeArgs(u256(1)));
	ABI_CHECK(callContractFunction("f(bool)", false), encodeArgs(u256(2)));
}

BOOST_AUTO_TEST_CASE(conditional_expression_different_types)
{
	char const* sourceCode = R"(
		contract test {
			function f(bool cond) public returns (uint) {
				uint8 x = 0xcd;
				uint16 y = 0xabab;
				return cond ? x : y;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f(bool)", true), encodeArgs(u256(0xcd)));
	ABI_CHECK(callContractFunction("f(bool)", false), encodeArgs(u256(0xabab)));
}

/* let's add this back when I figure out the correct type conversion.
BOOST_AUTO_TEST_CASE(conditional_expression_string_literal)
{
	char const* sourceCode = R"(
		contract test {
			function f(bool cond) public returns (bytes32) {
				return cond ? "true" : "false";
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f(bool)", true), encodeArgs(string("true", 4)));
	ABI_CHECK(callContractFunction("f(bool)", false), encodeArgs(string("false", 5)));
}
*/

BOOST_AUTO_TEST_CASE(conditional_expression_tuples)
{
	char const* sourceCode = R"(
		contract test {
			function f(bool cond) public returns (uint, uint) {
				return cond ? (1, 2) : (3, 4);
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f(bool)", true), encodeArgs(u256(1), u256(2)));
	ABI_CHECK(callContractFunction("f(bool)", false), encodeArgs(u256(3), u256(4)));
}

BOOST_AUTO_TEST_CASE(conditional_expression_functions)
{
	char const* sourceCode = R"(
		contract test {
			function x() public returns (uint) { return 1; }
			function y() public returns (uint) { return 2; }

			function f(bool cond) public returns (uint) {
				function () returns (uint) z = cond ? x : y;
				return z();
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f(bool)", true), encodeArgs(u256(1)));
	ABI_CHECK(callContractFunction("f(bool)", false), encodeArgs(u256(2)));
}

BOOST_AUTO_TEST_CASE(c99_scoping_activation)
{
	char const* sourceCode = R"(
		contract test {
			function f() pure public returns (uint) {
				uint x = 7;
				{
					x = 3; // This should still assign to the outer variable
					uint x;
					x = 4; // This should assign to the new one
				}
				return x;
			}
			function g() pure public returns (uint x) {
				x = 7;
				{
					x = 3;
					uint x;
					return x; // This returns the new variable, i.e. 0
				}
			}
			function h() pure public returns (uint x, uint a, uint b) {
				x = 7;
				{
					x = 3;
					a = x; // This should read from the outer
					uint x = 4;
					b = x;
				}
			}
			function i() pure public returns (uint x, uint a) {
				x = 7;
				{
					x = 3;
					uint x = x; // This should read from the outer and assign to the inner
					a = x;
				}
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f()"), encodeArgs(3));
	ABI_CHECK(callContractFunction("g()"), encodeArgs(0));
	ABI_CHECK(callContractFunction("h()"), encodeArgs(3, 3, 4));
	ABI_CHECK(callContractFunction("i()"), encodeArgs(3, 3));
}

BOOST_AUTO_TEST_CASE(recursive_calls)
{
	char const* sourceCode = R"(
		contract test {
			function f(uint n) public returns(uint nfac) {
				if (n <= 1) return 1;
				else return n * f(n - 1);
			}
		}
	)";
	compileAndRun(sourceCode);
	function<u256(u256)> recursive_calls_cpp = [&recursive_calls_cpp](u256 const& n) -> u256
	{
		if (n <= 1)
			return 1;
		else
			return n * recursive_calls_cpp(n - 1);
	};

	testContractAgainstCppOnRange("f(uint256)", recursive_calls_cpp, 0, 5);
}

BOOST_AUTO_TEST_CASE(multiple_functions)
{
	char const* sourceCode = R"(
		contract test {
			function a() public returns(uint n) { return 0; }
			function b() public returns(uint n) { return 1; }
			function c() public returns(uint n) { return 2; }
			function f() public returns(uint n) { return 3; }
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("a()", bytes()), toBigEndian(u256(0)));
	ABI_CHECK(callContractFunction("b()", bytes()), toBigEndian(u256(1)));
	ABI_CHECK(callContractFunction("c()", bytes()), toBigEndian(u256(2)));
	ABI_CHECK(callContractFunction("f()", bytes()), toBigEndian(u256(3)));
	ABI_CHECK(callContractFunction("i_am_not_there()", bytes()), bytes());
}

BOOST_AUTO_TEST_CASE(named_args)
{
	char const* sourceCode = R"(
		contract test {
			function a(uint a, uint b, uint c) public returns (uint r) { r = a * 100 + b * 10 + c * 1; }
			function b() public returns (uint r) { r = a({a: 1, b: 2, c: 3}); }
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("b()", bytes()), toBigEndian(u256(123)));
}

BOOST_AUTO_TEST_CASE(disorder_named_args)
{
	char const* sourceCode = R"(
		contract test {
			function a(uint a, uint b, uint c) public returns (uint r) { r = a * 100 + b * 10 + c * 1; }
			function b() public returns (uint r) { r = a({c: 3, a: 1, b: 2}); }
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("b()", bytes()), toBigEndian(u256(123)));
}

BOOST_AUTO_TEST_CASE(while_loop)
{
	char const* sourceCode = R"(
		contract test {
			function f(uint n) public returns(uint nfac) {
				nfac = 1;
				uint i = 2;
				while (i <= n) nfac *= i++;
			}
		}
	)";
	compileAndRun(sourceCode);

	auto while_loop_cpp = [](u256 const& n) -> u256
	{
		u256 nfac = 1;
		u256 i = 2;
		while (i <= n)
			nfac *= i++;

		return nfac;
	};

	testContractAgainstCppOnRange("f(uint256)", while_loop_cpp, 0, 5);
}


BOOST_AUTO_TEST_CASE(do_while_loop)
{
	char const* sourceCode = R"(
		contract test {
			function f(uint n) public returns(uint nfac) {
				nfac = 1;
				uint i = 2;
				do { nfac *= i++; } while (i <= n);
			}
		}
	)";
	compileAndRun(sourceCode);

	auto do_while_loop_cpp = [](u256 const& n) -> u256
	{
		u256 nfac = 1;
		u256 i = 2;
		do
		{
			nfac *= i++;
		}
		while (i <= n);

		return nfac;
	};

	testContractAgainstCppOnRange("f(uint256)", do_while_loop_cpp, 0, 5);
}

BOOST_AUTO_TEST_CASE(do_while_loop_continue)
{
	char const* sourceCode = R"(
		contract test {
			function f() public pure returns(uint r) {
				uint i = 0;
				do
				{
					if (i > 0) return 0;
					i++;
					continue;
				} while (false);
				return 42;
			}
		}
	)";
	compileAndRun(sourceCode);

	ABI_CHECK(callContractFunction("f()"), encodeArgs(42));
}

BOOST_AUTO_TEST_CASE(array_multiple_local_vars)
{
	char const* sourceCode = R"(
		contract test {
			function f(uint256[] calldata seq) external pure returns (uint256) {
				uint i = 0;
				uint sum = 0;
				while (i < seq.length)
				{
					uint idx = i;
					if (idx >= 10) break;
					uint x = seq[idx];
					if (x >= 1000) {
						uint n = i + 1;
						i = n;
						continue;
					}
					else {
						uint y = sum + x;
						sum = y;
					}
					if (sum >= 500) return sum;
					i++;
				}
				return sum;
			}
		}
	)";
	compileAndRun(sourceCode);

	ABI_CHECK(callContractFunction("f(uint256[])", 32, 3, u256(1000), u256(1), u256(2)), encodeArgs(3));
	ABI_CHECK(callContractFunction("f(uint256[])", 32, 3, u256(100), u256(500), u256(300)), encodeArgs(600));
	ABI_CHECK(callContractFunction(
		"f(uint256[])", 32, 11,
		u256(1), u256(2), u256(3), u256(4), u256(5), u256(6), u256(7), u256(8), u256(9), u256(10), u256(111)
		), encodeArgs(55));
}


BOOST_AUTO_TEST_CASE(do_while_loop_multiple_local_vars)
{
	char const* sourceCode = R"(
		contract test {
			function f(uint x) public pure returns(uint r) {
				uint i = 0;
				do
				{
					uint z = x * 2;
					if (z < 4) break;
					else {
						uint k = z + 1;
						if (k < 8) {
							x++;
							continue;
						}
					}
					if (z > 12) return 0;
					x++;
					i++;
				} while (true);
				return 42;
			}
		}
	)";
	compileAndRun(sourceCode);

	auto do_while = [](u256 n) -> u256
	{
		u256 i = 0;
		do
		{
			u256 z = n * 2;
			if (z < 4) break;
			else {
				u256 k = z + 1;
				if (k < 8) {
					n++;
					continue;
				}
			}
			if (z > 12) return 0;
			n++;
			i++;
		} while (true);
		return 42;
	};

	testContractAgainstCppOnRange("f(uint256)", do_while, 0, 12);
}

BOOST_AUTO_TEST_CASE(nested_loops)
{
	// tests that break and continue statements in nested loops jump to the correct place
	char const* sourceCode = R"(
		contract test {
			function f(uint x) public returns(uint y) {
				while (x > 1) {
					if (x == 10) break;
					while (x > 5) {
						if (x == 8) break;
						x--;
						if (x == 6) continue;
						return x;
					}
					x--;
					if (x == 3) continue;
					break;
				}
				return x;
			}
		}
	)";
	compileAndRun(sourceCode);

	auto nested_loops_cpp = [](u256 n) -> u256
	{
		while (n > 1)
		{
			if (n == 10)
				break;
			while (n > 5)
			{
				if (n == 8)
					break;
				n--;
				if (n == 6)
					continue;
				return n;
			}
			n--;
			if (n == 3)
				continue;
			break;
		}

		return n;
	};

	testContractAgainstCppOnRange("f(uint256)", nested_loops_cpp, 0, 12);
}

BOOST_AUTO_TEST_CASE(nested_loops_multiple_local_vars)
{
	// tests that break and continue statements in nested loops jump to the correct place
	// and free local variables properly
	char const* sourceCode = R"(
		contract test {
			function f(uint x) public returns(uint y) {
				while (x > 0) {
					uint z = x + 10;
					uint k = z + 1;
					if (k > 20) {
						break;
						uint p = 100;
						k += p;
					}
					if (k > 15) {
						x--;
						continue;
						uint t = 1000;
						x += t;
					}
					while (k > 10) {
						uint m = k - 1;
						if (m == 10) return x;
						return k;
						uint h = 10000;
						z += h;
					}
					x--;
					break;
				}
				return x;
			}
		}
	)";
	compileAndRun(sourceCode);

	auto nested_loops_cpp = [](u256 n) -> u256
	{
		while (n > 0)
		{
			u256 z = n + 10;
			u256 k = z + 1;
			if (k > 20) break;
			if (k > 15) {
				n--;
				continue;
			}
			while (k > 10)
			{
				u256 m = k - 1;
				if (m == 10) return n;
				return k;
			}
			n--;
			break;
		}

		return n;
	};

	testContractAgainstCppOnRange("f(uint256)", nested_loops_cpp, 0, 12);
}

BOOST_AUTO_TEST_CASE(for_loop_multiple_local_vars)
{
	char const* sourceCode = R"(
		contract test {
			function f(uint x) public pure returns(uint r) {
				for (uint i = 0; i < 12; i++)
				{
					uint z = x + 1;
					if (z < 4) break;
					else {
						uint k = z * 2;
						if (i + k < 10) {
							x++;
							continue;
						}
					}
					if (z > 8) return 0;
					x++;
				}
				return 42;
			}
		}
	)";
	compileAndRun(sourceCode);

	auto for_loop = [](u256 n) -> u256
	{
		for (u256 i = 0; i < 12; i++)
		{
			u256 z = n + 1;
			if (z < 4) break;
			else {
				u256 k = z * 2;
				if (i + k < 10) {
					n++;
					continue;
				}
			}
			if (z > 8) return 0;
			n++;
		}
		return 42;
	};

	testContractAgainstCppOnRange("f(uint256)", for_loop, 0, 12);
}

BOOST_AUTO_TEST_CASE(nested_for_loop_multiple_local_vars)
{
	char const* sourceCode = R"(
		contract test {
			function f(uint x) public pure returns(uint r) {
				for (uint i = 0; i < 5; i++)
				{
					uint z = x + 1;
					if (z < 3) {
						break;
						uint p = z + 2;
					}
					for (uint j = 0; j < 5; j++)
					{
						uint k = z * 2;
						if (j + k < 8) {
							x++;
							continue;
							uint t = z * 3;
						}
						x++;
						if (x > 20) {
							return 84;
							uint h = x + 42;
						}
					}
					if (x > 30) {
						return 42;
						uint b = 0xcafe;
					}
				}
				return 42;
			}
		}
	)";
	compileAndRun(sourceCode);

	auto for_loop = [](u256 n) -> u256
	{
		for (u256 i = 0; i < 5; i++)
		{
			u256 z = n + 1;
			if (z < 3) break;
			for (u256 j = 0; j < 5; j++)
			{
				u256 k = z * 2;
				if (j + k < 8) {
					n++;
					continue;
				}
				n++;
				if (n > 20) return 84;
			}
			if (n > 30) return 42;
		}
		return 42;
	};

	testContractAgainstCppOnRange("f(uint256)", for_loop, 0, 12);
}

BOOST_AUTO_TEST_CASE(for_loop)
{
	char const* sourceCode = R"(
		contract test {
			function f(uint n) public returns(uint nfac) {
				nfac = 1;
				uint i;
				for (i = 2; i <= n; i++)
					nfac *= i;
			}
		}
	)";
	compileAndRun(sourceCode);

	auto for_loop_cpp = [](u256 const& n) -> u256
	{
		u256 nfac = 1;
		for (auto i = 2; i <= n; i++)
			nfac *= i;
		return nfac;
	};

	testContractAgainstCppOnRange("f(uint256)", for_loop_cpp, 0, 5);
}

BOOST_AUTO_TEST_CASE(for_loop_empty)
{
	char const* sourceCode = R"(
		contract test {
			function f() public returns(uint ret) {
				ret = 1;
				for (;;) {
					ret += 1;
					if (ret >= 10) break;
				}
			}
		}
	)";
	compileAndRun(sourceCode);

	auto for_loop_empty_cpp = []() -> u256
	{
		u256 ret = 1;
		for (;;)
		{
			ret += 1;
			if (ret >= 10) break;
		}
		return ret;
	};

	testContractAgainstCpp("f()", for_loop_empty_cpp);
}

BOOST_AUTO_TEST_CASE(for_loop_simple_init_expr)
{
	char const* sourceCode = R"(
		contract test {
			function f(uint n) public returns(uint nfac) {
				nfac = 1;
				uint256 i;
				for (i = 2; i <= n; i++)
					nfac *= i;
			}
		}
	)";
	compileAndRun(sourceCode);

	auto for_loop_simple_init_expr_cpp = [](u256 const& n) -> u256
	{
		u256 nfac = 1;
		u256 i;
		for (i = 2; i <= n; i++)
			nfac *= i;
		return nfac;
	};

	testContractAgainstCppOnRange("f(uint256)", for_loop_simple_init_expr_cpp, 0, 5);
}

BOOST_AUTO_TEST_CASE(for_loop_break_continue)
{
	char const* sourceCode = R"(
		contract test {
			function f(uint n) public returns (uint r)
			{
				uint i = 1;
				uint k = 0;
				for (i *= 5; k < n; i *= 7)
				{
					k++;
					i += 4;
					if (n % 3 == 0)
						break;
					i += 9;
					if (n % 2 == 0)
						continue;
					i += 19;
				}
				return i;
			}
		}
	)";
	compileAndRun(sourceCode);

	auto breakContinue = [](u256 const& n) -> u256
	{
		u256 i = 1;
		u256 k = 0;
		for (i *= 5; k < n; i *= 7)
		{
			k++;
			i += 4;
			if (n % 3 == 0)
				break;
			i += 9;
			if (n % 2 == 0)
				continue;
			i += 19;
		}
		return i;
	};

	testContractAgainstCppOnRange("f(uint256)", breakContinue, 0, 10);
}

BOOST_AUTO_TEST_CASE(calling_other_functions)
{
	char const* sourceCode = R"(
		contract collatz {
			function run(uint x) public returns(uint y) {
				while ((y = x) > 1) {
					if (x % 2 == 0) x = evenStep(x);
					else x = oddStep(x);
				}
			}
			function evenStep(uint x) public returns(uint y) {
				return x / 2;
			}
			function oddStep(uint x) public returns(uint y) {
				return 3 * x + 1;
			}
		}
	)";
	compileAndRun(sourceCode);

	auto evenStep_cpp = [](u256 const& n) -> u256
	{
		return n / 2;
	};

	auto oddStep_cpp = [](u256 const& n) -> u256
	{
		return 3 * n + 1;
	};

	auto collatz_cpp = [&evenStep_cpp, &oddStep_cpp](u256 n) -> u256
	{
		u256 y;
		while ((y = n) > 1)
		{
			if (n % 2 == 0)
				n = evenStep_cpp(n);
			else
				n = oddStep_cpp(n);
		}
		return y;
	};

	testContractAgainstCpp("run(uint256)", collatz_cpp, u256(0));
	testContractAgainstCpp("run(uint256)", collatz_cpp, u256(1));
	testContractAgainstCpp("run(uint256)", collatz_cpp, u256(2));
	testContractAgainstCpp("run(uint256)", collatz_cpp, u256(8));
	testContractAgainstCpp("run(uint256)", collatz_cpp, u256(127));
}

BOOST_AUTO_TEST_CASE(many_local_variables)
{
	char const* sourceCode = R"(
		contract test {
			function run(uint x1, uint x2, uint x3) public returns(uint y) {
				uint8 a = 0x1; uint8 b = 0x10; uint16 c = 0x100;
				y = a + b + c + x1 + x2 + x3;
				y += b + x2;
			}
		}
	)";
	compileAndRun(sourceCode);
	auto f = [](u256 const& x1, u256 const& x2, u256 const& x3) -> u256
	{
		u256 a = 0x1;
		u256 b = 0x10;
		u256 c = 0x100;
		u256 y = a + b + c + x1 + x2 + x3;
		return y + b + x2;
	};
	testContractAgainstCpp("run(uint256,uint256,uint256)", f, u256(0x1000), u256(0x10000), u256(0x100000));
}

BOOST_AUTO_TEST_CASE(packing_unpacking_types)
{
	char const* sourceCode = R"(
		contract test {
			function run(bool a, uint32 b, uint64 c) public returns(uint256 y) {
				if (a) y = 1;
				y = y * 0x100000000 | ~b;
				y = y * 0x10000000000000000 | ~c;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(
		callContractFunction("run(bool,uint32,uint64)", true, fromHex("0f0f0f0f"), fromHex("f0f0f0f0f0f0f0f0")),
		fromHex("00000000000000000000000000000000000000""01""f0f0f0f0""0f0f0f0f0f0f0f0f")
	);
}

BOOST_AUTO_TEST_CASE(packing_signed_types)
{
	char const* sourceCode = R"(
		contract test {
			function run() public returns(int8 y) {
				uint8 x = 0xfa;
				return int8(x);
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(
		callContractFunction("run()"),
		fromHex("fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffa")
	);
}

BOOST_AUTO_TEST_CASE(multiple_return_values)
{
	char const* sourceCode = R"(
		contract test {
			function run(bool x1, uint x2) public returns(uint y1, bool y2, uint y3) {
				y1 = x2; y2 = x1;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("run(bool,uint256)", true, 0xcd), encodeArgs(0xcd, true, 0));
}

BOOST_AUTO_TEST_CASE(short_circuiting)
{
	char const* sourceCode = R"(
		contract test {
			function run(uint x) public returns(uint y) {
				x == 0 || ((x = 8) > 0);
				return x;
			}
		}
	)";
	compileAndRun(sourceCode);

	auto short_circuiting_cpp = [](u256 n) -> u256
	{
		(void)(n == 0 || (n = 8) > 0);
		return n;
	};

	testContractAgainstCppOnRange("run(uint256)", short_circuiting_cpp, 0, 2);
}

BOOST_AUTO_TEST_CASE(high_bits_cleaning)
{
	char const* sourceCode = R"(
		contract test {
			function run() public returns(uint256 y) {
				uint32 t = uint32(0xffffffff);
				uint32 x = t + 10;
				if (x >= 0xffffffff) return 0;
				return x;
			}
		}
	)";
	compileAndRun(sourceCode);
	auto high_bits_cleaning_cpp = []() -> u256
	{
		uint32_t t = uint32_t(0xffffffff);
		uint32_t x = t + 10;
		if (x >= 0xffffffff)
			return 0;
		return x;
	};
	testContractAgainstCpp("run()", high_bits_cleaning_cpp);
}

BOOST_AUTO_TEST_CASE(sign_extension)
{
	char const* sourceCode = R"(
		contract test {
			function run() public returns(uint256 y) {
				int64 x = -int32(0xff);
				if (x >= 0xff) return 0;
				return -uint256(x);
			}
		}
	)";
	compileAndRun(sourceCode);
	auto sign_extension_cpp = []() -> u256
	{
		int64_t x = -int32_t(0xff);
		if (x >= 0xff)
			return 0;
		return u256(x) * -1;
	};
	testContractAgainstCpp("run()", sign_extension_cpp);
}

BOOST_AUTO_TEST_CASE(small_unsigned_types)
{
	char const* sourceCode = R"(
		contract test {
			function run() public returns(uint256 y) {
				uint32 t = uint32(0xffffff);
				uint32 x = t * 0xffffff;
				return x / 0x100;
			}
		}
	)";
	compileAndRun(sourceCode);
	auto small_unsigned_types_cpp = []() -> u256
	{
		uint32_t t = uint32_t(0xffffff);
		uint32_t x = t * 0xffffff;
		return x / 0x100;
	};
	testContractAgainstCpp("run()", small_unsigned_types_cpp);
}

BOOST_AUTO_TEST_CASE(small_signed_types)
{
	char const* sourceCode = R"(
		contract test {
			function run() public returns(int256 y) {
				return -int32(10) * -int64(20);
			}
		}
	)";
	compileAndRun(sourceCode);
	auto small_signed_types_cpp = []() -> u256
	{
		return -int32_t(10) * -int64_t(20);
	};
	testContractAgainstCpp("run()", small_signed_types_cpp);
}

BOOST_AUTO_TEST_CASE(strings)
{
	char const* sourceCode = R"(
		contract test {
			function fixedBytes() public returns(bytes32 ret) {
				return "abc\x00\xff__";
			}
			function pipeThrough(bytes2 small, bool one) public returns(bytes16 large, bool oneRet) {
				oneRet = one;
				large = small;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("fixedBytes()"), encodeArgs(string("abc\0\xff__", 7)));
	ABI_CHECK(callContractFunction("pipeThrough(bytes2,bool)", string("\0\x02", 2), true), encodeArgs(string("\0\x2", 2), true));
}

BOOST_AUTO_TEST_CASE(inc_dec_operators)
{
	char const* sourceCode = R"(
		contract test {
			uint8 x;
			uint v;
			function f() public returns (uint r) {
				uint a = 6;
				r = a;
				r += (a++) * 0x10;
				r += (++a) * 0x100;
				v = 3;
				r += (v++) * 0x1000;
				r += (++v) * 0x10000;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f()"), encodeArgs(0x53866));
}

BOOST_AUTO_TEST_CASE(bytes_comparison)
{
	char const* sourceCode = R"(
		contract test {
			function f() public returns (bool) {
				bytes2 a = "a";
				bytes2 x = "aa";
				bytes2 b = "b";
				return a < x && x < b;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f()"), encodeArgs(true));
}

BOOST_AUTO_TEST_CASE(state_smoke_test)
{
	char const* sourceCode = R"(
		contract test {
			uint256 value1;
			uint256 value2;
			function get(uint8 which) public returns (uint256 value) {
				if (which == 0) return value1;
				else return value2;
			}
			function set(uint8 which, uint256 value) public {
				if (which == 0) value1 = value;
				else value2 = value;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("get(uint8)", uint8_t(0x00)), encodeArgs(0));
	ABI_CHECK(callContractFunction("get(uint8)", uint8_t(0x01)), encodeArgs(0));
	ABI_CHECK(callContractFunction("set(uint8,uint256)", uint8_t(0x00), 0x1234), encodeArgs());
	ABI_CHECK(callContractFunction("set(uint8,uint256)", uint8_t(0x01), 0x8765), encodeArgs());
	ABI_CHECK(callContractFunction("get(uint8)", uint8_t( 0x00)), encodeArgs(0x1234));
	ABI_CHECK(callContractFunction("get(uint8)", uint8_t(0x01)), encodeArgs(0x8765));
	ABI_CHECK(callContractFunction("set(uint8,uint256)", uint8_t(0x00), 0x3), encodeArgs());
	ABI_CHECK(callContractFunction("get(uint8)", uint8_t(0x00)), encodeArgs(0x3));
}

BOOST_AUTO_TEST_CASE(compound_assign)
{
	char const* sourceCode = R"(
		contract test {
			uint value1;
			uint value2;
			function f(uint x, uint y) public returns (uint w) {
				uint value3 = y;
				value1 += x;
				value3 *= x;
				value2 *= value3 + value1;
				return value2 += 7;
			}
		}
	)";
	compileAndRun(sourceCode);

	u256 value1;
	u256 value2;
	auto f = [&](u256 const& _x, u256 const& _y) -> u256
	{
		u256 value3 = _y;
		value1 += _x;
		value3 *= _x;
		value2 *= value3 + value1;
		return value2 += 7;
	};
	testContractAgainstCpp("f(uint256,uint256)", f, u256(0), u256(6));
	testContractAgainstCpp("f(uint256,uint256)", f, u256(1), u256(3));
	testContractAgainstCpp("f(uint256,uint256)", f, u256(2), u256(25));
	testContractAgainstCpp("f(uint256,uint256)", f, u256(3), u256(69));
	testContractAgainstCpp("f(uint256,uint256)", f, u256(4), u256(84));
	testContractAgainstCpp("f(uint256,uint256)", f, u256(5), u256(2));
	testContractAgainstCpp("f(uint256,uint256)", f, u256(6), u256(51));
	testContractAgainstCpp("f(uint256,uint256)", f, u256(7), u256(48));
}

BOOST_AUTO_TEST_CASE(simple_mapping)
{
	char const* sourceCode = R"(
		contract test {
			mapping(uint8 => uint8) table;
			function get(uint8 k) public returns (uint8 v) {
				return table[k];
			}
			function set(uint8 k, uint8 v) public {
				table[k] = v;
			}
		}
	)";
	compileAndRun(sourceCode);

	ABI_CHECK(callContractFunction("get(uint8)", uint8_t(0)), encodeArgs(uint8_t(0x00)));
	ABI_CHECK(callContractFunction("get(uint8)", uint8_t(0x01)), encodeArgs(uint8_t(0x00)));
	ABI_CHECK(callContractFunction("get(uint8)", uint8_t(0xa7)), encodeArgs(uint8_t(0x00)));
	callContractFunction("set(uint8,uint8)", uint8_t(0x01), uint8_t(0xa1));
	ABI_CHECK(callContractFunction("get(uint8)", uint8_t(0x00)), encodeArgs(uint8_t(0x00)));
	ABI_CHECK(callContractFunction("get(uint8)", uint8_t(0x01)), encodeArgs(uint8_t(0xa1)));
	ABI_CHECK(callContractFunction("get(uint8)", uint8_t(0xa7)), encodeArgs(uint8_t(0x00)));
	callContractFunction("set(uint8,uint8)", uint8_t(0x00), uint8_t(0xef));
	ABI_CHECK(callContractFunction("get(uint8)", uint8_t(0x00)), encodeArgs(uint8_t(0xef)));
	ABI_CHECK(callContractFunction("get(uint8)", uint8_t(0x01)), encodeArgs(uint8_t(0xa1)));
	ABI_CHECK(callContractFunction("get(uint8)", uint8_t(0xa7)), encodeArgs(uint8_t(0x00)));
	callContractFunction("set(uint8,uint8)", uint8_t(0x01), uint8_t(0x05));
	ABI_CHECK(callContractFunction("get(uint8)", uint8_t(0x00)), encodeArgs(uint8_t(0xef)));
	ABI_CHECK(callContractFunction("get(uint8)", uint8_t(0x01)), encodeArgs(uint8_t(0x05)));
	ABI_CHECK(callContractFunction("get(uint8)", uint8_t(0xa7)), encodeArgs(uint8_t(0x00)));
}

BOOST_AUTO_TEST_CASE(mapping_state)
{
	char const* sourceCode = R"(
		contract Ballot {
			mapping(address => bool) canVote;
			mapping(address => uint) voteCount;
			mapping(address => bool) voted;
			function getVoteCount(address addr) public returns (uint retVoteCount) {
				return voteCount[addr];
			}
			function grantVoteRight(address addr) public {
				canVote[addr] = true;
			}
			function vote(address voter, address vote) public returns (bool success) {
				if (!canVote[voter] || voted[voter]) return false;
				voted[voter] = true;
				voteCount[vote] = voteCount[vote] + 1;
				return true;
			}
		}
	)";
	compileAndRun(sourceCode);
	class Ballot
	{
	public:
		u256 getVoteCount(u160 _address) { return m_voteCount[_address]; }
		void grantVoteRight(u160 _address) { m_canVote[_address] = true; }
		bool vote(u160 _voter, u160 _vote)
		{
			if (!m_canVote[_voter] || m_voted[_voter]) return false;
			m_voted[_voter] = true;
			m_voteCount[_vote]++;
			return true;
		}
	private:
		map<u160, bool> m_canVote;
		map<u160, u256> m_voteCount;
		map<u160, bool> m_voted;
	} ballot;

	auto getVoteCount = bind(&Ballot::getVoteCount, &ballot, _1);
	auto grantVoteRight = bind(&Ballot::grantVoteRight, &ballot, _1);
	auto vote = bind(&Ballot::vote, &ballot, _1, _2);
	testContractAgainstCpp("getVoteCount(address)", getVoteCount, u160(0));
	testContractAgainstCpp("getVoteCount(address)", getVoteCount, u160(1));
	testContractAgainstCpp("getVoteCount(address)", getVoteCount, u160(2));
	// voting without vote right should be rejected
	testContractAgainstCpp("vote(address,address)", vote, u160(0), u160(2));
	testContractAgainstCpp("getVoteCount(address)", getVoteCount, u160(0));
	testContractAgainstCpp("getVoteCount(address)", getVoteCount, u160(1));
	testContractAgainstCpp("getVoteCount(address)", getVoteCount, u160(2));
	// grant vote rights
	testContractAgainstCpp("grantVoteRight(address)", grantVoteRight, u160(0));
	testContractAgainstCpp("grantVoteRight(address)", grantVoteRight, u160(1));
	// vote, should increase 2's vote count
	testContractAgainstCpp("vote(address,address)", vote, u160(0), u160(2));
	testContractAgainstCpp("getVoteCount(address)", getVoteCount, u160(0));
	testContractAgainstCpp("getVoteCount(address)", getVoteCount, u160(1));
	testContractAgainstCpp("getVoteCount(address)", getVoteCount, u160(2));
	// vote again, should be rejected
	testContractAgainstCpp("vote(address,address)", vote, u160(0), u160(1));
	testContractAgainstCpp("getVoteCount(address)", getVoteCount, u160(0));
	testContractAgainstCpp("getVoteCount(address)", getVoteCount, u160(1));
	testContractAgainstCpp("getVoteCount(address)", getVoteCount, u160(2));
	// vote without right to vote
	testContractAgainstCpp("vote(address,address)", vote, u160(2), u160(1));
	testContractAgainstCpp("getVoteCount(address)", getVoteCount, u160(0));
	testContractAgainstCpp("getVoteCount(address)", getVoteCount, u160(1));
	testContractAgainstCpp("getVoteCount(address)", getVoteCount, u160(2));
	// grant vote right and now vote again
	testContractAgainstCpp("grantVoteRight(address)", grantVoteRight, u160(2));
	testContractAgainstCpp("vote(address,address)", vote, u160(2), u160(1));
	testContractAgainstCpp("getVoteCount(address)", getVoteCount, u160(0));
	testContractAgainstCpp("getVoteCount(address)", getVoteCount, u160(1));
	testContractAgainstCpp("getVoteCount(address)", getVoteCount, u160(2));
}

BOOST_AUTO_TEST_CASE(mapping_state_inc_dec)
{
	char const* sourceCode = R"(
		contract test {
			uint value;
			mapping(uint => uint) table;
			function f(uint x) public returns (uint y) {
				value = x;
				if (x > 0) table[++value] = 8;
				if (x > 1) value--;
				if (x > 2) table[value]++;
				return --table[value++];
			}
		}
	)";
	compileAndRun(sourceCode);

	u256 value = 0;
	map<u256, u256> table;
	auto f = [&](u256 const& _x) -> u256
	{
		value = _x;
		if (_x > 0)
			table[++value] = 8;
		if (_x > 1)
			value --;
		if (_x > 2)
			table[value]++;
		return --table[value++];
	};
	testContractAgainstCppOnRange("f(uint256)", f, 0, 5);
}

BOOST_AUTO_TEST_CASE(multi_level_mapping)
{
	char const* sourceCode = R"(
		contract test {
			mapping(uint => mapping(uint => uint)) table;
			function f(uint x, uint y, uint z) public returns (uint w) {
				if (z == 0) return table[x][y];
				else return table[x][y] = z;
			}
		}
	)";
	compileAndRun(sourceCode);

	map<u256, map<u256, u256>> table;
	auto f = [&](u256 const& _x, u256 const& _y, u256 const& _z) -> u256
	{
		if (_z == 0) return table[_x][_y];
		else return table[_x][_y] = _z;
	};
	testContractAgainstCpp("f(uint256,uint256,uint256)", f, u256(4), u256(5), u256(0));
	testContractAgainstCpp("f(uint256,uint256,uint256)", f, u256(5), u256(4), u256(0));
	testContractAgainstCpp("f(uint256,uint256,uint256)", f, u256(4), u256(5), u256(9));
	testContractAgainstCpp("f(uint256,uint256,uint256)", f, u256(4), u256(5), u256(0));
	testContractAgainstCpp("f(uint256,uint256,uint256)", f, u256(5), u256(4), u256(0));
	testContractAgainstCpp("f(uint256,uint256,uint256)", f, u256(5), u256(4), u256(7));
	testContractAgainstCpp("f(uint256,uint256,uint256)", f, u256(4), u256(5), u256(0));
	testContractAgainstCpp("f(uint256,uint256,uint256)", f, u256(5), u256(4), u256(0));
}

BOOST_AUTO_TEST_CASE(mapping_local_assignment)
{
	char const* sourceCode = R"(
		contract test {
			mapping(uint8 => uint8) m1;
			mapping(uint8 => uint8) m2;
			function f() public returns (uint8, uint8, uint8, uint8) {
				mapping(uint8 => uint8) storage m = m1;
				m[1] = 42;

				m = m2;
				m[2] = 21;

				return (m1[1], m1[2], m2[1], m2[2]);
			}
		}
	)";
	compileAndRun(sourceCode);

	ABI_CHECK(callContractFunction("f()"), encodeArgs(uint8_t(42), uint8_t(0), uint8_t(0), uint8_t(21)));
}

BOOST_AUTO_TEST_CASE(mapping_local_tuple_assignment)
{
	char const* sourceCode = R"(
		contract test {
			mapping(uint8 => uint8) m1;
			mapping(uint8 => uint8) m2;
			function f() public returns (uint8, uint8, uint8, uint8) {
				mapping(uint8 => uint8) storage m = m1;
				m[1] = 42;

				uint8 v;
				(m, v) = (m2, 21);
				m[2] = v;

				return (m1[1], m1[2], m2[1], m2[2]);
			}
		}
	)";
	compileAndRun(sourceCode);

	ABI_CHECK(callContractFunction("f()"), encodeArgs(uint8_t(42), uint8_t(0), uint8_t(0), uint8_t(21)));
}

BOOST_AUTO_TEST_CASE(mapping_local_compound_assignment)
{
	char const* sourceCode = R"(
		contract test {
			mapping(uint8 => uint8) m1;
			mapping(uint8 => uint8) m2;
			function f() public returns (uint8, uint8, uint8, uint8) {
				mapping(uint8 => uint8) storage m = m1;
				m[1] = 42;

				(m = m2)[2] = 21;

				return (m1[1], m1[2], m2[1], m2[2]);
			}
		}
	)";
	compileAndRun(sourceCode);

	ABI_CHECK(callContractFunction("f()"), encodeArgs(uint8_t(42), uint8_t(0), uint8_t(0), uint8_t(21)));
}

BOOST_AUTO_TEST_CASE(mapping_internal_argument)
{
	char const* sourceCode = R"(
		contract test {
			mapping(uint8 => uint8) a;
			mapping(uint8 => uint8) b;
			function set_internal(mapping(uint8 => uint8) storage m, uint8 key, uint8 value) internal returns (uint8) {
				uint8 oldValue = m[key];
				m[key] = value;
				return oldValue;
			}
			function set(uint8 key, uint8 value_a, uint8 value_b) public returns (uint8 old_a, uint8 old_b) {
				old_a = set_internal(a, key, value_a);
				old_b = set_internal(b, key, value_b);
			}
			function get(uint8 key) public returns (uint8, uint8) {
				return (a[key], b[key]);
			}
		}
	)";
	compileAndRun(sourceCode);

	ABI_CHECK(callContractFunction("set(uint8,uint8,uint8)", uint8_t(1), uint8_t(21), uint8_t(42)), encodeArgs(uint8_t(0), uint8_t(0)));
	ABI_CHECK(callContractFunction("get(uint8)", uint8_t(1)), encodeArgs(uint8_t(21), uint8_t(42)));
	ABI_CHECK(callContractFunction("set(uint8,uint8,uint8)", uint8_t(1), uint8_t(10), uint8_t(11)), encodeArgs(uint8_t(21), uint8_t(42)));
	ABI_CHECK(callContractFunction("get(uint8)", uint8_t(1)), encodeArgs(uint8_t(10), uint8_t(11)));
}

BOOST_AUTO_TEST_CASE(mapping_array_internal_argument)
{
	char const* sourceCode = R"(
		contract test {
			mapping(uint8 => uint8)[2] a;
			mapping(uint8 => uint8)[2] b;
			function set_internal(mapping(uint8 => uint8)[2] storage m, uint8 key, uint8 value1, uint8 value2) internal returns (uint8, uint8) {
				uint8 oldValue1 = m[0][key];
				uint8 oldValue2 = m[1][key];
				m[0][key] = value1;
				m[1][key] = value2;
				return (oldValue1, oldValue2);
			}
			function set(uint8 key, uint8 value_a1, uint8 value_a2, uint8 value_b1, uint8 value_b2) public returns (uint8 old_a1, uint8 old_a2, uint8 old_b1, uint8 old_b2) {
				(old_a1, old_a2) = set_internal(a, key, value_a1, value_a2);
				(old_b1, old_b2) = set_internal(b, key, value_b1, value_b2);
			}
			function get(uint8 key) public returns (uint8, uint8, uint8, uint8) {
				return (a[0][key], a[1][key], b[0][key], b[1][key]);
			}
		}
	)";
	compileAndRun(sourceCode);

	ABI_CHECK(callContractFunction("set(uint8,uint8,uint8,uint8,uint8)", uint8_t(1), uint8_t(21), uint8_t(22), uint8_t(42), uint8_t(43)), encodeArgs(uint8_t(0), uint8_t(0), uint8_t(0), uint8_t(0)));
	ABI_CHECK(callContractFunction("get(uint8)", uint8_t(1)), encodeArgs(uint8_t(21), uint8_t(22), uint8_t(42), uint8_t(43)));
	ABI_CHECK(callContractFunction("set(uint8,uint8,uint8,uint8,uint8)", uint8_t(1), uint8_t(10), uint8_t(30), uint8_t(11), uint8_t(31)), encodeArgs(uint8_t(21), uint8_t(22), uint8_t(42), uint8_t(43)));
	ABI_CHECK(callContractFunction("get(uint8)", uint8_t(1)), encodeArgs(uint8_t(10), uint8_t(30), uint8_t(11), uint8_t(31)));
}

BOOST_AUTO_TEST_CASE(mapping_internal_return)
{
	char const* sourceCode = R"(
		contract test {
			mapping(uint8 => uint8) a;
			mapping(uint8 => uint8) b;
			function f() internal returns (mapping(uint8 => uint8) storage r) {
				r = a;
				r[1] = 42;
				r = b;
				r[1] = 84;
			}
			function g() public returns (uint8, uint8, uint8, uint8, uint8, uint8) {
				f()[2] = 21;
				return (a[0], a[1], a[2], b[0], b[1], b[2]);
			}
			function h() public returns (uint8, uint8, uint8, uint8, uint8, uint8) {
				mapping(uint8 => uint8) storage m = f();
				m[2] = 17;
				return (a[0], a[1], a[2], b[0], b[1], b[2]);
			}
		}
	)";
	compileAndRun(sourceCode);

	ABI_CHECK(callContractFunction("g()"), encodeArgs(uint8_t(0), uint8_t(42), uint8_t(0), uint8_t(0), uint8_t(84), uint8_t (21)));
	ABI_CHECK(callContractFunction("h()"), encodeArgs(uint8_t(0), uint8_t(42), uint8_t(0), uint8_t(0), uint8_t(84), uint8_t (17)));
}

BOOST_AUTO_TEST_CASE(structs)
{
	char const* sourceCode = R"(
		contract test {
			struct s1 {
				uint8 x;
				bool y;
			}
			struct s2 {
				uint32 z;
				s1 s1data;
				mapping(uint8 => s2) recursive;
			}
			s2 data;
			function check() public returns (bool ok) {
				return data.z == 1 && data.s1data.x == 2 &&
					data.s1data.y == true &&
					data.recursive[3].recursive[4].z == 5 &&
					data.recursive[4].recursive[3].z == 6 &&
					data.recursive[0].s1data.y == false &&
					data.recursive[4].z == 9;
			}
			function set() public {
				data.z = 1;
				data.s1data.x = 2;
				data.s1data.y = true;
				data.recursive[3].recursive[4].z = 5;
				data.recursive[4].recursive[3].z = 6;
				data.recursive[0].s1data.y = false;
				data.recursive[4].z = 9;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("check()"), encodeArgs(false));
	ABI_CHECK(callContractFunction("set()"), bytes());
	ABI_CHECK(callContractFunction("check()"), encodeArgs(true));
}

BOOST_AUTO_TEST_CASE(struct_reference)
{
	char const* sourceCode = R"(
		contract test {
			struct s2 {
				uint32 z;
				mapping(uint8 => s2) recursive;
			}
			s2 data;
			function check() public returns (bool ok) {
				return data.z == 2 &&
					data.recursive[0].z == 3 &&
					data.recursive[0].recursive[1].z == 0 &&
					data.recursive[0].recursive[0].z == 1;
			}
			function set() public {
				data.z = 2;
				mapping(uint8 => s2) storage map = data.recursive;
				s2 storage inner = map[0];
				inner.z = 3;
				inner.recursive[0].z = inner.recursive[1].z + 1;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("check()"), encodeArgs(false));
	ABI_CHECK(callContractFunction("set()"), bytes());
	ABI_CHECK(callContractFunction("check()"), encodeArgs(true));
}

BOOST_AUTO_TEST_CASE(deleteStruct)
{
	char const* sourceCode = R"(
		contract test {
			struct topStruct {
				nestedStruct nstr;
				uint topValue;
				mapping (uint => uint) topMapping;
			}
			uint toDelete;
			topStruct str;
			struct nestedStruct {
				uint nestedValue;
				mapping (uint => bool) nestedMapping;
			}
			constructor() public {
				toDelete = 5;
				str.topValue = 1;
				str.topMapping[0] = 1;
				str.topMapping[1] = 2;

				str.nstr.nestedValue = 2;
				str.nstr.nestedMapping[0] = true;
				str.nstr.nestedMapping[1] = false;
				delete str;
				delete toDelete;
			}
			function getToDelete() public returns (uint res){
				res = toDelete;
			}
			function getTopValue() public returns(uint topValue){
				topValue = str.topValue;
			}
			function getNestedValue() public returns(uint nestedValue){
				nestedValue = str.nstr.nestedValue;
			}
			function getTopMapping(uint index) public returns(uint ret) {
				ret = str.topMapping[index];
			}
			function getNestedMapping(uint index) public returns(bool ret) {
				return str.nstr.nestedMapping[index];
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("getToDelete()"), encodeArgs(0));
	ABI_CHECK(callContractFunction("getTopValue()"), encodeArgs(0));
	ABI_CHECK(callContractFunction("getNestedValue()"), encodeArgs(0));
	// mapping values should be the same
	ABI_CHECK(callContractFunction("getTopMapping(uint256)", 0), encodeArgs(1));
	ABI_CHECK(callContractFunction("getTopMapping(uint256)", 1), encodeArgs(2));
	ABI_CHECK(callContractFunction("getNestedMapping(uint256)", 0), encodeArgs(true));
	ABI_CHECK(callContractFunction("getNestedMapping(uint256)", 1), encodeArgs(false));
}

BOOST_AUTO_TEST_CASE(deleteLocal)
{
	char const* sourceCode = R"(
		contract test {
			function delLocal() public returns (uint res){
				uint v = 5;
				delete v;
				res = v;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("delLocal()"), encodeArgs(0));
}

BOOST_AUTO_TEST_CASE(deleteLocals)
{
	char const* sourceCode = R"(
		contract test {
			function delLocal() public returns (uint res1, uint res2){
				uint v = 5;
				uint w = 6;
				uint x = 7;
				delete v;
				res1 = w;
				res2 = x;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("delLocal()"), encodeArgs(6, 7));
}

BOOST_AUTO_TEST_CASE(deleteLength)
{
	char const* sourceCode = R"(
		contract test {
			uint[] x;
			function f() public returns (uint){
				x.length = 1;
				x[0] = 1;
				delete x.length;
				return x.length;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f()"), encodeArgs(0));
	BOOST_CHECK(storageEmpty(m_contractAddress));
}

BOOST_AUTO_TEST_CASE(constructor)
{
	char const* sourceCode = R"(
		contract test {
			mapping(uint => uint) data;
			constructor() public {
				data[7] = 8;
			}
			function get(uint key) public returns (uint value) {
				return data[key];
			}
		}
	)";
	compileAndRun(sourceCode);
	map<u256, uint8_t> data;
	data[7] = 8;
	auto get = [&](u256 const& _x) -> u256
	{
		return data[_x];
	};
	testContractAgainstCpp("get(uint256)", get, u256(6));
	testContractAgainstCpp("get(uint256)", get, u256(7));
}

BOOST_AUTO_TEST_CASE(simple_accessor)
{
	char const* sourceCode = R"(
		contract test {
			uint256 public data;
			constructor() public {
				data = 8;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("data()"), encodeArgs(8));
}

BOOST_AUTO_TEST_CASE(array_accessor)
{
	char const* sourceCode = R"(
		contract test {
			uint[8] public data;
			uint[] public dynamicData;
			uint24[] public smallTypeData;
			struct st { uint a; uint[] finalArray; }
			mapping(uint256 => mapping(uint256 => st[5])) public multiple_map;

			constructor() public {
				data[0] = 8;
				dynamicData.length = 3;
				dynamicData[2] = 8;
				smallTypeData.length = 128;
				smallTypeData[1] = 22;
				smallTypeData[127] = 2;
				multiple_map[2][1][2].a = 3;
				multiple_map[2][1][2].finalArray.length = 4;
				multiple_map[2][1][2].finalArray[3] = 5;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("data(uint256)", 0), encodeArgs(8));
	ABI_CHECK(callContractFunction("data(uint256)", 8), encodeArgs());
	ABI_CHECK(callContractFunction("dynamicData(uint256)", 2), encodeArgs(8));
	ABI_CHECK(callContractFunction("dynamicData(uint256)", 8), encodeArgs());
	ABI_CHECK(callContractFunction("smallTypeData(uint256)", 1), encodeArgs(22));
	ABI_CHECK(callContractFunction("smallTypeData(uint256)", 127), encodeArgs(2));
	ABI_CHECK(callContractFunction("smallTypeData(uint256)", 128), encodeArgs());
	ABI_CHECK(callContractFunction("multiple_map(uint256,uint256,uint256)", 2, 1, 2), encodeArgs(3));
}

BOOST_AUTO_TEST_CASE(accessors_mapping_for_array)
{
	char const* sourceCode = R"(
		contract test {
			mapping(uint => uint[8]) public data;
			mapping(uint => uint[]) public dynamicData;
			constructor() public {
				data[2][2] = 8;
				dynamicData[2].length = 3;
				dynamicData[2][2] = 8;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("data(uint256,uint256)", 2, 2), encodeArgs(8));
	ABI_CHECK(callContractFunction("data(uint256, 256)", 2, 8), encodeArgs());
	ABI_CHECK(callContractFunction("dynamicData(uint256,uint256)", 2, 2), encodeArgs(8));
	ABI_CHECK(callContractFunction("dynamicData(uint256,uint256)", 2, 8), encodeArgs());
}

BOOST_AUTO_TEST_CASE(multiple_elementary_accessors)
{
	char const* sourceCode = R"(
		contract test {
			uint256 public data;
			bytes6 public name;
			bytes32 public a_hash;
			address public an_address;
			constructor() public {
				data = 8;
				name = "Celina";
				a_hash = keccak256("\x7b");
				an_address = address(0x1337);
				super_secret_data = 42;
			}
			uint256 super_secret_data;
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("data()"), encodeArgs(8));
	ABI_CHECK(callContractFunction("name()"), encodeArgs("Celina"));
	ABI_CHECK(callContractFunction("a_hash()"), encodeArgs(dev::keccak256(bytes(1, 0x7b))));
	ABI_CHECK(callContractFunction("an_address()"), encodeArgs(toBigEndian(u160(0x1337))));
	ABI_CHECK(callContractFunction("super_secret_data()"), bytes());
}

BOOST_AUTO_TEST_CASE(complex_accessors)
{
	char const* sourceCode = R"(
		contract test {
			mapping(uint256 => bytes4) public to_string_map;
			mapping(uint256 => bool) public to_bool_map;
			mapping(uint256 => uint256) public to_uint_map;
			mapping(uint256 => mapping(uint256 => uint256)) public to_multiple_map;
			constructor() public {
				to_string_map[42] = "24";
				to_bool_map[42] = false;
				to_uint_map[42] = 12;
				to_multiple_map[42][23] = 31;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("to_string_map(uint256)", 42), encodeArgs("24"));
	ABI_CHECK(callContractFunction("to_bool_map(uint256)", 42), encodeArgs(false));
	ABI_CHECK(callContractFunction("to_uint_map(uint256)", 42), encodeArgs(12));
	ABI_CHECK(callContractFunction("to_multiple_map(uint256,uint256)", 42, 23), encodeArgs(31));
}

BOOST_AUTO_TEST_CASE(struct_accessor)
{
	char const* sourceCode = R"(
		contract test {
			struct Data { uint a; uint8 b; mapping(uint => uint) c; bool d; }
			mapping(uint => Data) public data;
			constructor() public {
				data[7].a = 1;
				data[7].b = 2;
				data[7].c[0] = 3;
				data[7].d = true;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("data(uint256)", 7), encodeArgs(1, 2, true));
}

BOOST_AUTO_TEST_CASE(balance)
{
	char const* sourceCode = R"(
		contract test {
			constructor() public payable {}
			function getBalance() public returns (uint256 balance) {
				return address(this).balance;
			}
		}
	)";
	compileAndRun(sourceCode, 23);
	ABI_CHECK(callContractFunction("getBalance()"), encodeArgs(23));
}

BOOST_AUTO_TEST_CASE(blockchain)
{
	char const* sourceCode = R"(
		contract test {
			constructor() public payable {}
			function someInfo() public payable returns (uint256 value, address coinbase, uint256 blockNumber) {
				value = msg.value;
				coinbase = block.coinbase;
				blockNumber = block.number;
			}
		}
	)";
	BOOST_CHECK(m_rpc.rpcCall("miner_setEtherbase", {"\"0x1212121212121212121212121212121212121212\""}).asBool() == true);
	m_rpc.test_mineBlocks(5);
	compileAndRun(sourceCode, 27);
	ABI_CHECK(callContractFunctionWithValue("someInfo()", 28), encodeArgs(28, u256("0x1212121212121212121212121212121212121212"), 7));
}

BOOST_AUTO_TEST_CASE(msg_sig)
{
	char const* sourceCode = R"(
		contract test {
			function foo(uint256 a) public returns (bytes4 value) {
				return msg.sig;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("foo(uint256)", 0), encodeArgs(asString(FixedHash<4>(dev::keccak256("foo(uint256)")).asBytes())));
}

BOOST_AUTO_TEST_CASE(msg_sig_after_internal_call_is_same)
{
	char const* sourceCode = R"(
		contract test {
			function boo() public returns (bytes4 value) {
				return msg.sig;
			}
			function foo(uint256 a) public returns (bytes4 value) {
				return boo();
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("foo(uint256)", 0), encodeArgs(asString(FixedHash<4>(dev::keccak256("foo(uint256)")).asBytes())));
}

BOOST_AUTO_TEST_CASE(now)
{
	char const* sourceCode = R"(
		contract test {
			function someInfo() public returns (bool equal, uint val) {
				equal = block.timestamp == now;
				val = now;
			}
		}
	)";
	compileAndRun(sourceCode);
	u256 startBlock = m_blockNumber;
	size_t startTime = blockTimestamp(startBlock);
	auto ret = callContractFunction("someInfo()");
	u256 endBlock = m_blockNumber;
	size_t endTime = blockTimestamp(endBlock);
	BOOST_CHECK(startBlock != endBlock);
	BOOST_CHECK(startTime != endTime);
	ABI_CHECK(ret, encodeArgs(true, endTime));
}

BOOST_AUTO_TEST_CASE(type_conversions_cleanup)
{
	// 22-byte integer converted to a contract (i.e. address, 20 bytes), converted to a 32 byte
	// integer should drop the first two bytes
	char const* sourceCode = R"(
		contract Test {
			function test() public returns (uint ret) { return uint(address(Test(address(0x11223344556677889900112233445566778899001122)))); }
		}
	)";
	compileAndRun(sourceCode);
	BOOST_REQUIRE(callContractFunction("test()") == bytes({0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
														   0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00, 0x11, 0x22,
														   0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00, 0x11, 0x22}));
}
// fixed bytes to fixed bytes conversion tests
BOOST_AUTO_TEST_CASE(convert_fixed_bytes_to_fixed_bytes_smaller_size)
{
	char const* sourceCode = R"(
		contract Test {
			function bytesToBytes(bytes4 input) public returns (bytes2 ret) {
				return bytes2(input);
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("bytesToBytes(bytes4)", "abcd"), encodeArgs("ab"));
}

BOOST_AUTO_TEST_CASE(convert_fixed_bytes_to_fixed_bytes_greater_size)
{
	char const* sourceCode = R"(
		contract Test {
			function bytesToBytes(bytes2 input) public returns (bytes4 ret) {
				return bytes4(input);
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("bytesToBytes(bytes2)", "ab"), encodeArgs("ab"));
}

BOOST_AUTO_TEST_CASE(convert_fixed_bytes_to_fixed_bytes_same_size)
{
	char const* sourceCode = R"(
		contract Test {
			function bytesToBytes(bytes4 input) public returns (bytes4 ret) {
				return bytes4(input);
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("bytesToBytes(bytes4)", "abcd"), encodeArgs("abcd"));
}

// fixed bytes to uint conversion tests
BOOST_AUTO_TEST_CASE(convert_fixed_bytes_to_uint_same_size)
{
	char const* sourceCode = R"(
		contract Test {
			function bytesToUint(bytes32 s) public returns (uint256 h) {
				return uint(s);
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(
		callContractFunction("bytesToUint(bytes32)", string("abc2")),
		encodeArgs(u256("0x6162633200000000000000000000000000000000000000000000000000000000"))
	);
}

BOOST_AUTO_TEST_CASE(convert_fixed_bytes_to_uint_same_min_size)
{
	char const* sourceCode = R"(
		contract Test {
			function bytesToUint(bytes1 s) public returns (uint8 h) {
				return uint8(s);
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(
		callContractFunction("bytesToUint(bytes1)", string("a")),
		encodeArgs(u256("0x61"))
	);
}

BOOST_AUTO_TEST_CASE(convert_fixed_bytes_to_uint_smaller_size)
{
	char const* sourceCode = R"(
		contract Test {
			function bytesToUint(bytes4 s) public returns (uint16 h) {
				return uint16(uint32(s));
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(
		callContractFunction("bytesToUint(bytes4)", string("abcd")),
		encodeArgs(u256("0x6364"))
	);
}

BOOST_AUTO_TEST_CASE(convert_fixed_bytes_to_uint_greater_size)
{
	char const* sourceCode = R"(
		contract Test {
			function bytesToUint(bytes4 s) public returns (uint64 h) {
				return uint64(uint32(s));
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(
		callContractFunction("bytesToUint(bytes4)", string("abcd")),
		encodeArgs(u256("0x61626364"))
	);
}

// uint fixed bytes conversion tests
BOOST_AUTO_TEST_CASE(convert_uint_to_fixed_bytes_same_size)
{
	char const* sourceCode = R"(
		contract Test {
			function uintToBytes(uint256 h) public returns (bytes32 s) {
				return bytes32(h);
			}
		}
	)";
	compileAndRun(sourceCode);
	u256 a("0x6162630000000000000000000000000000000000000000000000000000000000");
	ABI_CHECK(callContractFunction("uintToBytes(uint256)", a), encodeArgs(a));
}

BOOST_AUTO_TEST_CASE(convert_uint_to_fixed_bytes_same_min_size)
{
	char const* sourceCode = R"(
		contract Test {
			function UintToBytes(uint8 h) public returns (bytes1 s) {
				return bytes1(h);
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(
		callContractFunction("UintToBytes(uint8)", u256("0x61")),
		encodeArgs(string("a"))
	);
}

BOOST_AUTO_TEST_CASE(convert_uint_to_fixed_bytes_smaller_size)
{
	char const* sourceCode = R"(
		contract Test {
			function uintToBytes(uint32 h) public returns (bytes2 s) {
				return bytes2(uint16(h));
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(
		callContractFunction("uintToBytes(uint32)", u160("0x61626364")),
		encodeArgs(string("cd"))
	);
}

BOOST_AUTO_TEST_CASE(convert_uint_to_fixed_bytes_greater_size)
{
	char const* sourceCode = R"(
		contract Test {
			function UintToBytes(uint16 h) public returns (bytes8 s) {
				return bytes8(uint64(h));
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(
		callContractFunction("UintToBytes(uint16)", u256("0x6162")),
		encodeArgs(string("\0\0\0\0\0\0ab", 8))
	);
}

BOOST_AUTO_TEST_CASE(send_ether)
{
	char const* sourceCode = R"(
		contract test {
			constructor() payable public {}
			function a(address payable addr, uint amount) public returns (uint ret) {
				addr.send(amount);
				return address(this).balance;
			}
		}
	)";
	u256 amount(130);
	compileAndRun(sourceCode, amount + 1);
	u160 address(23);
	ABI_CHECK(callContractFunction("a(address,uint256)", address, amount), encodeArgs(1));
	BOOST_CHECK_EQUAL(balanceAt(address), amount);
}

BOOST_AUTO_TEST_CASE(transfer_ether)
{
	char const* sourceCode = R"(
		contract A {
			constructor() public payable {}
			function a(address payable addr, uint amount) public returns (uint) {
				addr.transfer(amount);
				return address(this).balance;
			}
			function b(address payable addr, uint amount) public {
				addr.transfer(amount);
			}
		}

		contract B {
		}

		contract C {
			function () external payable {
				revert();
			}
		}
	)";
	compileAndRun(sourceCode, 0, "B");
	u160 const nonPayableRecipient = m_contractAddress;
	compileAndRun(sourceCode, 0, "C");
	u160 const oogRecipient = m_contractAddress;
	compileAndRun(sourceCode, 20, "A");
	u160 payableRecipient(23);
	ABI_CHECK(callContractFunction("a(address,uint256)", payableRecipient, 10), encodeArgs(10));
	BOOST_CHECK_EQUAL(balanceAt(payableRecipient), 10);
	BOOST_CHECK_EQUAL(balanceAt(m_contractAddress), 10);
	ABI_CHECK(callContractFunction("b(address,uint256)", nonPayableRecipient, 10), encodeArgs());
	ABI_CHECK(callContractFunction("b(address,uint256)", oogRecipient, 10), encodeArgs());
}

BOOST_AUTO_TEST_CASE(uncalled_blockhash)
{
	char const* code = R"(
		contract C {
			function f() public view returns (bytes32)
			{
				return (blockhash)(block.number - 1);
			}
		}
	)";
	compileAndRun(code, 0, "C");
	bytes result = callContractFunction("f()");
	BOOST_REQUIRE_EQUAL(result.size(), 32);
	BOOST_CHECK(result[0] != 0 || result[1] != 0 || result[2] != 0);
}

BOOST_AUTO_TEST_CASE(blockhash_shadow_resolution)
{
	char const* code = R"(
		contract C {
			function blockhash(uint256 blockNumber) public returns(bytes32) { bytes32 x; return x; }
			function f() public returns(bytes32) { return blockhash(3); }
		}
	)";
	compileAndRun(code, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(0));
}

BOOST_AUTO_TEST_CASE(log0)
{
	char const* sourceCode = R"(
		contract test {
			function a() public {
				log0(bytes32(uint256(1)));
			}
		}
	)";
	compileAndRun(sourceCode);
	callContractFunction("a()");
	BOOST_REQUIRE_EQUAL(m_logs.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].address, m_contractAddress);
	BOOST_CHECK_EQUAL(h256(m_logs[0].data), h256(u256(1)));
	BOOST_CHECK_EQUAL(m_logs[0].topics.size(), 0);
}

BOOST_AUTO_TEST_CASE(log1)
{
	char const* sourceCode = R"(
		contract test {
			function a() public {
				log1(bytes32(uint256(1)), bytes32(uint256(2)));
			}
		}
	)";
	compileAndRun(sourceCode);
	callContractFunction("a()");
	BOOST_REQUIRE_EQUAL(m_logs.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].address, m_contractAddress);
	BOOST_CHECK_EQUAL(h256(m_logs[0].data), h256(u256(1)));
	BOOST_REQUIRE_EQUAL(m_logs[0].topics.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].topics[0], h256(u256(2)));
}

BOOST_AUTO_TEST_CASE(log2)
{
	char const* sourceCode = R"(
		contract test {
			function a() public {
				log2(bytes32(uint256(1)), bytes32(uint256(2)), bytes32(uint256(3)));
			}
		}
	)";
	compileAndRun(sourceCode);
	callContractFunction("a()");
	BOOST_REQUIRE_EQUAL(m_logs.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].address, m_contractAddress);
	BOOST_CHECK_EQUAL(h256(m_logs[0].data), h256(u256(1)));
	BOOST_REQUIRE_EQUAL(m_logs[0].topics.size(), 2);
	for (unsigned i = 0; i < 2; ++i)
		BOOST_CHECK_EQUAL(m_logs[0].topics[i], h256(u256(i + 2)));
}

BOOST_AUTO_TEST_CASE(log3)
{
	char const* sourceCode = R"(
		contract test {
			function a() public {
				log3(bytes32(uint256(1)), bytes32(uint256(2)), bytes32(uint256(3)), bytes32(uint256(4)));
			}
		}
	)";
	compileAndRun(sourceCode);
	callContractFunction("a()");
	BOOST_REQUIRE_EQUAL(m_logs.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].address, m_contractAddress);
	BOOST_CHECK_EQUAL(h256(m_logs[0].data), h256(u256(1)));
	BOOST_REQUIRE_EQUAL(m_logs[0].topics.size(), 3);
	for (unsigned i = 0; i < 3; ++i)
		BOOST_CHECK_EQUAL(m_logs[0].topics[i], h256(u256(i + 2)));
}

BOOST_AUTO_TEST_CASE(log4)
{
	char const* sourceCode = R"(
		contract test {
			function a() public {
				log4(bytes32(uint256(1)), bytes32(uint256(2)), bytes32(uint256(3)), bytes32(uint256(4)), bytes32(uint256(5)));
			}
		}
	)";
	compileAndRun(sourceCode);
	callContractFunction("a()");
	BOOST_REQUIRE_EQUAL(m_logs.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].address, m_contractAddress);
	BOOST_CHECK_EQUAL(h256(m_logs[0].data), h256(u256(1)));
	BOOST_REQUIRE_EQUAL(m_logs[0].topics.size(), 4);
	for (unsigned i = 0; i < 4; ++i)
		BOOST_CHECK_EQUAL(m_logs[0].topics[i], h256(u256(i + 2)));
}

BOOST_AUTO_TEST_CASE(log_in_constructor)
{
	char const* sourceCode = R"(
		contract test {
			constructor() public {
				log1(bytes32(uint256(1)), bytes32(uint256(2)));
			}
		}
	)";
	compileAndRun(sourceCode);
	BOOST_REQUIRE_EQUAL(m_logs.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].address, m_contractAddress);
	BOOST_CHECK_EQUAL(h256(m_logs[0].data), h256(u256(1)));
	BOOST_REQUIRE_EQUAL(m_logs[0].topics.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].topics[0], h256(u256(2)));
}

BOOST_AUTO_TEST_CASE(selfdestruct)
{
	char const* sourceCode = R"(
		contract test {
			constructor() public payable {}
			function a(address payable receiver) public returns (uint ret) {
				selfdestruct(receiver);
				return 10;
			}
		}
	)";
	u256 amount(130);
	compileAndRun(sourceCode, amount);
	u160 address(23);
	ABI_CHECK(callContractFunction("a(address)", address), bytes());
	BOOST_CHECK(!addressHasCode(m_contractAddress));
	BOOST_CHECK_EQUAL(balanceAt(address), amount);
}

BOOST_AUTO_TEST_CASE(keccak256)
{
	char const* sourceCode = R"(
		contract test {
			function a(bytes32 input) public returns (bytes32 hash) {
				return keccak256(abi.encodePacked(input));
			}
		}
	)";
	compileAndRun(sourceCode);
	auto f = [&](u256 const& _x) -> u256
	{
		return dev::keccak256(toBigEndian(_x));
	};
	testContractAgainstCpp("a(bytes32)", f, u256(4));
	testContractAgainstCpp("a(bytes32)", f, u256(5));
	testContractAgainstCpp("a(bytes32)", f, u256(-1));
}

BOOST_AUTO_TEST_CASE(sha256)
{
	char const* sourceCode = R"(
		contract test {
			function a(bytes32 input) public returns (bytes32 sha256hash) {
				return sha256(abi.encodePacked(input));
			}
		}
	)";
	compileAndRun(sourceCode);
	auto f = [&](u256 const& _x) -> bytes
	{
		if (_x == u256(4))
			return fromHex("e38990d0c7fc009880a9c07c23842e886c6bbdc964ce6bdd5817ad357335ee6f");
		if (_x == u256(5))
			return fromHex("96de8fc8c256fa1e1556d41af431cace7dca68707c78dd88c3acab8b17164c47");
		if (_x == u256(-1))
			return fromHex("af9613760f72635fbdb44a5a0a63c39f12af30f950a6ee5c971be188e89c4051");
		return fromHex("");
	};
	testContractAgainstCpp("a(bytes32)", f, u256(4));
	testContractAgainstCpp("a(bytes32)", f, u256(5));
	testContractAgainstCpp("a(bytes32)", f, u256(-1));
}

BOOST_AUTO_TEST_CASE(ripemd)
{
	char const* sourceCode = R"(
		contract test {
			function a(bytes32 input) public returns (bytes32 sha256hash) {
				return ripemd160(abi.encodePacked(input));
			}
		}
	)";
	compileAndRun(sourceCode);
	auto f = [&](u256 const& _x) -> bytes
	{
		if (_x == u256(4))
			return fromHex("1b0f3c404d12075c68c938f9f60ebea4f74941a0000000000000000000000000");
		if (_x == u256(5))
			return fromHex("ee54aa84fc32d8fed5a5fe160442ae84626829d9000000000000000000000000");
		if (_x == u256(-1))
			return fromHex("1cf4e77f5966e13e109703cd8a0df7ceda7f3dc3000000000000000000000000");
		return fromHex("");
	};
	testContractAgainstCpp("a(bytes32)", f, u256(4));
	testContractAgainstCpp("a(bytes32)", f, u256(5));
	testContractAgainstCpp("a(bytes32)", f, u256(-1));
}

BOOST_AUTO_TEST_CASE(packed_keccak256)
{
	char const* sourceCode = R"(
		contract test {
			function a(bytes32 input) public returns (bytes32 hash) {
				uint24 b = 65536;
				uint c = 256;
				return keccak256(abi.encodePacked(uint8(8), input, b, input, c));
			}
		}
	)";
	compileAndRun(sourceCode);
	auto f = [&](u256 const& _x) -> u256
	{
		return dev::keccak256(
			toCompactBigEndian(unsigned(8)) +
			toBigEndian(_x) +
			toCompactBigEndian(unsigned(65536)) +
			toBigEndian(_x) +
			toBigEndian(u256(256))
		);
	};
	testContractAgainstCpp("a(bytes32)", f, u256(4));
	testContractAgainstCpp("a(bytes32)", f, u256(5));
	testContractAgainstCpp("a(bytes32)", f, u256(-1));
}

BOOST_AUTO_TEST_CASE(packed_keccak256_complex_types)
{
	char const* sourceCode = R"(
		contract test {
			uint120[3] x;
			function f() public returns (bytes32 hash1, bytes32 hash2, bytes32 hash3) {
				uint120[] memory y = new uint120[](3);
				x[0] = y[0] = uint120(-2);
				x[1] = y[1] = uint120(-3);
				x[2] = y[2] = uint120(-4);
				hash1 = keccak256(abi.encodePacked(x));
				hash2 = keccak256(abi.encodePacked(y));
				hash3 = keccak256(abi.encodePacked(this.f));
			}
		}
	)";
	compileAndRun(sourceCode);
	// Strangely, arrays are encoded with intra-element padding.
	ABI_CHECK(callContractFunction("f()"), encodeArgs(
		dev::keccak256(encodeArgs(u256("0xfffffffffffffffffffffffffffffe"), u256("0xfffffffffffffffffffffffffffffd"), u256("0xfffffffffffffffffffffffffffffc"))),
		dev::keccak256(encodeArgs(u256("0xfffffffffffffffffffffffffffffe"), u256("0xfffffffffffffffffffffffffffffd"), u256("0xfffffffffffffffffffffffffffffc"))),
		dev::keccak256(fromHex(m_contractAddress.hex() + "26121ff0"))
	));
}

BOOST_AUTO_TEST_CASE(packed_sha256)
{
	char const* sourceCode = R"(
		contract test {
			function a(bytes32 input) public returns (bytes32 hash) {
				uint24 b = 65536;
				uint c = 256;
				return sha256(abi.encodePacked(uint8(8), input, b, input, c));
			}
		}
	)";
	compileAndRun(sourceCode);
	auto f = [&](u256 const& _x) -> bytes
	{
		if (_x == u256(4))
			return fromHex("804e0d7003cfd70fc925dc103174d9f898ebb142ecc2a286da1abd22ac2ce3ac");
		if (_x == u256(5))
			return fromHex("e94921945f9068726c529a290a954f412bcac53184bb41224208a31edbf63cf0");
		if (_x == u256(-1))
			return fromHex("f14def4d07cd185ddd8b10a81b2238326196a38867e6e6adbcc956dc913488c7");
		return fromHex("");
	};
	testContractAgainstCpp("a(bytes32)", f, u256(4));
	testContractAgainstCpp("a(bytes32)", f, u256(5));
	testContractAgainstCpp("a(bytes32)", f, u256(-1));
}

BOOST_AUTO_TEST_CASE(packed_ripemd160)
{
	char const* sourceCode = R"(
		contract test {
			function a(bytes32 input) public returns (bytes32 hash) {
				uint24 b = 65536;
				uint c = 256;
				return ripemd160(abi.encodePacked(uint8(8), input, b, input, c));
			}
		}
	)";
	compileAndRun(sourceCode);
	auto f = [&](u256 const& _x) -> bytes
	{
		if (_x == u256(4))
			return fromHex("f93175303eba2a7b372174fc9330237f5ad202fc000000000000000000000000");
		if (_x == u256(5))
			return fromHex("04f4fc112e2bfbe0d38f896a46629e08e2fcfad5000000000000000000000000");
		if (_x == u256(-1))
			return fromHex("c0a2e4b1f3ff766a9a0089e7a410391730872495000000000000000000000000");
		return fromHex("");
	};
	testContractAgainstCpp("a(bytes32)", f, u256(4));
	testContractAgainstCpp("a(bytes32)", f, u256(5));
	testContractAgainstCpp("a(bytes32)", f, u256(-1));
}

BOOST_AUTO_TEST_CASE(ecrecover)
{
	char const* sourceCode = R"(
		contract test {
			function a(bytes32 h, uint8 v, bytes32 r, bytes32 s) public returns (address addr) {
				return ecrecover(h, v, r, s);
			}
		}
	)";
	compileAndRun(sourceCode);
	u256 h("0x18c547e4f7b0f325ad1e56f57e26c745b09a3e503d86e00e5255ff7f715d3d1c");
	uint8_t v = 28;
	u256 r("0x73b1693892219d736caba55bdb67216e485557ea6b6af75f37096c9aa6a5a75f");
	u256 s("0xeeb940b1d03b21e36b0e47e79769f095fe2ab855bd91e3a38756b7d75a9c4549");
	u160 addr("0xa94f5374fce5edbc8e2a8697c15331677e6ebf0b");
	ABI_CHECK(callContractFunction("a(bytes32,uint8,bytes32,bytes32)", h, v, r, s), encodeArgs(addr));
}

BOOST_AUTO_TEST_CASE(inter_contract_calls)
{
	char const* sourceCode = R"(
		contract Helper {
			function multiply(uint a, uint b) public returns (uint c) {
				return a * b;
			}
		}
		contract Main {
			Helper h;
			function callHelper(uint a, uint b) public returns (uint c) {
				return h.multiply(a, b);
			}
			function getHelper() public returns (address haddress) {
				return address(h);
			}
			function setHelper(address haddress) public {
				h = Helper(haddress);
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Helper");
	u160 const c_helperAddress = m_contractAddress;
	compileAndRun(sourceCode, 0, "Main");
	BOOST_REQUIRE(callContractFunction("setHelper(address)", c_helperAddress) == bytes());
	BOOST_REQUIRE(callContractFunction("getHelper()", c_helperAddress) == encodeArgs(c_helperAddress));
	u256 a(3456789);
	u256 b("0x282837623374623234aa74");
	BOOST_REQUIRE(callContractFunction("callHelper(uint256,uint256)", a, b) == encodeArgs(a * b));
}

BOOST_AUTO_TEST_CASE(inter_contract_calls_with_complex_parameters)
{
	char const* sourceCode = R"(
		contract Helper {
			function sel(uint a, bool select, uint b) public returns (uint c) {
				if (select) return a; else return b;
			}
		}
		contract Main {
			Helper h;
			function callHelper(uint a, bool select, uint b) public returns (uint c) {
				return h.sel(a, select, b) * 3;
			}
			function getHelper() public returns (address haddress) {
				return address(h);
			}
			function setHelper(address haddress) public {
				h = Helper(haddress);
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Helper");
	u160 const c_helperAddress = m_contractAddress;
	compileAndRun(sourceCode, 0, "Main");
	BOOST_REQUIRE(callContractFunction("setHelper(address)", c_helperAddress) == bytes());
	BOOST_REQUIRE(callContractFunction("getHelper()", c_helperAddress) == encodeArgs(c_helperAddress));
	u256 a(3456789);
	u256 b("0x282837623374623234aa74");
	BOOST_REQUIRE(callContractFunction("callHelper(uint256,bool,uint256)", a, true, b) == encodeArgs(a * 3));
	BOOST_REQUIRE(callContractFunction("callHelper(uint256,bool,uint256)", a, false, b) == encodeArgs(b * 3));
}

BOOST_AUTO_TEST_CASE(inter_contract_calls_accessing_this)
{
	char const* sourceCode = R"(
		contract Helper {
			function getAddress() public returns (address addr) {
				return address(this);
			}
		}
		contract Main {
			Helper h;
			function callHelper() public returns (address addr) {
				return h.getAddress();
			}
			function getHelper() public returns (address addr) {
				return address(h);
			}
			function setHelper(address addr) public {
				h = Helper(addr);
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Helper");
	u160 const c_helperAddress = m_contractAddress;
	compileAndRun(sourceCode, 0, "Main");
	BOOST_REQUIRE(callContractFunction("setHelper(address)", c_helperAddress) == bytes());
	BOOST_REQUIRE(callContractFunction("getHelper()", c_helperAddress) == encodeArgs(c_helperAddress));
	BOOST_REQUIRE(callContractFunction("callHelper()") == encodeArgs(c_helperAddress));
}

BOOST_AUTO_TEST_CASE(calls_to_this)
{
	char const* sourceCode = R"(
		contract Helper {
			function invoke(uint a, uint b) public returns (uint c) {
				return this.multiply(a, b, 10);
			}
			function multiply(uint a, uint b, uint8 c) public returns (uint ret) {
				return a * b + c;
			}
		}
		contract Main {
			Helper h;
			function callHelper(uint a, uint b) public returns (uint ret) {
				return h.invoke(a, b);
			}
			function getHelper() public returns (address addr) {
				return address(h);
			}
			function setHelper(address addr) public {
				h = Helper(addr);
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Helper");
	u160 const c_helperAddress = m_contractAddress;
	compileAndRun(sourceCode, 0, "Main");
	BOOST_REQUIRE(callContractFunction("setHelper(address)", c_helperAddress) == bytes());
	BOOST_REQUIRE(callContractFunction("getHelper()", c_helperAddress) == encodeArgs(c_helperAddress));
	u256 a(3456789);
	u256 b("0x282837623374623234aa74");
	BOOST_REQUIRE(callContractFunction("callHelper(uint256,uint256)", a, b) == encodeArgs(a * b + 10));
}

BOOST_AUTO_TEST_CASE(inter_contract_calls_with_local_vars)
{
	// note that a reference to another contract's function occupies two stack slots,
	// so this tests correct stack slot allocation
	char const* sourceCode = R"(
		contract Helper {
			function multiply(uint a, uint b) public returns (uint c) {
				return a * b;
			}
		}
		contract Main {
			Helper h;
			function callHelper(uint a, uint b) public returns (uint c) {
				uint8 y = 9;
				uint256 ret = h.multiply(a, b);
				return ret + y;
			}
			function getHelper() public returns (address haddress) {
				return address(h);
			}
			function setHelper(address haddress) public {
				h = Helper(haddress);
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Helper");
	u160 const c_helperAddress = m_contractAddress;
	compileAndRun(sourceCode, 0, "Main");
	BOOST_REQUIRE(callContractFunction("setHelper(address)", c_helperAddress) == bytes());
	BOOST_REQUIRE(callContractFunction("getHelper()", c_helperAddress) == encodeArgs(c_helperAddress));
	u256 a(3456789);
	u256 b("0x282837623374623234aa74");
	BOOST_REQUIRE(callContractFunction("callHelper(uint256,uint256)", a, b) == encodeArgs(a * b + 9));
}

BOOST_AUTO_TEST_CASE(fixed_bytes_in_calls)
{
	char const* sourceCode = R"(
		contract Helper {
			function invoke(bytes3 x, bool stop) public returns (bytes4 ret) {
				return x;
			}
		}
		contract Main {
			Helper h;
			function callHelper(bytes2 x, bool stop) public returns (bytes5 ret) {
				return h.invoke(x, stop);
			}
			function getHelper() public returns (address addr) {
				return address(h);
			}
			function setHelper(address addr) public {
				h = Helper(addr);
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Helper");
	u160 const c_helperAddress = m_contractAddress;
	compileAndRun(sourceCode, 0, "Main");
	BOOST_REQUIRE(callContractFunction("setHelper(address)", c_helperAddress) == bytes());
	BOOST_REQUIRE(callContractFunction("getHelper()", c_helperAddress) == encodeArgs(c_helperAddress));
	ABI_CHECK(callContractFunction("callHelper(bytes2,bool)", string("\0a", 2), true), encodeArgs(string("\0a\0\0\0", 5)));
}

BOOST_AUTO_TEST_CASE(constructor_arguments_internal)
{
	char const* sourceCode = R"(
		contract Helper {
			bytes3 name;
			bool flag;

			constructor(bytes3 x, bool f) public {
				name = x;
				flag = f;
			}
			function getName() public returns (bytes3 ret) { return name; }
			function getFlag() public returns (bool ret) { return flag; }
		}
		contract Main {
			Helper h;
			constructor() public {
				h = new Helper("abc", true);
			}
			function getFlag() public returns (bool ret) { return h.getFlag(); }
			function getName() public returns (bytes3 ret) { return h.getName(); }
		}
	)";
	compileAndRun(sourceCode, 0, "Main");
	ABI_CHECK(callContractFunction("getFlag()"), encodeArgs(true));
	ABI_CHECK(callContractFunction("getName()"), encodeArgs("abc"));
}

BOOST_AUTO_TEST_CASE(constructor_arguments_external)
{
	char const* sourceCode = R"(
		contract Main {
			bytes3 name;
			bool flag;

			constructor(bytes3 x, bool f) public {
				name = x;
				flag = f;
			}
			function getName() public returns (bytes3 ret) { return name; }
			function getFlag() public returns (bool ret) { return flag; }
		}
	)";
	compileAndRun(sourceCode, 0, "Main", encodeArgs("abc", true));
	ABI_CHECK(callContractFunction("getFlag()"), encodeArgs(true));
	ABI_CHECK(callContractFunction("getName()"), encodeArgs("abc"));
}

BOOST_AUTO_TEST_CASE(constructor_with_long_arguments)
{
	char const* sourceCode = R"(
		contract Main {
			string public a;
			string public b;

			constructor(string memory _a, string memory _b) public {
				a = _a;
				b = _b;
			}
		}
	)";
	string a = "01234567890123gabddunaouhdaoneudapcgadi4567890789012oneudapcgadi4567890789012oneudapcgadi4567890789012oneudapcgadi4567890789012oneudapcgadi4567890789012oneudapcgadi4567890789012oneudapcgadi4567890789012oneudapcgadi45678907890123456789abcd123456787890123456789abcd90123456789012345678901234567890123456789aboneudapcgadi4567890789012oneudapcgadi4567890789012oneudapcgadi45678907890123456789abcd123456787890123456789abcd90123456789012345678901234567890123456789aboneudapcgadi4567890789012oneudapcgadi4567890789012oneudapcgadi45678907890123456789abcd123456787890123456789abcd90123456789012345678901234567890123456789aboneudapcgadi4567890789012cdef";
	string b = "AUTAHIACIANOTUHAOCUHAOEUNAOEHUNTHDYDHPYDRCPYDRSTITOEUBXHUDGO>PYAUTAHIACIANOTUHAOCUHAOEUNAOEHUNTHDYDHPYDRCPYDRSTITOEUBXHUDGO>PYAUTAHIACIANOTUHAOCUHAOEUNAOEHUNTHDYDHPYDRCPYDRSTITOEUBXHUDGO>PYAUTAHIACIANOTUHAOCUHAOEUNAOEHUNTHDYDHPYDRCPYDRSTITOEUBXHUDGO>PYAUTAHIACIANOTUHAOCUHAOEUNAOEHUNTHDYDHPYDRCPYDRSTITOEUBXHUDGO>PYAUTAHIACIANOTUHAOCUHAOEUNAOEHUNTHDYDHPYDRCPYDRSTITOEUBXHUDGO>PYAUTAHIACIANOTUHAOCUHAOEUNAOEHUNTHDYDHPYDRCPYDRSTITOEUBXHUDGO>PYAUTAHIACIANOTUHAOCUHAOEUNAOEHUNTHDYDHPYDRCPYDRSTITOEUBXHUDGO>PYAUTAHIACIANOTUHAOCUHAOEUNAOEHUNTHDYDHPYDRCPYDRSTITOEUBXHUDGO>PYAUTAHIACIANOTUHAOCUHAOEUNAOEHUNTHDYDHPYDRCPYDRSTITOEUBXHUDGO>PYAUTAHIACIANOTUHAOCUHAOEUNAOEHUNTHDYDHPYDRCPYDRSTITOEUBXHUDGO>PYAUTAHIACIANOTUHAOCUHAOEUNAOEHUNTHDYDHPYDRCPYDRSTITOEUBXHUDGO>PYAUTAHIACIANOTUHAOCUHAOEUNAOEHUNTHDYDHPYDRCPYDRSTITOEUBXHUDGO>PYAUTAHIACIANOTUHAOCUHAOEUNAOEHUNTHDYDHPYDRCPYDRSTITOEUBXHUDGO>PYAUTAHIACIANOTUHAOCUHAOEUNAOEHUNTHDYDHPYDRCPYDRSTITOEUBXHUDGO>PYAUTAHIACIANOTUHAOCUHAOEUNAOEHUNTHDYDHPYDRCPYDRSTITOEUBXHUDGO>PYAUTAHIACIANOTUHAOCUHAOEUNAOEHUNTHDYDHPYDRCPYDRSTITOEUBXHUDGO>PYAUTAHIACIANOTUHAOCUHAOEUNAOEHUNTHDYDHPYDRCPYDRSTITOEUBXHUDGO>PY";

	compileAndRun(sourceCode, 0, "Main", encodeArgs(
		u256(0x40),
		u256(0x40 + 0x20 + ((a.length() + 31) / 32) * 32),
		u256(a.length()),
		a,
		u256(b.length()),
		b
	));
	ABI_CHECK(callContractFunction("a()"), encodeDyn(a));
	ABI_CHECK(callContractFunction("b()"), encodeDyn(b));
}

BOOST_AUTO_TEST_CASE(constructor_static_array_argument)
{
	char const* sourceCode = R"(
		contract C {
			uint public a;
			uint[3] public b;

			constructor(uint _a, uint[3] memory _b) public {
				a = _a;
				b = _b;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C", encodeArgs(u256(1), u256(2), u256(3), u256(4)));
	ABI_CHECK(callContractFunction("a()"), encodeArgs(u256(1)));
	ABI_CHECK(callContractFunction("b(uint256)", u256(0)), encodeArgs(u256(2)));
	ABI_CHECK(callContractFunction("b(uint256)", u256(1)), encodeArgs(u256(3)));
	ABI_CHECK(callContractFunction("b(uint256)", u256(2)), encodeArgs(u256(4)));
}

BOOST_AUTO_TEST_CASE(constant_var_as_array_length)
{
	char const* sourceCode = R"(
		contract C {
			uint constant LEN = 3;
			uint[LEN] public a;

			constructor(uint[LEN] memory _a) public {
				a = _a;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C", encodeArgs(u256(1), u256(2), u256(3)));
	ABI_CHECK(callContractFunction("a(uint256)", u256(0)), encodeArgs(u256(1)));
	ABI_CHECK(callContractFunction("a(uint256)", u256(1)), encodeArgs(u256(2)));
	ABI_CHECK(callContractFunction("a(uint256)", u256(2)), encodeArgs(u256(3)));
}

BOOST_AUTO_TEST_CASE(functions_called_by_constructor)
{
	char const* sourceCode = R"(
		contract Test {
			bytes3 name;
			bool flag;
			constructor() public {
				setName("abc");
			}
			function getName() public returns (bytes3 ret) { return name; }
			function setName(bytes3 _name) private { name = _name; }
		}
	)";
	compileAndRun(sourceCode);
	BOOST_REQUIRE(callContractFunction("getName()") == encodeArgs("abc"));
}

BOOST_AUTO_TEST_CASE(contracts_as_addresses)
{
	char const* sourceCode = R"(
		contract helper {
			function() external payable { } // can receive ether
		}
		contract test {
			helper h;
			constructor() public payable { h = new helper(); address(h).send(5); }
			function getBalance() public returns (uint256 myBalance, uint256 helperBalance) {
				myBalance = address(this).balance;
				helperBalance = address(h).balance;
			}
		}
	)";
	compileAndRun(sourceCode, 20);
	BOOST_CHECK_EQUAL(balanceAt(m_contractAddress), 20 - 5);
	BOOST_REQUIRE(callContractFunction("getBalance()") == encodeArgs(u256(20 - 5), u256(5)));
}

BOOST_AUTO_TEST_CASE(gas_and_value_basic)
{
	char const* sourceCode = R"(
		contract helper {
			bool flag;
			function getBalance() payable public returns (uint256 myBalance) {
				return address(this).balance;
			}
			function setFlag() public { flag = true; }
			function getFlag() public returns (bool fl) { return flag; }
		}
		contract test {
			helper h;
			constructor() public payable { h = new helper(); }
			function sendAmount(uint amount) public payable returns (uint256 bal) {
				return h.getBalance.value(amount)();
			}
			function outOfGas() public returns (bool ret) {
				h.setFlag.gas(2)(); // should fail due to OOG
				return true;
			}
			function checkState() public returns (bool flagAfter, uint myBal) {
				flagAfter = h.getFlag();
				myBal = address(this).balance;
			}
		}
	)";
	compileAndRun(sourceCode, 20);
	BOOST_REQUIRE(callContractFunction("sendAmount(uint256)", 5) == encodeArgs(5));
	// call to helper should not succeed but amount should be transferred anyway
	BOOST_REQUIRE(callContractFunction("outOfGas()") == bytes());
	BOOST_REQUIRE(callContractFunction("checkState()") == encodeArgs(false, 20 - 5));
}

BOOST_AUTO_TEST_CASE(gasleft_decrease)
{
	char const* sourceCode = R"(
		contract C {
			uint v;
			function f() public returns (bool) {
				uint startGas = gasleft();
				v++;
				assert(startGas > gasleft());
				return true;
			}
			function g() public returns (bool) {
				uint startGas = gasleft();
				assert(startGas > gasleft());
				return true;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f()"), encodeArgs(true));
	ABI_CHECK(callContractFunction("g()"), encodeArgs(true));
}

BOOST_AUTO_TEST_CASE(gaslimit)
{
	char const* sourceCode = R"(
		contract C {
			function f() public returns (uint) {
				return block.gaslimit;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f()"), encodeArgs(gasLimit()));
}

BOOST_AUTO_TEST_CASE(gasprice)
{
	char const* sourceCode = R"(
		contract C {
			function f() public returns (uint) {
				return tx.gasprice;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f()"), encodeArgs(gasPrice()));
}

BOOST_AUTO_TEST_CASE(blockhash)
{
	// depending on the aleth version, this test only works for pre-constantinople
	if (Options::get().evmVersion() < EVMVersion::constantinople())
	{
		char const* sourceCode = R"(
			contract C {
				uint256 counter;
				function g() public returns (bool) { counter++; return true; }
				function f() public returns (bytes32[] memory r) {
					r = new bytes32[](259);
					for (uint i = 0; i < 259; i++)
						r[i] = blockhash(block.number - 257 + i);
				}
			}
		)";
		compileAndRun(sourceCode);
		// generate a sufficient amount of blocks
		while (blockNumber() < u256(255))
			ABI_CHECK(callContractFunction("g()"), encodeArgs(true));

		vector<u256> hashes;
		// ``blockhash()`` is only valid for the last 256 blocks, otherwise zero
		hashes.emplace_back(0);
		for (u256 i = blockNumber() - u256(255); i <= blockNumber(); i++)
			hashes.emplace_back(blockHash(i));
		// the current block hash is not yet known at execution time and therefore zero
		hashes.emplace_back(0);
		// future block hashes are zero
		hashes.emplace_back(0);

		ABI_CHECK(callContractFunction("f()"), encodeDyn(hashes));
	}
}

BOOST_AUTO_TEST_CASE(value_complex)
{
	char const* sourceCode = R"(
		contract helper {
			function getBalance() payable public returns (uint256 myBalance) {
				return address(this).balance;
			}
		}
		contract test {
			helper h;
			constructor() public payable { h = new helper(); }
			function sendAmount(uint amount) public payable returns (uint256 bal) {
				uint someStackElement = 20;
				return h.getBalance.value(amount).gas(1000).value(amount + 3)();
			}
		}
	)";
	compileAndRun(sourceCode, 20);
	BOOST_REQUIRE(callContractFunction("sendAmount(uint256)", 5) == encodeArgs(8));
}

BOOST_AUTO_TEST_CASE(value_insane)
{
	char const* sourceCode = R"(
		contract helper {
			function getBalance() payable public returns (uint256 myBalance) {
				return address(this).balance;
			}
		}
		contract test {
			helper h;
			constructor() public payable { h = new helper(); }
			function sendAmount(uint amount) public returns (uint256 bal) {
				return h.getBalance.value(amount).gas(1000).value(amount + 3)();// overwrite value
			}
		}
	)";
	compileAndRun(sourceCode, 20);
	BOOST_REQUIRE(callContractFunction("sendAmount(uint256)", 5) == encodeArgs(8));
}

BOOST_AUTO_TEST_CASE(value_for_constructor)
{
	char const* sourceCode = R"(
		contract Helper {
			bytes3 name;
			bool flag;
			constructor(bytes3 x, bool f) public payable {
				name = x;
				flag = f;
			}
			function getName() public returns (bytes3 ret) { return name; }
			function getFlag() public returns (bool ret) { return flag; }
		}
		contract Main {
			Helper h;
			constructor() public payable {
				h = (new Helper).value(10)("abc", true);
			}
			function getFlag() public returns (bool ret) { return h.getFlag(); }
			function getName() public returns (bytes3 ret) { return h.getName(); }
			function getBalances() public returns (uint me, uint them) { me = address(this).balance; them = address(h).balance;}
		}
	)";
	compileAndRun(sourceCode, 22, "Main");
	BOOST_REQUIRE(callContractFunction("getFlag()") == encodeArgs(true));
	BOOST_REQUIRE(callContractFunction("getName()") == encodeArgs("abc"));
	BOOST_REQUIRE(callContractFunction("getBalances()") == encodeArgs(12, 10));
}

BOOST_AUTO_TEST_CASE(virtual_function_calls)
{
	char const* sourceCode = R"(
		contract Base {
			function f() public returns (uint i) { return g(); }
			function g() public returns (uint i) { return 1; }
		}
		contract Derived is Base {
			function g() public returns (uint i) { return 2; }
		}
	)";
	compileAndRun(sourceCode, 0, "Derived");
	ABI_CHECK(callContractFunction("g()"), encodeArgs(2));
	ABI_CHECK(callContractFunction("f()"), encodeArgs(2));
}

BOOST_AUTO_TEST_CASE(access_base_storage)
{
	char const* sourceCode = R"(
		contract Base {
			uint dataBase;
			function getViaBase() public returns (uint i) { return dataBase; }
		}
		contract Derived is Base {
			uint dataDerived;
			function setData(uint base, uint derived) public returns (bool r) {
				dataBase = base;
				dataDerived = derived;
				return true;
			}
			function getViaDerived() public returns (uint base, uint derived) {
				base = dataBase;
				derived = dataDerived;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Derived");
	ABI_CHECK(callContractFunction("setData(uint256,uint256)", 1, 2), encodeArgs(true));
	ABI_CHECK(callContractFunction("getViaBase()"), encodeArgs(1));
	ABI_CHECK(callContractFunction("getViaDerived()"), encodeArgs(1, 2));
}

BOOST_AUTO_TEST_CASE(single_copy_with_multiple_inheritance)
{
	char const* sourceCode = R"(
		contract Base {
			uint data;
			function setData(uint i) public { data = i; }
			function getViaBase() public returns (uint i) { return data; }
		}
		contract A is Base { function setViaA(uint i) public { setData(i); } }
		contract B is Base { function getViaB() public returns (uint i) { return getViaBase(); } }
		contract Derived is Base, B, A { }
	)";
	compileAndRun(sourceCode, 0, "Derived");
	ABI_CHECK(callContractFunction("getViaB()"), encodeArgs(0));
	ABI_CHECK(callContractFunction("setViaA(uint256)", 23), encodeArgs());
	ABI_CHECK(callContractFunction("getViaB()"), encodeArgs(23));
}

BOOST_AUTO_TEST_CASE(explicit_base_class)
{
	char const* sourceCode = R"(
		contract BaseBase { function g() public returns (uint r) { return 1; } }
		contract Base is BaseBase { function g() public returns (uint r) { return 2; } }
		contract Derived is Base {
			function f() public returns (uint r) { return BaseBase.g(); }
			function g() public returns (uint r) { return 3; }
		}
	)";
	compileAndRun(sourceCode, 0, "Derived");
	ABI_CHECK(callContractFunction("g()"), encodeArgs(3));
	ABI_CHECK(callContractFunction("f()"), encodeArgs(1));
}

BOOST_AUTO_TEST_CASE(base_constructor_arguments)
{
	char const* sourceCode = R"(
		contract BaseBase {
			uint m_a;
			constructor(uint a) public {
				m_a = a;
			}
		}
		contract Base is BaseBase(7) {
			constructor() public {
				m_a *= m_a;
			}
		}
		contract Derived is Base() {
			function getA() public returns (uint r) { return m_a; }
		}
	)";
	compileAndRun(sourceCode, 0, "Derived");
	ABI_CHECK(callContractFunction("getA()"), encodeArgs(7 * 7));
}

BOOST_AUTO_TEST_CASE(function_usage_in_constructor_arguments)
{
	char const* sourceCode = R"(
		contract BaseBase {
			uint m_a;
			constructor(uint a) public {
				m_a = a;
			}
			function g() public returns (uint r) { return 2; }
		}
		contract Base is BaseBase(BaseBase.g()) {
		}
		contract Derived is Base() {
			function getA() public returns (uint r) { return m_a; }
		}
	)";
	compileAndRun(sourceCode, 0, "Derived");
	ABI_CHECK(callContractFunction("getA()"), encodeArgs(2));
}

BOOST_AUTO_TEST_CASE(virtual_function_usage_in_constructor_arguments)
{
	char const* sourceCode = R"(
		contract BaseBase {
			uint m_a;
			constructor(uint a) public {
				m_a = a;
			}
			function overridden() public returns (uint r) { return 1; }
			function g() public returns (uint r) { return overridden(); }
		}
		contract Base is BaseBase(BaseBase.g()) {
		}
		contract Derived is Base() {
			function getA() public returns (uint r) { return m_a; }
			function overridden() public returns (uint r) { return 2; }
		}
	)";
	compileAndRun(sourceCode, 0, "Derived");
	ABI_CHECK(callContractFunction("getA()"), encodeArgs(2));
}

BOOST_AUTO_TEST_CASE(internal_constructor)
{
	char const* sourceCode = R"(
		contract C {
			constructor() internal {}
		}
	)";
	BOOST_CHECK(compileAndRunWithoutCheck(sourceCode, 0, "C").empty());
}

BOOST_AUTO_TEST_CASE(function_modifier)
{
	char const* sourceCode = R"(
		contract C {
			function getOne() payable nonFree public returns (uint r) { return 1; }
			modifier nonFree { if (msg.value > 0) _; }
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("getOne()"), encodeArgs(0));
	ABI_CHECK(callContractFunctionWithValue("getOne()", 1), encodeArgs(1));
}

BOOST_AUTO_TEST_CASE(function_modifier_local_variables)
{
	char const* sourceCode = R"(
		contract C {
			modifier mod1 { uint8 a = 1; uint8 b = 2; _; }
			modifier mod2(bool a) { if (a) return; else _; }
			function f(bool a) mod1 mod2(a) public returns (uint r) { return 3; }
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f(bool)", true), encodeArgs(0));
	ABI_CHECK(callContractFunction("f(bool)", false), encodeArgs(3));
}

BOOST_AUTO_TEST_CASE(function_modifier_loop)
{
	char const* sourceCode = R"(
		contract C {
			modifier repeat(uint count) { uint i; for (i = 0; i < count; ++i) _; }
			function f() repeat(10) public returns (uint r) { r += 1; }
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f()"), encodeArgs(10));
}

BOOST_AUTO_TEST_CASE(function_modifier_multi_invocation)
{
	char const* sourceCode = R"(
		contract C {
			modifier repeat(bool twice) { if (twice) _; _; }
			function f(bool twice) repeat(twice) public returns (uint r) { r += 1; }
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f(bool)", false), encodeArgs(1));
	ABI_CHECK(callContractFunction("f(bool)", true), encodeArgs(2));
}

BOOST_AUTO_TEST_CASE(function_modifier_multi_with_return)
{
	// Note that return sets the return variable and jumps to the end of the current function or
	// modifier code block.
	char const* sourceCode = R"(
		contract C {
			modifier repeat(bool twice) { if (twice) _; _; }
			function f(bool twice) repeat(twice) public returns (uint r) { r += 1; return r; }
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f(bool)", false), encodeArgs(1));
	ABI_CHECK(callContractFunction("f(bool)", true), encodeArgs(2));
}

BOOST_AUTO_TEST_CASE(function_modifier_overriding)
{
	char const* sourceCode = R"(
		contract A {
			function f() mod public returns (bool r) { return true; }
			modifier mod { _; }
		}
		contract C is A {
			modifier mod { if (false) _; }
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f()"), encodeArgs(false));
}

BOOST_AUTO_TEST_CASE(function_modifier_calling_functions_in_creation_context)
{
	char const* sourceCode = R"(
		contract A {
			uint data;
			constructor() mod1 public { f1(); }
			function f1() mod2 public { data |= 0x1; }
			function f2() public { data |= 0x20; }
			function f3() public { }
			modifier mod1 { f2(); _; }
			modifier mod2 { f3(); if (false) _; }
			function getData() public returns (uint r) { return data; }
		}
		contract C is A {
			modifier mod1 { f4(); _; }
			function f3() public { data |= 0x300; }
			function f4() public { data |= 0x4000; }
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("getData()"), encodeArgs(0x4300));
}

BOOST_AUTO_TEST_CASE(function_modifier_for_constructor)
{
	char const* sourceCode = R"(
		contract A {
			uint data;
			constructor() mod1 public { data |= 2; }
			modifier mod1 { data |= 1; _; }
			function getData() public returns (uint r) { return data; }
		}
		contract C is A {
			modifier mod1 { data |= 4; _; }
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("getData()"), encodeArgs(4 | 2));
}

BOOST_AUTO_TEST_CASE(function_modifier_multiple_times)
{
	char const* sourceCode = R"(
		contract C {
			uint public a;
			modifier mod(uint x) { a += x; _; }
			function f(uint x) mod(2) mod(5) mod(x) public returns(uint) { return a; }
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f(uint256)", u256(3)), encodeArgs(2 + 5 + 3));
	ABI_CHECK(callContractFunction("a()"), encodeArgs(2 + 5 + 3));
}

BOOST_AUTO_TEST_CASE(function_modifier_multiple_times_local_vars)
{
	char const* sourceCode = R"(
		contract C {
			uint public a;
			modifier mod(uint x) { uint b = x; a += b; _; a -= b; assert(b == x); }
			function f(uint x) mod(2) mod(5) mod(x) public returns(uint) { return a; }
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f(uint256)", u256(3)), encodeArgs(2 + 5 + 3));
	ABI_CHECK(callContractFunction("a()"), encodeArgs(0));
}

BOOST_AUTO_TEST_CASE(function_modifier_library)
{
	char const* sourceCode = R"(
		library L {
			struct S { uint v; }
			modifier mod(S storage s) { s.v++; _; }
			function libFun(S storage s) mod(s) internal { s.v += 0x100; }
		}

		contract Test {
			using L for *;
			L.S s;

			function f() public returns (uint) {
				s.libFun();
				L.libFun(s);
				return s.v;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f()"), encodeArgs(0x202));
}

BOOST_AUTO_TEST_CASE(function_modifier_library_inheritance)
{
	// Tests that virtual lookup for modifiers in libraries does not consider
	// the current inheritance hierarchy.

	char const* sourceCode = R"(
		library L {
			struct S { uint v; }
			modifier mod(S storage s) { s.v++; _; }
			function libFun(S storage s) mod(s) internal { s.v += 0x100; }
		}

		contract Test {
			using L for *;
			L.S s;
			modifier mod(L.S storage) { revert(); _; }

			function f() public returns (uint) {
				s.libFun();
				L.libFun(s);
				return s.v;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f()"), encodeArgs(0x202));
}

BOOST_AUTO_TEST_CASE(crazy_elementary_typenames_on_stack)
{
	char const* sourceCode = R"(
		contract C {
			function f() public returns (uint r) {
				uint; uint; uint; uint;
				int x = -7;
				return uint(x);
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(-7)));
}

BOOST_AUTO_TEST_CASE(super)
{
	char const* sourceCode = R"(
		contract A { function f() public returns (uint r) { return 1; } }
		contract B is A { function f() public returns (uint r) { return super.f() | 2; } }
		contract C is A { function f() public returns (uint r) { return super.f() | 4; } }
		contract D is B, C { function f() public returns (uint r) { return super.f() | 8; } }
	)";
	compileAndRun(sourceCode, 0, "D");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(1 | 2 | 4 | 8));
}

BOOST_AUTO_TEST_CASE(super_in_constructor)
{
	char const* sourceCode = R"(
		contract A { function f() public returns (uint r) { return 1; } }
		contract B is A { function f() public returns (uint r) { return super.f() | 2; } }
		contract C is A { function f() public returns (uint r) { return super.f() | 4; } }
		contract D is B, C { uint data; constructor() public { data = super.f() | 8; } function f() public returns (uint r) { return data; } }
	)";
	compileAndRun(sourceCode, 0, "D");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(1 | 2 | 4 | 8));
}

BOOST_AUTO_TEST_CASE(super_alone)
{
	char const* sourceCode = R"(
		contract A { function f() public { super; } }
	)";
	compileAndRun(sourceCode, 0, "A");
	ABI_CHECK(callContractFunction("f()"), encodeArgs());
}

BOOST_AUTO_TEST_CASE(fallback_function)
{
	char const* sourceCode = R"(
		contract A {
			uint data;
			function() external { data = 1; }
			function getData() public returns (uint r) { return data; }
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("getData()"), encodeArgs(0));
	ABI_CHECK(callContractFunction(""), encodeArgs());
	ABI_CHECK(callContractFunction("getData()"), encodeArgs(1));
}

BOOST_AUTO_TEST_CASE(inherited_fallback_function)
{
	char const* sourceCode = R"(
		contract A {
			uint data;
			function() external { data = 1; }
			function getData() public returns (uint r) { return data; }
		}
		contract B is A {}
	)";
	compileAndRun(sourceCode, 0, "B");
	ABI_CHECK(callContractFunction("getData()"), encodeArgs(0));
	ABI_CHECK(callContractFunction(""), encodeArgs());
	ABI_CHECK(callContractFunction("getData()"), encodeArgs(1));
}

BOOST_AUTO_TEST_CASE(default_fallback_throws)
{
	char const* sourceCode = R"YY(
		contract A {
			function f() public returns (bool) {
				(bool success,) = address(this).call("");
				return success;
			}
		}
	)YY";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f()"), encodeArgs(0));

	if (dev::test::Options::get().evmVersion().hasStaticCall())
	{
		char const* sourceCode = R"YY(
			contract A {
				function f() public returns (bool) {
					(bool success, bytes memory data) = address(this).staticcall("");
					assert(data.length == 0);
					return success;
				}
			}
		)YY";
		compileAndRun(sourceCode);
		ABI_CHECK(callContractFunction("f()"), encodeArgs(0));
	}
}

BOOST_AUTO_TEST_CASE(short_data_calls_fallback)
{
	char const* sourceCode = R"(
		contract A {
			uint public x;
			// Signature is d88e0b00
			function fow() public { x = 3; }
			function () external { x = 2; }
		}
	)";
	compileAndRun(sourceCode);
	// should call fallback
	sendMessage(asBytes("\xd8\x8e\x0b"), false, 0);
	BOOST_CHECK(m_transactionSuccessful);
	ABI_CHECK(callContractFunction("x()"), encodeArgs(2));
	// should call function
	sendMessage(asBytes(string("\xd8\x8e\x0b") + string(1, 0)), false, 0);
	BOOST_CHECK(m_transactionSuccessful);
	ABI_CHECK(callContractFunction("x()"), encodeArgs(3));
}

BOOST_AUTO_TEST_CASE(event)
{
	char const* sourceCode = R"(
		contract ClientReceipt {
			event Deposit(address indexed _from, bytes32 indexed _id, uint _value);
			function deposit(bytes32 _id, bool _manually) public payable {
				if (_manually) {
					bytes32 s = 0x19dacbf83c5de6658e14cbf7bcae5c15eca2eedecf1c66fbca928e4d351bea0f;
					log3(bytes32(msg.value), s, bytes32(uint256(msg.sender)), _id);
				} else {
					emit Deposit(msg.sender, _id, msg.value);
				}
			}
		}
	)";
	compileAndRun(sourceCode);
	u256 value(18);
	u256 id(0x1234);
	for (bool manually: {true, false})
	{
		callContractFunctionWithValue("deposit(bytes32,bool)", value, id, manually);
		BOOST_REQUIRE_EQUAL(m_logs.size(), 1);
		BOOST_CHECK_EQUAL(m_logs[0].address, m_contractAddress);
		BOOST_CHECK_EQUAL(h256(m_logs[0].data), h256(u256(value)));
		BOOST_REQUIRE_EQUAL(m_logs[0].topics.size(), 3);
		BOOST_CHECK_EQUAL(m_logs[0].topics[0], dev::keccak256(string("Deposit(address,bytes32,uint256)")));
		BOOST_CHECK_EQUAL(m_logs[0].topics[1], h256(m_sender, h256::AlignRight));
		BOOST_CHECK_EQUAL(m_logs[0].topics[2], h256(id));
	}
}

BOOST_AUTO_TEST_CASE(event_emit)
{
	char const* sourceCode = R"(
		contract ClientReceipt {
			event Deposit(address indexed _from, bytes32 indexed _id, uint _value);
			function deposit(bytes32 _id) public payable {
				emit Deposit(msg.sender, _id, msg.value);
			}
		}
	)";
	compileAndRun(sourceCode);
	u256 value(18);
	u256 id(0x1234);
	callContractFunctionWithValue("deposit(bytes32)", value, id);
	BOOST_REQUIRE_EQUAL(m_logs.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].address, m_contractAddress);
	BOOST_CHECK_EQUAL(h256(m_logs[0].data), h256(u256(value)));
	BOOST_REQUIRE_EQUAL(m_logs[0].topics.size(), 3);
	BOOST_CHECK_EQUAL(m_logs[0].topics[0], dev::keccak256(string("Deposit(address,bytes32,uint256)")));
	BOOST_CHECK_EQUAL(m_logs[0].topics[1], h256(m_sender, h256::AlignRight));
	BOOST_CHECK_EQUAL(m_logs[0].topics[2], h256(id));
}

BOOST_AUTO_TEST_CASE(event_no_arguments)
{
	char const* sourceCode = R"(
		contract ClientReceipt {
			event Deposit();
			function deposit() public {
				emit Deposit();
			}
		}
	)";

	compileAndRun(sourceCode);
	callContractFunction("deposit()");
	BOOST_REQUIRE_EQUAL(m_logs.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].address, m_contractAddress);
	BOOST_CHECK(m_logs[0].data.empty());
	BOOST_REQUIRE_EQUAL(m_logs[0].topics.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].topics[0], dev::keccak256(string("Deposit()")));
}

BOOST_AUTO_TEST_CASE(event_access_through_base_name_emit)
{
	char const* sourceCode = R"(
		contract A {
			event x();
		}
		contract B is A {
			function f() public returns (uint) {
				emit A.x();
				return 1;
			}
		}
	)";
	compileAndRun(sourceCode);
	callContractFunction("f()");
	BOOST_REQUIRE_EQUAL(m_logs.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].address, m_contractAddress);
	BOOST_CHECK(m_logs[0].data.empty());
	BOOST_REQUIRE_EQUAL(m_logs[0].topics.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].topics[0], dev::keccak256(string("x()")));
}

BOOST_AUTO_TEST_CASE(events_with_same_name)
{
	char const* sourceCode = R"(
		contract ClientReceipt {
			event Deposit();
			event Deposit(address _addr);
			event Deposit(address _addr, uint _amount);
			function deposit() public returns (uint) {
				emit Deposit();
				return 1;
			}
			function deposit(address _addr) public returns (uint) {
				emit Deposit(_addr);
				return 1;
			}
			function deposit(address _addr, uint _amount) public returns (uint) {
				emit Deposit(_addr, _amount);
				return 1;
			}
		}
	)";
	u160 const c_loggedAddress = m_contractAddress;

	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("deposit()"), encodeArgs(u256(1)));
	BOOST_REQUIRE_EQUAL(m_logs.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].address, m_contractAddress);
	BOOST_CHECK(m_logs[0].data.empty());
	BOOST_REQUIRE_EQUAL(m_logs[0].topics.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].topics[0], dev::keccak256(string("Deposit()")));

	ABI_CHECK(callContractFunction("deposit(address)", c_loggedAddress), encodeArgs(u256(1)));
	BOOST_REQUIRE_EQUAL(m_logs.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].address, m_contractAddress);
	BOOST_CHECK(m_logs[0].data == encodeArgs(c_loggedAddress));
	BOOST_REQUIRE_EQUAL(m_logs[0].topics.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].topics[0], dev::keccak256(string("Deposit(address)")));

	ABI_CHECK(callContractFunction("deposit(address,uint256)", c_loggedAddress, u256(100)), encodeArgs(u256(1)));
	BOOST_REQUIRE_EQUAL(m_logs.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].address, m_contractAddress);
	BOOST_CHECK(m_logs[0].data == encodeArgs(c_loggedAddress, 100));
	BOOST_REQUIRE_EQUAL(m_logs[0].topics.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].topics[0], dev::keccak256(string("Deposit(address,uint256)")));
}

BOOST_AUTO_TEST_CASE(events_with_same_name_inherited_emit)
{
	char const* sourceCode = R"(
		contract A {
			event Deposit();
		}

		contract B {
			event Deposit(address _addr);
		}

		contract ClientReceipt is A, B {
			event Deposit(address _addr, uint _amount);
			function deposit() public returns (uint) {
				emit Deposit();
				return 1;
			}
			function deposit(address _addr) public returns (uint) {
				emit Deposit(_addr);
				return 1;
			}
			function deposit(address _addr, uint _amount) public returns (uint) {
				emit Deposit(_addr, _amount);
				return 1;
			}
		}
	)";
	u160 const c_loggedAddress = m_contractAddress;

	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("deposit()"), encodeArgs(u256(1)));
	BOOST_REQUIRE_EQUAL(m_logs.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].address, m_contractAddress);
	BOOST_CHECK(m_logs[0].data.empty());
	BOOST_REQUIRE_EQUAL(m_logs[0].topics.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].topics[0], dev::keccak256(string("Deposit()")));

	ABI_CHECK(callContractFunction("deposit(address)", c_loggedAddress), encodeArgs(u256(1)));
	BOOST_REQUIRE_EQUAL(m_logs.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].address, m_contractAddress);
	BOOST_CHECK(m_logs[0].data == encodeArgs(c_loggedAddress));
	BOOST_REQUIRE_EQUAL(m_logs[0].topics.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].topics[0], dev::keccak256(string("Deposit(address)")));

	ABI_CHECK(callContractFunction("deposit(address,uint256)", c_loggedAddress, u256(100)), encodeArgs(u256(1)));
	BOOST_REQUIRE_EQUAL(m_logs.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].address, m_contractAddress);
	BOOST_CHECK(m_logs[0].data == encodeArgs(c_loggedAddress, 100));
	BOOST_REQUIRE_EQUAL(m_logs[0].topics.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].topics[0], dev::keccak256(string("Deposit(address,uint256)")));
}

BOOST_AUTO_TEST_CASE(event_anonymous)
{
	char const* sourceCode = R"(
		contract ClientReceipt {
			event Deposit() anonymous;
			function deposit() public {
				emit Deposit();
			}
		}
	)";
	compileAndRun(sourceCode);
	callContractFunction("deposit()");
	BOOST_REQUIRE_EQUAL(m_logs[0].topics.size(), 0);
}

BOOST_AUTO_TEST_CASE(event_anonymous_with_topics)
{
	char const* sourceCode = R"(
		contract ClientReceipt {
			event Deposit(address indexed _from, bytes32 indexed _id, uint indexed _value, uint indexed _value2, bytes32 data) anonymous;
			function deposit(bytes32 _id) public payable {
				emit Deposit(msg.sender, _id, msg.value, 2, "abc");
			}
		}
	)";
	compileAndRun(sourceCode);
	u256 value(18);
	u256 id(0x1234);
	callContractFunctionWithValue("deposit(bytes32)", value, id);
	BOOST_REQUIRE_EQUAL(m_logs.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].address, m_contractAddress);
	BOOST_CHECK(m_logs[0].data == encodeArgs("abc"));
	BOOST_REQUIRE_EQUAL(m_logs[0].topics.size(), 4);
	BOOST_CHECK_EQUAL(m_logs[0].topics[0], h256(m_sender, h256::AlignRight));
	BOOST_CHECK_EQUAL(m_logs[0].topics[1], h256(id));
	BOOST_CHECK_EQUAL(m_logs[0].topics[2], h256(value));
	BOOST_CHECK_EQUAL(m_logs[0].topics[3], h256(2));
}

BOOST_AUTO_TEST_CASE(event_lots_of_data)
{
	char const* sourceCode = R"(
		contract ClientReceipt {
			event Deposit(address _from, bytes32 _id, uint _value, bool _flag);
			function deposit(bytes32 _id) public payable {
				emit Deposit(msg.sender, _id, msg.value, true);
			}
		}
	)";
	compileAndRun(sourceCode);
	u256 value(18);
	u256 id(0x1234);
	callContractFunctionWithValue("deposit(bytes32)", value, id);
	BOOST_REQUIRE_EQUAL(m_logs.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].address, m_contractAddress);
	BOOST_CHECK(m_logs[0].data == encodeArgs((u160)m_sender, id, value, true));
	BOOST_REQUIRE_EQUAL(m_logs[0].topics.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].topics[0], dev::keccak256(string("Deposit(address,bytes32,uint256,bool)")));
}

BOOST_AUTO_TEST_CASE(event_really_lots_of_data)
{
	char const* sourceCode = R"(
		contract ClientReceipt {
			event Deposit(uint fixeda, bytes dynx, uint fixedb);
			function deposit() public {
				emit Deposit(10, msg.data, 15);
			}
		}
	)";
	compileAndRun(sourceCode);
	callContractFunction("deposit()");
	BOOST_REQUIRE_EQUAL(m_logs.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].address, m_contractAddress);
	BOOST_CHECK_EQUAL(toHex(m_logs[0].data), toHex(encodeArgs(10, 0x60, 15, 4, asString(FixedHash<4>(dev::keccak256("deposit()")).asBytes()))));
	BOOST_REQUIRE_EQUAL(m_logs[0].topics.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].topics[0], dev::keccak256(string("Deposit(uint256,bytes,uint256)")));
}

BOOST_AUTO_TEST_CASE(event_really_lots_of_data_from_storage)
{
	char const* sourceCode = R"(
		contract ClientReceipt {
			bytes x;
			event Deposit(uint fixeda, bytes dynx, uint fixedb);
			function deposit() public {
				x.length = 3;
				x[0] = "A";
				x[1] = "B";
				x[2] = "C";
				emit Deposit(10, x, 15);
			}
		}
	)";
	compileAndRun(sourceCode);
	callContractFunction("deposit()");
	BOOST_REQUIRE_EQUAL(m_logs.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].address, m_contractAddress);
	BOOST_CHECK_EQUAL(toHex(m_logs[0].data), toHex(encodeArgs(10, 0x60, 15, 3, string("ABC"))));
	BOOST_REQUIRE_EQUAL(m_logs[0].topics.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].topics[0], dev::keccak256(string("Deposit(uint256,bytes,uint256)")));
}

BOOST_AUTO_TEST_CASE(event_really_really_lots_of_data_from_storage)
{
	char const* sourceCode = R"(
		contract ClientReceipt {
			bytes x;
			event Deposit(uint fixeda, bytes dynx, uint fixedb);
			function deposit() public {
				x.length = 31;
				x[0] = "A";
				x[1] = "B";
				x[2] = "C";
				x[30] = "Z";
				emit Deposit(10, x, 15);
			}
		}
	)";
	compileAndRun(sourceCode);
	callContractFunction("deposit()");
	BOOST_REQUIRE_EQUAL(m_logs.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].address, m_contractAddress);
	BOOST_CHECK(m_logs[0].data == encodeArgs(10, 0x60, 15, 31, string("ABC") + string(27, 0) + "Z"));
	BOOST_REQUIRE_EQUAL(m_logs[0].topics.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].topics[0], dev::keccak256(string("Deposit(uint256,bytes,uint256)")));
}

BOOST_AUTO_TEST_CASE(event_struct_memory_v2)
{
	char const* sourceCode = R"(
		pragma experimental ABIEncoderV2;
		contract C {
			struct S { uint a; }
			event E(S);
			function createEvent(uint x) public {
				emit E(S(x));
			}
		}
	)";
	compileAndRun(sourceCode);
	u256 x(42);
	callContractFunction("createEvent(uint256)", x);
	BOOST_REQUIRE_EQUAL(m_logs.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].address, m_contractAddress);
	BOOST_CHECK(m_logs[0].data == encodeArgs(x));
	BOOST_REQUIRE_EQUAL(m_logs[0].topics.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].topics[0], dev::keccak256(string("E((uint256))")));
}

BOOST_AUTO_TEST_CASE(event_struct_storage_v2)
{
	char const* sourceCode = R"(
		pragma experimental ABIEncoderV2;
		contract C {
			struct S { uint a; }
			event E(S);
			S s;
			function createEvent(uint x) public {
				s.a = x;
				emit E(s);
			}
		}
	)";
	compileAndRun(sourceCode);
	u256 x(42);
	callContractFunction("createEvent(uint256)", x);
	BOOST_REQUIRE_EQUAL(m_logs.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].address, m_contractAddress);
	BOOST_CHECK(m_logs[0].data == encodeArgs(x));
	BOOST_REQUIRE_EQUAL(m_logs[0].topics.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].topics[0], dev::keccak256(string("E((uint256))")));
}

BOOST_AUTO_TEST_CASE(event_dynamic_array_memory)
{
	char const* sourceCode = R"(
		contract C {
			event E(uint[]);
			function createEvent(uint x) public {
				uint[] memory arr = new uint[](3);
				arr[0] = x;
				arr[1] = x + 1;
				arr[2] = x + 2;
				emit E(arr);
			}
		}
	)";
	compileAndRun(sourceCode);
	u256 x(42);
	callContractFunction("createEvent(uint256)", x);
	BOOST_REQUIRE_EQUAL(m_logs.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].address, m_contractAddress);
	BOOST_CHECK(m_logs[0].data == encodeArgs(0x20, 3, x, x + 1, x + 2));
	BOOST_REQUIRE_EQUAL(m_logs[0].topics.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].topics[0], dev::keccak256(string("E(uint256[])")));
}

BOOST_AUTO_TEST_CASE(event_dynamic_array_memory_v2)
{
	char const* sourceCode = R"(
		pragma experimental ABIEncoderV2;
		contract C {
			event E(uint[]);
			function createEvent(uint x) public {
				uint[] memory arr = new uint[](3);
				arr[0] = x;
				arr[1] = x + 1;
				arr[2] = x + 2;
				emit E(arr);
			}
		}
	)";
	compileAndRun(sourceCode);
	u256 x(42);
	callContractFunction("createEvent(uint256)", x);
	BOOST_REQUIRE_EQUAL(m_logs.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].address, m_contractAddress);
	BOOST_CHECK(m_logs[0].data == encodeArgs(0x20, 3, x, x + 1, x + 2));
	BOOST_REQUIRE_EQUAL(m_logs[0].topics.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].topics[0], dev::keccak256(string("E(uint256[])")));
}

BOOST_AUTO_TEST_CASE(event_dynamic_nested_array_memory_v2)
{
	char const* sourceCode = R"(
		pragma experimental ABIEncoderV2;
		contract C {
			event E(uint[][]);
			function createEvent(uint x) public {
				uint[][] memory arr = new uint[][](2);
				arr[0] = new uint[](2);
				arr[1] = new uint[](2);
				arr[0][0] = x;
				arr[0][1] = x + 1;
				arr[1][0] = x + 2;
				arr[1][1] = x + 3;
				emit E(arr);
			}
		}
	)";
	compileAndRun(sourceCode);
	u256 x(42);
	callContractFunction("createEvent(uint256)", x);
	BOOST_REQUIRE_EQUAL(m_logs.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].address, m_contractAddress);
	BOOST_CHECK(m_logs[0].data == encodeArgs(0x20, 2, 0x40, 0xa0, 2, x, x + 1, 2, x + 2, x + 3));
	BOOST_REQUIRE_EQUAL(m_logs[0].topics.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].topics[0], dev::keccak256(string("E(uint256[][])")));
}

BOOST_AUTO_TEST_CASE(event_dynamic_array_storage)
{
	char const* sourceCode = R"(
		contract C {
			event E(uint[]);
			uint[] arr;
			function createEvent(uint x) public {
				arr.length = 3;
				arr[0] = x;
				arr[1] = x + 1;
				arr[2] = x + 2;
				emit E(arr);
			}
		}
	)";
	compileAndRun(sourceCode);
	u256 x(42);
	callContractFunction("createEvent(uint256)", x);
	BOOST_REQUIRE_EQUAL(m_logs.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].address, m_contractAddress);
	BOOST_CHECK(m_logs[0].data == encodeArgs(0x20, 3, x, x + 1, x + 2));
	BOOST_REQUIRE_EQUAL(m_logs[0].topics.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].topics[0], dev::keccak256(string("E(uint256[])")));
}

BOOST_AUTO_TEST_CASE(event_dynamic_array_storage_v2)
{
	char const* sourceCode = R"(
		pragma experimental ABIEncoderV2;
		contract C {
			event E(uint[]);
			uint[] arr;
			function createEvent(uint x) public {
				arr.length = 3;
				arr[0] = x;
				arr[1] = x + 1;
				arr[2] = x + 2;
				emit E(arr);
			}
		}
	)";
	compileAndRun(sourceCode);
	u256 x(42);
	callContractFunction("createEvent(uint256)", x);
	BOOST_REQUIRE_EQUAL(m_logs.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].address, m_contractAddress);
	BOOST_CHECK(m_logs[0].data == encodeArgs(0x20, 3, x, x + 1, x + 2));
	BOOST_REQUIRE_EQUAL(m_logs[0].topics.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].topics[0], dev::keccak256(string("E(uint256[])")));
}

BOOST_AUTO_TEST_CASE(event_dynamic_nested_array_storage_v2)
{
	char const* sourceCode = R"(
		pragma experimental ABIEncoderV2;
		contract C {
			event E(uint[][]);
			uint[][] arr;
			function createEvent(uint x) public {
				arr.length = 2;
				arr[0].length = 2;
				arr[1].length = 2;
				arr[0][0] = x;
				arr[0][1] = x + 1;
				arr[1][0] = x + 2;
				arr[1][1] = x + 3;
				emit E(arr);
			}
		}
	)";
	compileAndRun(sourceCode);
	u256 x(42);
	callContractFunction("createEvent(uint256)", x);
	BOOST_REQUIRE_EQUAL(m_logs.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].address, m_contractAddress);
	BOOST_CHECK(m_logs[0].data == encodeArgs(0x20, 2, 0x40, 0xa0, 2, x, x + 1, 2, x + 2, x + 3));
	BOOST_REQUIRE_EQUAL(m_logs[0].topics.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].topics[0], dev::keccak256(string("E(uint256[][])")));
}

BOOST_AUTO_TEST_CASE(event_indexed_string)
{
	char const* sourceCode = R"(
		contract C {
			string x;
			uint[4] y;
			event E(string indexed r, uint[4] indexed t);
			function deposit() public {
				bytes(x).length = 90;
				for (uint8 i = 0; i < 90; i++)
					bytes(x)[i] = byte(i);
				y[0] = 4;
				y[1] = 5;
				y[2] = 6;
				y[3] = 7;
				emit E(x, y);
			}
		}
	)";
	compileAndRun(sourceCode);
	callContractFunction("deposit()");
	BOOST_REQUIRE_EQUAL(m_logs.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].address, m_contractAddress);
	string dynx(90, 0);
	for (size_t i = 0; i < dynx.size(); ++i)
		dynx[i] = i;
	BOOST_CHECK(m_logs[0].data == bytes());
	BOOST_REQUIRE_EQUAL(m_logs[0].topics.size(), 3);
	BOOST_CHECK_EQUAL(m_logs[0].topics[1], dev::keccak256(dynx));
	BOOST_CHECK_EQUAL(m_logs[0].topics[2], dev::keccak256(
		encodeArgs(u256(4), u256(5), u256(6), u256(7))
	));
	BOOST_CHECK_EQUAL(m_logs[0].topics[0], dev::keccak256(string("E(string,uint256[4])")));
}

BOOST_AUTO_TEST_CASE(empty_name_input_parameter_with_named_one)
{
	char const* sourceCode = R"(
		contract test {
			function f(uint, uint k) public returns(uint ret_k, uint ret_g){
				uint g = 8;
				ret_k = k;
				ret_g = g;
			}
		}
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callContractFunction("f(uint256,uint256)", 5, 9) != encodeArgs(5, 8));
	ABI_CHECK(callContractFunction("f(uint256,uint256)", 5, 9), encodeArgs(9, 8));
}

BOOST_AUTO_TEST_CASE(empty_name_return_parameter)
{
	char const* sourceCode = R"(
		contract test {
			function f(uint k) public returns(uint){
				return k;
		}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f(uint256)", 9), encodeArgs(9));
}

BOOST_AUTO_TEST_CASE(sha256_empty)
{
	char const* sourceCode = R"(
		contract C {
			function f() public returns (bytes32) {
				return sha256("");
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f()"), fromHex("0xe3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"));
}

BOOST_AUTO_TEST_CASE(ripemd160_empty)
{
	char const* sourceCode = R"(
		contract C {
			function f() public returns (bytes20) {
				return ripemd160("");
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f()"), fromHex("0x9c1185a5c5e9fc54612808977ee8f548b2258d31000000000000000000000000"));
}

BOOST_AUTO_TEST_CASE(keccak256_empty)
{
	char const* sourceCode = R"(
		contract C {
			function f() public returns (bytes32) {
				return keccak256("");
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f()"), fromHex("0xc5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470"));
}

BOOST_AUTO_TEST_CASE(keccak256_multiple_arguments)
{
	char const* sourceCode = R"(
		contract c {
			function foo(uint a, uint b, uint c) public returns (bytes32 d)
			{
				d = keccak256(abi.encodePacked(a, b, c));
			}
		}
	)";
	compileAndRun(sourceCode);

	ABI_CHECK(callContractFunction("foo(uint256,uint256,uint256)", 10, 12, 13), encodeArgs(
		dev::keccak256(
			toBigEndian(u256(10)) +
			toBigEndian(u256(12)) +
			toBigEndian(u256(13))
		)
	));
}

BOOST_AUTO_TEST_CASE(keccak256_multiple_arguments_with_numeric_literals)
{
	char const* sourceCode = R"(
		contract c {
			function foo(uint a, uint16 b) public returns (bytes32 d)
			{
				d = keccak256(abi.encodePacked(a, b, uint8(145)));
			}
		}
	)";
	compileAndRun(sourceCode);

	ABI_CHECK(callContractFunction("foo(uint256,uint16)", 10, 12), encodeArgs(
		dev::keccak256(
			toBigEndian(u256(10)) +
			bytes{0x0, 0xc} +
			bytes(1, 0x91)
		)
	));
}

BOOST_AUTO_TEST_CASE(keccak256_multiple_arguments_with_string_literals)
{
	char const* sourceCode = R"(
		contract c {
			function foo() public returns (bytes32 d)
			{
				d = keccak256("foo");
			}
			function bar(uint a, uint16 b) public returns (bytes32 d)
			{
				d = keccak256(abi.encodePacked(a, b, uint8(145), "foo"));
			}
		}
	)";
	compileAndRun(sourceCode);

	ABI_CHECK(callContractFunction("foo()"), encodeArgs(dev::keccak256("foo")));

	ABI_CHECK(callContractFunction("bar(uint256,uint16)", 10, 12), encodeArgs(
		dev::keccak256(
			toBigEndian(u256(10)) +
			bytes{0x0, 0xc} +
			bytes(1, 0x91) +
			bytes{0x66, 0x6f, 0x6f}
		)
	));
}

BOOST_AUTO_TEST_CASE(keccak256_with_bytes)
{
	char const* sourceCode = R"(
		contract c {
			bytes data;
			function foo() public returns (bool)
			{
				data.length = 3;
				data[0] = "f";
				data[1] = "o";
				data[2] = "o";
				return keccak256(data) == keccak256("foo");
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("foo()"), encodeArgs(true));
}

BOOST_AUTO_TEST_CASE(iterated_keccak256_with_bytes)
{
	char const* sourceCode = R"ABC(
		contract c {
			bytes data;
			function foo() public returns (bytes32)
			{
				data.length = 3;
				data[0] = "x";
				data[1] = "y";
				data[2] = "z";
				return keccak256(abi.encodePacked("b", keccak256(data), "a"));
			}
		}
	)ABC";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("foo()"), encodeArgs(
		u256(dev::keccak256(bytes{'b'} + dev::keccak256("xyz").asBytes() + bytes{'a'}))
	));
}

BOOST_AUTO_TEST_CASE(generic_call)
{
	char const* sourceCode = R"**(
			contract receiver {
				uint public received;
				function receive(uint256 x) public payable { received = x; }
			}
			contract sender {
				constructor() public payable {}
				function doSend(address rec) public returns (uint d)
				{
					bytes4 signature = bytes4(bytes32(keccak256("receive(uint256)")));
					rec.call.value(2)(abi.encodeWithSelector(signature, 23));
					return receiver(rec).received();
				}
			}
	)**";
	compileAndRun(sourceCode, 0, "receiver");
	u160 const c_receiverAddress = m_contractAddress;
	compileAndRun(sourceCode, 50, "sender");
	BOOST_REQUIRE(callContractFunction("doSend(address)", c_receiverAddress) == encodeArgs(23));
	BOOST_CHECK_EQUAL(balanceAt(m_contractAddress), 50 - 2);
}

BOOST_AUTO_TEST_CASE(generic_delegatecall)
{
	char const* sourceCode = R"**(
			contract Receiver {
				uint public received;
				address public sender;
				uint public value;
				constructor() public payable {}
				function receive(uint256 x) public payable { received = x; sender = msg.sender; value = msg.value; }
			}
			contract Sender {
				uint public received;
				address public sender;
				uint public value;
				constructor() public payable {}
				function doSend(address rec) public payable
				{
					bytes4 signature = bytes4(bytes32(keccak256("receive(uint256)")));
					(bool success,) = rec.delegatecall(abi.encodeWithSelector(signature, 23));
					success;
				}
			}
	)**";
	compileAndRun(sourceCode, 0, "Receiver");
	u160 const c_receiverAddress = m_contractAddress;
	compileAndRun(sourceCode, 50, "Sender");
	u160 const c_senderAddress = m_contractAddress;
	BOOST_CHECK(m_sender != c_senderAddress); // just for sanity
	ABI_CHECK(callContractFunctionWithValue("doSend(address)", 11, c_receiverAddress), encodeArgs());
	ABI_CHECK(callContractFunction("received()"), encodeArgs(u256(23)));
	ABI_CHECK(callContractFunction("sender()"), encodeArgs(u160(m_sender)));
	ABI_CHECK(callContractFunction("value()"), encodeArgs(u256(11)));
	m_contractAddress = c_receiverAddress;
	ABI_CHECK(callContractFunction("received()"), encodeArgs(u256(0)));
	ABI_CHECK(callContractFunction("sender()"), encodeArgs(u256(0)));
	ABI_CHECK(callContractFunction("value()"), encodeArgs(u256(0)));
	BOOST_CHECK(storageEmpty(c_receiverAddress));
	BOOST_CHECK(!storageEmpty(c_senderAddress));
	BOOST_CHECK_EQUAL(balanceAt(c_receiverAddress), 0);
	BOOST_CHECK_EQUAL(balanceAt(c_senderAddress), 50 + 11);
}

BOOST_AUTO_TEST_CASE(generic_staticcall)
{
	if (dev::test::Options::get().evmVersion().hasStaticCall())
	{
		char const* sourceCode = R"**(
				contract A {
					uint public x;
					constructor() public { x = 42; }
					function pureFunction(uint256 p) public pure returns (uint256) { return p; }
					function viewFunction(uint256 p) public view returns (uint256) { return p + x; }
					function nonpayableFunction(uint256 p) public returns (uint256) { x = p; return x; }
					function assertFunction(uint256 p) public view returns (uint256) { assert(x == p); return x; }
				}
				contract C {
					function f(address a) public view returns (bool, bytes memory)
					{
						return a.staticcall(abi.encodeWithSignature("pureFunction(uint256)", 23));
					}
					function g(address a) public view returns (bool, bytes memory)
					{
						return a.staticcall(abi.encodeWithSignature("viewFunction(uint256)", 23));
					}
					function h(address a) public view returns (bool, bytes memory)
					{
						return a.staticcall(abi.encodeWithSignature("nonpayableFunction(uint256)", 23));
					}
					function i(address a, uint256 v) public view returns (bool, bytes memory)
					{
						return a.staticcall(abi.encodeWithSignature("assertFunction(uint256)", v));
					}
				}
		)**";
		compileAndRun(sourceCode, 0, "A");
		u160 const c_addressA = m_contractAddress;
		compileAndRun(sourceCode, 0, "C");
		ABI_CHECK(callContractFunction("f(address)", c_addressA), encodeArgs(true, 0x40, 0x20, 23));
		ABI_CHECK(callContractFunction("g(address)", c_addressA), encodeArgs(true, 0x40, 0x20, 23 + 42));
		ABI_CHECK(callContractFunction("h(address)", c_addressA), encodeArgs(false, 0x40, 0x00));
		ABI_CHECK(callContractFunction("i(address,uint256)", c_addressA, 42), encodeArgs(true, 0x40, 0x20, 42));
		ABI_CHECK(callContractFunction("i(address,uint256)", c_addressA, 23), encodeArgs(false, 0x40, 0x00));
	}
}

BOOST_AUTO_TEST_CASE(library_call_in_homestead)
{
	char const* sourceCode = R"(
		library Lib { function m() public returns (address) { return msg.sender; } }
		contract Test {
			address public sender;
			function f() public {
				sender = Lib.m();
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Lib");
	compileAndRun(sourceCode, 0, "Test", bytes(), map<string, Address>{{"Lib", m_contractAddress}});
	ABI_CHECK(callContractFunction("f()"), encodeArgs());
	ABI_CHECK(callContractFunction("sender()"), encodeArgs(u160(m_sender)));
}

BOOST_AUTO_TEST_CASE(library_call_protection)
{
	// This tests code that reverts a call if it is a direct call to a library
	// as opposed to a delegatecall.
	char const* sourceCode = R"(
		library Lib {
			struct S { uint x; }
			// a direct call to this should revert
			function np(S storage s) public returns (address) { s.x = 3; return msg.sender; }
			// a direct call to this is fine
			function v(S storage) public view returns (address) { return msg.sender; }
			// a direct call to this is fine
			function pu() public pure returns (uint) { return 2; }
		}
		contract Test {
			Lib.S public s;
			function np() public returns (address) { return Lib.np(s); }
			function v() public view returns (address) { return Lib.v(s); }
			function pu() public pure returns (uint) { return Lib.pu(); }
		}
	)";
	compileAndRun(sourceCode, 0, "Lib");
	ABI_CHECK(callContractFunction("np(Lib.S storage)", 0), encodeArgs());
	ABI_CHECK(callContractFunction("v(Lib.S storage)", 0), encodeArgs(u160(m_sender)));
	ABI_CHECK(callContractFunction("pu()"), encodeArgs(2));
	compileAndRun(sourceCode, 0, "Test", bytes(), map<string, Address>{{"Lib", m_contractAddress}});
	ABI_CHECK(callContractFunction("s()"), encodeArgs(0));
	ABI_CHECK(callContractFunction("np()"), encodeArgs(u160(m_sender)));
	ABI_CHECK(callContractFunction("s()"), encodeArgs(3));
	ABI_CHECK(callContractFunction("v()"), encodeArgs(u160(m_sender)));
	ABI_CHECK(callContractFunction("pu()"), encodeArgs(2));
}


BOOST_AUTO_TEST_CASE(library_staticcall_delegatecall)
{
	char const* sourceCode = R"(
		 library Lib {
			 function x() public view returns (uint) {
				 return 1;
			 }
		 }
		 contract Test {
			 uint t;
			 function f() public returns (uint) {
				 t = 2;
				 return this.g();
			 }
			 function g() public view returns (uint) {
				 return Lib.x();
			 }
		 }
	)";
	compileAndRun(sourceCode, 0, "Lib");
	compileAndRun(sourceCode, 0, "Test", bytes(), map<string, Address>{{"Lib", m_contractAddress}});
	ABI_CHECK(callContractFunction("f()"), encodeArgs(1));
}

BOOST_AUTO_TEST_CASE(store_bytes)
{
	// this test just checks that the copy loop does not mess up the stack
	char const* sourceCode = R"(
		contract C {
			function save() public returns (uint r) {
				r = 23;
				savedData = msg.data;
				r = 24;
			}
			bytes savedData;
		}
	)";
	compileAndRun(sourceCode);
	// empty copy loop
	ABI_CHECK(callContractFunction("save()"), encodeArgs(24));
	ABI_CHECK(callContractFunction("save()", "abcdefg"), encodeArgs(24));
}

BOOST_AUTO_TEST_CASE(bytes_from_calldata_to_memory)
{
	char const* sourceCode = R"(
		contract C {
			function f() public returns (bytes32) {
				return keccak256(abi.encodePacked("abc", msg.data));
			}
		}
	)";
	compileAndRun(sourceCode);
	bytes calldata1 = FixedHash<4>(dev::keccak256("f()")).asBytes() + bytes(61, 0x22) + bytes(12, 0x12);
	sendMessage(calldata1, false);
	BOOST_CHECK(m_transactionSuccessful);
	BOOST_CHECK(m_output == encodeArgs(dev::keccak256(bytes{'a', 'b', 'c'} + calldata1)));
}

BOOST_AUTO_TEST_CASE(call_forward_bytes)
{
	char const* sourceCode = R"(
		contract receiver {
			uint public received;
			function receive(uint x) public { received += x + 1; }
			function() external { received = 0x80; }
		}
		contract sender {
			constructor() public { rec = new receiver(); }
			function() external { savedData = msg.data; }
			function forward() public returns (bool) { address(rec).call(savedData); return true; }
			function clear() public returns (bool) { delete savedData; return true; }
			function val() public returns (uint) { return rec.received(); }
			receiver rec;
			bytes savedData;
		}
	)";
	compileAndRun(sourceCode, 0, "sender");
	ABI_CHECK(callContractFunction("receive(uint256)", 7), bytes());
	ABI_CHECK(callContractFunction("val()"), encodeArgs(0));
	ABI_CHECK(callContractFunction("forward()"), encodeArgs(true));
	ABI_CHECK(callContractFunction("val()"), encodeArgs(8));
	ABI_CHECK(callContractFunction("clear()"), encodeArgs(true));
	ABI_CHECK(callContractFunction("val()"), encodeArgs(8));
	ABI_CHECK(callContractFunction("forward()"), encodeArgs(true));
	ABI_CHECK(callContractFunction("val()"), encodeArgs(0x80));
}

BOOST_AUTO_TEST_CASE(call_forward_bytes_length)
{
	char const* sourceCode = R"(
		contract receiver {
			uint public calledLength;
			function() external { calledLength = msg.data.length; }
		}
		contract sender {
			receiver rec;
			constructor() public { rec = new receiver(); }
			function viaCalldata() public returns (uint) {
				(bool success,) = address(rec).call(msg.data);
				require(success);
				return rec.calledLength();
			}
			function viaMemory() public returns (uint) {
				bytes memory x = msg.data;
				(bool success,) = address(rec).call(x);
				require(success);
				return rec.calledLength();
			}
			bytes s;
			function viaStorage() public returns (uint) {
				s = msg.data;
				(bool success,) = address(rec).call(s);
				require(success);
				return rec.calledLength();
			}
		}
	)";
	compileAndRun(sourceCode, 0, "sender");

	// No additional data, just function selector
	ABI_CHECK(callContractFunction("viaCalldata()"), encodeArgs(4));
	ABI_CHECK(callContractFunction("viaMemory()"), encodeArgs(4));
	ABI_CHECK(callContractFunction("viaStorage()"), encodeArgs(4));

	// Some additional unpadded data
	bytes unpadded = asBytes(string("abc"));
	ABI_CHECK(callContractFunctionNoEncoding("viaCalldata()", unpadded), encodeArgs(7));
	ABI_CHECK(callContractFunctionNoEncoding("viaMemory()", unpadded), encodeArgs(7));
	ABI_CHECK(callContractFunctionNoEncoding("viaStorage()", unpadded), encodeArgs(7));
}

BOOST_AUTO_TEST_CASE(copying_bytes_multiassign)
{
	char const* sourceCode = R"(
		contract receiver {
			uint public received;
			function receive(uint x) public { received += x + 1; }
			function() external { received = 0x80; }
		}
		contract sender {
			constructor() public { rec = new receiver(); }
			function() external { savedData1 = savedData2 = msg.data; }
			function forward(bool selector) public returns (bool) {
				if (selector) { address(rec).call(savedData1); delete savedData1; }
				else { address(rec).call(savedData2); delete savedData2; }
				return true;
			}
			function val() public returns (uint) { return rec.received(); }
			receiver rec;
			bytes savedData1;
			bytes savedData2;
		}
	)";
	compileAndRun(sourceCode, 0, "sender");
	ABI_CHECK(callContractFunction("receive(uint256)", 7), bytes());
	ABI_CHECK(callContractFunction("val()"), encodeArgs(0));
	ABI_CHECK(callContractFunction("forward(bool)", true), encodeArgs(true));
	ABI_CHECK(callContractFunction("val()"), encodeArgs(8));
	ABI_CHECK(callContractFunction("forward(bool)", false), encodeArgs(true));
	ABI_CHECK(callContractFunction("val()"), encodeArgs(16));
	ABI_CHECK(callContractFunction("forward(bool)", true), encodeArgs(true));
	ABI_CHECK(callContractFunction("val()"), encodeArgs(0x80));
}

BOOST_AUTO_TEST_CASE(delete_removes_bytes_data)
{
	char const* sourceCode = R"(
		contract c {
			function() external { data = msg.data; }
			function del() public returns (bool) { delete data; return true; }
			bytes data;
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("---", 7), bytes());
	BOOST_CHECK(!storageEmpty(m_contractAddress));
	ABI_CHECK(callContractFunction("del()", 7), encodeArgs(true));
	BOOST_CHECK(storageEmpty(m_contractAddress));
}

BOOST_AUTO_TEST_CASE(copy_from_calldata_removes_bytes_data)
{
	char const* sourceCode = R"(
		contract c {
			function set() public returns (bool) { data = msg.data; return true; }
			function() external { data = msg.data; }
			bytes data;
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("set()", 1, 2, 3, 4, 5), encodeArgs(true));
	BOOST_CHECK(!storageEmpty(m_contractAddress));
	sendMessage(bytes(), false);
	BOOST_CHECK(m_transactionSuccessful);
	BOOST_CHECK(m_output.empty());
	BOOST_CHECK(storageEmpty(m_contractAddress));
}

BOOST_AUTO_TEST_CASE(copy_removes_bytes_data)
{
	char const* sourceCode = R"(
		contract c {
			function set() public returns (bool) { data1 = msg.data; return true; }
			function reset() public returns (bool) { data1 = data2; return true; }
			bytes data1;
			bytes data2;
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("set()", 1, 2, 3, 4, 5), encodeArgs(true));
	BOOST_CHECK(!storageEmpty(m_contractAddress));
	ABI_CHECK(callContractFunction("reset()"), encodeArgs(true));
	BOOST_CHECK(storageEmpty(m_contractAddress));
}

BOOST_AUTO_TEST_CASE(bytes_inside_mappings)
{
	char const* sourceCode = R"(
		contract c {
			function set(uint key) public returns (bool) { data[key] = msg.data; return true; }
			function copy(uint from, uint to) public returns (bool) { data[to] = data[from]; return true; }
			mapping(uint => bytes) data;
		}
	)";
	compileAndRun(sourceCode);
	// store a short byte array at 1 and a longer one at 2
	ABI_CHECK(callContractFunction("set(uint256)", 1, 2), encodeArgs(true));
	ABI_CHECK(callContractFunction("set(uint256)", 2, 2, 3, 4, 5), encodeArgs(true));
	BOOST_CHECK(!storageEmpty(m_contractAddress));
	// copy shorter to longer
	ABI_CHECK(callContractFunction("copy(uint256,uint256)", 1, 2), encodeArgs(true));
	BOOST_CHECK(!storageEmpty(m_contractAddress));
	// copy empty to both
	ABI_CHECK(callContractFunction("copy(uint256,uint256)", 99, 1), encodeArgs(true));
	BOOST_CHECK(!storageEmpty(m_contractAddress));
	ABI_CHECK(callContractFunction("copy(uint256,uint256)", 99, 2), encodeArgs(true));
	BOOST_CHECK(storageEmpty(m_contractAddress));
}

BOOST_AUTO_TEST_CASE(bytes_length_member)
{
	char const* sourceCode = R"(
		contract c {
			function set() public returns (bool) { data = msg.data; return true; }
			function getLength() public returns (uint) { return data.length; }
			bytes data;
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("getLength()"), encodeArgs(0));
	ABI_CHECK(callContractFunction("set()", 1, 2), encodeArgs(true));
	ABI_CHECK(callContractFunction("getLength()"), encodeArgs(4+32+32));
}

BOOST_AUTO_TEST_CASE(struct_copy)
{
	char const* sourceCode = R"(
		contract c {
			struct Nested { uint x; uint y; }
			struct Struct { uint a; mapping(uint => Struct) b; Nested nested; uint c; }
			mapping(uint => Struct) data;
			function set(uint k) public returns (bool) {
				data[k].a = 1;
				data[k].nested.x = 3;
				data[k].nested.y = 4;
				data[k].c = 2;
				return true;
			}
			function copy(uint from, uint to) public returns (bool) {
				data[to] = data[from];
				return true;
			}
			function retrieve(uint k) public returns (uint a, uint x, uint y, uint c)
			{
				a = data[k].a;
				x = data[k].nested.x;
				y = data[k].nested.y;
				c = data[k].c;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("set(uint256)", 7), encodeArgs(true));
	ABI_CHECK(callContractFunction("retrieve(uint256)", 7), encodeArgs(1, 3, 4, 2));
	ABI_CHECK(callContractFunction("copy(uint256,uint256)", 7, 8), encodeArgs(true));
	ABI_CHECK(callContractFunction("retrieve(uint256)", 7), encodeArgs(1, 3, 4, 2));
	ABI_CHECK(callContractFunction("retrieve(uint256)", 8), encodeArgs(1, 3, 4, 2));
	ABI_CHECK(callContractFunction("copy(uint256,uint256)", 0, 7), encodeArgs(true));
	ABI_CHECK(callContractFunction("retrieve(uint256)", 7), encodeArgs(0, 0, 0, 0));
	ABI_CHECK(callContractFunction("retrieve(uint256)", 8), encodeArgs(1, 3, 4, 2));
	ABI_CHECK(callContractFunction("copy(uint256,uint256)", 7, 8), encodeArgs(true));
	ABI_CHECK(callContractFunction("retrieve(uint256)", 8), encodeArgs(0, 0, 0, 0));
}

BOOST_AUTO_TEST_CASE(struct_containing_bytes_copy_and_delete)
{
	char const* sourceCode = R"(
		contract c {
			struct Struct { uint a; bytes data; uint b; }
			Struct data1;
			Struct data2;
			function set(uint _a, bytes calldata _data, uint _b) external returns (bool) {
				data1.a = _a;
				data1.b = _b;
				data1.data = _data;
				return true;
			}
			function copy() public returns (bool) {
				data1 = data2;
				return true;
			}
			function del() public returns (bool) {
				delete data1;
				return true;
			}
		}
	)";
	compileAndRun(sourceCode);
	string data = "123456789012345678901234567890123";
	BOOST_CHECK(storageEmpty(m_contractAddress));
	ABI_CHECK(callContractFunction("set(uint256,bytes,uint256)", 12, 0x60, 13, u256(data.length()), data), encodeArgs(true));
	BOOST_CHECK(!storageEmpty(m_contractAddress));
	ABI_CHECK(callContractFunction("copy()"), encodeArgs(true));
	BOOST_CHECK(storageEmpty(m_contractAddress));
	ABI_CHECK(callContractFunction("set(uint256,bytes,uint256)", 12, 0x60, 13, u256(data.length()), data), encodeArgs(true));
	BOOST_CHECK(!storageEmpty(m_contractAddress));
	ABI_CHECK(callContractFunction("del()"), encodeArgs(true));
	BOOST_CHECK(storageEmpty(m_contractAddress));
}

BOOST_AUTO_TEST_CASE(struct_copy_via_local)
{
	char const* sourceCode = R"(
		contract c {
			struct Struct { uint a; uint b; }
			Struct data1;
			Struct data2;
			function test() public returns (bool) {
				data1.a = 1;
				data1.b = 2;
				Struct memory x = data1;
				data2 = x;
				return data2.a == data1.a && data2.b == data1.b;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("test()"), encodeArgs(true));
}

BOOST_AUTO_TEST_CASE(using_enums)
{
	char const* sourceCode = R"(
			contract test {
				enum ActionChoices { GoLeft, GoRight, GoStraight, Sit }
				constructor() public
				{
					choices = ActionChoices.GoStraight;
				}
				function getChoice() public returns (uint d)
				{
					d = uint256(choices);
				}
				ActionChoices choices;
			}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("getChoice()"), encodeArgs(2));
}

BOOST_AUTO_TEST_CASE(enum_explicit_overflow)
{
	char const* sourceCode = R"(
			contract test {
				enum ActionChoices { GoLeft, GoRight, GoStraight }
				constructor() public
				{
				}
				function getChoiceExp(uint x) public returns (uint d)
				{
					choice = ActionChoices(x);
					d = uint256(choice);
				}
				function getChoiceFromSigned(int x) public returns (uint d)
				{
					choice = ActionChoices(x);
					d = uint256(choice);
				}
				function getChoiceFromNegativeLiteral() public returns (uint d)
				{
					choice = ActionChoices(-1);
					d = uint256(choice);
				}
				ActionChoices choice;
			}
	)";
	compileAndRun(sourceCode);
	// These should throw
	ABI_CHECK(callContractFunction("getChoiceExp(uint256)", 3), encodeArgs());
	ABI_CHECK(callContractFunction("getChoiceFromSigned(int256)", -1), encodeArgs());
	ABI_CHECK(callContractFunction("getChoiceFromNegativeLiteral()"), encodeArgs());
	// These should work
	ABI_CHECK(callContractFunction("getChoiceExp(uint256)", 2), encodeArgs(2));
	ABI_CHECK(callContractFunction("getChoiceExp(uint256)", 0), encodeArgs(0));
}

BOOST_AUTO_TEST_CASE(storing_invalid_boolean)
{
	char const* sourceCode = R"(
		contract C {
			event Ev(bool);
			bool public perm;
			function set() public returns(uint) {
				bool tmp;
				assembly {
					tmp := 5
				}
				perm = tmp;
				return 1;
			}
			function ret() public returns(bool) {
				bool tmp;
				assembly {
					tmp := 5
				}
				return tmp;
			}
			function ev() public returns(uint) {
				bool tmp;
				assembly {
					tmp := 5
				}
				emit Ev(tmp);
				return 1;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("set()"), encodeArgs(1));
	ABI_CHECK(callContractFunction("perm()"), encodeArgs(1));
	ABI_CHECK(callContractFunction("ret()"), encodeArgs(1));
	ABI_CHECK(callContractFunction("ev()"), encodeArgs(1));
	BOOST_REQUIRE_EQUAL(m_logs.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].address, m_contractAddress);
	BOOST_CHECK(m_logs[0].data == encodeArgs(1));
	BOOST_REQUIRE_EQUAL(m_logs[0].topics.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].topics[0], dev::keccak256(string("Ev(bool)")));
}


BOOST_AUTO_TEST_CASE(using_contract_enums_with_explicit_contract_name)
{
	char const* sourceCode = R"(
			contract test {
				enum Choice { A, B, C }
				function answer () public returns (test.Choice _ret)
				{
					_ret = test.Choice.B;
				}
			}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("answer()"), encodeArgs(1));
}

BOOST_AUTO_TEST_CASE(using_inherited_enum)
{
	char const* sourceCode = R"(
			contract base {
				enum Choice { A, B, C }
			}

			contract test is base {
				function answer () public returns (Choice _ret)
				{
					_ret = Choice.B;
				}
			}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("answer()"), encodeArgs(1));
}

BOOST_AUTO_TEST_CASE(using_inherited_enum_excplicitly)
{
	char const* sourceCode = R"(
			contract base {
				enum Choice { A, B, C }
			}

			contract test is base {
				function answer () public returns (base.Choice _ret)
				{
					_ret = base.Choice.B;
				}
			}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("answer()"), encodeArgs(1));
}

BOOST_AUTO_TEST_CASE(constructing_enums_from_ints)
{
	char const* sourceCode = R"(
			contract c {
				enum Truth { False, True }
				function test() public returns (uint)
				{
					return uint(Truth(uint8(0x701)));
				}
			}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("test()"), encodeArgs(1));
}

BOOST_AUTO_TEST_CASE(struct_referencing)
{
	static char const* sourceCode = R"(
		pragma experimental ABIEncoderV2;
		interface I {
			struct S { uint a; }
		}
		library L {
			struct S { uint b; uint a; }
			function f() public pure returns (S memory) {
				S memory s;
				s.a = 3;
				return s;
			}
			function g() public pure returns (I.S memory) {
				I.S memory s;
				s.a = 4;
				return s;
			}
			// argument-dependant lookup tests
			function a(I.S memory) public pure returns (uint) { return 1; }
			function a(S memory) public pure returns (uint) { return 2; }
		}
		contract C is I {
			function f() public pure returns (S memory) {
				S memory s;
				s.a = 1;
				return s;
			}
			function g() public pure returns (I.S memory) {
				I.S memory s;
				s.a = 2;
				return s;
			}
			function h() public pure returns (L.S memory) {
				L.S memory s;
				s.a = 5;
				return s;
			}
			function x() public pure returns (L.S memory) {
				return L.f();
			}
			function y() public pure returns (I.S memory) {
				return L.g();
			}
			function a1() public pure returns (uint) { S memory s; return L.a(s); }
			function a2() public pure returns (uint) { L.S memory s; return L.a(s); }
		}
	)";
	compileAndRun(sourceCode, 0, "L");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(0, 3));
	ABI_CHECK(callContractFunction("g()"), encodeArgs(4));
	compileAndRun(sourceCode, 0, "C", bytes(), map<string, Address>{ {"L", m_contractAddress}});
	ABI_CHECK(callContractFunction("f()"), encodeArgs(1));
	ABI_CHECK(callContractFunction("g()"), encodeArgs(2));
	ABI_CHECK(callContractFunction("h()"), encodeArgs(0, 5));
	ABI_CHECK(callContractFunction("x()"), encodeArgs(0, 3));
	ABI_CHECK(callContractFunction("y()"), encodeArgs(4));
	ABI_CHECK(callContractFunction("a1()"), encodeArgs(1));
	ABI_CHECK(callContractFunction("a2()"), encodeArgs(2));
}

BOOST_AUTO_TEST_CASE(enum_referencing)
{
	char const* sourceCode = R"(
		interface I {
			enum Direction { A, B, Left, Right }
		}
		library L {
			enum Direction { Left, Right }
			function f() public pure returns (Direction) {
				return Direction.Right;
			}
			function g() public pure returns (I.Direction) {
				return I.Direction.Right;
			}
		}
		contract C is I {
			function f() public pure returns (Direction) {
				return Direction.Right;
			}
			function g() public pure returns (I.Direction) {
				return I.Direction.Right;
			}
			function h() public pure returns (L.Direction) {
				return L.Direction.Right;
			}
			function x() public pure returns (L.Direction) {
				return L.f();
			}
			function y() public pure returns (I.Direction) {
				return L.g();
			}
		}
	)";
	compileAndRun(sourceCode, 0, "L");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(1));
	ABI_CHECK(callContractFunction("g()"), encodeArgs(3));
	compileAndRun(sourceCode, 0, "C", bytes(), map<string, Address>{{"L", m_contractAddress}});
	ABI_CHECK(callContractFunction("f()"), encodeArgs(3));
	ABI_CHECK(callContractFunction("g()"), encodeArgs(3));
	ABI_CHECK(callContractFunction("h()"), encodeArgs(1));
	ABI_CHECK(callContractFunction("x()"), encodeArgs(1));
	ABI_CHECK(callContractFunction("y()"), encodeArgs(3));
}

BOOST_AUTO_TEST_CASE(inline_member_init)
{
	char const* sourceCode = R"(
		contract test {
			constructor() public {
				m_b = 6;
				m_c = 8;
			}
			uint m_a = 5;
			uint m_b;
			uint m_c = 7;
			function get() public returns (uint a, uint b, uint c){
				a = m_a;
				b = m_b;
				c = m_c;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("get()"), encodeArgs(5, 6, 8));
}

BOOST_AUTO_TEST_CASE(inline_member_init_inheritence)
{
	char const* sourceCode = R"(
		contract Base {
			constructor() public {}
			uint m_base = 5;
			function getBMember() public returns (uint i) { return m_base; }
		}
		contract Derived is Base {
			constructor() public {}
			uint m_derived = 6;
			function getDMember() public returns (uint i) { return m_derived; }
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("getBMember()"), encodeArgs(5));
	ABI_CHECK(callContractFunction("getDMember()"), encodeArgs(6));
}

BOOST_AUTO_TEST_CASE(inline_member_init_inheritence_without_constructor)
{
	char const* sourceCode = R"(
		contract Base {
			uint m_base = 5;
			function getBMember() public returns (uint i) { return m_base; }
		}
		contract Derived is Base {
			uint m_derived = 6;
			function getDMember() public returns (uint i) { return m_derived; }
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("getBMember()"), encodeArgs(5));
	ABI_CHECK(callContractFunction("getDMember()"), encodeArgs(6));
}

BOOST_AUTO_TEST_CASE(external_function)
{
	char const* sourceCode = R"(
		contract c {
			function f(uint a) public returns (uint) { return a; }
			function test(uint a, uint b) external returns (uint r_a, uint r_b) {
				r_a = f(a + 7);
				r_b = b;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("test(uint256,uint256)", 2, 3), encodeArgs(2+7, 3));
}

BOOST_AUTO_TEST_CASE(bytes_in_arguments)
{
	char const* sourceCode = R"(
		contract c {
			uint result;
			function f(uint a, uint b) public { result += a + b; }
			function g(uint a) public { result *= a; }
			function test(uint a, bytes calldata data1, bytes calldata data2, uint b) external returns (uint r_a, uint r, uint r_b, uint l) {
				r_a = a;
				address(this).call(data1);
				address(this).call(data2);
				r = result;
				r_b = b;
				l = data1.length;
			}
		}
	)";
	compileAndRun(sourceCode);

	string innercalldata1 = asString(FixedHash<4>(dev::keccak256("f(uint256,uint256)")).asBytes() + encodeArgs(8, 9));
	string innercalldata2 = asString(FixedHash<4>(dev::keccak256("g(uint256)")).asBytes() + encodeArgs(3));
	bytes calldata = encodeArgs(
		12, 32 * 4, u256(32 * 4 + 32 + (innercalldata1.length() + 31) / 32 * 32), 13,
		u256(innercalldata1.length()), innercalldata1,
		u256(innercalldata2.length()), innercalldata2);
	ABI_CHECK(
		callContractFunction("test(uint256,bytes,bytes,uint256)", calldata),
		encodeArgs(12, (8 + 9) * 3, 13, u256(innercalldata1.length()))
	);
}

BOOST_AUTO_TEST_CASE(fixed_arrays_in_storage)
{
	char const* sourceCode = R"(
		contract c {
			struct Data { uint x; uint y; }
			Data[2**10] data;
			uint[2**10 + 3] ids;
			function setIDStatic(uint id) public { ids[2] = id; }
			function setID(uint index, uint id) public { ids[index] = id; }
			function setData(uint index, uint x, uint y) public { data[index].x = x; data[index].y = y; }
			function getID(uint index) public returns (uint) { return ids[index]; }
			function getData(uint index) public returns (uint x, uint y) { x = data[index].x; y = data[index].y; }
			function getLengths() public returns (uint l1, uint l2) { l1 = data.length; l2 = ids.length; }
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("setIDStatic(uint256)", 11), bytes());
	ABI_CHECK(callContractFunction("getID(uint256)", 2), encodeArgs(11));
	ABI_CHECK(callContractFunction("setID(uint256,uint256)", 7, 8), bytes());
	ABI_CHECK(callContractFunction("getID(uint256)", 7), encodeArgs(8));
	ABI_CHECK(callContractFunction("setData(uint256,uint256,uint256)", 7, 8, 9), bytes());
	ABI_CHECK(callContractFunction("setData(uint256,uint256,uint256)", 8, 10, 11), bytes());
	ABI_CHECK(callContractFunction("getData(uint256)", 7), encodeArgs(8, 9));
	ABI_CHECK(callContractFunction("getData(uint256)", 8), encodeArgs(10, 11));
	ABI_CHECK(callContractFunction("getLengths()"), encodeArgs(u256(1) << 10, (u256(1) << 10) + 3));
}

BOOST_AUTO_TEST_CASE(dynamic_arrays_in_storage)
{
	char const* sourceCode = R"(
		contract c {
			struct Data { uint x; uint y; }
			Data[] data;
			uint[] ids;
			function setIDStatic(uint id) public { ids[2] = id; }
			function setID(uint index, uint id) public { ids[index] = id; }
			function setData(uint index, uint x, uint y) public { data[index].x = x; data[index].y = y; }
			function getID(uint index) public returns (uint) { return ids[index]; }
			function getData(uint index) public returns (uint x, uint y) { x = data[index].x; y = data[index].y; }
			function getLengths() public returns (uint l1, uint l2) { l1 = data.length; l2 = ids.length; }
			function setLengths(uint l1, uint l2) public { data.length = l1; ids.length = l2; }
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("getLengths()"), encodeArgs(0, 0));
	ABI_CHECK(callContractFunction("setLengths(uint256,uint256)", 48, 49), bytes());
	ABI_CHECK(callContractFunction("getLengths()"), encodeArgs(48, 49));
	ABI_CHECK(callContractFunction("setIDStatic(uint256)", 11), bytes());
	ABI_CHECK(callContractFunction("getID(uint256)", 2), encodeArgs(11));
	ABI_CHECK(callContractFunction("setID(uint256,uint256)", 7, 8), bytes());
	ABI_CHECK(callContractFunction("getID(uint256)", 7), encodeArgs(8));
	ABI_CHECK(callContractFunction("setData(uint256,uint256,uint256)", 7, 8, 9), bytes());
	ABI_CHECK(callContractFunction("setData(uint256,uint256,uint256)", 8, 10, 11), bytes());
	ABI_CHECK(callContractFunction("getData(uint256)", 7), encodeArgs(8, 9));
	ABI_CHECK(callContractFunction("getData(uint256)", 8), encodeArgs(10, 11));
}

BOOST_AUTO_TEST_CASE(fixed_out_of_bounds_array_access)
{
	char const* sourceCode = R"(
		contract c {
			uint[4] data;
			function set(uint index, uint value) public returns (bool) { data[index] = value; return true; }
			function get(uint index) public returns (uint) { return data[index]; }
			function length() public returns (uint) { return data.length; }
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("length()"), encodeArgs(4));
	ABI_CHECK(callContractFunction("set(uint256,uint256)", 3, 4), encodeArgs(true));
	ABI_CHECK(callContractFunction("set(uint256,uint256)", 4, 5), bytes());
	ABI_CHECK(callContractFunction("set(uint256,uint256)", 400, 5), bytes());
	ABI_CHECK(callContractFunction("get(uint256)", 3), encodeArgs(4));
	ABI_CHECK(callContractFunction("get(uint256)", 4), bytes());
	ABI_CHECK(callContractFunction("get(uint256)", 400), bytes());
	ABI_CHECK(callContractFunction("length()"), encodeArgs(4));
}

BOOST_AUTO_TEST_CASE(dynamic_out_of_bounds_array_access)
{
	char const* sourceCode = R"(
		contract c {
			uint[] data;
			function enlarge(uint amount) public returns (uint) { return data.length += amount; }
			function set(uint index, uint value) public returns (bool) { data[index] = value; return true; }
			function get(uint index) public returns (uint) { return data[index]; }
			function length() public returns (uint) { return data.length; }
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("length()"), encodeArgs(0));
	ABI_CHECK(callContractFunction("get(uint256)", 3), bytes());
	ABI_CHECK(callContractFunction("enlarge(uint256)", 4), encodeArgs(4));
	ABI_CHECK(callContractFunction("length()"), encodeArgs(4));
	ABI_CHECK(callContractFunction("set(uint256,uint256)", 3, 4), encodeArgs(true));
	ABI_CHECK(callContractFunction("get(uint256)", 3), encodeArgs(4));
	ABI_CHECK(callContractFunction("length()"), encodeArgs(4));
	ABI_CHECK(callContractFunction("set(uint256,uint256)", 4, 8), bytes());
	ABI_CHECK(callContractFunction("length()"), encodeArgs(4));
}

BOOST_AUTO_TEST_CASE(fixed_array_cleanup)
{
	char const* sourceCode = R"(
		contract c {
			uint spacer1;
			uint spacer2;
			uint[20] data;
			function fill() public {
				for (uint i = 0; i < data.length; ++i) data[i] = i+1;
			}
			function clear() public { delete data; }
		}
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(storageEmpty(m_contractAddress));
	ABI_CHECK(callContractFunction("fill()"), bytes());
	BOOST_CHECK(!storageEmpty(m_contractAddress));
	ABI_CHECK(callContractFunction("clear()"), bytes());
	BOOST_CHECK(storageEmpty(m_contractAddress));
}

BOOST_AUTO_TEST_CASE(short_fixed_array_cleanup)
{
	char const* sourceCode = R"(
		contract c {
			uint spacer1;
			uint spacer2;
			uint[3] data;
			function fill() public {
				for (uint i = 0; i < data.length; ++i) data[i] = i+1;
			}
			function clear() public { delete data; }
		}
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(storageEmpty(m_contractAddress));
	ABI_CHECK(callContractFunction("fill()"), bytes());
	BOOST_CHECK(!storageEmpty(m_contractAddress));
	ABI_CHECK(callContractFunction("clear()"), bytes());
	BOOST_CHECK(storageEmpty(m_contractAddress));
}

BOOST_AUTO_TEST_CASE(dynamic_array_cleanup)
{
	char const* sourceCode = R"(
		contract c {
			uint[20] spacer;
			uint[] dynamic;
			function fill() public {
				dynamic.length = 21;
				for (uint i = 0; i < dynamic.length; ++i) dynamic[i] = i+1;
			}
			function halfClear() public { dynamic.length = 5; }
			function fullClear() public { delete dynamic; }
		}
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(storageEmpty(m_contractAddress));
	ABI_CHECK(callContractFunction("fill()"), bytes());
	BOOST_CHECK(!storageEmpty(m_contractAddress));
	ABI_CHECK(callContractFunction("halfClear()"), bytes());
	BOOST_CHECK(!storageEmpty(m_contractAddress));
	ABI_CHECK(callContractFunction("fullClear()"), bytes());
	BOOST_CHECK(storageEmpty(m_contractAddress));
}

BOOST_AUTO_TEST_CASE(dynamic_multi_array_cleanup)
{
	char const* sourceCode = R"(
		contract c {
			struct s { uint[][] d; }
			s[] data;
			function fill() public returns (uint) {
				data.length = 3;
				data[2].d.length = 4;
				data[2].d[3].length = 5;
				data[2].d[3][4] = 8;
				return data[2].d[3][4];
			}
			function clear() public { delete data; }
		}
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(storageEmpty(m_contractAddress));
	ABI_CHECK(callContractFunction("fill()"), encodeArgs(8));
	BOOST_CHECK(!storageEmpty(m_contractAddress));
	ABI_CHECK(callContractFunction("clear()"), bytes());
	BOOST_CHECK(storageEmpty(m_contractAddress));
}

BOOST_AUTO_TEST_CASE(array_copy_storage_storage_dyn_dyn)
{
	char const* sourceCode = R"(
		contract c {
			uint[] data1;
			uint[] data2;
			function setData1(uint length, uint index, uint value) public {
				data1.length = length; if (index < length) data1[index] = value;
			}
			function copyStorageStorage() public { data2 = data1; }
			function getData2(uint index) public returns (uint len, uint val) {
				len = data2.length; if (index < len) val = data2[index];
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("setData1(uint256,uint256,uint256)", 10, 5, 4), bytes());
	ABI_CHECK(callContractFunction("copyStorageStorage()"), bytes());
	ABI_CHECK(callContractFunction("getData2(uint256)", 5), encodeArgs(10, 4));
	ABI_CHECK(callContractFunction("setData1(uint256,uint256,uint256)", 0, 0, 0), bytes());
	ABI_CHECK(callContractFunction("copyStorageStorage()"), bytes());
	ABI_CHECK(callContractFunction("getData2(uint256)", 0), encodeArgs(0, 0));
	BOOST_CHECK(storageEmpty(m_contractAddress));
}

BOOST_AUTO_TEST_CASE(array_copy_storage_storage_static_static)
{
	char const* sourceCode = R"(
		contract c {
			uint[40] data1;
			uint[20] data2;
			function test() public returns (uint x, uint y){
				data1[30] = 4;
				data1[2] = 7;
				data1[3] = 9;
				data2[3] = 8;
				data1 = data2;
				x = data1[3];
				y = data1[30]; // should be cleared
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("test()"), encodeArgs(8, 0));
}

BOOST_AUTO_TEST_CASE(array_copy_storage_storage_static_dynamic)
{
	char const* sourceCode = R"(
		contract c {
			uint[9] data1;
			uint[] data2;
			function test() public returns (uint x, uint y){
				data1[8] = 4;
				data2 = data1;
				x = data2.length;
				y = data2[8];
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("test()"), encodeArgs(9, 4));
}

BOOST_AUTO_TEST_CASE(array_copy_different_packing)
{
	char const* sourceCode = R"(
		contract c {
			bytes8[] data1; // 4 per slot
			bytes10[] data2; // 3 per slot
			function test() public returns (bytes10 a, bytes10 b, bytes10 c, bytes10 d, bytes10 e) {
				data1.length = 9;
				for (uint i = 0; i < data1.length; ++i)
					data1[i] = bytes8(uint64(i));
				data2 = data1;
				a = data2[1];
				b = data2[2];
				c = data2[3];
				d = data2[4];
				e = data2[5];
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("test()"), encodeArgs(
		asString(fromHex("0000000000000001")),
		asString(fromHex("0000000000000002")),
		asString(fromHex("0000000000000003")),
		asString(fromHex("0000000000000004")),
		asString(fromHex("0000000000000005"))
	));
}

BOOST_AUTO_TEST_CASE(array_copy_target_simple)
{
	char const* sourceCode = R"(
		contract c {
			bytes8[9] data1; // 4 per slot
			bytes17[10] data2; // 1 per slot, no offset counter
			function test() public returns (bytes17 a, bytes17 b, bytes17 c, bytes17 d, bytes17 e) {
				for (uint i = 0; i < data1.length; ++i)
					data1[i] = bytes8(uint64(i));
				data2[8] = data2[9] = bytes8(uint64(2));
				data2 = data1;
				a = data2[1];
				b = data2[2];
				c = data2[3];
				d = data2[4];
				e = data2[9];
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("test()"), encodeArgs(
		asString(fromHex("0000000000000001")),
		asString(fromHex("0000000000000002")),
		asString(fromHex("0000000000000003")),
		asString(fromHex("0000000000000004")),
		asString(fromHex("0000000000000000"))
	));
}

BOOST_AUTO_TEST_CASE(array_copy_target_leftover)
{
	// test that leftover elements in the last slot of target are correctly cleared during assignment
	char const* sourceCode = R"(
		contract c {
			byte[10] data1;
			bytes2[32] data2;
			function test() public returns (uint check, uint res1, uint res2) {
				uint i;
				for (i = 0; i < data2.length; ++i)
					data2[i] = 0xffff;
				check = uint(uint16(data2[31])) * 0x10000 | uint(uint16(data2[14]));
				for (i = 0; i < data1.length; ++i)
					data1[i] = byte(uint8(1 + i));
				data2 = data1;
				for (i = 0; i < 16; ++i)
					res1 |= uint(uint16(data2[i])) * 0x10000**i;
				for (i = 0; i < 16; ++i)
					res2 |= uint(uint16(data2[16 + i])) * 0x10000**i;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("test()"), encodeArgs(
		u256("0xffffffff"),
		asString(fromHex("0000000000000000""000000000a000900""0800070006000500""0400030002000100")),
		asString(fromHex("0000000000000000""0000000000000000""0000000000000000""0000000000000000"))
	));
}

BOOST_AUTO_TEST_CASE(array_copy_target_leftover2)
{
	// since the copy always copies whole slots, we have to make sure that the source size maxes
	// out a whole slot and at the same time there are still elements left in the target at that point
	char const* sourceCode = R"(
		contract c {
			bytes8[4] data1; // fits into one slot
			bytes10[6] data2; // 4 elements need two slots
			function test() public returns (bytes10 r1, bytes10 r2, bytes10 r3) {
				data1[0] = bytes8(uint64(1));
				data1[1] = bytes8(uint64(2));
				data1[2] = bytes8(uint64(3));
				data1[3] = bytes8(uint64(4));
				for (uint i = 0; i < data2.length; ++i)
					data2[i] = bytes10(uint80(0xffff00 | (1 + i)));
				data2 = data1;
				r1 = data2[3];
				r2 = data2[4];
				r3 = data2[5];
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("test()"), encodeArgs(
		asString(fromHex("0000000000000004")),
		asString(fromHex("0000000000000000")),
		asString(fromHex("0000000000000000"))
	));
}

BOOST_AUTO_TEST_CASE(array_copy_storage_storage_struct)
{
	char const* sourceCode = R"(
		contract c {
			struct Data { uint x; uint y; }
			Data[] data1;
			Data[] data2;
			function test() public returns (uint x, uint y) {
				data1.length = 9;
				data1[8].x = 4;
				data1[8].y = 5;
				data2 = data1;
				x = data2[8].x;
				y = data2[8].y;
				data1.length = 0;
				data2 = data1;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("test()"), encodeArgs(4, 5));
	BOOST_CHECK(storageEmpty(m_contractAddress));
}

BOOST_AUTO_TEST_CASE(array_copy_storage_abi)
{
	// NOTE: This does not really test copying from storage to ABI directly,
	// because it will always copy to memory first.
	char const* sourceCode = R"(
		pragma experimental ABIEncoderV2;
		contract c {
			uint8[] x;
			uint16[] y;
			uint24[] z;
			uint24[][] w;
			function test1() public returns (uint8[] memory) {
				for (uint i = 0; i < 101; ++i)
					x.push(uint8(i));
				return x;
			}
			function test2() public returns (uint16[] memory) {
				for (uint i = 0; i < 101; ++i)
					y.push(uint16(i));
				return y;
			}
			function test3() public returns (uint24[] memory) {
				for (uint i = 0; i < 101; ++i)
					z.push(uint24(i));
				return z;
			}
			function test4() public returns (uint24[][] memory) {
				w.length = 5;
				for (uint i = 0; i < 5; ++i)
					for (uint j = 0; j < 101; ++j)
						w[i].push(uint24(j));
				return w;
			}
		}
	)";
	compileAndRun(sourceCode);
	bytes valueSequence;
	for (size_t i = 0; i < 101; ++i)
		valueSequence += toBigEndian(u256(i));
	ABI_CHECK(callContractFunction("test1()"), encodeArgs(0x20, 101) + valueSequence);
	ABI_CHECK(callContractFunction("test2()"), encodeArgs(0x20, 101) + valueSequence);
	ABI_CHECK(callContractFunction("test3()"), encodeArgs(0x20, 101) + valueSequence);
	ABI_CHECK(callContractFunction("test4()"),
		encodeArgs(0x20, 5, 0xa0, 0xa0 + 102 * 32 * 1, 0xa0 + 102 * 32 * 2, 0xa0 + 102 * 32 * 3, 0xa0 + 102 * 32 * 4) +
		encodeArgs(101) + valueSequence +
		encodeArgs(101) + valueSequence +
		encodeArgs(101) + valueSequence +
		encodeArgs(101) + valueSequence +
		encodeArgs(101) + valueSequence
	);
}

BOOST_AUTO_TEST_CASE(array_copy_storage_abi_signed)
{
	// NOTE: This does not really test copying from storage to ABI directly,
	// because it will always copy to memory first.
	char const* sourceCode = R"(
		contract c {
			int16[] x;
			function test() public returns (int16[] memory) {
				x.push(int16(-1));
				x.push(int16(-1));
				x.push(int16(8));
				x.push(int16(-16));
				x.push(int16(-2));
				x.push(int16(6));
				x.push(int16(8));
				x.push(int16(-1));
				return x;
			}
		}
	)";
	compileAndRun(sourceCode);
	bytes valueSequence;
	ABI_CHECK(callContractFunction("test()"), encodeArgs(0x20, 8,
		u256(-1),
		u256(-1),
		u256(8),
		u256(-16),
		u256(-2),
		u256(6),
		u256(8),
		u256(-1)
	));
}

BOOST_AUTO_TEST_CASE(array_push)
{
	char const* sourceCode = R"(
		contract c {
			uint[] data;
			function test() public returns (uint x, uint y, uint z, uint l) {
				data.push(5);
				x = data[0];
				data.push(4);
				y = data[1];
				l = data.push(3);
				z = data[2];
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("test()"), encodeArgs(5, 4, 3, 3));
}

BOOST_AUTO_TEST_CASE(array_push_struct)
{
	char const* sourceCode = R"(
		contract c {
			struct S { uint16 a; uint16 b; uint16[3] c; uint16[] d; }
			S[] data;
			function test() public returns (uint16, uint16, uint16, uint16) {
				S memory s;
				s.a = 2;
				s.b = 3;
				s.c[2] = 4;
				s.d = new uint16[](4);
				s.d[2] = 5;
				data.push(s);
				return (data[0].a, data[0].b, data[0].c[2], data[0].d[2]);
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("test()"), encodeArgs(2, 3, 4, 5));
}

BOOST_AUTO_TEST_CASE(array_push_packed_array)
{
	char const* sourceCode = R"(
		contract c {
			uint80[] x;
			function test() public returns (uint80, uint80, uint80, uint80) {
				x.push(1);
				x.push(2);
				x.push(3);
				x.push(4);
				x.push(5);
				x.length = 4;
				return (x[0], x[1], x[2], x[3]);
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("test()"), encodeArgs(1, 2, 3, 4));
}

BOOST_AUTO_TEST_CASE(byte_array_push)
{
	char const* sourceCode = R"(
		contract c {
			bytes data;
			function test() public returns (bool x) {
				if (data.push(0x05) != 1)  return true;
				if (data[0] != 0x05) return true;
				data.push(0x04);
				if (data[1] != 0x04) return true;
				uint l = data.push(0x03);
				if (data[2] != 0x03) return true;
				if (l != 0x03) return true;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("test()"), encodeArgs(false));
}

BOOST_AUTO_TEST_CASE(byte_array_push_transition)
{
	// Tests transition between short and long encoding
	char const* sourceCode = R"(
		contract c {
			bytes data;
			function test() public returns (uint) {
				for (uint8 i = 1; i < 40; i++)
				{
					data.push(byte(i));
					if (data.length != i) return 0x1000 + i;
					if (data[data.length - 1] != byte(i)) return i;
				}
				for (uint8 i = 1; i < 40; i++)
					if (data[i - 1] != byte(i)) return 0x1000000 + i;
				return 0;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("test()"), encodeArgs(0));
}

BOOST_AUTO_TEST_CASE(array_pop)
{
	char const* sourceCode = R"(
		contract c {
			uint[] data;
			function test() public returns (uint x, uint l) {
				data.push(7);
				x = data.push(3);
				data.pop();
				x = data.length;
				data.pop();
				l = data.length;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("test()"), encodeArgs(1, 0));
}

BOOST_AUTO_TEST_CASE(array_pop_uint16_transition)
{
	char const* sourceCode = R"(
		contract c {
			uint16[] data;
			function test() public returns (uint16 x, uint16 y, uint16 z) {
				for (uint i = 1; i <= 48; i++)
					data.push(uint16(i));
				for (uint j = 1; j <= 10; j++)
					data.pop();
				x = data[data.length - 1];
				for (uint k = 1; k <= 10; k++)
					data.pop();
				y = data[data.length - 1];
				for (uint l = 1; l <= 10; l++)
					data.pop();
				z = data[data.length - 1];
				for (uint m = 1; m <= 18; m++)
					data.pop();
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("test()"), encodeArgs(38, 28, 18));
	BOOST_CHECK(storageEmpty(m_contractAddress));
}

BOOST_AUTO_TEST_CASE(array_pop_uint24_transition)
{
	char const* sourceCode = R"(
		contract c {
			uint256 a;
			uint256 b;
			uint256 c;
			uint24[] data;
			function test() public returns (uint24 x, uint24 y) {
				for (uint i = 1; i <= 30; i++)
					data.push(uint24(i));
				for (uint j = 1; j <= 10; j++)
					data.pop();
				x = data[data.length - 1];
				for (uint k = 1; k <= 10; k++)
					data.pop();
				y = data[data.length - 1];
				for (uint l = 1; l <= 10; l++)
					data.pop();
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("test()"), encodeArgs(20, 10));
	BOOST_CHECK(storageEmpty(m_contractAddress));
}

BOOST_AUTO_TEST_CASE(array_pop_array_transition)
{
	char const* sourceCode = R"(
		contract c {
			uint256 a;
			uint256 b;
			uint256 c;
			uint16[] inner = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16];
			uint16[][] data;
			function test() public returns (uint x, uint y, uint z) {
				for (uint i = 1; i <= 48; i++)
					data.push(inner);
				for (uint j = 1; j <= 10; j++)
					data.pop();
				x = data[data.length - 1][0];
				for (uint k = 1; k <= 10; k++)
					data.pop();
				y = data[data.length - 1][1];
				for (uint l = 1; l <= 10; l++)
					data.pop();
				z = data[data.length - 1][2];
				for (uint m = 1; m <= 18; m++)
					data.pop();
				delete inner;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("test()"), encodeArgs(1, 2, 3));
	BOOST_CHECK(storageEmpty(m_contractAddress));
}

BOOST_AUTO_TEST_CASE(array_pop_empty_exception)
{
	char const* sourceCode = R"(
		contract c {
			uint[] data;
			function test() public returns (bool) {
				data.pop();
				return true;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("test()"), encodeArgs());
}

BOOST_AUTO_TEST_CASE(array_pop_storage_empty)
{
	char const* sourceCode = R"(
		contract c {
			uint[] data;
			function test() public {
				data.push(7);
				data.pop();
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("test()"), encodeArgs());
	BOOST_CHECK(storageEmpty(m_contractAddress));
}

BOOST_AUTO_TEST_CASE(byte_array_pop)
{
	char const* sourceCode = R"(
		contract c {
			bytes data;
			function test() public returns (uint x, uint y, uint l) {
				data.push(0x07);
				x = data.push(0x03);
				data.pop();
				data.pop();
				y = data.push(0x02);
				l = data.length;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("test()"), encodeArgs(2, 1, 1));
}

BOOST_AUTO_TEST_CASE(byte_array_pop_empty_exception)
{
	char const* sourceCode = R"(
		contract c {
			uint256 a;
			uint256 b;
			uint256 c;
			bytes data;
			function test() public returns (bool) {
				data.pop();
				return true;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("test()"), encodeArgs());
}

BOOST_AUTO_TEST_CASE(byte_array_pop_storage_empty)
{
	char const* sourceCode = R"(
		contract c {
			bytes data;
			function test() public {
				data.push(0x07);
				data.push(0x05);
				data.push(0x03);
				data.pop();
				data.pop();
				data.pop();
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("test()"), encodeArgs());
	BOOST_CHECK(storageEmpty(m_contractAddress));
}

BOOST_AUTO_TEST_CASE(byte_array_pop_long_storage_empty)
{
	char const* sourceCode = R"(
		contract c {
			uint256 a;
			uint256 b;
			uint256 c;
			bytes data;
			function test() public returns (bool) {
				for (uint8 i = 0; i <= 40; i++)
					data.push(byte(i+1));
				for (int8 j = 40; j >= 0; j--) {
					require(data[uint8(j)] == byte(j+1));
					require(data.length == uint8(j+1));
					data.pop();
				}
				return true;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("test()"), encodeArgs(true));
	BOOST_CHECK(storageEmpty(m_contractAddress));
}

BOOST_AUTO_TEST_CASE(byte_array_pop_long_storage_empty_garbage_ref)
{
	char const* sourceCode = R"(
		contract c {
			uint256 a;
			uint256 b;
			bytes data;
			function test() public {
				for (uint8 i = 0; i <= 40; i++)
					data.push(0x03);
				for (uint8 j = 0; j <= 40; j++) {
					assembly {
						mstore(0, "garbage")
					}
					data.pop();
				}
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("test()"), encodeArgs());
	BOOST_CHECK(storageEmpty(m_contractAddress));
}

BOOST_AUTO_TEST_CASE(byte_array_pop_masking_long)
{
	char const* sourceCode = R"(
		contract c {
			bytes data;
			function test() public returns (bytes memory) {
				for (uint i = 0; i < 34; i++)
					data.push(0x03);
				data.pop();
				return data;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("test()"), encodeArgs(
		u256(0x20),
		u256(33),
		asString(fromHex("0303030303030303030303030303030303030303030303030303030303030303")),
		asString(fromHex("03"))
	));
}

BOOST_AUTO_TEST_CASE(byte_array_pop_copy_long)
{
	char const* sourceCode = R"(
		contract c {
			bytes data;
			function test() public returns (bytes memory) {
				for (uint i = 0; i < 33; i++)
					data.push(0x03);
				for (uint j = 0; j < 4; j++)
					data.pop();
				return data;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("test()"), encodeArgs(
		u256(0x20),
		u256(29),
		asString(fromHex("0303030303030303030303030303030303030303030303030303030303"))
	));
}

BOOST_AUTO_TEST_CASE(array_pop_isolated)
{
	char const* sourceCode = R"(
		// This tests that the compiler knows the correct size of the function on the stack.
		contract c {
			uint[] data;
			function test() public returns (uint x) {
				x = 2;
				data.pop;
				x = 3;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("test()"), encodeArgs(3));
}

BOOST_AUTO_TEST_CASE(byte_array_pop_isolated)
{
	char const* sourceCode = R"(
		// This tests that the compiler knows the correct size of the function on the stack.
		contract c {
			bytes data;
			function test() public returns (uint x) {
				x = 2;
				data.pop;
				x = 3;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("test()"), encodeArgs(3));
}

BOOST_AUTO_TEST_CASE(external_array_args)
{
	char const* sourceCode = R"(
		contract c {
			function test(uint[8] calldata a, uint[] calldata b, uint[5] calldata c, uint a_index, uint b_index, uint c_index)
					external returns (uint av, uint bv, uint cv) {
				av = a[a_index];
				bv = b[b_index];
				cv = c[c_index];
			}
		}
	)";
	compileAndRun(sourceCode);
	bytes params = encodeArgs(
		1, 2, 3, 4, 5, 6, 7, 8, // a
		32 * (8 + 1 + 5 + 1 + 1 + 1), // offset to b
		21, 22, 23, 24, 25, // c
		0, 1, 2, // (a,b,c)_index
		3, // b.length
		11, 12, 13 // b
		);
	ABI_CHECK(callContractFunction("test(uint256[8],uint256[],uint256[5],uint256,uint256,uint256)", params), encodeArgs(1, 12, 23));
}

BOOST_AUTO_TEST_CASE(bytes_index_access)
{
	char const* sourceCode = R"(
		contract c {
			bytes data;
			function direct(bytes calldata arg, uint index) external returns (uint) {
				return uint(uint8(arg[index]));
			}
			function storageCopyRead(bytes calldata arg, uint index) external returns (uint) {
				data = arg;
				return uint(uint8(data[index]));
			}
			function storageWrite() external returns (uint) {
				data.length = 35;
				data[31] = 0x77;
				data[32] = 0x14;

				data[31] = 0x01;
				data[31] |= 0x08;
				data[30] = 0x01;
				data[32] = 0x03;
				return uint(uint8(data[30])) * 0x100 | uint(uint8(data[31])) * 0x10 | uint(uint8(data[32]));
			}
		}
	)";
	compileAndRun(sourceCode);
	string array{
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
		10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
		20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
		30, 31, 32, 33};
	ABI_CHECK(callContractFunction("direct(bytes,uint256)", 64, 33, u256(array.length()), array), encodeArgs(33));
	ABI_CHECK(callContractFunction("storageCopyRead(bytes,uint256)", 64, 33, u256(array.length()), array), encodeArgs(33));
	ABI_CHECK(callContractFunction("storageWrite()"), encodeArgs(0x193));
}

BOOST_AUTO_TEST_CASE(bytes_delete_element)
{
	char const* sourceCode = R"(
		contract c {
			bytes data;
			function test1() external returns (bool) {
				data.length = 100;
				for (uint i = 0; i < data.length; i++)
					data[i] = byte(uint8(i));
				delete data[94];
				delete data[96];
				delete data[98];
				return data[94] == 0 && uint8(data[95]) == 95 && data[96] == 0 && uint8(data[97]) == 97;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("test1()"), encodeArgs(true));
}

BOOST_AUTO_TEST_CASE(array_copy_calldata_storage)
{
	char const* sourceCode = R"(
		contract c {
			uint[9] m_data;
			uint[] m_data_dyn;
			uint8[][] m_byte_data;
			function store(uint[9] calldata a, uint8[3][] calldata b) external returns (uint8) {
				m_data = a;
				m_data_dyn = a;
				m_byte_data = b;
				return b[3][1]; // note that access and declaration are reversed to each other
			}
			function retrieve() public returns (uint a, uint b, uint c, uint d, uint e, uint f, uint g) {
				a = m_data.length;
				b = m_data[7];
				c = m_data_dyn.length;
				d = m_data_dyn[7];
				e = m_byte_data.length;
				f = m_byte_data[3].length;
				g = m_byte_data[3][1];
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("store(uint256[9],uint8[3][])", encodeArgs(
		21, 22, 23, 24, 25, 26, 27, 28, 29, // a
		u256(32 * (9 + 1)),
		4, // size of b
		1, 2, 3, // b[0]
		11, 12, 13, // b[1]
		21, 22, 23, // b[2]
		31, 32, 33 // b[3]
	)), encodeArgs(32));
	ABI_CHECK(callContractFunction("retrieve()"), encodeArgs(
		9, 28, 9, 28,
		4, 3, 32));
}

BOOST_AUTO_TEST_CASE(array_copy_nested_array)
{
	char const* sourceCode = R"(
		contract c {
			uint[4][] a;
			uint[10][] b;
			uint[][] c;
			function test(uint[2][] calldata d) external returns (uint) {
				a = d;
				b = a;
				c = b;
				return c[1][1] | c[1][2] | c[1][3] | c[1][4];
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("test(uint256[2][])", encodeArgs(
		32, 3,
		7, 8,
		9, 10,
		11, 12
	)), encodeArgs(10));
}

BOOST_AUTO_TEST_CASE(array_copy_including_mapping)
{
	char const* sourceCode = R"(
		contract c {
			mapping(uint=>uint)[90][] large;
			mapping(uint=>uint)[3][] small;
			function test() public returns (uint r) {
				large.length = small.length = 7;
				large[3][2][0] = 2;
				large[1] = large[3];
				small[3][2][0] = 2;
				small[1] = small[2];
				r = ((
					small[3][2][0] * 0x100 |
					small[1][2][0]) * 0x100 |
					large[3][2][0]) * 0x100 |
					large[1][2][0];
				delete small;
				delete large;
			}
			function clear() public returns (uint r) {
				large.length = small.length = 7;
				small[3][2][0] = 0;
				large[3][2][0] = 0;
				small.length = large.length = 0;
				return 7;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("test()"), encodeArgs(0x02000200));
	// storage is not empty because we cannot delete the mappings
	BOOST_CHECK(!storageEmpty(m_contractAddress));
	ABI_CHECK(callContractFunction("clear()"), encodeArgs(7));
	BOOST_CHECK(storageEmpty(m_contractAddress));
}

BOOST_AUTO_TEST_CASE(swap_in_storage_overwrite)
{
	// This tests a swap in storage which does not work as one
	// might expect because we do not have temporary storage.
	// (x, y) = (y, x) is the same as
	// y = x;
	// x = y;
	char const* sourceCode = R"(
		contract c {
			struct S { uint a; uint b; }
			S public x;
			S public y;
			function set() public {
				x.a = 1; x.b = 2;
				y.a = 3; y.b = 4;
			}
			function swap() public {
				(x, y) = (y, x);
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("x()"), encodeArgs(u256(0), u256(0)));
	ABI_CHECK(callContractFunction("y()"), encodeArgs(u256(0), u256(0)));
	ABI_CHECK(callContractFunction("set()"), encodeArgs());
	ABI_CHECK(callContractFunction("x()"), encodeArgs(u256(1), u256(2)));
	ABI_CHECK(callContractFunction("y()"), encodeArgs(u256(3), u256(4)));
	ABI_CHECK(callContractFunction("swap()"), encodeArgs());
	ABI_CHECK(callContractFunction("x()"), encodeArgs(u256(1), u256(2)));
	ABI_CHECK(callContractFunction("y()"), encodeArgs(u256(1), u256(2)));
}

BOOST_AUTO_TEST_CASE(pass_dynamic_arguments_to_the_base)
{
	char const* sourceCode = R"(
		contract Base {
			constructor(uint i) public
			{
				m_i = i;
			}
			uint public m_i;
		}
		contract Derived is Base {
			constructor(uint i) Base(i) public
			{}
		}
		contract Final is Derived(4) {
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("m_i()"), encodeArgs(4));
}

BOOST_AUTO_TEST_CASE(pass_dynamic_arguments_to_the_base_base)
{
	char const* sourceCode = R"(
		contract Base {
			constructor(uint j) public
			{
				m_i = j;
			}
			uint public m_i;
		}
		contract Base1 is Base {
			constructor(uint k) Base(k) public {}
		}
		contract Derived is Base, Base1 {
			constructor(uint i) Base1(i) public
			{}
		}
		contract Final is Derived(4) {
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("m_i()"), encodeArgs(4));
}

BOOST_AUTO_TEST_CASE(pass_dynamic_arguments_to_the_base_base_with_gap)
{
	char const* sourceCode = R"(
		contract Base {
			constructor(uint i) public
			{
				m_i = i;
			}
			uint public m_i;
		}
		contract Base1 is Base {
			constructor(uint k) public {}
		}
		contract Derived is Base, Base1 {
			constructor(uint i) Base(i) Base1(7) public {}
		}
		contract Final is Derived(4) {
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("m_i()"), encodeArgs(4));
}

BOOST_AUTO_TEST_CASE(simple_constant_variables_test)
{
	char const* sourceCode = R"(
		contract Foo {
			function getX() public returns (uint r) { return x; }
			uint constant x = 56;
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("getX()"), encodeArgs(56));
}

BOOST_AUTO_TEST_CASE(constant_variables)
{
	char const* sourceCode = R"(
		contract Foo {
			uint constant x = 56;
			enum ActionChoices { GoLeft, GoRight, GoStraight, Sit }
			ActionChoices constant choices = ActionChoices.GoLeft;
			bytes32 constant st = "abc\x00\xff__";
		}
	)";
	compileAndRun(sourceCode);
}

BOOST_AUTO_TEST_CASE(assignment_to_const_var_involving_expression)
{
	char const* sourceCode = R"(
		contract C {
			uint constant x = 0x123 + 0x456;
			function f() public returns (uint) { return x + 1; }
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f()"), encodeArgs(0x123 + 0x456 + 1));
}

BOOST_AUTO_TEST_CASE(assignment_to_const_var_involving_keccak)
{
	char const* sourceCode = R"(
		contract C {
			bytes32 constant x = keccak256("abc");
			function f() public returns (bytes32) { return x; }
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f()"), encodeArgs(dev::keccak256("abc")));
}

// Disabled until https://github.com/ethereum/solidity/issues/715 is implemented
//BOOST_AUTO_TEST_CASE(assignment_to_const_array_vars)
//{
//	char const* sourceCode = R"(
//		contract C {
//			uint[3] constant x = [uint(1), 2, 3];
//			uint constant y = x[0] + x[1] + x[2];
//			function f() public returns (uint) { return y; }
//		}
//	)";
//	compileAndRun(sourceCode);
//	ABI_CHECK(callContractFunction("f()"), encodeArgs(1 + 2 + 3));
//}

// Disabled until https://github.com/ethereum/solidity/issues/715 is implemented
//BOOST_AUTO_TEST_CASE(constant_struct)
//{
//	char const* sourceCode = R"(
//		contract C {
//			struct S { uint x; uint[] y; }
//			S constant x = S(5, new uint[](4));
//			function f() public returns (uint) { return x.x; }
//		}
//	)";
//	compileAndRun(sourceCode);
//	ABI_CHECK(callContractFunction("f()"), encodeArgs(5));
//}

BOOST_AUTO_TEST_CASE(packed_storage_structs_uint)
{
	char const* sourceCode = R"(
		contract C {
			struct str { uint8 a; uint16 b; uint248 c; }
			str data;
			function test() public returns (uint) {
				data.a = 2;
				if (data.a != 2) return 2;
				data.b = 0xabcd;
				if (data.b != 0xabcd) return 3;
				data.c = 0x1234567890;
				if (data.c != 0x1234567890) return 4;
				if (data.a != 2) return 5;
				data.a = 8;
				if (data.a != 8) return 6;
				if (data.b != 0xabcd) return 7;
				data.b = 0xdcab;
				if (data.b != 0xdcab) return 8;
				if (data.c != 0x1234567890) return 9;
				data.c = 0x9876543210;
				if (data.c != 0x9876543210) return 10;
				return 1;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("test()"), encodeArgs(1));
}

BOOST_AUTO_TEST_CASE(packed_storage_structs_enum)
{
	char const* sourceCode = R"(
		contract C {
			enum small { A, B, C, D }
			enum larger { A, B, C, D, E}
			struct str { small a; small b; larger c; larger d; }
			str data;
			function test() public returns (uint) {
				data.a = small.B;
				if (data.a != small.B) return 2;
				data.b = small.C;
				if (data.b != small.C) return 3;
				data.c = larger.D;
				if (data.c != larger.D) return 4;
				if (data.a != small.B) return 5;
				data.a = small.C;
				if (data.a != small.C) return 6;
				if (data.b != small.C) return 7;
				data.b = small.D;
				if (data.b != small.D) return 8;
				if (data.c != larger.D) return 9;
				data.c = larger.B;
				if (data.c != larger.B) return 10;
				return 1;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("test()"), encodeArgs(1));
}

BOOST_AUTO_TEST_CASE(packed_storage_structs_bytes)
{
	char const* sourceCode = R"(
		contract C {
			struct s1 { byte a; byte b; bytes10 c; bytes9 d; bytes10 e; }
			struct s2 { byte a; s1 inner; byte b; byte c; }
			byte x;
			s2 data;
			byte y;
			function test() public returns (bool) {
				x = 0x01;
				data.a = 0x02;
				data.inner.a = 0x03;
				data.inner.b = 0x04;
				data.inner.c = "1234567890";
				data.inner.d = "123456789";
				data.inner.e = "abcdefghij";
				data.b = 0x05;
				data.c = byte(0x06);
				y = 0x07;
				return x == 0x01 && data.a == 0x02 && data.inner.a == 0x03 && data.inner.b == 0x04 &&
					data.inner.c == "1234567890" && data.inner.d == "123456789" &&
					data.inner.e == "abcdefghij" && data.b == 0x05 && data.c == byte(0x06) && y == 0x07;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("test()"), encodeArgs(true));
}

BOOST_AUTO_TEST_CASE(packed_storage_structs_delete)
{
	char const* sourceCode = R"(
		contract C {
			struct str { uint8 a; uint16 b; uint8 c; }
			uint8 x;
			uint16 y;
			str data;
			function test() public returns (uint) {
				x = 1;
				y = 2;
				data.a = 2;
				data.b = 0xabcd;
				data.c = 0xfa;
				if (x != 1 || y != 2 || data.a != 2 || data.b != 0xabcd || data.c != 0xfa)
					return 2;
				delete y;
				delete data.b;
				if (x != 1 || y != 0 || data.a != 2 || data.b != 0 || data.c != 0xfa)
					return 3;
				delete x;
				delete data;
				return 1;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("test()"), encodeArgs(1));
	BOOST_CHECK(storageEmpty(m_contractAddress));
}

BOOST_AUTO_TEST_CASE(overloaded_function_call_resolve_to_first)
{
	char const* sourceCode = R"(
		contract test {
			function f(uint k) public returns(uint d) { return k; }
			function f(uint a, uint b) public returns(uint d) { return a + b; }
			function g() public returns(uint d) { return f(3); }
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("g()"), encodeArgs(3));
}

BOOST_AUTO_TEST_CASE(overloaded_function_call_resolve_to_second)
{
	char const* sourceCode = R"(
		contract test {
			function f(uint a, uint b) public returns(uint d) { return a + b; }
			function f(uint k) public returns(uint d) { return k; }
			function g() public returns(uint d) { return f(3, 7); }
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("g()"), encodeArgs(10));
}

BOOST_AUTO_TEST_CASE(overloaded_function_call_with_if_else)
{
	char const* sourceCode = R"(
		contract test {
			function f(uint a, uint b) public returns(uint d) { return a + b; }
			function f(uint k) public returns(uint d) { return k; }
			function g(bool flag) public returns(uint d) {
				if (flag)
					return f(3);
				else
					return f(3, 7);
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("g(bool)", true), encodeArgs(3));
	ABI_CHECK(callContractFunction("g(bool)", false), encodeArgs(10));
}

BOOST_AUTO_TEST_CASE(derived_overload_base_function_direct)
{
	char const* sourceCode = R"(
		contract B { function f() public returns(uint) { return 10; } }
		contract C is B {
			function f(uint i) public returns(uint) { return 2 * i; }
			function g() public returns(uint) { return f(1); }
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("g()"), encodeArgs(2));
}

BOOST_AUTO_TEST_CASE(derived_overload_base_function_indirect)
{
	char const* sourceCode = R"(
		contract A { function f(uint a) public returns(uint) { return 2 * a; } }
		contract B { function f() public returns(uint) { return 10; } }
		contract C is A, B {
			function g() public returns(uint) { return f(); }
			function h() public returns(uint) { return f(1); }
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("g()"), encodeArgs(10));
	ABI_CHECK(callContractFunction("h()"), encodeArgs(2));
}

BOOST_AUTO_TEST_CASE(super_overload)
{
	char const* sourceCode = R"(
		contract A { function f(uint a) public returns(uint) { return 2 * a; } }
		contract B { function f(bool b) public returns(uint) { return 10; } }
		contract C is A, B {
			function g() public returns(uint) { return super.f(true); }
			function h() public returns(uint) { return super.f(1); }
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("g()"), encodeArgs(10));
	ABI_CHECK(callContractFunction("h()"), encodeArgs(2));
}

BOOST_AUTO_TEST_CASE(gasleft_shadow_resolution)
{
	char const* sourceCode = R"(
		contract C {
			function gasleft() public returns(uint256) { return 0; }
			function f() public returns(uint256) { return gasleft(); }
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(0));
}

BOOST_AUTO_TEST_CASE(bool_conversion)
{
	char const* sourceCode = R"(
		contract C {
			function f(bool _b) public returns(uint) {
				if (_b)
					return 1;
				else
					return 0;
			}
			function g(bool _in) public returns (bool _out) {
				_out = _in;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f(bool)", 0), encodeArgs(0));
	ABI_CHECK(callContractFunction("f(bool)", 1), encodeArgs(1));
	ABI_CHECK(callContractFunction("f(bool)", 2), encodeArgs(1));
	ABI_CHECK(callContractFunction("f(bool)", 3), encodeArgs(1));
	ABI_CHECK(callContractFunction("f(bool)", 255), encodeArgs(1));
	ABI_CHECK(callContractFunction("g(bool)", 0), encodeArgs(0));
	ABI_CHECK(callContractFunction("g(bool)", 1), encodeArgs(1));
	ABI_CHECK(callContractFunction("g(bool)", 2), encodeArgs(1));
	ABI_CHECK(callContractFunction("g(bool)", 3), encodeArgs(1));
	ABI_CHECK(callContractFunction("g(bool)", 255), encodeArgs(1));
}

BOOST_AUTO_TEST_CASE(packed_storage_signed)
{
	char const* sourceCode = R"(
		contract C {
			int8 a;
			uint8 b;
			int8 c;
			uint8 d;
			function test() public returns (uint x1, uint x2, uint x3, uint x4) {
				a = -2;
				b = -uint8(a) * 2;
				c = a * int8(120) * int8(121);
				x1 = uint(a);
				x2 = b;
				x3 = uint(c);
				x4 = d;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("test()"), encodeArgs(u256(-2), u256(4), u256(-112), u256(0)));
}

BOOST_AUTO_TEST_CASE(external_types_in_calls)
{
	char const* sourceCode = R"(
		contract C1 { C1 public bla; constructor(C1 x) public { bla = x; } }
		contract C {
			function test() public returns (C1 x, C1 y) {
				C1 c = new C1(C1(9));
				x = c.bla();
				y = this.t1(C1(7));
			}
			function t1(C1 a) public returns (C1) { return a; }
			function t2() public returns (C1) { return C1(9); }
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("test()"), encodeArgs(u256(9), u256(7)));
	ABI_CHECK(callContractFunction("t2()"), encodeArgs(u256(9)));
}

BOOST_AUTO_TEST_CASE(invalid_enum_compared)
{
	char const* sourceCode = R"(
		contract C {
			enum X { A, B }

			function test_eq() public returns (bool) {
				X garbled;
				assembly {
					garbled := 5
				}
				return garbled == garbled;
			}
			function test_eq_ok() public returns (bool) {
				X garbled = X.A;
				return garbled == garbled;
			}
			function test_neq() public returns (bool) {
				X garbled;
				assembly {
					garbled := 5
				}
				return garbled != garbled;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("test_eq_ok()"), encodeArgs(u256(1)));
	// both should throw
	ABI_CHECK(callContractFunction("test_eq()"), encodeArgs());
	ABI_CHECK(callContractFunction("test_neq()"), encodeArgs());
}

BOOST_AUTO_TEST_CASE(invalid_enum_logged)
{
	char const* sourceCode = R"(
		contract C {
			enum X { A, B }
			event Log(X);

			function test_log() public returns (uint) {
				X garbled = X.A;
				assembly {
					garbled := 5
				}
				emit Log(garbled);
				return 1;
			}
			function test_log_ok() public returns (uint) {
				X x = X.A;
				emit Log(x);
				return 1;
			}
		}
		)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("test_log_ok()"), encodeArgs(u256(1)));
	BOOST_REQUIRE_EQUAL(m_logs.size(), 1);
	BOOST_CHECK_EQUAL(m_logs[0].address, m_contractAddress);
	BOOST_REQUIRE_EQUAL(m_logs[0].topics.size(), 1);
	BOOST_REQUIRE_EQUAL(m_logs[0].topics[0], dev::keccak256(string("Log(uint8)")));
	BOOST_CHECK_EQUAL(h256(m_logs[0].data), h256(u256(0)));

	// should throw
	ABI_CHECK(callContractFunction("test_log()"), encodeArgs());
}

BOOST_AUTO_TEST_CASE(invalid_enum_stored)
{
	char const* sourceCode = R"(
		contract C {
			enum X { A, B }
			X public x;

			function test_store() public returns (uint) {
				X garbled = X.A;
				assembly {
					garbled := 5
				}
				x = garbled;
				return 1;
			}
			function test_store_ok() public returns (uint) {
				x = X.A;
				return 1;
			}
		}
		)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("test_store_ok()"), encodeArgs(u256(1)));
	ABI_CHECK(callContractFunction("x()"), encodeArgs(u256(0)));

	// should throw
	ABI_CHECK(callContractFunction("test_store()"), encodeArgs());
}

BOOST_AUTO_TEST_CASE(invalid_enum_as_external_ret)
{
	char const* sourceCode = R"(
		contract C {
			enum X { A, B }

			function test_return() public returns (X) {
				X garbled;
				assembly {
					garbled := 5
				}
				return garbled;
			}
			function test_inline_assignment() public returns (X _ret) {
				assembly {
					_ret := 5
				}
			}
			function test_assignment() public returns (X _ret) {
				X tmp;
				assembly {
					tmp := 5
				}
				_ret = tmp;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	// both should throw
	ABI_CHECK(callContractFunction("test_return()"), encodeArgs());
	ABI_CHECK(callContractFunction("test_inline_assignment()"), encodeArgs());
	ABI_CHECK(callContractFunction("test_assignment()"), encodeArgs());
}

BOOST_AUTO_TEST_CASE(invalid_enum_as_external_arg)
{
	char const* sourceCode = R"(
		contract C {
			enum X { A, B }

			function tested (X x) public returns (uint) {
				return 1;
			}

			function test() public returns (uint) {
				X garbled;

				assembly {
					garbled := 5
				}

				return this.tested(garbled);
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	// should throw
	ABI_CHECK(callContractFunction("test()"), encodeArgs());
}


BOOST_AUTO_TEST_CASE(proper_order_of_overwriting_of_attributes)
{
	// bug #1798
	char const* sourceCode = R"(
		contract init {
			function isOk() public returns (bool) { return false; }
			bool public ok = false;
		}
		contract fix {
			function isOk() public returns (bool) { return true; }
			bool public ok = true;
		}

		contract init_fix is init, fix {
			function checkOk() public returns (bool) { return ok; }
		}
		contract fix_init is fix, init {
			function checkOk() public returns (bool) { return ok; }
		}
	)";
	compileAndRun(sourceCode, 0, "init_fix");
	ABI_CHECK(callContractFunction("isOk()"), encodeArgs(true));
	ABI_CHECK(callContractFunction("ok()"), encodeArgs(true));

	compileAndRun(sourceCode, 0, "fix_init");
	ABI_CHECK(callContractFunction("isOk()"), encodeArgs(false));
	ABI_CHECK(callContractFunction("ok()"), encodeArgs(false));
}

BOOST_AUTO_TEST_CASE(struct_assign_reference_to_struct)
{
	char const* sourceCode = R"(
		contract test {
			struct testStruct
			{
				uint m_value;
			}
			testStruct data1;
			testStruct data2;
			testStruct data3;
			constructor() public
			{
				data1.m_value = 2;
			}
			function assign() public returns (uint ret_local, uint ret_global, uint ret_global3, uint ret_global1)
			{
				testStruct storage x = data1; //x is a reference data1.m_value == 2 as well as x.m_value = 2
				data2 = data1; // should copy data. data2.m_value == 2

				ret_local = x.m_value; // = 2
				ret_global = data2.m_value; // = 2

				x.m_value = 3;
				data3 = x; //should copy the data. data3.m_value == 3
				ret_global3 = data3.m_value; // = 3
				ret_global1 = data1.m_value; // = 3. Changed due to the assignment to x.m_value
			}
		}
	)";
	compileAndRun(sourceCode, 0, "test");
	ABI_CHECK(callContractFunction("assign()"), encodeArgs(2, 2, 3, 3));
}

BOOST_AUTO_TEST_CASE(struct_delete_member)
{
	char const* sourceCode = R"(
		contract test {
			struct testStruct
			{
				uint m_value;
			}
			testStruct data1;
			constructor() public
			{
				data1.m_value = 2;
			}
			function deleteMember() public returns (uint ret_value)
			{
				testStruct storage x = data1; //should not copy the data. data1.m_value == 2 but x.m_value = 0
				x.m_value = 4;
				delete x.m_value;
				ret_value = data1.m_value;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "test");
	ABI_CHECK(callContractFunction("deleteMember()"), encodeArgs(0));
}

BOOST_AUTO_TEST_CASE(struct_delete_struct_in_mapping)
{
	char const* sourceCode = R"(
		contract test {
			struct testStruct
			{
				uint m_value;
			}
			mapping (uint => testStruct) campaigns;

			constructor() public
			{
				campaigns[0].m_value = 2;
			}
			function deleteIt() public returns (uint)
			{
				delete campaigns[0];
				return campaigns[0].m_value;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "test");
	ABI_CHECK(callContractFunction("deleteIt()"), encodeArgs(0));
}

BOOST_AUTO_TEST_CASE(evm_exceptions_out_of_band_access)
{
	char const* sourceCode = R"(
		contract A {
			uint[3] arr;
			bool public test = false;
			function getElement(uint i) public returns (uint)
			{
				return arr[i];
			}
			function testIt() public returns (bool)
			{
				uint i = this.getElement(5);
				test = true;
				return true;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "A");
	ABI_CHECK(callContractFunction("test()"), encodeArgs(false));
	ABI_CHECK(callContractFunction("testIt()"), encodeArgs());
	ABI_CHECK(callContractFunction("test()"), encodeArgs(false));
}

BOOST_AUTO_TEST_CASE(evm_exceptions_in_constructor_call_fail)
{
	char const* sourceCode = R"(
		contract A {
			constructor() public
			{
				address(this).call("123");
			}
		}
		contract B {
			uint public test = 1;
			function testIt() public
			{
				A a = new A();
				++test;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "B");

	ABI_CHECK(callContractFunction("testIt()"), encodeArgs());
	ABI_CHECK(callContractFunction("test()"), encodeArgs(2));
}

BOOST_AUTO_TEST_CASE(evm_exceptions_in_constructor_out_of_baund)
{
	char const* sourceCode = R"(
		contract A {
			uint public test = 1;
			uint[3] arr;
			constructor() public
			{
				uint index = 5;
				test = arr[index];
				++test;
			}
		}
	)";
	ABI_CHECK(compileAndRunWithoutCheck(sourceCode, 0, "A"), encodeArgs());
	BOOST_CHECK(!m_transactionSuccessful);
}

BOOST_AUTO_TEST_CASE(positive_integers_to_signed)
{
	char const* sourceCode = R"(
		contract test {
			int8 public x = 2;
			int8 public y = 127;
			int16 public q = 250;
		}
	)";
	compileAndRun(sourceCode, 0, "test");
	ABI_CHECK(callContractFunction("x()"), encodeArgs(2));
	ABI_CHECK(callContractFunction("y()"), encodeArgs(127));
	ABI_CHECK(callContractFunction("q()"), encodeArgs(250));
}

BOOST_AUTO_TEST_CASE(failing_send)
{
	char const* sourceCode = R"(
		contract Helper {
			uint[] data;
			function () external {
				data[9]; // trigger exception
			}
		}
		contract Main {
			constructor() public payable {}
			function callHelper(address payable _a) public returns (bool r, uint bal) {
				r = !_a.send(5);
				bal = address(this).balance;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Helper");
	u160 const c_helperAddress = m_contractAddress;
	compileAndRun(sourceCode, 20, "Main");
	BOOST_REQUIRE(callContractFunction("callHelper(address)", c_helperAddress) == encodeArgs(true, 20));
}

BOOST_AUTO_TEST_CASE(send_zero_ether)
{
	// Sending zero ether to a contract should still invoke the fallback function
	// (it previously did not because the gas stipend was not provided by the EVM)
	char const* sourceCode = R"(
		contract Receiver {
			function () external payable {
			}
		}
		contract Main {
			constructor() public payable {}
			function s() public returns (bool) {
				Receiver r = new Receiver();
				return address(r).send(0);
			}
		}
	)";
	compileAndRun(sourceCode, 20, "Main");
	BOOST_REQUIRE(callContractFunction("s()") == encodeArgs(true));
}

BOOST_AUTO_TEST_CASE(reusing_memory)
{
	// Invoke some features that use memory and test that they do not interfere with each other.
	char const* sourceCode = R"(
		contract Helper {
			uint public flag;
			constructor(uint x) public {
				flag = x;
			}
		}
		contract Main {
			mapping(uint => uint) map;
			function f(uint x) public returns (uint) {
				map[x] = x;
				return (new Helper(uint(keccak256(abi.encodePacked(this.g(map[x])))))).flag();
			}
			function g(uint a) public returns (uint)
			{
				return map[a];
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Main");
	BOOST_REQUIRE(callContractFunction("f(uint256)", 0x34) == encodeArgs(dev::keccak256(dev::toBigEndian(u256(0x34)))));
}

BOOST_AUTO_TEST_CASE(return_string)
{
	char const* sourceCode = R"(
		contract Main {
			string public s;
			function set(string calldata _s) external {
				s = _s;
			}
			function get1() public returns (string memory r) {
				return s;
			}
			function get2() public returns (string memory r) {
				r = s;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Main");
	string s("Julia");
	bytes args = encodeArgs(u256(0x20), u256(s.length()), s);
	BOOST_REQUIRE(callContractFunction("set(string)", asString(args)) == encodeArgs());
	ABI_CHECK(callContractFunction("get1()"), args);
	ABI_CHECK(callContractFunction("get2()"), args);
	ABI_CHECK(callContractFunction("s()"), args);
}

BOOST_AUTO_TEST_CASE(return_multiple_strings_of_various_sizes)
{
	char const* sourceCode = R"(
		contract Main {
			string public s1;
			string public s2;
			function set(string calldata _s1, uint x, string calldata _s2) external returns (uint) {
				s1 = _s1;
				s2 = _s2;
				return x;
			}
			function get() public returns (string memory r1, string memory r2) {
				r1 = s1;
				r2 = s2;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Main");
	string s1(
		"abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz"
		"abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz"
		"abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz"
		"abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz"
	);
	string s2(
		"ABCDEFGHIJKLMNOPQRSTUVXYZABCDEFGHIJKLMNOPQRSTUVXYZABCDEFGHIJKLMNOPQRSTUVXYZ"
		"ABCDEFGHIJKLMNOPQRSTUVXYZABCDEFGHIJKLMNOPQRSTUVXYZABCDEFGHIJKLMNOPQRSTUVXYZ"
		"ABCDEFGHIJKLMNOPQRSTUVXYZABCDEFGHIJKLMNOPQRSTUVXYZABCDEFGHIJKLMNOPQRSTUVXYZ"
		"ABCDEFGHIJKLMNOPQRSTUVXYZABCDEFGHIJKLMNOPQRSTUVXYZABCDEFGHIJKLMNOPQRSTUVXYZ"
		"ABCDEFGHIJKLMNOPQRSTUVXYZABCDEFGHIJKLMNOPQRSTUVXYZABCDEFGHIJKLMNOPQRSTUVXYZ"
	);
	vector<size_t> lengths{0, 30, 32, 63, 64, 65, 210, 300};
	for (auto l1: lengths)
		for (auto l2: lengths)
		{
			bytes dyn1 = encodeArgs(u256(l1), s1.substr(0, l1));
			bytes dyn2 = encodeArgs(u256(l2), s2.substr(0, l2));
			bytes args = encodeArgs(u256(0x60), u256(l1), u256(0x60 + dyn1.size())) + dyn1 + dyn2;
			BOOST_REQUIRE(
				callContractFunction("set(string,uint256,string)", asString(args)) ==
				encodeArgs(u256(l1))
			);
			bytes result = encodeArgs(u256(0x40), u256(0x40 + dyn1.size())) + dyn1 + dyn2;
			ABI_CHECK(callContractFunction("get()"), result);
			ABI_CHECK(callContractFunction("s1()"), encodeArgs(0x20) + dyn1);
			ABI_CHECK(callContractFunction("s2()"), encodeArgs(0x20) + dyn2);
		}
}

BOOST_AUTO_TEST_CASE(accessor_involving_strings)
{
	char const* sourceCode = R"(
		contract Main {
			struct stringData { string a; uint b; string c; }
			mapping(uint => stringData[]) public data;
			function set(uint x, uint y, string calldata a, uint b, string calldata c) external returns (bool) {
				data[x].length = y + 1;
				data[x][y].a = a;
				data[x][y].b = b;
				data[x][y].c = c;
				return true;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Main");
	string s1("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz");
	string s2("ABCDEFGHIJKLMNOPQRSTUVXYZABCDEFGHIJKLMNOPQRSTUVXYZABCDEFGHIJKLMNOPQRSTUVXYZ");
	bytes s1Data = encodeArgs(u256(s1.length()), s1);
	bytes s2Data = encodeArgs(u256(s2.length()), s2);
	u256 b = 765;
	u256 x = 7;
	u256 y = 123;
	bytes args = encodeArgs(x, y, u256(0xa0), b, u256(0xa0 + s1Data.size()), s1Data, s2Data);
	bytes result = encodeArgs(u256(0x60), b, u256(0x60 + s1Data.size()), s1Data, s2Data);
	BOOST_REQUIRE(callContractFunction("set(uint256,uint256,string,uint256,string)", asString(args)) == encodeArgs(true));
	BOOST_REQUIRE(callContractFunction("data(uint256,uint256)", x, y) == result);
}

BOOST_AUTO_TEST_CASE(bytes_in_function_calls)
{
	char const* sourceCode = R"(
		contract Main {
			string public s1;
			string public s2;
			function set(string memory _s1, uint x, string memory _s2) public returns (uint) {
				s1 = _s1;
				s2 = _s2;
				return x;
			}
			function setIndirectFromMemory(string memory _s1, uint x, string memory _s2) public returns (uint) {
				return this.set(_s1, x, _s2);
			}
			function setIndirectFromCalldata(string calldata _s1, uint x, string calldata _s2) external returns (uint) {
				return this.set(_s1, x, _s2);
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Main");
	string s1("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz");
	string s2("ABCDEFGHIJKLMNOPQRSTUVXYZABCDEFGHIJKLMNOPQRSTUVXYZABCDEFGHIJKLMNOPQRSTUVXYZ");
	vector<size_t> lengths{0, 31, 64, 65};
	for (auto l1: lengths)
		for (auto l2: lengths)
		{
			bytes dyn1 = encodeArgs(u256(l1), s1.substr(0, l1));
			bytes dyn2 = encodeArgs(u256(l2), s2.substr(0, l2));
			bytes args1 = encodeArgs(u256(0x60), u256(l1), u256(0x60 + dyn1.size())) + dyn1 + dyn2;
			BOOST_REQUIRE(
				callContractFunction("setIndirectFromMemory(string,uint256,string)", asString(args1)) ==
				encodeArgs(u256(l1))
			);
			ABI_CHECK(callContractFunction("s1()"), encodeArgs(0x20) + dyn1);
			ABI_CHECK(callContractFunction("s2()"), encodeArgs(0x20) + dyn2);
			// swapped
			bytes args2 = encodeArgs(u256(0x60), u256(l1), u256(0x60 + dyn2.size())) + dyn2 + dyn1;
			BOOST_REQUIRE(
				callContractFunction("setIndirectFromCalldata(string,uint256,string)", asString(args2)) ==
				encodeArgs(u256(l1))
			);
			ABI_CHECK(callContractFunction("s1()"), encodeArgs(0x20) + dyn2);
			ABI_CHECK(callContractFunction("s2()"), encodeArgs(0x20) + dyn1);
		}
}

BOOST_AUTO_TEST_CASE(return_bytes_internal)
{
	char const* sourceCode = R"(
		contract Main {
			bytes s1;
			function doSet(bytes memory _s1) public returns (bytes memory _r1) {
				s1 = _s1;
				_r1 = s1;
			}
			function set(bytes calldata _s1) external returns (uint _r, bytes memory _r1) {
				_r1 = doSet(_s1);
				_r = _r1.length;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Main");
	string s1("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz");
	vector<size_t> lengths{0, 31, 64, 65};
	for (auto l1: lengths)
	{
		bytes dyn1 = encodeArgs(u256(l1), s1.substr(0, l1));
		bytes args1 = encodeArgs(u256(0x20)) + dyn1;
		BOOST_REQUIRE(
			callContractFunction("set(bytes)", asString(args1)) ==
			encodeArgs(u256(l1), u256(0x40)) + dyn1
		);
	}
}

BOOST_AUTO_TEST_CASE(bytes_index_access_memory)
{
	char const* sourceCode = R"(
		contract Main {
			function f(bytes memory _s1, uint i1, uint i2, uint i3) public returns (byte c1, byte c2, byte c3) {
				c1 = _s1[i1];
				c2 = intern(_s1, i2);
				c3 = internIndirect(_s1)[i3];
			}
			function intern(bytes memory _s1, uint i) public returns (byte c) {
				return _s1[i];
			}
			function internIndirect(bytes memory _s1) public returns (bytes memory) {
				return _s1;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Main");
	string s1("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz");
	bytes dyn1 = encodeArgs(u256(s1.length()), s1);
	bytes args1 = encodeArgs(u256(0x80), u256(3), u256(4), u256(5)) + dyn1;
	BOOST_REQUIRE(
		callContractFunction("f(bytes,uint256,uint256,uint256)", asString(args1)) ==
		encodeArgs(string{s1[3]}, string{s1[4]}, string{s1[5]})
	);
}

BOOST_AUTO_TEST_CASE(bytes_in_constructors_unpacker)
{
	char const* sourceCode = R"(
		contract Test {
			uint public m_x;
			bytes public m_s;
			constructor(uint x, bytes memory s) public {
				m_x = x;
				m_s = s;
			}
		}
	)";
	string s1("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz");
	bytes dyn1 = encodeArgs(u256(s1.length()), s1);
	u256 x = 7;
	bytes args1 = encodeArgs(x, u256(0x40)) + dyn1;
	compileAndRun(sourceCode, 0, "Test", args1);
	BOOST_REQUIRE(callContractFunction("m_x()") == encodeArgs(x));
	BOOST_REQUIRE(callContractFunction("m_s()") == encodeArgs(u256(0x20)) + dyn1);
}

BOOST_AUTO_TEST_CASE(bytes_in_constructors_packer)
{
	char const* sourceCode = R"(
		contract Base {
			uint public m_x;
			bytes m_s;
			constructor(uint x, bytes memory s) public {
				m_x = x;
				m_s = s;
			}
			function part(uint i) public returns (byte) {
				return m_s[i];
			}
		}
		contract Main is Base {
			constructor(bytes memory s, uint x) Base(x, f(s)) public {}
			function f(bytes memory s) public returns (bytes memory) {
				return s;
			}
		}
		contract Creator {
			function f(uint x, bytes memory s) public returns (uint r, byte ch) {
				Main c = new Main(s, x);
				r = c.m_x();
				ch = c.part(x);
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Creator");
	string s1("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz");
	bytes dyn1 = encodeArgs(u256(s1.length()), s1);
	u256 x = 7;
	bytes args1 = encodeArgs(x, u256(0x40)) + dyn1;
	BOOST_REQUIRE(
		callContractFunction("f(uint256,bytes)", asString(args1)) ==
		encodeArgs(x, string{s1[unsigned(x)]})
	);
}

BOOST_AUTO_TEST_CASE(arrays_in_constructors)
{
	char const* sourceCode = R"(
		contract Base {
			uint public m_x;
			address[] m_s;
			constructor(uint x, address[] memory s) public {
				m_x = x;
				m_s = s;
			}
			function part(uint i) public returns (address) {
				return m_s[i];
			}
		}
		contract Main is Base {
			constructor(address[] memory s, uint x) Base(x, f(s)) public {}
			function f(address[] memory s) public returns (address[] memory) {
				return s;
			}
		}
		contract Creator {
			function f(uint x, address[] memory s) public returns (uint r, address ch) {
				Main c = new Main(s, x);
				r = c.m_x();
				ch = c.part(x);
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Creator");
	vector<u256> s1{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	bytes dyn1 = encodeArgs(u256(s1.size()), s1);
	u256 x = 7;
	bytes args1 = encodeArgs(x, u256(0x40)) + dyn1;
	BOOST_REQUIRE(
		callContractFunction("f(uint256,address[])", asString(args1)) ==
		encodeArgs(x, s1[unsigned(x)])
	);
}

BOOST_AUTO_TEST_CASE(fixed_arrays_in_constructors)
{
	char const* sourceCode = R"(
		contract Creator {
			uint public r;
			address public ch;
			constructor(address[3] memory s, uint x) public {
				r = x;
				ch = s[2];
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Creator", encodeArgs(u256(1), u256(2), u256(3), u256(4)));
	BOOST_REQUIRE(callContractFunction("r()") == encodeArgs(u256(4)));
	BOOST_REQUIRE(callContractFunction("ch()") == encodeArgs(u256(3)));
}

BOOST_AUTO_TEST_CASE(arrays_from_and_to_storage)
{
	char const* sourceCode = R"(
		contract Test {
			uint24[] public data;
			function set(uint24[] memory _data) public returns (uint) {
				data = _data;
				return data.length;
			}
			function get() public returns (uint24[] memory) {
				return data;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Test");

	vector<u256> data{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18};
	BOOST_REQUIRE(
		callContractFunction("set(uint24[])", u256(0x20), u256(data.size()), data) ==
		encodeArgs(u256(data.size()))
	);
	ABI_CHECK(callContractFunction("data(uint256)", u256(7)), encodeArgs(u256(8)));
	ABI_CHECK(callContractFunction("data(uint256)", u256(15)), encodeArgs(u256(16)));
	ABI_CHECK(callContractFunction("data(uint256)", u256(18)), encodeArgs());
	ABI_CHECK(callContractFunction("get()"), encodeArgs(u256(0x20), u256(data.size()), data));
}

BOOST_AUTO_TEST_CASE(arrays_complex_from_and_to_storage)
{
	char const* sourceCode = R"(
		contract Test {
			uint24[3][] public data;
			function set(uint24[3][] memory _data) public returns (uint) {
				data = _data;
				return data.length;
			}
			function get() public returns (uint24[3][] memory) {
				return data;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Test");

	vector<u256> data{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18};
	BOOST_REQUIRE(
		callContractFunction("set(uint24[3][])", u256(0x20), u256(data.size() / 3), data) ==
		encodeArgs(u256(data.size() / 3))
	);
	ABI_CHECK(callContractFunction("data(uint256,uint256)", u256(2), u256(2)), encodeArgs(u256(9)));
	ABI_CHECK(callContractFunction("data(uint256,uint256)", u256(5), u256(1)), encodeArgs(u256(17)));
	ABI_CHECK(callContractFunction("data(uint256,uint256)", u256(6), u256(0)), encodeArgs());
	ABI_CHECK(callContractFunction("get()"), encodeArgs(u256(0x20), u256(data.size() / 3), data));
}

BOOST_AUTO_TEST_CASE(arrays_complex_memory_index_access)
{
	char const* sourceCode = R"(
		contract Test {
			function set(uint24[3][] memory _data, uint a, uint b) public returns (uint l, uint e) {
				l = _data.length;
				e = _data[a][b];
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Test");

	vector<u256> data{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18};
	BOOST_REQUIRE(callContractFunction(
			"set(uint24[3][],uint256,uint256)",
			u256(0x60),
			u256(3),
			u256(2),
			u256(data.size() / 3),
			data
	) == encodeArgs(u256(data.size() / 3), u256(data[3 * 3 + 2])));
}

BOOST_AUTO_TEST_CASE(bytes_memory_index_access)
{
	char const* sourceCode = R"(
		contract Test {
			function set(bytes memory _data, uint i) public returns (uint l, byte c) {
				l = _data.length;
				c = _data[i];
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Test");

	string data("abcdefgh");
	BOOST_REQUIRE(callContractFunction(
			"set(bytes,uint256)",
			u256(0x40),
			u256(3),
			u256(data.size()),
			data
	) == encodeArgs(u256(data.size()), string("d")));
}

BOOST_AUTO_TEST_CASE(storage_array_ref)
{
	char const* sourceCode = R"(
		contract BinarySearch {
		  /// Finds the position of _value in the sorted list _data.
		  /// Note that "internal" is important here, because storage references only work for internal or private functions
		  function find(uint[] storage _data, uint _value) internal returns (uint o_position) {
			return find(_data, 0, _data.length, _value);
		  }
		  function find(uint[] storage _data, uint _begin, uint _len, uint _value) private returns (uint o_position) {
			if (_len == 0 || (_len == 1 && _data[_begin] != _value))
			  return uint(-1); // failure
			uint halfLen = _len / 2;
			uint v = _data[_begin + halfLen];
			if (_value < v)
			  return find(_data, _begin, halfLen, _value);
			else if (_value > v)
			  return find(_data, _begin + halfLen + 1, halfLen - 1, _value);
			else
			  return _begin + halfLen;
		  }
		}

		contract Store is BinarySearch {
			uint[] data;
			function add(uint v) public {
				data.length++;
				data[data.length - 1] = v;
			}
			function find(uint v) public returns (uint) {
				return find(data, v);
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Store");
	BOOST_REQUIRE(callContractFunction("find(uint256)", u256(7)) == encodeArgs(u256(-1)));
	BOOST_REQUIRE(callContractFunction("add(uint256)", u256(7)) == encodeArgs());
	BOOST_REQUIRE(callContractFunction("find(uint256)", u256(7)) == encodeArgs(u256(0)));
	ABI_CHECK(callContractFunction("add(uint256)", u256(11)), encodeArgs());
	ABI_CHECK(callContractFunction("add(uint256)", u256(17)), encodeArgs());
	ABI_CHECK(callContractFunction("add(uint256)", u256(27)), encodeArgs());
	ABI_CHECK(callContractFunction("add(uint256)", u256(31)), encodeArgs());
	ABI_CHECK(callContractFunction("add(uint256)", u256(32)), encodeArgs());
	ABI_CHECK(callContractFunction("add(uint256)", u256(66)), encodeArgs());
	ABI_CHECK(callContractFunction("add(uint256)", u256(177)), encodeArgs());
	ABI_CHECK(callContractFunction("find(uint256)", u256(7)), encodeArgs(u256(0)));
	ABI_CHECK(callContractFunction("find(uint256)", u256(27)), encodeArgs(u256(3)));
	ABI_CHECK(callContractFunction("find(uint256)", u256(32)), encodeArgs(u256(5)));
	ABI_CHECK(callContractFunction("find(uint256)", u256(176)), encodeArgs(u256(-1)));
	ABI_CHECK(callContractFunction("find(uint256)", u256(0)), encodeArgs(u256(-1)));
	ABI_CHECK(callContractFunction("find(uint256)", u256(400)), encodeArgs(u256(-1)));
}

BOOST_AUTO_TEST_CASE(memory_types_initialisation)
{
	char const* sourceCode = R"(
		contract Test {
			mapping(uint=>uint) data;
			function stat() public returns (uint[5] memory)
			{
				data[2] = 3; // make sure to use some memory
			}
			function dyn() public returns (uint[] memory) { stat(); }
			function nested() public returns (uint[3][] memory) { stat(); }
			function nestedStat() public returns (uint[3][7] memory) { stat(); }
		}
	)";
	compileAndRun(sourceCode, 0, "Test");

	ABI_CHECK(callContractFunction("stat()"), encodeArgs(vector<u256>(5)));
	ABI_CHECK(callContractFunction("dyn()"), encodeArgs(u256(0x20), u256(0)));
	ABI_CHECK(callContractFunction("nested()"), encodeArgs(u256(0x20), u256(0)));
	ABI_CHECK(callContractFunction("nestedStat()"), encodeArgs(vector<u256>(3 * 7)));
}

BOOST_AUTO_TEST_CASE(memory_arrays_delete)
{
	char const* sourceCode = R"(
		contract Test {
			function del() public returns (uint24[3][4] memory) {
				uint24[3][4] memory x;
				for (uint24 i = 0; i < x.length; i ++)
					for (uint24 j = 0; j < x[i].length; j ++)
						x[i][j] = i * 0x10 + j;
				delete x[1];
				delete x[3][2];
				return x;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Test");

	vector<u256> data(3 * 4);
	for (unsigned i = 0; i < 4; i++)
		for (unsigned j = 0; j < 3; j++)
		{
			u256 v = 0;
			if (!(i == 1 || (i == 3 && j == 2)))
				v = i * 0x10 + j;
			data[i * 3 + j] = v;
		}
	ABI_CHECK(callContractFunction("del()"), encodeArgs(data));
}

BOOST_AUTO_TEST_CASE(memory_arrays_index_access_write)
{
	char const* sourceCode = R"(
		contract Test {
			function set(uint24[3][4] memory x) public {
				x[2][2] = 1;
				x[3][2] = 7;
			}
			function f() public returns (uint24[3][4] memory){
				uint24[3][4] memory data;
				set(data);
				return data;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Test");

	vector<u256> data(3 * 4);
	data[3 * 2 + 2] = 1;
	data[3 * 3 + 2] = 7;
	ABI_CHECK(callContractFunction("f()"), encodeArgs(data));
}

BOOST_AUTO_TEST_CASE(memory_arrays_dynamic_index_access_write)
{
	char const* sourceCode = R"(
		contract Test {
			uint24[3][][4] data;
			function set(uint24[3][][4] memory x) internal returns (uint24[3][][4] memory) {
				x[1][2][2] = 1;
				x[1][3][2] = 7;
				return x;
			}
			function f() public returns (uint24[3][] memory) {
				data[1].length = 4;
				return set(data)[1];
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Test");

	vector<u256> data(3 * 4);
	data[3 * 2 + 2] = 1;
	data[3 * 3 + 2] = 7;
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(0x20), u256(4), data));
}

BOOST_AUTO_TEST_CASE(memory_structs_read_write)
{
	char const* sourceCode = R"(
		contract Test {
			struct S { uint8 x; uint16 y; uint z; uint8[2] a; }
			S[5] data;
			function testInit() public returns (uint8 x, uint16 y, uint z, uint8 a, bool flag) {
				S[2] memory d;
				x = d[0].x;
				y = d[0].y;
				z = d[0].z;
				a = d[0].a[1];
				flag = true;
			}
			function testCopyRead() public returns (uint8 x, uint16 y, uint z, uint8 a) {
				data[2].x = 1;
				data[2].y = 2;
				data[2].z = 3;
				data[2].a[1] = 4;
				S memory s = data[2];
				x = s.x;
				y = s.y;
				z = s.z;
				a = s.a[1];
			}
			function testAssign() public returns (uint8 x, uint16 y, uint z, uint8 a) {
				S memory s;
				s.x = 1;
				s.y = 2;
				s.z = 3;
				s.a[1] = 4;
				x = s.x;
				y = s.y;
				z = s.z;
				a = s.a[1];
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Test");

	ABI_CHECK(callContractFunction("testInit()"), encodeArgs(u256(0), u256(0), u256(0), u256(0), true));
	ABI_CHECK(callContractFunction("testCopyRead()"), encodeArgs(u256(1), u256(2), u256(3), u256(4)));
	ABI_CHECK(callContractFunction("testAssign()"), encodeArgs(u256(1), u256(2), u256(3), u256(4)));
}

BOOST_AUTO_TEST_CASE(memory_structs_as_function_args)
{
	char const* sourceCode = R"(
		contract Test {
			struct S { uint8 x; uint16 y; uint z; }
			function test() public returns (uint x, uint y, uint z) {
				S memory data = combine(1, 2, 3);
				x = extract(data, 0);
				y = extract(data, 1);
				z = extract(data, 2);
			}
			function extract(S memory s, uint which) internal returns (uint x) {
				if (which == 0) return s.x;
				else if (which == 1) return s.y;
				else return s.z;
			}
			function combine(uint8 x, uint16 y, uint z) internal returns (S memory s) {
				s.x = x;
				s.y = y;
				s.z = z;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Test");

	ABI_CHECK(callContractFunction("test()"), encodeArgs(u256(1), u256(2), u256(3)));
}

BOOST_AUTO_TEST_CASE(memory_structs_nested)
{
	char const* sourceCode = R"(
		contract Test {
			struct S { uint8 x; uint16 y; uint z; }
			struct X { uint8 x; S s; }
			function test() public returns (uint a, uint x, uint y, uint z) {
				X memory d = combine(1, 2, 3, 4);
				a = extract(d, 0);
				x = extract(d, 1);
				y = extract(d, 2);
				z = extract(d, 3);
			}
			function extract(X memory s, uint which) internal returns (uint x) {
				if (which == 0) return s.x;
				else if (which == 1) return s.s.x;
				else if (which == 2) return s.s.y;
				else return s.s.z;
			}
			function combine(uint8 a, uint8 x, uint16 y, uint z) internal returns (X memory s) {
				s.x = a;
				s.s.x = x;
				s.s.y = y;
				s.s.z = z;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Test");

	ABI_CHECK(callContractFunction("test()"), encodeArgs(u256(1), u256(2), u256(3), u256(4)));
}

BOOST_AUTO_TEST_CASE(memory_structs_nested_load)
{
	char const* sourceCode = R"(
		contract Test {
			struct S { uint8 x; uint16 y; uint z; }
			struct X { uint8 x; S s; uint8[2] a; }
			X m_x;
			function load() public returns (uint a, uint x, uint y, uint z, uint a1, uint a2) {
				m_x.x = 1;
				m_x.s.x = 2;
				m_x.s.y = 3;
				m_x.s.z = 4;
				m_x.a[0] = 5;
				m_x.a[1] = 6;
				X memory d = m_x;
				a = d.x;
				x = d.s.x;
				y = d.s.y;
				z = d.s.z;
				a1 = d.a[0];
				a2 = d.a[1];
			}
			function store() public returns (uint a, uint x, uint y, uint z, uint a1, uint a2) {
				X memory d;
				d.x = 1;
				d.s.x = 2;
				d.s.y = 3;
				d.s.z = 4;
				d.a[0] = 5;
				d.a[1] = 6;
				m_x = d;
				a = m_x.x;
				x = m_x.s.x;
				y = m_x.s.y;
				z = m_x.s.z;
				a1 = m_x.a[0];
				a2 = m_x.a[1];
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Test");

	auto out = encodeArgs(u256(1), u256(2), u256(3), u256(4), u256(5), u256(6));
	ABI_CHECK(callContractFunction("load()"), out);
	ABI_CHECK(callContractFunction("store()"), out);
}

BOOST_AUTO_TEST_CASE(struct_constructor_nested)
{
	char const* sourceCode = R"(
		contract C {
			struct X { uint x1; uint x2; }
			struct S { uint s1; uint[3] s2; X s3; }
			S s;
			constructor() public {
				uint[3] memory s2;
				s2[1] = 9;
				s = S(1, s2, X(4, 5));
			}
			function get() public returns (uint s1, uint[3] memory s2, uint x1, uint x2)
			{
				s1 = s.s1;
				s2 = s.s2;
				x1 = s.s3.x1;
				x2 = s.s3.x2;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");

	auto out = encodeArgs(u256(1), u256(0), u256(9), u256(0), u256(4), u256(5));
	ABI_CHECK(callContractFunction("get()"), out);
}

BOOST_AUTO_TEST_CASE(struct_named_constructor)
{
	char const* sourceCode = R"(
		contract C {
			struct S { uint a; bool x; }
			S public s;
			constructor() public {
				s = S({a: 1, x: true});
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");

	ABI_CHECK(callContractFunction("s()"), encodeArgs(u256(1), true));
}

BOOST_AUTO_TEST_CASE(literal_strings)
{
	char const* sourceCode = R"(
		contract Test {
			string public long;
			string public medium;
			string public short;
			string public empty;
			function f() public returns (string memory) {
				long = "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789001234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678900123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789001234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890";
				medium = "01234567890123456789012345678901234567890123456789012345678901234567890123456789";
				short = "123";
				empty = "";
				return "Hello, World!";
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Test");
	string longStr = "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789001234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678900123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789001234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890";
	string medium = "01234567890123456789012345678901234567890123456789012345678901234567890123456789";
	string shortStr = "123";
	string hello = "Hello, World!";

	ABI_CHECK(callContractFunction("f()"), encodeDyn(hello));
	ABI_CHECK(callContractFunction("long()"), encodeDyn(longStr));
	ABI_CHECK(callContractFunction("medium()"), encodeDyn(medium));
	ABI_CHECK(callContractFunction("short()"), encodeDyn(shortStr));
	ABI_CHECK(callContractFunction("empty()"), encodeDyn(string()));
}

BOOST_AUTO_TEST_CASE(initialise_string_constant)
{
	char const* sourceCode = R"(
		contract Test {
			string public short = "abcdef";
			string public long = "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789001234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678900123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789001234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890";
		}
	)";
	compileAndRun(sourceCode, 0, "Test");
	string longStr = "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789001234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678900123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789001234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890";
	string shortStr = "abcdef";

	ABI_CHECK(callContractFunction("long()"), encodeDyn(longStr));
	ABI_CHECK(callContractFunction("short()"), encodeDyn(shortStr));
}

BOOST_AUTO_TEST_CASE(memory_structs_with_mappings)
{
	char const* sourceCode = R"(
		contract Test {
			struct S { uint8 a; mapping(uint => uint) b; uint8 c; }
			S s;
			function f() public returns (uint) {
				S memory x;
				if (x.a != 0 || x.c != 0) return 1;
				x.a = 4; x.c = 5;
				s = x;
				if (s.a != 4 || s.c != 5) return 2;
				x = S(2, 3);
				if (x.a != 2 || x.c != 3) return 3;
				x = s;
				if (s.a != 4 || s.c != 5) return 4;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Test");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(0)));
}

BOOST_AUTO_TEST_CASE(string_bytes_conversion)
{
	char const* sourceCode = R"(
		contract Test {
			string s;
			bytes b;
			function f(string memory _s, uint n) public returns (byte) {
				b = bytes(_s);
				s = string(b);
				return bytes(s)[n];
			}
			function l() public returns (uint) { return bytes(s).length; }
		}
	)";
	compileAndRun(sourceCode, 0, "Test");
	ABI_CHECK(callContractFunction(
		"f(string,uint256)",
		u256(0x40),
		u256(2),
		u256(6),
		string("abcdef")
	), encodeArgs("c"));
	ABI_CHECK(callContractFunction("l()"), encodeArgs(u256(6)));
}

BOOST_AUTO_TEST_CASE(string_as_mapping_key)
{
	char const* sourceCode = R"(
		contract Test {
			mapping(string => uint) data;
			function set(string memory _s, uint _v) public { data[_s] = _v; }
			function get(string memory _s) public returns (uint) { return data[_s]; }
		}
	)";
	compileAndRun(sourceCode, 0, "Test");
	vector<string> strings{
		"Hello, World!",
		"Hello,                            World!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1111",
		"",
		"1"
	};
	for (unsigned i = 0; i < strings.size(); i++)
		ABI_CHECK(callContractFunction(
			"set(string,uint256)",
			u256(0x40),
			u256(7 + i),
			u256(strings[i].size()),
			strings[i]
		), encodeArgs());
	for (unsigned i = 0; i < strings.size(); i++)
		ABI_CHECK(callContractFunction(
			"get(string)",
			u256(0x20),
			u256(strings[i].size()),
			strings[i]
		), encodeArgs(u256(7 + i)));
}

BOOST_AUTO_TEST_CASE(accessor_for_state_variable)
{
	char const* sourceCode = R"(
		contract Lotto {
			uint public ticketPrice = 500;
		}
	)";

	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("ticketPrice()"), encodeArgs(u256(500)));
}

BOOST_AUTO_TEST_CASE(accessor_for_const_state_variable)
{
	char const* sourceCode = R"(
		contract Lotto{
			uint constant public ticketPrice = 555;
		}
	)";

	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("ticketPrice()"), encodeArgs(u256(555)));
}

BOOST_AUTO_TEST_CASE(state_variable_under_contract_name)
{
	char const* text = R"(
		contract Scope {
			uint stateVar = 42;

			function getStateVar() public view returns (uint stateVar) {
				stateVar = Scope.stateVar;
			}
		}
	)";
	compileAndRun(text);
	ABI_CHECK(callContractFunction("getStateVar()"), encodeArgs(u256(42)));
}

BOOST_AUTO_TEST_CASE(state_variable_local_variable_mixture)
{
	char const* sourceCode = R"(
		contract A {
			uint x = 1;
			uint y = 2;
			function a() public returns (uint x) {
				x = A.y;
			}
		}
	)";

	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("a()"), encodeArgs(u256(2)));
}

BOOST_AUTO_TEST_CASE(inherited_function) {
	char const* sourceCode = R"(
		contract A { function f() internal returns (uint) { return 1; } }
		contract B is A {
			function f() internal returns (uint) { return 2; }
			function g() public returns (uint) {
				return A.f();
			}
		}
	)";

	compileAndRun(sourceCode, 0, "B");
	ABI_CHECK(callContractFunction("g()"), encodeArgs(u256(1)));
}

BOOST_AUTO_TEST_CASE(inherited_function_from_a_library) {
	char const* sourceCode = R"(
		library A { function f() internal returns (uint) { return 1; } }
		contract B {
			function f() internal returns (uint) { return 2; }
			function g() public returns (uint) {
				return A.f();
			}
		}
	)";

	compileAndRun(sourceCode, 0, "B");
	ABI_CHECK(callContractFunction("g()"), encodeArgs(u256(1)));
}

BOOST_AUTO_TEST_CASE(inherited_constant_state_var)
{
	char const* sourceCode = R"(
		contract A {
			uint constant x = 7;
		}
		contract B is A {
			function f() public returns (uint) {
				return A.x;
			}
		}
	)";

	compileAndRun(sourceCode, 0, "B");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(7)));
}

BOOST_AUTO_TEST_CASE(multiple_inherited_state_vars)
{
	char const* sourceCode = R"(
		contract A {
			uint x = 7;
		}
		contract B {
			uint x = 9;
		}
		contract C is A, B {
			function a() public returns (uint) {
				return A.x;
			}
			function b() public returns (uint) {
				return B.x;
			}
			function a_set(uint _x) public returns (uint) {
				A.x = _x;
				return 1;
			}
			function b_set(uint _x) public returns (uint) {
				B.x = _x;
				return 1;
			}
		}
	)";

	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("a()"), encodeArgs(u256(7)));
	ABI_CHECK(callContractFunction("b()"), encodeArgs(u256(9)));
	ABI_CHECK(callContractFunction("a_set(uint256)", u256(1)), encodeArgs(u256(1)));
	ABI_CHECK(callContractFunction("b_set(uint256)", u256(3)), encodeArgs(u256(1)));
	ABI_CHECK(callContractFunction("a()"), encodeArgs(u256(1)));
	ABI_CHECK(callContractFunction("b()"), encodeArgs(u256(3)));
}

BOOST_AUTO_TEST_CASE(constant_string_literal)
{
	char const* sourceCode = R"(
		contract Test {
			bytes32 constant public b = "abcdefghijklmnopq";
			string constant public x = "abefghijklmnopqabcdefghijklmnopqabcdefghijklmnopqabca";

			constructor() public {
				string memory xx = x;
				bytes32 bb = b;
			}
			function getB() public returns (bytes32) { return b; }
			function getX() public returns (string memory) { return x; }
			function getX2() public returns (string memory r) { r = x; }
			function unused() public returns (uint) {
				"unusedunusedunusedunusedunusedunusedunusedunusedunusedunusedunusedunused";
				return 2;
			}
		}
	)";

	compileAndRun(sourceCode);
	string longStr = "abefghijklmnopqabcdefghijklmnopqabcdefghijklmnopqabca";
	string shortStr = "abcdefghijklmnopq";
	ABI_CHECK(callContractFunction("b()"), encodeArgs(shortStr));
	ABI_CHECK(callContractFunction("x()"), encodeDyn(longStr));
	ABI_CHECK(callContractFunction("getB()"), encodeArgs(shortStr));
	ABI_CHECK(callContractFunction("getX()"), encodeDyn(longStr));
	ABI_CHECK(callContractFunction("getX2()"), encodeDyn(longStr));
	ABI_CHECK(callContractFunction("unused()"), encodeArgs(2));
}

BOOST_AUTO_TEST_CASE(storage_string_as_mapping_key_without_variable)
{
	char const* sourceCode = R"(
		contract Test {
			mapping(string => uint) data;
			function f() public returns (uint) {
				data["abc"] = 2;
				return data["abc"];
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Test");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(2)));
}

BOOST_AUTO_TEST_CASE(library_call)
{
	char const* sourceCode = R"(
		library Lib { function m(uint x, uint y) public returns (uint) { return x * y; } }
		contract Test {
			function f(uint x) public returns (uint) {
				return Lib.m(x, 9);
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Lib");
	compileAndRun(sourceCode, 0, "Test", bytes(), map<string, Address>{{"Lib", m_contractAddress}});
	ABI_CHECK(callContractFunction("f(uint256)", u256(33)), encodeArgs(u256(33) * 9));
}

BOOST_AUTO_TEST_CASE(library_function_external)
{
	char const* sourceCode = R"(
		library Lib { function m(bytes calldata b) external pure returns (byte) { return b[2]; } }
		contract Test {
			function f(bytes memory b) public pure returns (byte) {
				return Lib.m(b);
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Lib");
	compileAndRun(sourceCode, 0, "Test", bytes(), map<string, Address>{{"Lib", m_contractAddress}});
	ABI_CHECK(callContractFunction("f(bytes)", u256(0x20), u256(5), "abcde"), encodeArgs("c"));
}

BOOST_AUTO_TEST_CASE(library_stray_values)
{
	char const* sourceCode = R"(
		library Lib { function m(uint x, uint y) public returns (uint) { return x * y; } }
		contract Test {
			function f(uint x) public returns (uint) {
				Lib;
				Lib.m;
				return x + 9;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Lib");
	compileAndRun(sourceCode, 0, "Test", bytes(), map<string, Address>{{"Lib", m_contractAddress}});
	ABI_CHECK(callContractFunction("f(uint256)", u256(33)), encodeArgs(u256(42)));
}

BOOST_AUTO_TEST_CASE(cross_contract_types)
{
	char const* sourceCode = R"(
		contract Lib { struct S {uint a; uint b; } }
		contract Test {
			function f() public returns (uint r) {
				Lib.S memory x = Lib.S({a: 2, b: 3});
				r = x.b;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Test");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(3)));
}

BOOST_AUTO_TEST_CASE(simple_throw)
{
	char const* sourceCode = R"(
		contract Test {
			function f(uint x) public returns (uint) {
				if (x > 10)
					return x + 10;
				else
					revert();
				return 2;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f(uint256)", u256(11)), encodeArgs(u256(21)));
	ABI_CHECK(callContractFunction("f(uint256)", u256(1)), encodeArgs());
}

BOOST_AUTO_TEST_CASE(strings_in_struct)
{
	char const* sourceCode = R"(
		contract buggystruct {
			Buggy public bug;

			struct Buggy {
				uint first;
				uint second;
				uint third;
				string last;
			}

			constructor() public {
				bug = Buggy(10, 20, 30, "asdfghjkl");
			}
			function getFirst() public returns (uint)
			{
				return bug.first;
			}
			function getSecond() public returns (uint)
			{
				return bug.second;
			}
			function getThird() public returns (uint)
			{
				return bug.third;
			}
			function getLast() public returns (string memory)
			{
				return bug.last;
			}
		}
		)";
	compileAndRun(sourceCode);
	string s = "asdfghjkl";
	ABI_CHECK(callContractFunction("getFirst()"), encodeArgs(u256(10)));
	ABI_CHECK(callContractFunction("getSecond()"), encodeArgs(u256(20)));
	ABI_CHECK(callContractFunction("getThird()"), encodeArgs(u256(30)));
	ABI_CHECK(callContractFunction("getLast()"), encodeDyn(s));
}

BOOST_AUTO_TEST_CASE(fixed_arrays_as_return_type)
{
	char const* sourceCode = R"(
		contract A {
			function f(uint16 input) public pure returns (uint16[5] memory arr)
			{
				arr[0] = input;
				arr[1] = ++input;
				arr[2] = ++input;
				arr[3] = ++input;
				arr[4] = ++input;
			}
		}
		contract B {
			function f() public returns (uint16[5] memory res, uint16[5] memory res2)
			{
				A a = new A();
				res = a.f(2);
				res2 = a.f(1000);
			}
		}
	)";
	compileAndRun(sourceCode, 0, "B");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(
		u256(2), u256(3), u256(4), u256(5), u256(6), // first return argument
		u256(1000), u256(1001), u256(1002), u256(1003), u256(1004)) // second return argument
	);
}

BOOST_AUTO_TEST_CASE(internal_types_in_library)
{
	char const* sourceCode = R"(
		library Lib {
			function find(uint16[] storage _haystack, uint16 _needle) public view returns (uint)
			{
				for (uint i = 0; i < _haystack.length; ++i)
					if (_haystack[i] == _needle)
						return i;
				return uint(-1);
			}
		}
		contract Test {
			mapping(string => uint16[]) data;
			function f() public returns (uint a, uint b)
			{
				data["abc"].length = 20;
				data["abc"][4] = 9;
				data["abc"][17] = 3;
				a = Lib.find(data["abc"], 9);
				b = Lib.find(data["abc"], 3);
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Lib");
	compileAndRun(sourceCode, 0, "Test", bytes(), map<string, Address>{{"Lib", m_contractAddress}});
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(4), u256(17)));
}

BOOST_AUTO_TEST_CASE(mapping_arguments_in_library)
{
	char const* sourceCode = R"(
		library Lib {
			function set(mapping(uint => uint) storage m, uint key, uint value) internal
			{
				m[key] = value;
			}
			function get(mapping(uint => uint) storage m, uint key) internal view returns (uint)
			{
				return m[key];
			}
		}
		contract Test {
			mapping(uint => uint) m;
			function set(uint256 key, uint256 value) public returns (uint)
			{
				uint oldValue = Lib.get(m, key);
				Lib.set(m, key, value);
				return oldValue;
			}
			function get(uint256 key) public view returns (uint) {
				return Lib.get(m, key);
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Lib");
	compileAndRun(sourceCode, 0, "Test", bytes(), map<string, Address>{{"Lib", m_contractAddress}});
	ABI_CHECK(callContractFunction("set(uint256,uint256)", u256(1), u256(42)), encodeArgs(u256(0)));
	ABI_CHECK(callContractFunction("set(uint256,uint256)", u256(2), u256(84)), encodeArgs(u256(0)));
	ABI_CHECK(callContractFunction("set(uint256,uint256)", u256(21), u256(7)), encodeArgs(u256(0)));
	ABI_CHECK(callContractFunction("get(uint256)", u256(0)), encodeArgs(u256(0)));
	ABI_CHECK(callContractFunction("get(uint256)", u256(1)), encodeArgs(u256(42)));
	ABI_CHECK(callContractFunction("get(uint256)", u256(2)), encodeArgs(u256(84)));
	ABI_CHECK(callContractFunction("get(uint256)", u256(21)), encodeArgs(u256(7)));
	ABI_CHECK(callContractFunction("set(uint256,uint256)", u256(1), u256(21)), encodeArgs(u256(42)));
	ABI_CHECK(callContractFunction("set(uint256,uint256)", u256(2), u256(42)), encodeArgs(u256(84)));
	ABI_CHECK(callContractFunction("set(uint256,uint256)", u256(21), u256(14)), encodeArgs(u256(7)));
	ABI_CHECK(callContractFunction("get(uint256)", u256(0)), encodeArgs(u256(0)));
	ABI_CHECK(callContractFunction("get(uint256)", u256(1)), encodeArgs(u256(21)));
	ABI_CHECK(callContractFunction("get(uint256)", u256(2)), encodeArgs(u256(42)));
	ABI_CHECK(callContractFunction("get(uint256)", u256(21)), encodeArgs(u256(14)));
}

BOOST_AUTO_TEST_CASE(mapping_returns_in_library)
{
	char const* sourceCode = R"(
		library Lib {
			function choose_mapping(mapping(uint => uint) storage a, mapping(uint => uint) storage b, bool c) internal pure returns(mapping(uint=>uint) storage)
			{
				return c ? a : b;
			}
		}
		contract Test {
			mapping(uint => uint) a;
			mapping(uint => uint) b;
			function set(bool choice, uint256 key, uint256 value) public returns (uint)
			{
				mapping(uint => uint) storage m = Lib.choose_mapping(a, b, choice);
				uint oldValue = m[key];
				m[key] = value;
				return oldValue;
			}
			function get(bool choice, uint256 key) public view returns (uint) {
				return Lib.choose_mapping(a, b, choice)[key];
			}
			function get_a(uint256 key) public view returns (uint) {
				return a[key];
			}
			function get_b(uint256 key) public view returns (uint) {
				return b[key];
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Lib");
	compileAndRun(sourceCode, 0, "Test", bytes(), map<string, Address>{{"Lib", m_contractAddress}});
	ABI_CHECK(callContractFunction("set(bool,uint256,uint256)", true, u256(1), u256(42)), encodeArgs(u256(0)));
	ABI_CHECK(callContractFunction("set(bool,uint256,uint256)", true, u256(2), u256(84)), encodeArgs(u256(0)));
	ABI_CHECK(callContractFunction("set(bool,uint256,uint256)", true, u256(21), u256(7)), encodeArgs(u256(0)));
	ABI_CHECK(callContractFunction("set(bool,uint256,uint256)", false, u256(1), u256(10)), encodeArgs(u256(0)));
	ABI_CHECK(callContractFunction("set(bool,uint256,uint256)", false, u256(2), u256(11)), encodeArgs(u256(0)));
	ABI_CHECK(callContractFunction("set(bool,uint256,uint256)", false, u256(21), u256(12)), encodeArgs(u256(0)));
	ABI_CHECK(callContractFunction("get(bool,uint256)", true, u256(0)), encodeArgs(u256(0)));
	ABI_CHECK(callContractFunction("get(bool,uint256)", true, u256(1)), encodeArgs(u256(42)));
	ABI_CHECK(callContractFunction("get(bool,uint256)", true, u256(2)), encodeArgs(u256(84)));
	ABI_CHECK(callContractFunction("get(bool,uint256)", true, u256(21)), encodeArgs(u256(7)));
	ABI_CHECK(callContractFunction("get_a(uint256)", u256(0)), encodeArgs(u256(0)));
	ABI_CHECK(callContractFunction("get_a(uint256)", u256(1)), encodeArgs(u256(42)));
	ABI_CHECK(callContractFunction("get_a(uint256)", u256(2)), encodeArgs(u256(84)));
	ABI_CHECK(callContractFunction("get_a(uint256)", u256(21)), encodeArgs(u256(7)));
	ABI_CHECK(callContractFunction("get(bool,uint256)", false, u256(0)), encodeArgs(u256(0)));
	ABI_CHECK(callContractFunction("get(bool,uint256)", false, u256(1)), encodeArgs(u256(10)));
	ABI_CHECK(callContractFunction("get(bool,uint256)", false, u256(2)), encodeArgs(u256(11)));
	ABI_CHECK(callContractFunction("get(bool,uint256)", false, u256(21)), encodeArgs(u256(12)));
	ABI_CHECK(callContractFunction("get_b(uint256)", u256(0)), encodeArgs(u256(0)));
	ABI_CHECK(callContractFunction("get_b(uint256)", u256(1)), encodeArgs(u256(10)));
	ABI_CHECK(callContractFunction("get_b(uint256)", u256(2)), encodeArgs(u256(11)));
	ABI_CHECK(callContractFunction("get_b(uint256)", u256(21)), encodeArgs(u256(12)));
	ABI_CHECK(callContractFunction("set(bool,uint256,uint256)", true, u256(1), u256(21)), encodeArgs(u256(42)));
	ABI_CHECK(callContractFunction("set(bool,uint256,uint256)", true, u256(2), u256(42)), encodeArgs(u256(84)));
	ABI_CHECK(callContractFunction("set(bool,uint256,uint256)", true, u256(21), u256(14)), encodeArgs(u256(7)));
	ABI_CHECK(callContractFunction("set(bool,uint256,uint256)", false, u256(1), u256(30)), encodeArgs(u256(10)));
	ABI_CHECK(callContractFunction("set(bool,uint256,uint256)", false, u256(2), u256(31)), encodeArgs(u256(11)));
	ABI_CHECK(callContractFunction("set(bool,uint256,uint256)", false, u256(21), u256(32)), encodeArgs(u256(12)));
	ABI_CHECK(callContractFunction("get_a(uint256)", u256(0)), encodeArgs(u256(0)));
	ABI_CHECK(callContractFunction("get_a(uint256)", u256(1)), encodeArgs(u256(21)));
	ABI_CHECK(callContractFunction("get_a(uint256)", u256(2)), encodeArgs(u256(42)));
	ABI_CHECK(callContractFunction("get_a(uint256)", u256(21)), encodeArgs(u256(14)));
	ABI_CHECK(callContractFunction("get(bool,uint256)", true, u256(0)), encodeArgs(u256(0)));
	ABI_CHECK(callContractFunction("get(bool,uint256)", true, u256(1)), encodeArgs(u256(21)));
	ABI_CHECK(callContractFunction("get(bool,uint256)", true, u256(2)), encodeArgs(u256(42)));
	ABI_CHECK(callContractFunction("get(bool,uint256)", true, u256(21)), encodeArgs(u256(14)));
	ABI_CHECK(callContractFunction("get_b(uint256)", u256(0)), encodeArgs(u256(0)));
	ABI_CHECK(callContractFunction("get_b(uint256)", u256(1)), encodeArgs(u256(30)));
	ABI_CHECK(callContractFunction("get_b(uint256)", u256(2)), encodeArgs(u256(31)));
	ABI_CHECK(callContractFunction("get_b(uint256)", u256(21)), encodeArgs(u256(32)));
	ABI_CHECK(callContractFunction("get(bool,uint256)", false, u256(0)), encodeArgs(u256(0)));
	ABI_CHECK(callContractFunction("get(bool,uint256)", false, u256(1)), encodeArgs(u256(30)));
	ABI_CHECK(callContractFunction("get(bool,uint256)", false, u256(2)), encodeArgs(u256(31)));
	ABI_CHECK(callContractFunction("get(bool,uint256)", false, u256(21)), encodeArgs(u256(32)));
}

BOOST_AUTO_TEST_CASE(mapping_returns_in_library_named)
{
	char const* sourceCode = R"(
		library Lib {
			function f(mapping(uint => uint) storage a, mapping(uint => uint) storage b) internal returns(mapping(uint=>uint) storage r)
			{
				r = a;
				r[1] = 42;
				r = b;
				r[1] = 21;
			}
		}
		contract Test {
			mapping(uint => uint) a;
			mapping(uint => uint) b;
			function f() public returns (uint, uint, uint, uint, uint, uint)
			{
				Lib.f(a, b)[2] = 84;
				return (a[0], a[1], a[2], b[0], b[1], b[2]);
			}
			function g() public returns (uint, uint, uint, uint, uint, uint)
			{
				mapping(uint => uint) storage m = Lib.f(a, b);
				m[2] = 17;
				return (a[0], a[1], a[2], b[0], b[1], b[2]);
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Lib");
	compileAndRun(sourceCode, 0, "Test", bytes(), map<string, Address>{{"Lib", m_contractAddress}});
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(0), u256(42), u256(0), u256(0), u256(21), u256(84)));
	ABI_CHECK(callContractFunction("g()"), encodeArgs(u256(0), u256(42), u256(0), u256(0), u256(21), u256(17)));
}

BOOST_AUTO_TEST_CASE(using_library_structs)
{
	char const* sourceCode = R"(
		library Lib {
			struct Data { uint a; uint[] b; }
			function set(Data storage _s) public
			{
				_s.a = 7;
				_s.b.length = 20;
				_s.b[19] = 8;
			}
		}
		contract Test {
			mapping(string => Lib.Data) data;
			function f() public returns (uint a, uint b)
			{
				Lib.set(data["abc"]);
				a = data["abc"].a;
				b = data["abc"].b[19];
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Lib");
	compileAndRun(sourceCode, 0, "Test", bytes(), map<string, Address>{{"Lib", m_contractAddress}});
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(7), u256(8)));
}

BOOST_AUTO_TEST_CASE(library_struct_as_an_expression)
{
	char const* sourceCode = R"(
		library Arst {
			struct Foo {
				int Things;
				int Stuff;
			}
		}

		contract Tsra {
			function f() public returns(uint) {
				Arst.Foo;
				return 1;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Tsra");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(1)));
}

BOOST_AUTO_TEST_CASE(library_enum_as_an_expression)
{
	char const* sourceCode = R"(
		library Arst {
			enum Foo {
				Things,
				Stuff
			}
		}

		contract Tsra {
			function f() public returns(uint) {
				Arst.Foo;
				return 1;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "Tsra");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(1)));
}

BOOST_AUTO_TEST_CASE(short_strings)
{
	// This test verifies that the byte array encoding that combines length and data works
	// correctly.
	char const* sourceCode = R"(
		contract A {
			bytes public data1 = "123";
			bytes data2;
			function lengthChange() public returns (uint)
			{
				// store constant in short and long string
				data1 = "123";
				if (!equal(data1, "123")) return 1;
				data2 = "12345678901234567890123456789012345678901234567890a";
				if (data2[17] != "8") return 3;
				if (data2.length != 51) return 4;
				if (data2[data2.length - 1] != "a") return 5;
				// change length: short -> short
				data1.length = 5;
				if (data1.length != 5) return 6;
				data1[4] = "4";
				if (data1[0] != "1") return 7;
				if (data1[4] != "4") return 8;
				// change length: short -> long
				data1.length = 80;
				if (data1.length != 80) return 9;
				data1.length = 70;
				if (data1.length != 70) return 9;
				if (data1[0] != "1") return 10;
				if (data1[4] != "4") return 11;
				for (uint i = 0; i < data1.length; i ++)
					data1[i] = byte(uint8(i * 3));
				if (uint8(data1[4]) != 4 * 3) return 12;
				if (uint8(data1[67]) != 67 * 3) return 13;
				// change length: long -> short
				data1.length = 22;
				if (data1.length != 22) return 14;
				if (uint8(data1[21]) != 21 * 3) return 15;
				if (uint8(data1[2]) != 2 * 3) return 16;
				// change length: short -> shorter
				data1.length = 19;
				if (data1.length != 19) return 17;
				if (uint8(data1[7]) != 7 * 3) return 18;
				// and now again to original size
				data1.length = 22;
				if (data1.length != 22) return 19;
				if (data1[21] != 0) return 20;
				data1.length = 0;
				data2.length = 0;
			}
			function copy() public returns (uint) {
				bytes memory x = "123";
				bytes memory y = "012345678901234567890123456789012345678901234567890123456789";
				bytes memory z = "1234567";
				data1 = x;
				data2 = y;
				if (!equal(data1, x)) return 1;
				if (!equal(data2, y)) return 2;
				// lengthen
				data1 = y;
				if (!equal(data1, y)) return 3;
				// shorten
				data1 = x;
				if (!equal(data1, x)) return 4;
				// change while keeping short
				data1 = z;
				if (!equal(data1, z)) return 5;
				// copy storage -> storage
				data1 = x;
				data2 = y;
				// lengthen
				data1 = data2;
				if (!equal(data1, y)) return 6;
				// shorten
				data1 = x;
				data2 = data1;
				if (!equal(data2, x)) return 7;
				bytes memory c = data2;
				data1 = c;
				if (!equal(data1, x)) return 8;
				data1 = "";
				data2 = "";
			}
			function deleteElements() public returns (uint) {
				data1 = "01234";
				delete data1[2];
				if (data1[2] != 0) return 1;
				if (data1[0] != "0") return 2;
				if (data1[3] != "3") return 3;
				delete data1;
				if (data1.length != 0) return 4;
			}

			function equal(bytes storage a, bytes memory b) internal returns (bool) {
				if (a.length != b.length) return false;
				for (uint i = 0; i < a.length; ++i) if (a[i] != b[i]) return false;
				return true;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "A");
	ABI_CHECK(callContractFunction("data1()"), encodeDyn(string("123")));
	ABI_CHECK(callContractFunction("lengthChange()"), encodeArgs(u256(0)));
	BOOST_CHECK(storageEmpty(m_contractAddress));
	ABI_CHECK(callContractFunction("deleteElements()"), encodeArgs(u256(0)));
	BOOST_CHECK(storageEmpty(m_contractAddress));
	ABI_CHECK(callContractFunction("copy()"), encodeArgs(u256(0)));
	BOOST_CHECK(storageEmpty(m_contractAddress));
}

BOOST_AUTO_TEST_CASE(calldata_offset)
{
	// This tests a specific bug that was caused by not using the correct memory offset in the
	// calldata unpacker.
	char const* sourceCode = R"(
		contract CB
		{
			address[] _arr;
			string public last = "nd";
			constructor(address[] memory guardians) public
			{
				_arr = guardians;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "CB", encodeArgs(u256(0x20)));
	ABI_CHECK(callContractFunction("last()", encodeArgs()), encodeDyn(string("nd")));
}

BOOST_AUTO_TEST_CASE(contract_binary_dependencies)
{
	char const* sourceCode = R"(
		contract A { function f() public { new B(); } }
		contract B { function f() public { } }
		contract C { function f() public { new B(); } }
	)";
	compileAndRun(sourceCode);
}

BOOST_AUTO_TEST_CASE(reject_ether_sent_to_library)
{
	char const* sourceCode = R"(
		library lib {}
		contract c {
			constructor() public payable {}
			function f(address payable x) public returns (bool) {
				return x.send(1);
			}
			function () external payable {}
		}
	)";
	compileAndRun(sourceCode, 0, "lib");
	Address libraryAddress = m_contractAddress;
	compileAndRun(sourceCode, 10, "c");
	BOOST_CHECK_EQUAL(balanceAt(m_contractAddress), 10);
	BOOST_CHECK_EQUAL(balanceAt(libraryAddress), 0);
	ABI_CHECK(callContractFunction("f(address)", encodeArgs(u160(libraryAddress))), encodeArgs(false));
	BOOST_CHECK_EQUAL(balanceAt(m_contractAddress), 10);
	BOOST_CHECK_EQUAL(balanceAt(libraryAddress), 0);
	ABI_CHECK(callContractFunction("f(address)", encodeArgs(u160(m_contractAddress))), encodeArgs(true));
	BOOST_CHECK_EQUAL(balanceAt(m_contractAddress), 10);
	BOOST_CHECK_EQUAL(balanceAt(libraryAddress), 0);
}

BOOST_AUTO_TEST_CASE(multi_variable_declaration)
{
	char const* sourceCode = R"(
		contract C {
			function g() public returns (uint a, uint b, uint c) {
				a = 1; b = 2; c = 3;
			}
			function h() public returns (uint a, uint b, uint c, uint d) {
				a = 1; b = 2; c = 3; d = 4;
			}
			function f1() public returns (bool) {
				(uint x, uint y, uint z) = g();
				if (x != 1 || y != 2 || z != 3) return false;
				(, uint a,) = g();
				if (a != 2) return false;
				(uint b, , ) = g();
				if (b != 1) return false;
				(, , uint c) = g();
				if (c != 3) return false;
				return true;
			}
			function f2() public returns (bool) {
				(uint a1, , uint a3, ) = h();
				if (a1 != 1 || a3 != 3) return false;
				(uint b1, uint b2, , ) = h();
				if (b1 != 1 || b2 != 2) return false;
				(, uint c2, uint c3, ) = h();
				if (c2 != 2 || c3 != 3) return false;
				(, , uint d3, uint d4) = h();
				if (d3 != 3 || d4 != 4) return false;
				(uint e1, , uint e3, uint e4) = h();
				if (e1 != 1 || e3 != 3 || e4 != 4) return false;
				return true;
			}
			function f() public returns (bool) {
				return f1() && f2();
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f()", encodeArgs()), encodeArgs(true));
}

BOOST_AUTO_TEST_CASE(typed_multi_variable_declaration)
{
	char const* sourceCode = R"(
		contract C {
			struct S { uint x; }
			S s;
			function g() internal returns (uint, S storage, uint) {
				s.x = 7;
				return (1, s, 2);
			}
			function f() public returns (bool) {
				(uint x1, S storage y1, uint z1) = g();
				if (x1 != 1 || y1.x != 7 || z1 != 2) return false;
				(, S storage y2,) = g();
				if (y2.x != 7) return false;
				(uint x2,,) = g();
				if (x2 != 1) return false;
				(,,uint z2) = g();
				if (z2 != 2) return false;
				return true;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f()", encodeArgs()), encodeArgs(true));
}

BOOST_AUTO_TEST_CASE(tuples)
{
	char const* sourceCode = R"(
		contract C {
			uint[] data;
			uint[] m_c;
			function g() internal returns (uint a, uint b, uint[] storage c) {
				return (1, 2, data);
			}
			function h() external returns (uint a, uint b) {
				return (5, 6);
			}
			function f() public returns (uint) {
				data.length = 1;
				data[0] = 3;
				uint a; uint b;
				(a, b) = this.h();
				if (a != 5 || b != 6) return 1;
				uint[] storage c = m_c;
				(a, b, c) = g();
				if (a != 1 || b != 2 || c[0] != 3) return 2;
				(a, b) = (b, a);
				if (a != 2 || b != 1) return 3;
				(a, , b, , ) = (8, 9, 10, 11, 12);
				if (a != 8 || b != 10) return 4;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(0)));
}

BOOST_AUTO_TEST_CASE(string_tuples)
{
	char const* sourceCode = R"(
		contract C {
			function f() public returns (string memory, uint) {
				return ("abc", 8);
			}
			function g() public returns (string memory, string memory) {
				return (h(), "def");
			}
			function h() public returns (string memory) {
				return ("abc");
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(0x40), u256(8), u256(3), string("abc")));
	ABI_CHECK(callContractFunction("g()"), encodeArgs(u256(0x40), u256(0x80), u256(3), string("abc"), u256(3), string("def")));
}

BOOST_AUTO_TEST_CASE(decayed_tuple)
{
	char const* sourceCode = R"(
		contract C {
			function f() public returns (uint) {
				uint x = 1;
				(x) = 2;
				return x;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(2)));
}

BOOST_AUTO_TEST_CASE(inline_tuple_with_rational_numbers)
{
	char const* sourceCode = R"(
		contract c {
			function f() public returns (int8) {
				int8[5] memory foo3 = [int8(1), -1, 0, 0, 0];
				return foo3[0];
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(1)));
}

BOOST_AUTO_TEST_CASE(destructuring_assignment)
{
	char const* sourceCode = R"(
		contract C {
			uint x = 7;
			bytes data;
			uint[] y;
			uint[] arrayData;
			function returnsArray() public returns (uint[] memory) {
				arrayData.length = 9;
				arrayData[2] = 5;
				arrayData[7] = 4;
				return arrayData;
			}
			function f(bytes memory s) public returns (uint) {
				uint loc;
				uint[] memory memArray;
				(loc, x, y, data, arrayData[3]) = (8, 4, returnsArray(), s, 2);
				if (loc != 8) return 1;
				if (x != 4) return 2;
				if (y.length != 9) return 3;
				if (y[2] != 5) return 4;
				if (y[7] != 4) return 5;
				if (data.length != s.length) return 6;
				if (data[3] != s[3]) return 7;
				if (arrayData[3] != 2) return 8;
				(memArray, loc) = (arrayData, 3);
				if (loc != 3) return 9;
				if (memArray.length != arrayData.length) return 10;
				bytes memory memBytes;
				(x, memBytes, y[2], , ) = (456, s, 789, 101112, 131415);
				if (x != 456 || memBytes.length != s.length || y[2] != 789) return 11;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f(bytes)", u256(0x20), u256(5), string("abcde")), encodeArgs(u256(0)));
}

BOOST_AUTO_TEST_CASE(lone_struct_array_type)
{
	char const* sourceCode = R"(
		contract C {
			struct s { uint a; uint b;}
			function f() public returns (uint) {
				s[7][]; // This is only the type, should not have any effect
				return 3;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(3)));
}

BOOST_AUTO_TEST_CASE(create_memory_array)
{
	char const* sourceCode = R"(
		contract C {
			struct S { uint[2] a; bytes b; }
			function f() public returns (byte, uint, uint, byte) {
				bytes memory x = new bytes(200);
				x[199] = 'A';
				uint[2][] memory y = new uint[2][](300);
				y[203][1] = 8;
				S[] memory z = new S[](180);
				z[170].a[1] = 4;
				z[170].b = new bytes(102);
				z[170].b[99] = 'B';
				return (x[199], y[203][1], z[170].a[1], z[170].b[99]);
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f()"), encodeArgs(string("A"), u256(8), u256(4), string("B")));
}

BOOST_AUTO_TEST_CASE(create_memory_array_allocation_size)
{
	// Check allocation size of byte array. Should be 32 plus length rounded up to next
	// multiple of 32
	char const* sourceCode = R"(
		contract C {
			function f() public pure returns (uint d1, uint d2, uint d3, uint memsize) {
				bytes memory b1 = new bytes(31);
				bytes memory b2 = new bytes(32);
				bytes memory b3 = new bytes(256);
				bytes memory b4 = new bytes(31);
				assembly {
					d1 := sub(b2, b1)
					d2 := sub(b3, b2)
					d3 := sub(b4, b3)
					memsize := msize()
				}
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f()"), encodeArgs(0x40, 0x40, 0x20 + 256, 0x260));
}

BOOST_AUTO_TEST_CASE(memory_arrays_of_various_sizes)
{
	// Computes binomial coefficients the chinese way
	char const* sourceCode = R"(
		contract C {
			function f(uint n, uint k) public returns (uint) {
				uint[][] memory rows = new uint[][](n + 1);
				for (uint i = 1; i <= n; i++) {
					rows[i] = new uint[](i);
					rows[i][0] = rows[i][rows[i].length - 1] = 1;
					for (uint j = 1; j < i - 1; j++)
						rows[i][j] = rows[i - 1][j - 1] + rows[i - 1][j];
				}
				return rows[n][k - 1];
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f(uint256,uint256)", encodeArgs(u256(3), u256(1))), encodeArgs(u256(1)));
	ABI_CHECK(callContractFunction("f(uint256,uint256)", encodeArgs(u256(9), u256(5))), encodeArgs(u256(70)));
}

BOOST_AUTO_TEST_CASE(create_multiple_dynamic_arrays)
{
	char const* sourceCode = R"(
		contract C {
			function f() public returns (uint) {
				uint[][] memory x = new uint[][](42);
				assert(x[0].length == 0);
				x[0] = new uint[](1);
				x[0][0] = 1;
				assert(x[4].length == 0);
				x[4] = new uint[](1);
				x[4][0] = 2;
				assert(x[10].length == 0);
				x[10] = new uint[](1);
				x[10][0] = 44;
				uint[][] memory y = new uint[][](24);
				assert(y[0].length == 0);
				y[0] = new uint[](1);
				y[0][0] = 1;
				assert(y[4].length == 0);
				y[4] = new uint[](1);
				y[4][0] = 2;
				assert(y[10].length == 0);
				y[10] = new uint[](1);
				y[10][0] = 88;
				if ((x[0][0] == y[0][0]) && (x[4][0] == y[4][0]) && (x[10][0] == 44) && (y[10][0] == 88))
					return 7;
				return 0;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(7)));
}

BOOST_AUTO_TEST_CASE(memory_overwrite)
{
	char const* sourceCode = R"(
		contract C {
			function f() public returns (bytes memory x) {
				x = "12345";
				x[3] = 0x61;
				x[0] = 0x62;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f()"), encodeDyn(string("b23a5")));
}

BOOST_AUTO_TEST_CASE(addmod_mulmod)
{
	char const* sourceCode = R"(
		contract C {
			function test() public returns (uint) {
				// Note that this only works because computation on literals is done using
				// unbounded integers.
				if ((2**255 + 2**255) % 7 != addmod(2**255, 2**255, 7))
					return 1;
				if ((2**255 + 2**255) % 7 != addmod(2**255, 2**255, 7))
					return 2;
				return 0;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("test()"), encodeArgs(u256(0)));
}

BOOST_AUTO_TEST_CASE(addmod_mulmod_zero)
{
	char const* sourceCode = R"(
		contract C {
			function f(uint d) public pure returns (uint) {
				addmod(1, 2, d);
				return 2;
			}
			function g(uint d) public pure returns (uint) {
				mulmod(1, 2, d);
				return 2;
			}
			function h() public pure returns (uint) {
				mulmod(0, 1, 2);
				mulmod(1, 0, 2);
				addmod(0, 1, 2);
				addmod(1, 0, 2);
				return 2;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f(uint)", 0), encodeArgs());
	ABI_CHECK(callContractFunction("g(uint)", 0), encodeArgs());
	ABI_CHECK(callContractFunction("h()"), encodeArgs(2));
}

BOOST_AUTO_TEST_CASE(divisiod_by_zero)
{
	char const* sourceCode = R"(
		contract C {
			function div(uint a, uint b) public returns (uint) {
				return a / b;
			}
			function mod(uint a, uint b) public returns (uint) {
				return a % b;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("div(uint256,uint256)", 7, 2), encodeArgs(u256(3)));
	// throws
	ABI_CHECK(callContractFunction("div(uint256,uint256)", 7, 0), encodeArgs());
	ABI_CHECK(callContractFunction("mod(uint256,uint256)", 7, 2), encodeArgs(u256(1)));
	// throws
	ABI_CHECK(callContractFunction("mod(uint256,uint256)", 7, 0), encodeArgs());
}

BOOST_AUTO_TEST_CASE(string_allocation_bug)
{
	char const* sourceCode = R"(
		contract Sample
		{
			struct s { uint16 x; uint16 y; string a; string b;}
			s[2] public p;
			constructor() public {
				s memory m;
				m.x = 0xbbbb;
				m.y = 0xcccc;
				m.a = "hello";
				m.b = "world";
				p[0] = m;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("p(uint256)", 0), encodeArgs(
		u256(0xbbbb),
		u256(0xcccc),
		u256(0x80),
		u256(0xc0),
		u256(5),
		string("hello"),
		u256(5),
		string("world")
	));
}

BOOST_AUTO_TEST_CASE(using_for_function_on_int)
{
	char const* sourceCode = R"(
		library D { function double(uint self) public returns (uint) { return 2*self; } }
		contract C {
			using D for uint;
			function f(uint a) public returns (uint) {
				return a.double();
			}
		}
	)";
	compileAndRun(sourceCode, 0, "D");
	compileAndRun(sourceCode, 0, "C", bytes(), map<string, Address>{{"D", m_contractAddress}});
	ABI_CHECK(callContractFunction("f(uint256)", u256(9)), encodeArgs(u256(2 * 9)));
}

BOOST_AUTO_TEST_CASE(using_for_function_on_struct)
{
	char const* sourceCode = R"(
		library D { struct s { uint a; } function mul(s storage self, uint x) public returns (uint) { return self.a *= x; } }
		contract C {
			using D for D.s;
			D.s public x;
			function f(uint a) public returns (uint) {
				x.a = 3;
				return x.mul(a);
			}
		}
	)";
	compileAndRun(sourceCode, 0, "D");
	compileAndRun(sourceCode, 0, "C", bytes(), map<string, Address>{{"D", m_contractAddress}});
	ABI_CHECK(callContractFunction("f(uint256)", u256(7)), encodeArgs(u256(3 * 7)));
	ABI_CHECK(callContractFunction("x()"), encodeArgs(u256(3 * 7)));
}

BOOST_AUTO_TEST_CASE(using_for_overload)
{
	char const* sourceCode = R"(
		library D {
			struct s { uint a; }
			function mul(s storage self, uint x) public returns (uint) { return self.a *= x; }
			function mul(s storage self, bytes32 x) public returns (bytes32) { }
		}
		contract C {
			using D for D.s;
			D.s public x;
			function f(uint a) public returns (uint) {
				x.a = 6;
				return x.mul(a);
			}
		}
	)";
	compileAndRun(sourceCode, 0, "D");
	compileAndRun(sourceCode, 0, "C", bytes(), map<string, Address>{{"D", m_contractAddress}});
	ABI_CHECK(callContractFunction("f(uint256)", u256(7)), encodeArgs(u256(6 * 7)));
	ABI_CHECK(callContractFunction("x()"), encodeArgs(u256(6 * 7)));
}

BOOST_AUTO_TEST_CASE(using_for_by_name)
{
	char const* sourceCode = R"(
		library D { struct s { uint a; } function mul(s storage self, uint x) public returns (uint) { return self.a *= x; } }
		contract C {
			using D for D.s;
			D.s public x;
			function f(uint a) public returns (uint) {
				x.a = 6;
				return x.mul({x: a});
			}
		}
	)";
	compileAndRun(sourceCode, 0, "D");
	compileAndRun(sourceCode, 0, "C", bytes(), map<string, Address>{{"D", m_contractAddress}});
	ABI_CHECK(callContractFunction("f(uint256)", u256(7)), encodeArgs(u256(6 * 7)));
	ABI_CHECK(callContractFunction("x()"), encodeArgs(u256(6 * 7)));
}

BOOST_AUTO_TEST_CASE(bound_function_in_var)
{
	char const* sourceCode = R"(
		library D { struct s { uint a; } function mul(s storage self, uint x) public returns (uint) { return self.a *= x; } }
		contract C {
			using D for D.s;
			D.s public x;
			function f(uint a) public returns (uint) {
				x.a = 6;
				return (x.mul)({x: a});
			}
		}
	)";
	compileAndRun(sourceCode, 0, "D");
	compileAndRun(sourceCode, 0, "C", bytes(), map<string, Address>{{"D", m_contractAddress}});
	ABI_CHECK(callContractFunction("f(uint256)", u256(7)), encodeArgs(u256(6 * 7)));
	ABI_CHECK(callContractFunction("x()"), encodeArgs(u256(6 * 7)));
}

BOOST_AUTO_TEST_CASE(bound_function_to_string)
{
	char const* sourceCode = R"(
		library D { function length(string memory self) public returns (uint) { return bytes(self).length; } }
		contract C {
			using D for string;
			string x;
			function f() public returns (uint) {
				x = "abc";
				return x.length();
			}
			function g() public returns (uint) {
				string memory s = "abc";
				return s.length();
			}
		}
	)";
	compileAndRun(sourceCode, 0, "D");
	compileAndRun(sourceCode, 0, "C", bytes(), map<string, Address>{{"D", m_contractAddress}});
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(3)));
	ABI_CHECK(callContractFunction("g()"), encodeArgs(u256(3)));
}

BOOST_AUTO_TEST_CASE(inline_array_storage_to_memory_conversion_strings)
{
	char const* sourceCode = R"(
		contract C {
			string s = "doh";
			function f() public returns (string memory, string memory) {
				string memory t = "ray";
				string[3] memory x = [s, t, "mi"];
				return (x[1], x[2]);
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(0x40), u256(0x80), u256(3), string("ray"), u256(2), string("mi")));
}

BOOST_AUTO_TEST_CASE(inline_array_strings_from_document)
{
	char const* sourceCode = R"(
		contract C {
			function f(uint i) public returns (string memory) {
				string[4] memory x = ["This", "is", "an", "array"];
				return (x[i]);
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f(uint256)", u256(0)), encodeArgs(u256(0x20), u256(4), string("This")));
	ABI_CHECK(callContractFunction("f(uint256)", u256(1)), encodeArgs(u256(0x20), u256(2), string("is")));
	ABI_CHECK(callContractFunction("f(uint256)", u256(2)), encodeArgs(u256(0x20), u256(2), string("an")));
	ABI_CHECK(callContractFunction("f(uint256)", u256(3)), encodeArgs(u256(0x20), u256(5), string("array")));
}

BOOST_AUTO_TEST_CASE(inline_array_storage_to_memory_conversion_ints)
{
	char const* sourceCode = R"(
		contract C {
			function f() public returns (uint x, uint y) {
				x = 3;
				y = 6;
				uint[2] memory z = [x, y];
				return (z[0], z[1]);
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(3, 6));
}

BOOST_AUTO_TEST_CASE(inline_array_index_access_ints)
{
	char const* sourceCode = R"(
		contract C {
			function f() public returns (uint) {
				return ([1, 2, 3, 4][2]);
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(3));
}

BOOST_AUTO_TEST_CASE(inline_array_index_access_strings)
{
	char const* sourceCode = R"(
		contract C {
			string public tester;
			function f() public returns (string memory) {
				return (["abc", "def", "g"][0]);
			}
			function test() public {
				tester = f();
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("test()"), encodeArgs());
	ABI_CHECK(callContractFunction("tester()"), encodeArgs(u256(0x20), u256(3), string("abc")));
}

BOOST_AUTO_TEST_CASE(inline_array_return)
{
	char const* sourceCode = R"(
		contract C {
			uint8[] tester;
			function f() public returns (uint8[5] memory) {
				return ([1,2,3,4,5]);
			}
			function test() public returns (uint8, uint8, uint8, uint8, uint8) {
				tester = f();
				return (tester[0], tester[1], tester[2], tester[3], tester[4]);
			}

		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(1, 2, 3, 4, 5));
}

BOOST_AUTO_TEST_CASE(inline_array_singleton)
{
	// This caused a failure since the type was not converted to its mobile type.
	char const* sourceCode = R"(
		contract C {
			function f() public returns (uint) {
				return [4][0];
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(4)));
}

BOOST_AUTO_TEST_CASE(inline_long_string_return)
{
		char const* sourceCode = R"(
		contract C {
			function f() public returns (string memory) {
				return (["somethingShort", "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789001234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678900123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789001234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"][1]);
			}
		}
	)";

	string strLong = "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789001234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678900123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789001234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeDyn(strLong));
}

BOOST_AUTO_TEST_CASE(fixed_bytes_index_access)
{
	char const* sourceCode = R"(
		contract C {
			bytes16[] public data;
			function f(bytes32 x) public returns (byte) {
				return x[2];
			}
			function g(bytes32 x) public returns (uint) {
				data = [x[0], x[1], x[2]];
				data[0] = "12345";
				return uint(uint8(data[0][4]));
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f(bytes32)", "789"), encodeArgs("9"));
	ABI_CHECK(callContractFunction("g(bytes32)", "789"), encodeArgs(u256(int('5'))));
	ABI_CHECK(callContractFunction("data(uint256)", u256(1)), encodeArgs("8"));
}

BOOST_AUTO_TEST_CASE(fixed_bytes_length_access)
{
	char const* sourceCode = R"(
		contract C {
			byte a;
			function f(bytes32 x) public returns (uint, uint, uint) {
				return (x.length, bytes16(uint128(2)).length, a.length + 7);
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f(bytes32)", "789"), encodeArgs(u256(32), u256(16), u256(8)));
}

BOOST_AUTO_TEST_CASE(inline_assembly_write_to_stack)
{
	char const* sourceCode = R"(
		contract C {
			function f() public returns (uint r, bytes32 r2) {
				assembly { r := 7 r2 := "abcdef" }
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(7), string("abcdef")));
}

BOOST_AUTO_TEST_CASE(inline_assembly_read_and_write_stack)
{
	char const* sourceCode = R"(
		contract C {
			function f() public returns (uint r) {
				for (uint x = 0; x < 10; ++x)
					assembly { r := add(r, x) }
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(45)));
}

BOOST_AUTO_TEST_CASE(inline_assembly_memory_access)
{
	char const* sourceCode = R"(
		contract C {
			function test() public returns (bytes memory) {
				bytes memory x = new bytes(5);
				for (uint i = 0; i < x.length; ++i)
					x[i] = byte(uint8(i + 1));
				assembly { mstore(add(x, 32), "12345678901234567890123456789012") }
				return x;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("test()"), encodeArgs(u256(0x20), u256(5), string("12345")));
}

BOOST_AUTO_TEST_CASE(inline_assembly_storage_access)
{
	char const* sourceCode = R"(
		contract C {
			uint16 x;
			uint16 public y;
			uint public z;
			function f() public returns (bool) {
				uint off1;
				uint off2;
				assembly {
					sstore(z_slot, 7)
					off1 := z_offset
					off2 := y_offset
				}
				assert(off1 == 0);
				assert(off2 == 2);
				return true;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(true));
	ABI_CHECK(callContractFunction("z()"), encodeArgs(u256(7)));
}

BOOST_AUTO_TEST_CASE(inline_assembly_storage_access_inside_function)
{
	char const* sourceCode = R"(
		contract C {
			uint16 x;
			uint16 public y;
			uint public z;
			function f() public returns (bool) {
				uint off1;
				uint off2;
				assembly {
					function f() -> o1 {
						sstore(z_slot, 7)
						o1 := y_offset
					}
					off2 := f()
				}
				assert(off2 == 2);
				return true;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(true));
	ABI_CHECK(callContractFunction("z()"), encodeArgs(u256(7)));
}

BOOST_AUTO_TEST_CASE(inline_assembly_storage_access_via_pointer)
{
	char const* sourceCode = R"(
		contract C {
			struct Data { uint contents; }
			uint public separator;
			Data public a;
			uint public separator2;
			function f() public returns (bool) {
				Data storage x = a;
				uint off;
				assembly {
					sstore(x_slot, 7)
					off := x_offset
				}
				assert(off == 0);
				return true;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(true));
	ABI_CHECK(callContractFunction("a()"), encodeArgs(u256(7)));
	ABI_CHECK(callContractFunction("separator()"), encodeArgs(u256(0)));
	ABI_CHECK(callContractFunction("separator2()"), encodeArgs(u256(0)));
}

BOOST_AUTO_TEST_CASE(inline_assembly_function_call)
{
	char const* sourceCode = R"(
		contract C {
			function f() public {
				assembly {
					function asmfun(a, b, c) -> x, y, z {
						x := a
						y := b
						z := 7
					}
					let a1, b1, c1 := asmfun(1, 2, 3)
					mstore(0x00, a1)
					mstore(0x20, b1)
					mstore(0x40, c1)
					return(0, 0x60)
				}
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(1), u256(2), u256(7)));
}

BOOST_AUTO_TEST_CASE(inline_assembly_function_call_assignment)
{
	char const* sourceCode = R"(
		contract C {
			function f() public {
				assembly {
					let a1, b1, c1
					function asmfun(a, b, c) -> x, y, z {
						x := a
						y := b
						z := 7
					}
					a1, b1, c1 := asmfun(1, 2, 3)
					mstore(0x00, a1)
					mstore(0x20, b1)
					mstore(0x40, c1)
					return(0, 0x60)
				}
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(1), u256(2), u256(7)));
}

BOOST_AUTO_TEST_CASE(inline_assembly_function_call2)
{
	char const* sourceCode = R"(
		contract C {
			function f() public {
				assembly {
					let d := 0x10
					function asmfun(a, b, c) -> x, y, z {
						x := a
						y := b
						z := 7
					}
					let a1, b1, c1 := asmfun(1, 2, 3)
					mstore(0x00, a1)
					mstore(0x20, b1)
					mstore(0x40, c1)
					mstore(0x60, d)
					return(0, 0x80)
				}
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(1), u256(2), u256(7), u256(0x10)));
}

BOOST_AUTO_TEST_CASE(inline_assembly_embedded_function_call)
{
	char const* sourceCode = R"(
		contract C {
			function f() public {
				assembly {
					let d := 0x10
					function asmfun(a, b, c) -> x, y, z {
						x := g(a)
						function g(r) -> s { s := mul(r, r) }
						y := g(b)
						z := 7
					}
					let a1, b1, c1 := asmfun(1, 2, 3)
					mstore(0x00, a1)
					mstore(0x20, b1)
					mstore(0x40, c1)
					mstore(0x60, d)
					return(0, 0x80)
				}
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(1), u256(4), u256(7), u256(0x10)));
}

BOOST_AUTO_TEST_CASE(inline_assembly_if)
{
	char const* sourceCode = R"(
		contract C {
			function f(uint a) public returns (uint b) {
				assembly {
					if gt(a, 1) { b := 2 }
				}
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f(uint256)", u256(0)), encodeArgs(u256(0)));
	ABI_CHECK(callContractFunction("f(uint256)", u256(1)), encodeArgs(u256(0)));
	ABI_CHECK(callContractFunction("f(uint256)", u256(2)), encodeArgs(u256(2)));
	ABI_CHECK(callContractFunction("f(uint256)", u256(3)), encodeArgs(u256(2)));
}

BOOST_AUTO_TEST_CASE(inline_assembly_switch)
{
	char const* sourceCode = R"(
		contract C {
			function f(uint a) public returns (uint b) {
				assembly {
					switch a
					case 1 { b := 8 }
					case 2 { b := 9 }
					default { b := 2 }
				}
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f(uint256)", u256(0)), encodeArgs(u256(2)));
	ABI_CHECK(callContractFunction("f(uint256)", u256(1)), encodeArgs(u256(8)));
	ABI_CHECK(callContractFunction("f(uint256)", u256(2)), encodeArgs(u256(9)));
	ABI_CHECK(callContractFunction("f(uint256)", u256(3)), encodeArgs(u256(2)));
}

BOOST_AUTO_TEST_CASE(inline_assembly_recursion)
{
	char const* sourceCode = R"(
		contract C {
			function f(uint a) public returns (uint b) {
				assembly {
					function fac(n) -> nf {
						switch n
						case 0 { nf := 1 }
						case 1 { nf := 1 }
						default { nf := mul(n, fac(sub(n, 1))) }
					}
					b := fac(a)
				}
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f(uint256)", u256(0)), encodeArgs(u256(1)));
	ABI_CHECK(callContractFunction("f(uint256)", u256(1)), encodeArgs(u256(1)));
	ABI_CHECK(callContractFunction("f(uint256)", u256(2)), encodeArgs(u256(2)));
	ABI_CHECK(callContractFunction("f(uint256)", u256(3)), encodeArgs(u256(6)));
	ABI_CHECK(callContractFunction("f(uint256)", u256(4)), encodeArgs(u256(24)));
}

BOOST_AUTO_TEST_CASE(inline_assembly_for)
{
	char const* sourceCode = R"(
		contract C {
			function f(uint a) public returns (uint b) {
				assembly {
					function fac(n) -> nf {
						nf := 1
						for { let i := n } gt(i, 0) { i := sub(i, 1) } {
							nf := mul(nf, i)
						}
					}
					b := fac(a)
				}
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f(uint256)", u256(0)), encodeArgs(u256(1)));
	ABI_CHECK(callContractFunction("f(uint256)", u256(1)), encodeArgs(u256(1)));
	ABI_CHECK(callContractFunction("f(uint256)", u256(2)), encodeArgs(u256(2)));
	ABI_CHECK(callContractFunction("f(uint256)", u256(3)), encodeArgs(u256(6)));
	ABI_CHECK(callContractFunction("f(uint256)", u256(4)), encodeArgs(u256(24)));
}

BOOST_AUTO_TEST_CASE(inline_assembly_for2)
{
	char const* sourceCode = R"(
		contract C {
			uint st;
			function f(uint a) public returns (uint b, uint c, uint d) {
				st = 0;
				assembly {
					function sideeffect(r) -> x { sstore(0, add(sload(0), r)) x := 1}
					for { let i := a } eq(i, sideeffect(2)) { d := add(d, 3) } {
						b := i
						i := 0
					}
				}
				c = st;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f(uint256)", u256(0)), encodeArgs(u256(0), u256(2), u256(0)));
	ABI_CHECK(callContractFunction("f(uint256)", u256(1)), encodeArgs(u256(1), u256(4), u256(3)));
	ABI_CHECK(callContractFunction("f(uint256)", u256(2)), encodeArgs(u256(0), u256(2), u256(0)));
}

BOOST_AUTO_TEST_CASE(index_access_with_type_conversion)
{
	// Test for a bug where higher order bits cleanup was not done for array index access.
	char const* sourceCode = R"(
			contract C {
				function f(uint x) public returns (uint[256] memory r){
					r[uint8(x)] = 2;
				}
			}
	)";
	compileAndRun(sourceCode, 0, "C");
	// neither of the two should throw due to out-of-bounds access
	BOOST_CHECK(callContractFunction("f(uint256)", u256(0x01)).size() == 256 * 32);
	BOOST_CHECK(callContractFunction("f(uint256)", u256(0x101)).size() == 256 * 32);
}

BOOST_AUTO_TEST_CASE(delete_on_array_of_structs)
{
	// Test for a bug where we did not increment the counter properly while deleting a dynamic array.
	char const* sourceCode = R"(
		contract C {
			struct S { uint x; uint[] y; }
			S[] data;
			function f() public returns (bool) {
				data.length = 2;
				data[0].x = 2**200;
				data[1].x = 2**200;
				delete data;
				return true;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	// This code interprets x as an array length and thus will go out of gas.
	// neither of the two should throw due to out-of-bounds access
	ABI_CHECK(callContractFunction("f()"), encodeArgs(true));

}

BOOST_AUTO_TEST_CASE(internal_library_function)
{
	// tests that internal library functions can be called from outside
	// and retain the same memory context (i.e. are pulled into the caller's code)
	char const* sourceCode = R"(
		library L {
			function f(uint[] memory _data) internal {
				_data[3] = 2;
			}
		}
		contract C {
			function f() public returns (uint) {
				uint[] memory x = new uint[](7);
				x[3] = 8;
				L.f(x);
				return x[3];
			}
		}
	)";
	// This has to work without linking, because everything will be inlined.
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(2)));
}

BOOST_AUTO_TEST_CASE(internal_library_function_calling_private)
{
	// tests that internal library functions that are called from outside and that
	// themselves call private functions are still able to (i.e. the private function
	// also has to be pulled into the caller's code)
	char const* sourceCode = R"(
		library L {
			function g(uint[] memory _data) private {
				_data[3] = 2;
			}
			function f(uint[] memory _data) internal {
				g(_data);
			}
		}
		contract C {
			function f() public returns (uint) {
				uint[] memory x = new uint[](7);
				x[3] = 8;
				L.f(x);
				return x[3];
			}
		}
	)";
	// This has to work without linking, because everything will be inlined.
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(2)));
}

BOOST_AUTO_TEST_CASE(internal_library_function_bound)
{
	char const* sourceCode = R"(
		library L {
			struct S { uint[] data; }
			function f(S memory _s) internal {
				_s.data[3] = 2;
			}
		}
		contract C {
			using L for L.S;
			function f() public returns (uint) {
				L.S memory x;
				x.data = new uint[](7);
				x.data[3] = 8;
				x.f();
				return x.data[3];
			}
		}
	)";
	// This has to work without linking, because everything will be inlined.
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(2)));
}

BOOST_AUTO_TEST_CASE(internal_library_function_return_var_size)
{
	char const* sourceCode = R"(
		library L {
			struct S { uint[] data; }
			function f(S memory _s) internal returns (uint[] memory) {
				_s.data[3] = 2;
				return _s.data;
			}
		}
		contract C {
			using L for L.S;
			function f() public returns (uint) {
				L.S memory x;
				x.data = new uint[](7);
				x.data[3] = 8;
				return x.f()[3];
			}
		}
	)";
	// This has to work without linking, because everything will be inlined.
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(2)));
}

BOOST_AUTO_TEST_CASE(iszero_bnot_correct)
{
	// A long time ago, some opcodes were renamed, which involved the opcodes
	// "iszero" and "not".
	char const* sourceCode = R"(
		contract C {
			function f() public returns (bool) {
				bytes32 x = bytes32(uint256(1));
				assembly { x := not(x) }
				if (x != ~bytes32(uint256(1))) return false;
				assembly { x := iszero(x) }
				if (x != bytes32(0)) return false;
				return true;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(true));
}

BOOST_AUTO_TEST_CASE(cleanup_bytes_types)
{
	// Checks that bytesXX types are properly cleaned before they are compared.
	char const* sourceCode = R"(
		contract C {
			function f(bytes2 a, uint16 x) public returns (uint) {
				if (a != "ab") return 1;
				if (x != 0x0102) return 2;
				if (bytes3(uint24(x)) != 0x000102) return 3;
				return 0;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	// We input longer data on purpose.
	ABI_CHECK(callContractFunction("f(bytes2,uint16)", string("abc"), u256(0x040102)), encodeArgs(0));
}

BOOST_AUTO_TEST_CASE(cleanup_bytes_types_shortening)
{
	char const* sourceCode = R"(
		contract C {
			function f() public pure returns (bytes32 r) {
				bytes4 x = 0xffffffff;
				bytes2 y = bytes2(x);
				assembly { r := y }
				// At this point, r and y both store four bytes, but
				// y is properly cleaned before the equality check
				require(y == bytes2(0xffff));
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs("\xff\xff\xff\xff"));
}

BOOST_AUTO_TEST_CASE(cleanup_address_types)
{
	// Checks that address types are properly cleaned before they are compared.
	char const* sourceCode = R"(
		contract C {
			function f(address a) public returns (uint) {
				if (a != 0x1234567890123456789012345678901234567890) return 1;
				return 0;
			}
			function g(address payable a) public returns (uint) {
				if (a != 0x1234567890123456789012345678901234567890) return 1;
				return 0;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	// We input longer data on purpose.
	ABI_CHECK(callContractFunction("f(address)", u256("0xFFFF1234567890123456789012345678901234567890")), encodeArgs(0));
	ABI_CHECK(callContractFunction("g(address)", u256("0xFFFF1234567890123456789012345678901234567890")), encodeArgs(0));
}

BOOST_AUTO_TEST_CASE(cleanup_address_types_shortening)
{
	char const* sourceCode = R"(
		contract C {
			function f() public pure returns (address r) {
				bytes21 x = 0x1122334455667788990011223344556677889900ff;
				bytes20 y;
				assembly { y := x }
				address z = address(y);
				assembly { r := z }
				require(z == 0x1122334455667788990011223344556677889900);
			}
			function g() public pure returns (address payable r) {
				bytes21 x = 0x1122334455667788990011223344556677889900ff;
				bytes20 y;
				assembly { y := x }
				address payable z = address(y);
				assembly { r := z }
				require(z == 0x1122334455667788990011223344556677889900);
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256("0x1122334455667788990011223344556677889900")));
	ABI_CHECK(callContractFunction("g()"), encodeArgs(u256("0x1122334455667788990011223344556677889900")));
}

BOOST_AUTO_TEST_CASE(skip_dynamic_types)
{
	// The EVM cannot provide access to dynamically-sized return values, so we have to skip them.
	char const* sourceCode = R"(
		contract C {
			function f() public returns (uint, uint[] memory, uint) {
				return (7, new uint[](2), 8);
			}
			function g() public returns (uint, uint) {
				// Previous implementation "moved" b to the second place and did not skip.
				(uint a,, uint b) = this.f();
				return (a, b);
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("g()"), encodeArgs(u256(7), u256(8)));
}

BOOST_AUTO_TEST_CASE(skip_dynamic_types_for_structs)
{
	// For accessors, the dynamic types are already removed in the external signature itself.
	char const* sourceCode = R"(
		contract C {
			struct S {
				uint x;
				string a; // this is present in the accessor
				uint[] b; // this is not present
				uint y;
			}
			S public s;
			function g() public returns (uint, uint) {
				s.x = 2;
				s.a = "abc";
				s.b = [7, 8, 9];
				s.y = 6;
				(uint x,, uint y) = this.s();
				return (x, y);
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("g()"), encodeArgs(u256(2), u256(6)));
}

BOOST_AUTO_TEST_CASE(failed_create)
{
	char const* sourceCode = R"(
		contract D { constructor() public payable {} }
		contract C {
			uint public x;
			constructor() public payable {}
			function f(uint amount) public returns (D) {
				x++;
				return (new D).value(amount)();
			}
			function stack(uint depth) public returns (address) {
				if (depth < 1024)
					return this.stack(depth - 1);
				else
					return address(f(0));
			}
		}
	)";
	compileAndRun(sourceCode, 20, "C");
	BOOST_CHECK(callContractFunction("f(uint256)", 20) != encodeArgs(u256(0)));
	ABI_CHECK(callContractFunction("x()"), encodeArgs(u256(1)));
	ABI_CHECK(callContractFunction("f(uint256)", 20), encodeArgs());
	ABI_CHECK(callContractFunction("x()"), encodeArgs(u256(1)));
	ABI_CHECK(callContractFunction("stack(uint256)", 1023), encodeArgs());
	ABI_CHECK(callContractFunction("x()"), encodeArgs(u256(1)));
}

BOOST_AUTO_TEST_CASE(create_dynamic_array_with_zero_length)
{
	char const* sourceCode = R"(
		contract C {
			function f() public returns (uint) {
				uint[][] memory a = new uint[][](0);
				return 7;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(7)));
}

BOOST_AUTO_TEST_CASE(correctly_initialize_memory_array_in_constructor)
{
	// Memory arrays are initialized using codecopy past the size of the code.
	// This test checks that it also works in the constructor context.
	char const* sourceCode = R"(
		contract C {
			bool public success;
			constructor() public {
				// Make memory dirty.
				assembly {
					for { let i := 0 } lt(i, 64) { i := add(i, 1) } {
						mstore(msize, not(0))
					}
				}
				uint16[3] memory c;
				require(c[0] == 0 && c[1] == 0 && c[2] == 0);
				uint16[] memory x = new uint16[](3);
				require(x[0] == 0 && x[1] == 0 && x[2] == 0);
				success = true;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("success()"), encodeArgs(u256(1)));
}

BOOST_AUTO_TEST_CASE(return_does_not_skip_modifier)
{
	char const* sourceCode = R"(
		contract C {
			uint public x;
			modifier setsx {
				_;
				x = 9;
			}
			function f() setsx public returns (uint) {
				return 2;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("x()"), encodeArgs(u256(0)));
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(2)));
	ABI_CHECK(callContractFunction("x()"), encodeArgs(u256(9)));
}

BOOST_AUTO_TEST_CASE(break_in_modifier)
{
	char const* sourceCode = R"(
		contract C {
			uint public x;
			modifier run() {
				for (uint i = 0; i < 10; i++) {
					_;
					break;
				}
			}
			function f() run public {
				uint k = x;
				uint t = k + 1;
				x = t;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("x()"), encodeArgs(u256(0)));
	ABI_CHECK(callContractFunction("f()"), encodeArgs());
	ABI_CHECK(callContractFunction("x()"), encodeArgs(u256(1)));
}

BOOST_AUTO_TEST_CASE(continue_in_modifier)
{
	char const* sourceCode = R"(
		contract C {
			uint public x;
			modifier run() {
				for (uint i = 0; i < 10; i++) {
					if (i % 2 == 1) continue;
					_;
				}
			}
			function f() run public {
				uint k = x;
				uint t = k + 1;
				x = t;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("x()"), encodeArgs(u256(0)));
	ABI_CHECK(callContractFunction("f()"), encodeArgs());
	ABI_CHECK(callContractFunction("x()"), encodeArgs(u256(5)));
}

BOOST_AUTO_TEST_CASE(return_in_modifier)
{
	char const* sourceCode = R"(
		contract C {
			uint public x;
			modifier run() {
				for (uint i = 1; i < 10; i++) {
					if (i == 5) return;
					_;
				}
			}
			function f() run public {
				uint k = x;
				uint t = k + 1;
				x = t;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("x()"), encodeArgs(u256(0)));
	ABI_CHECK(callContractFunction("f()"), encodeArgs());
	ABI_CHECK(callContractFunction("x()"), encodeArgs(u256(4)));
}

BOOST_AUTO_TEST_CASE(stacked_return_with_modifiers)
{
	char const* sourceCode = R"(
		contract C {
			uint public x;
			modifier run() {
				for (uint i = 0; i < 10; i++) {
					_;
					break;
				}
			}
			function f() run public {
				uint k = x;
				uint t = k + 1;
				x = t;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("x()"), encodeArgs(u256(0)));
	ABI_CHECK(callContractFunction("f()"), encodeArgs());
	ABI_CHECK(callContractFunction("x()"), encodeArgs(u256(1)));
}

BOOST_AUTO_TEST_CASE(mutex)
{
	char const* sourceCode = R"(
		contract mutexed {
			bool locked;
			modifier protected {
				if (locked) revert();
				locked = true;
				_;
				locked = false;
			}
		}
		contract Fund is mutexed {
			uint shares;
			constructor() public payable { shares = msg.value; }
			function withdraw(uint amount) public protected returns (uint) {
				// NOTE: It is very bad practice to write this function this way.
				// Please refer to the documentation of how to do this properly.
				if (amount > shares) revert();
				(bool success,) = msg.sender.call.value(amount)("");
				require(success);
				shares -= amount;
				return shares;
			}
			function withdrawUnprotected(uint amount) public returns (uint) {
				// NOTE: It is very bad practice to write this function this way.
				// Please refer to the documentation of how to do this properly.
				if (amount > shares) revert();
				(bool success,) = msg.sender.call.value(amount)("");
				require(success);
				shares -= amount;
				return shares;
			}
		}
		contract Attacker {
			Fund public fund;
			uint callDepth;
			bool protected;
			function setProtected(bool _protected) public { protected = _protected; }
			constructor(Fund _fund) public { fund = _fund; }
			function attack() public returns (uint) {
				callDepth = 0;
				return attackInternal();
			}
			function attackInternal() internal returns (uint) {
				if (protected)
					return fund.withdraw(10);
				else
					return fund.withdrawUnprotected(10);
			}
			function() external payable {
				callDepth++;
				if (callDepth < 4)
					attackInternal();
			}
		}
	)";
	compileAndRun(sourceCode, 500, "Fund");
	auto fund = m_contractAddress;
	BOOST_CHECK_EQUAL(balanceAt(fund), 500);
	compileAndRun(sourceCode, 0, "Attacker", encodeArgs(u160(fund)));
	ABI_CHECK(callContractFunction("setProtected(bool)", true), encodeArgs());
	ABI_CHECK(callContractFunction("attack()"), encodeArgs());
	BOOST_CHECK_EQUAL(balanceAt(fund), 500);
	ABI_CHECK(callContractFunction("setProtected(bool)", false), encodeArgs());
	ABI_CHECK(callContractFunction("attack()"), encodeArgs(u256(460)));
	BOOST_CHECK_EQUAL(balanceAt(fund), 460);
}

BOOST_AUTO_TEST_CASE(failing_ecrecover_invalid_input)
{
	// ecrecover should return zero for malformed input
	// (v should be 27 or 28, not 1)
	// Note that the precompile does not return zero but returns nothing.
	char const* sourceCode = R"(
		contract C {
			function f() public returns (address) {
				return ecrecover(bytes32(uint(-1)), 1, bytes32(uint(2)), bytes32(uint(3)));
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(0)));
}

BOOST_AUTO_TEST_CASE(failing_ecrecover_invalid_input_proper)
{
	char const* sourceCode = R"(
		contract C {
			function f() public returns (address) {
				return recover(
					0x77e5189111eb6557e8a637b27ef8fbb15bc61d61c2f00cc48878f3a296e5e0ca,
					0, // invalid v value
					0x6944c77849b18048f6abe0db8084b0d0d0689cdddb53d2671c36967b58691ad4,
					0xef4f06ba4f78319baafd0424365777241af4dfd3da840471b4b4b087b7750d0d,
					0x000000000000000000000000ca35b7d915458ef540ade6068dfe2f44e8fa733c,
					0x000000000000000000000000ca35b7d915458ef540ade6068dfe2f44e8fa733c
				);
			}
			function recover(bytes32 hash, uint8 v, bytes32 r, bytes32 s, uint blockExpired, bytes32 salt)
				public returns (address)
			{
				require(hash == keccak256(abi.encodePacked(blockExpired, salt)));
				return ecrecover(hash, v, r, s);
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(0)));
}

BOOST_AUTO_TEST_CASE(failing_ecrecover_invalid_input_asm)
{
	char const* sourceCode = R"(
		contract C {
			function f() public returns (address) {
				assembly {
					mstore(mload(0x40), 0xca35b7d915458ef540ade6068dfe2f44e8fa733c)
				}
				return ecrecover(
					0x77e5189111eb6557e8a637b27ef8fbb15bc61d61c2f00cc48878f3a296e5e0ca,
					0, // invalid v value
					0x6944c77849b18048f6abe0db8084b0d0d0689cdddb53d2671c36967b58691ad4,
					0xef4f06ba4f78319baafd0424365777241af4dfd3da840471b4b4b087b7750d0d
				);
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(0)));
}

BOOST_AUTO_TEST_CASE(calling_nonexisting_contract_throws)
{
	char const* sourceCode = R"YY(
		contract D { function g() public; }
		contract C {
			D d = D(0x1212);
			function f() public returns (uint) {
				d.g();
				return 7;
			}
			function g() public returns (uint) {
				d.g.gas(200)();
				return 7;
			}
			function h() public returns (uint) {
				address(d).call(""); // this does not throw (low-level)
				return 7;
			}
		}
	)YY";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs());
	ABI_CHECK(callContractFunction("g()"), encodeArgs());
	ABI_CHECK(callContractFunction("h()"), encodeArgs(u256(7)));
}

BOOST_AUTO_TEST_CASE(payable_constructor)
{
	char const* sourceCode = R"(
		contract C {
			constructor() public payable { }
		}
	)";
	compileAndRun(sourceCode, 27, "C");
}

BOOST_AUTO_TEST_CASE(payable_function)
{
	char const* sourceCode = R"(
		contract C {
			uint public a;
			function f() payable public returns (uint) {
				return msg.value;
			}
			function() external payable {
				a = msg.value + 1;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunctionWithValue("f()", 27), encodeArgs(u256(27)));
	BOOST_CHECK_EQUAL(balanceAt(m_contractAddress), 27);
	ABI_CHECK(callContractFunctionWithValue("", 27), encodeArgs());
	BOOST_CHECK_EQUAL(balanceAt(m_contractAddress), 27 + 27);
	ABI_CHECK(callContractFunction("a()"), encodeArgs(u256(28)));
	BOOST_CHECK_EQUAL(balanceAt(m_contractAddress), 27 + 27);
}

BOOST_AUTO_TEST_CASE(payable_function_calls_library)
{
	char const* sourceCode = R"(
		library L {
			function f() public returns (uint) { return 7; }
		}
		contract C {
			function f() public payable returns (uint) {
				return L.f();
			}
		}
	)";
	compileAndRun(sourceCode, 0, "L");
	compileAndRun(sourceCode, 0, "C", bytes(), map<string, Address>{{"L", m_contractAddress}});
	ABI_CHECK(callContractFunctionWithValue("f()", 27), encodeArgs(u256(7)));
}

BOOST_AUTO_TEST_CASE(non_payable_throw)
{
	char const* sourceCode = R"(
		contract C {
			uint public a;
			function f() public returns (uint) {
				return msgvalue();
			}
			function msgvalue() internal returns (uint) {
				return msg.value;
			}
			function() external {
				update();
			}
			function update() internal {
				a = msg.value + 1;
			}

		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunctionWithValue("f()", 27), encodeArgs());
	BOOST_CHECK_EQUAL(balanceAt(m_contractAddress), 0);
	ABI_CHECK(callContractFunction(""), encodeArgs());
	ABI_CHECK(callContractFunction("a()"), encodeArgs(u256(1)));
	ABI_CHECK(callContractFunctionWithValue("", 27), encodeArgs());
	BOOST_CHECK_EQUAL(balanceAt(m_contractAddress), 0);
	ABI_CHECK(callContractFunction("a()"), encodeArgs(u256(1)));
	ABI_CHECK(callContractFunctionWithValue("a()", 27), encodeArgs());
	BOOST_CHECK_EQUAL(balanceAt(m_contractAddress), 0);
}

BOOST_AUTO_TEST_CASE(no_nonpayable_circumvention_by_modifier)
{
	char const* sourceCode = R"(
		contract C {
			modifier tryCircumvent {
				if (false) _; // avoid the function, we should still not accept ether
			}
			function f() tryCircumvent public returns (uint) {
				return msgvalue();
			}
			function msgvalue() internal returns (uint) {
				return msg.value;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunctionWithValue("f()", 27), encodeArgs());
	BOOST_CHECK_EQUAL(balanceAt(m_contractAddress), 0);
}

BOOST_AUTO_TEST_CASE(mem_resize_is_not_paid_at_call)
{
	// This tests that memory resize for return values is not paid during the call, which would
	// make the gas calculation overly complex. We access the end of the output area before
	// the call is made.
	// Tests that this also survives the optimizer.
	char const* sourceCode = R"(
		contract C {
			function f() public returns (uint[200] memory) {}
		}
		contract D {
			function f(C c) public returns (uint) { c.f(); return 7; }
		}
	)";

	compileAndRun(sourceCode, 0, "C");
	u160 cAddr = m_contractAddress;
	compileAndRun(sourceCode, 0, "D");
	ABI_CHECK(callContractFunction("f(address)", cAddr), encodeArgs(u256(7)));
}

BOOST_AUTO_TEST_CASE(calling_uninitialized_function)
{
	char const* sourceCode = R"(
		contract C {
			function intern() public returns (uint) {
				function (uint) internal returns (uint) x;
				x(2);
				return 7;
			}
			function extern() public returns (uint) {
				function (uint) external returns (uint) x;
				x(2);
				return 7;
			}
		}
	)";

	compileAndRun(sourceCode, 0, "C");
	// This should throw exceptions
	ABI_CHECK(callContractFunction("intern()"), encodeArgs());
	ABI_CHECK(callContractFunction("extern()"), encodeArgs());
}

BOOST_AUTO_TEST_CASE(calling_uninitialized_function_in_detail)
{
	char const* sourceCode = R"(
		contract C {
			function() internal returns (uint) x;
			int mutex;
			function t() public returns (uint) {
				if (mutex > 0)
					{ assembly { mstore(0, 7) return(0, 0x20) } }
				mutex = 1;
				// Avoid re-executing this function if we jump somewhere.
				x();
				return 2;
			}
		}
	)";

	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("t()"), encodeArgs());
}

BOOST_AUTO_TEST_CASE(calling_uninitialized_function_through_array)
{
	char const* sourceCode = R"(
		contract C {
			int mutex;
			function t() public returns (uint) {
				if (mutex > 0)
					{ assembly { mstore(0, 7) return(0, 0x20) } }
				mutex = 1;
				// Avoid re-executing this function if we jump somewhere.
				function() internal returns (uint)[200] memory x;
				x[0]();
				return 2;
			}
		}
	)";

	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("t()"), encodeArgs());
}

BOOST_AUTO_TEST_CASE(pass_function_types_internally)
{
	char const* sourceCode = R"(
		contract C {
			function f(uint x) public returns (uint) {
				return eval(g, x);
			}
			function eval(function(uint) internal returns (uint) x, uint a) internal returns (uint) {
				return x(a);
			}
			function g(uint x) public returns (uint) { return x + 1; }
		}
	)";

	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f(uint256)", 7), encodeArgs(u256(8)));
}

BOOST_AUTO_TEST_CASE(pass_function_types_externally)
{
	char const* sourceCode = R"(
		contract C {
			function f(uint x) public returns (uint) {
				return this.eval(this.g, x);
			}
			function f2(uint x) public returns (uint) {
				return eval(this.g, x);
			}
			function eval(function(uint) external returns (uint) x, uint a) public returns (uint) {
				return x(a);
			}
			function g(uint x) public returns (uint) { return x + 1; }
		}
	)";

	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f(uint256)", 7), encodeArgs(u256(8)));
	ABI_CHECK(callContractFunction("f2(uint256)", 7), encodeArgs(u256(8)));
}

BOOST_AUTO_TEST_CASE(receive_external_function_type)
{
	char const* sourceCode = R"(
		contract C {
			function g() public returns (uint) { return 7; }
			function f(function() external returns (uint) g) public returns (uint) {
				return g();
			}
		}
	)";

	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction(
		"f(function)",
		m_contractAddress.asBytes() + FixedHash<4>(dev::keccak256("g()")).asBytes() + bytes(32 - 4 - 20, 0)
	), encodeArgs(u256(7)));
}

BOOST_AUTO_TEST_CASE(return_external_function_type)
{
	char const* sourceCode = R"(
		contract C {
			function g() public {}
			function f() public returns (function() external) {
				return this.g;
			}
		}
	)";

	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(
		callContractFunction("f()"),
		m_contractAddress.asBytes() + FixedHash<4>(dev::keccak256("g()")).asBytes() + bytes(32 - 4 - 20, 0)
	);
}

BOOST_AUTO_TEST_CASE(store_function)
{
	char const* sourceCode = R"(
		contract Other {
			function addTwo(uint x) public returns (uint) { return x + 2; }
		}
		contract C {
			function (function (uint) external returns (uint)) internal returns (uint) ev;
			function (uint) external returns (uint) x;
			function store(function(uint) external returns (uint) y) public {
				x = y;
			}
			function eval(function(uint) external returns (uint) y) public returns (uint) {
				return y(7);
			}
			function t() public returns (uint) {
				ev = eval;
				this.store((new Other()).addTwo);
				return ev(x);
			}
		}
	)";

	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("t()"), encodeArgs(u256(9)));
}

BOOST_AUTO_TEST_CASE(store_function_in_constructor)
{
	char const* sourceCode = R"(
		contract C {
			uint public result_in_constructor;
			function (uint) internal returns (uint) x;
			constructor() public {
				x = double;
				result_in_constructor = use(2);
			}
			function double(uint _arg) public returns (uint _ret) {
				_ret = _arg * 2;
			}
			function use(uint _arg) public returns (uint) {
				return x(_arg);
			}
		}
	)";

	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("use(uint256)", encodeArgs(u256(3))), encodeArgs(u256(6)));
	ABI_CHECK(callContractFunction("result_in_constructor()"), encodeArgs(u256(4)));
}

// TODO: store bound internal library functions

BOOST_AUTO_TEST_CASE(store_internal_unused_function_in_constructor)
{
	char const* sourceCode = R"(
		contract C {
			function () internal returns (uint) x;
			constructor() public {
				x = unused;
			}
			function unused() internal returns (uint) {
				return 7;
			}
			function t() public returns (uint) {
				return x();
			}
		}
	)";

	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("t()"), encodeArgs(u256(7)));
}

BOOST_AUTO_TEST_CASE(store_internal_unused_library_function_in_constructor)
{
	char const* sourceCode = R"(
		library L { function x() internal returns (uint) { return 7; } }
		contract C {
			function () internal returns (uint) x;
			constructor() public {
				x = L.x;
			}
			function t() public returns (uint) {
				return x();
			}
		}
	)";

	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("t()"), encodeArgs(u256(7)));
}

BOOST_AUTO_TEST_CASE(same_function_in_construction_and_runtime)
{
	char const* sourceCode = R"(
		contract C {
			uint public initial;
			constructor() public {
				initial = double(2);
			}
			function double(uint _arg) public returns (uint _ret) {
				_ret = _arg * 2;
			}
			function runtime(uint _arg) public returns (uint) {
				return double(_arg);
			}
		}
	)";

	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("runtime(uint256)", encodeArgs(u256(3))), encodeArgs(u256(6)));
	ABI_CHECK(callContractFunction("initial()"), encodeArgs(u256(4)));
}

BOOST_AUTO_TEST_CASE(same_function_in_construction_and_runtime_equality_check)
{
	char const* sourceCode = R"(
		contract C {
			function (uint) internal returns (uint) x;
			constructor() public {
				x = double;
			}
			function test() public returns (bool) {
				return x == double;
			}
			function double(uint _arg) public returns (uint _ret) {
				_ret = _arg * 2;
			}
		}
	)";

	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("test()"), encodeArgs(true));
}

BOOST_AUTO_TEST_CASE(function_type_library_internal)
{
	char const* sourceCode = R"(
		library Utils {
			function reduce(uint[] memory array, function(uint, uint) internal returns (uint) f, uint init) internal returns (uint) {
				for (uint i = 0; i < array.length; i++) {
					init = f(array[i], init);
				}
				return init;
			}
			function sum(uint a, uint b) internal returns (uint) {
				return a + b;
			}
		}
		contract C {
			function f(uint[] memory x) public returns (uint) {
				return Utils.reduce(x, Utils.sum, 0);
			}
		}
	)";

	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f(uint256[])", 0x20, 3, u256(1), u256(7), u256(3)), encodeArgs(u256(11)));
}


BOOST_AUTO_TEST_CASE(call_function_returning_function)
{
	char const* sourceCode = R"(
		contract test {
			function f0() public returns (uint) {
				return 2;
			}
			function f1() internal returns (function() internal returns (uint)) {
				return f0;
			}
			function f2() internal returns (function() internal returns (function () internal returns (uint))) {
				return f1;
			}
			function f3() internal returns (function() internal returns (function () internal returns (function () internal returns (uint))))
			{
				return f2;
			}
			function f() public returns (uint) {
				function() internal returns(function() internal returns(function() internal returns(function() internal returns(uint)))) x;
				x = f3;
				return x()()()();
			}
		}
	)";

	compileAndRun(sourceCode, 0, "test");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(2)));
}

BOOST_AUTO_TEST_CASE(mapping_of_functions)
{
	char const* sourceCode = R"(
		contract Flow {
			bool public success;

			mapping (address => function () internal) stages;

			function stage0() internal {
				stages[msg.sender] = stage1;
			}

			function stage1() internal {
				stages[msg.sender] = stage2;
			}

			function stage2() internal {
				success = true;
			}

			constructor() public {
				stages[msg.sender] = stage0;
			}

			function f() public returns (uint) {
				stages[msg.sender]();
				return 7;
			}
		}
	)";

	compileAndRun(sourceCode, 0, "Flow");
	ABI_CHECK(callContractFunction("success()"), encodeArgs(false));
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(7)));
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(7)));
	ABI_CHECK(callContractFunction("success()"), encodeArgs(false));
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(7)));
	ABI_CHECK(callContractFunction("success()"), encodeArgs(true));
}

BOOST_AUTO_TEST_CASE(packed_functions)
{
	char const* sourceCode = R"(
		contract C {
			// these should take the same slot
			function() internal returns (uint) a;
			function() external returns (uint) b;
			function() external returns (uint) c;
			function() internal returns (uint) d;
			uint8 public x;

			function set() public {
				x = 2;
				d = g;
				c = this.h;
				b = this.h;
				a = g;
			}
			function t1() public returns (uint) {
				return a();
			}
			function t2() public returns (uint) {
				return b();
			}
			function t3() public returns (uint) {
				return a();
			}
			function t4() public returns (uint) {
				return b();
			}
			function g() public returns (uint) {
				return 7;
			}
			function h() public returns (uint) {
				return 8;
			}
		}
	)";

	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("set()"), encodeArgs());
	ABI_CHECK(callContractFunction("t1()"), encodeArgs(u256(7)));
	ABI_CHECK(callContractFunction("t2()"), encodeArgs(u256(8)));
	ABI_CHECK(callContractFunction("t3()"), encodeArgs(u256(7)));
	ABI_CHECK(callContractFunction("t4()"), encodeArgs(u256(8)));
	ABI_CHECK(callContractFunction("x()"), encodeArgs(u256(2)));
}

BOOST_AUTO_TEST_CASE(function_memory_array)
{
	char const* sourceCode = R"(
		contract C {
			function a(uint x) public returns (uint) { return x + 1; }
			function b(uint x) public returns (uint) { return x + 2; }
			function c(uint x) public returns (uint) { return x + 3; }
			function d(uint x) public returns (uint) { return x + 5; }
			function e(uint x) public returns (uint) { return x + 8; }
			function test(uint x, uint i) public returns (uint) {
				function(uint) internal returns (uint)[] memory arr =
					new function(uint) internal returns (uint)[](10);
				arr[0] = a;
				arr[1] = b;
				arr[2] = c;
				arr[3] = d;
				arr[4] = e;
				return arr[i](x);
			}
		}
	)";

	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("test(uint256,uint256)", u256(10), u256(0)), encodeArgs(u256(11)));
	ABI_CHECK(callContractFunction("test(uint256,uint256)", u256(10), u256(1)), encodeArgs(u256(12)));
	ABI_CHECK(callContractFunction("test(uint256,uint256)", u256(10), u256(2)), encodeArgs(u256(13)));
	ABI_CHECK(callContractFunction("test(uint256,uint256)", u256(10), u256(3)), encodeArgs(u256(15)));
	ABI_CHECK(callContractFunction("test(uint256,uint256)", u256(10), u256(4)), encodeArgs(u256(18)));
	ABI_CHECK(callContractFunction("test(uint256,uint256)", u256(10), u256(5)), encodeArgs());
}

BOOST_AUTO_TEST_CASE(function_delete_storage)
{
	char const* sourceCode = R"(
		contract C {
			function a() public returns (uint) { return 7; }
			function() internal returns (uint) y;
			function set() public returns (uint) {
				y = a;
				return y();
			}
			function d() public returns (uint) {
				delete y;
				return 1;
			}
			function ca() public returns (uint) {
				return y();
			}
		}
	)";

	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("set()"), encodeArgs(u256(7)));
	ABI_CHECK(callContractFunction("ca()"), encodeArgs(u256(7)));
	ABI_CHECK(callContractFunction("d()"), encodeArgs(u256(1)));
	ABI_CHECK(callContractFunction("ca()"), encodeArgs());
}

BOOST_AUTO_TEST_CASE(function_delete_stack)
{
	char const* sourceCode = R"(
		contract C {
			function a() public returns (uint) { return 7; }
			function test() public returns (uint) {
				function () returns (uint) y = a;
				delete y;
				y();
			}
		}
	)";

	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("test()"), encodeArgs());
}

BOOST_AUTO_TEST_CASE(copy_function_storage_array)
{
	char const* sourceCode = R"(
		contract C {
			function() internal returns (uint)[] x;
			function() internal returns (uint)[] y;
			function test() public returns (uint) {
				x.length = 10;
				x[9] = a;
				y = x;
				return y[9]();
			}
			function a() public returns (uint) {
				return 7;
			}
		}
	)";

	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("test()"), encodeArgs(u256(7)));
}

BOOST_AUTO_TEST_CASE(function_array_cross_calls)
{
	char const* sourceCode = R"(
		contract D {
			function f(function() external returns (function() external returns (uint))[] memory x)
				public returns (function() external returns (uint)[3] memory r)
			{
				r[0] = x[0]();
				r[1] = x[1]();
				r[2] = x[2]();
			}
		}
		contract C {
			function test() public returns (uint, uint, uint) {
				function() external returns (function() external returns (uint))[] memory x =
					new function() external returns (function() external returns (uint))[](10);
				for (uint i = 0; i < x.length; i ++)
					x[i] = this.h;
				x[0] = this.htwo;
				function() external returns (uint)[3] memory y = (new D()).f(x);
				return (y[0](), y[1](), y[2]());
			}
			function e() public returns (uint) { return 5; }
			function f() public returns (uint) { return 6; }
			function g() public returns (uint) { return 7; }
			uint counter;
			function h() public returns (function() external returns (uint)) {
				return counter++ == 0 ? this.f : this.g;
			}
			function htwo() public returns (function() external returns (uint)) {
				return this.e;
			}
		}
	)";

	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("test()"), encodeArgs(u256(5), u256(6), u256(7)));
}

BOOST_AUTO_TEST_CASE(external_function_to_address)
{
	char const* sourceCode = R"(
		contract C {
			function f() public returns (bool) {
				return address(this.f) == address(this);
			}
			function g(function() external cb) public returns (address) {
				return address(cb);
			}
		}
	)";

	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(true));
	ABI_CHECK(callContractFunction("g(function)", fromHex("00000000000000000000000000000000000004226121ff00000000000000000")), encodeArgs(u160(0x42)));
}


BOOST_AUTO_TEST_CASE(copy_internal_function_array_to_storage)
{
	char const* sourceCode = R"(
		contract C {
			function() internal returns (uint)[20] x;
			int mutex;
			function one() public returns (uint) {
				function() internal returns (uint)[20] memory xmem;
				x = xmem;
				return 3;
			}
			function two() public returns (uint) {
				if (mutex > 0)
					return 7;
				mutex = 1;
				// If this test fails, it might re-execute this function.
				x[0]();
				return 2;
			}
		}
	)";

	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("one()"), encodeArgs(u256(3)));
	ABI_CHECK(callContractFunction("two()"), encodeArgs());
}

BOOST_AUTO_TEST_CASE(shift_constant_left)
{
	char const* sourceCode = R"(
		contract C {
			uint public a = 0x42 << 8;
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("a()"), encodeArgs(u256(0x4200)));
}

BOOST_AUTO_TEST_CASE(shift_negative_constant_left)
{
	char const* sourceCode = R"(
		contract C {
			int public a = -0x42 << 8;
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("a()"), encodeArgs(u256(-0x4200)));
}

BOOST_AUTO_TEST_CASE(shift_constant_right)
{
	char const* sourceCode = R"(
		contract C {
			uint public a = 0x4200 >> 8;
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("a()"), encodeArgs(u256(0x42)));
}

BOOST_AUTO_TEST_CASE(shift_negative_constant_right)
{
	char const* sourceCode = R"(
		contract C {
			int public a = -0x4200 >> 8;
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("a()"), encodeArgs(u256(-0x42)));
}

BOOST_AUTO_TEST_CASE(shift_left)
{
	char const* sourceCode = R"(
		contract C {
			function f(uint a, uint b) public returns (uint) {
				return a << b;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f(uint256,uint256)", u256(0x4266), u256(0)), encodeArgs(u256(0x4266)));
	ABI_CHECK(callContractFunction("f(uint256,uint256)", u256(0x4266), u256(8)), encodeArgs(u256(0x426600)));
	ABI_CHECK(callContractFunction("f(uint256,uint256)", u256(0x4266), u256(16)), encodeArgs(u256(0x42660000)));
	ABI_CHECK(callContractFunction("f(uint256,uint256)", u256(0x4266), u256(17)), encodeArgs(u256(0x84cc0000)));
	ABI_CHECK(callContractFunction("f(uint256,uint256)", u256(0x4266), u256(240)), fromHex("4266000000000000000000000000000000000000000000000000000000000000"));
	ABI_CHECK(callContractFunction("f(uint256,uint256)", u256(0x4266), u256(256)), encodeArgs(u256(0)));
}

BOOST_AUTO_TEST_CASE(shift_left_uint32)
{
	char const* sourceCode = R"(
		contract C {
			function f(uint32 a, uint32 b) public returns (uint) {
				return a << b;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f(uint32,uint32)", u256(0x4266), u256(0)), encodeArgs(u256(0x4266)));
	ABI_CHECK(callContractFunction("f(uint32,uint32)", u256(0x4266), u256(8)), encodeArgs(u256(0x426600)));
	ABI_CHECK(callContractFunction("f(uint32,uint32)", u256(0x4266), u256(16)), encodeArgs(u256(0x42660000)));
	ABI_CHECK(callContractFunction("f(uint32,uint32)", u256(0x4266), u256(17)), encodeArgs(u256(0x84cc0000)));
	ABI_CHECK(callContractFunction("f(uint32,uint32)", u256(0x4266), u256(32)), encodeArgs(u256(0)));
}

BOOST_AUTO_TEST_CASE(shift_left_uint8)
{
	char const* sourceCode = R"(
		contract C {
			function f(uint8 a, uint8 b) public returns (uint) {
				return a << b;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f(uint8,uint8)", u256(0x66), u256(0)), encodeArgs(u256(0x66)));
	ABI_CHECK(callContractFunction("f(uint8,uint8)", u256(0x66), u256(8)), encodeArgs(u256(0)));
}

BOOST_AUTO_TEST_CASE(shift_left_larger_type)
{
	// This basically tests proper cleanup and conversion. It should not convert x to int8.
	char const* sourceCode = R"(
		contract C {
			function f() public returns (int8) {
				uint8 x = 254;
				int8 y = 1;
				return y << x;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(0)));
}

BOOST_AUTO_TEST_CASE(shift_left_assignment)
{
	char const* sourceCode = R"(
		contract C {
			function f(uint a, uint b) public returns (uint) {
				a <<= b;
				return a;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f(uint256,uint256)", u256(0x4266), u256(0)), encodeArgs(u256(0x4266)));
	ABI_CHECK(callContractFunction("f(uint256,uint256)", u256(0x4266), u256(8)), encodeArgs(u256(0x426600)));
	ABI_CHECK(callContractFunction("f(uint256,uint256)", u256(0x4266), u256(16)), encodeArgs(u256(0x42660000)));
	ABI_CHECK(callContractFunction("f(uint256,uint256)", u256(0x4266), u256(17)), encodeArgs(u256(0x84cc0000)));
	ABI_CHECK(callContractFunction("f(uint256,uint256)", u256(0x4266), u256(240)), fromHex("4266000000000000000000000000000000000000000000000000000000000000"));
	ABI_CHECK(callContractFunction("f(uint256,uint256)", u256(0x4266), u256(256)), encodeArgs(u256(0)));
}

BOOST_AUTO_TEST_CASE(shift_left_assignment_different_type)
{
	char const* sourceCode = R"(
		contract C {
			function f(uint a, uint8 b) public returns (uint) {
				a <<= b;
				return a;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f(uint256,uint8)", u256(0x4266), u256(0)), encodeArgs(u256(0x4266)));
	ABI_CHECK(callContractFunction("f(uint256,uint8)", u256(0x4266), u256(8)), encodeArgs(u256(0x426600)));
	ABI_CHECK(callContractFunction("f(uint256,uint8)", u256(0x4266), u256(16)), encodeArgs(u256(0x42660000)));
	ABI_CHECK(callContractFunction("f(uint256,uint8)", u256(0x4266), u256(17)), encodeArgs(u256(0x84cc0000)));
	ABI_CHECK(callContractFunction("f(uint256,uint8)", u256(0x4266), u256(240)), fromHex("4266000000000000000000000000000000000000000000000000000000000000"));
}

BOOST_AUTO_TEST_CASE(shift_right)
{
	char const* sourceCode = R"(
		contract C {
			function f(uint a, uint b) public returns (uint) {
				return a >> b;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f(uint256,uint256)", u256(0x4266), u256(0)), encodeArgs(u256(0x4266)));
	ABI_CHECK(callContractFunction("f(uint256,uint256)", u256(0x4266), u256(8)), encodeArgs(u256(0x42)));
	ABI_CHECK(callContractFunction("f(uint256,uint256)", u256(0x4266), u256(16)), encodeArgs(u256(0)));
	ABI_CHECK(callContractFunction("f(uint256,uint256)", u256(0x4266), u256(17)), encodeArgs(u256(0)));
	ABI_CHECK(callContractFunction("f(uint256,uint256)", u256(1)<<255, u256(5)), encodeArgs(u256(1)<<250));
}

BOOST_AUTO_TEST_CASE(shift_right_garbled)
{
	char const* sourceCode = R"(
		contract C {
			function f(uint8 a, uint8 b) public returns (uint) {
				assembly {
					a := 0xffffffff
				}
				// Higher bits should be cleared before the shift
				return a >> b;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f(uint8,uint8)", u256(0x0), u256(4)), encodeArgs(u256(0xf)));
	ABI_CHECK(callContractFunction("f(uint8,uint8)", u256(0x0), u256(0x1004)), encodeArgs(u256(0xf)));
}

BOOST_AUTO_TEST_CASE(shift_right_garbled_signed)
{
	char const* sourceCode = R"(
			contract C {
				function f(int8 a, uint8 b) public returns (int) {
					assembly {
						a := 0xfffffff0
					}
					// Higher bits should be signextended before the shift
					return a >> b;
				}
				function g(int8 a, uint8 b) public returns (int) {
					assembly {
						a := 0xf0
					}
					// Higher bits should be signextended before the shift
					return a >> b;
				}
			}
		)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f(int8,uint8)", u256(0x0), u256(3)), encodeArgs(u256(-2)));
	ABI_CHECK(callContractFunction("f(int8,uint8)", u256(0x0), u256(4)), encodeArgs(u256(-1)));
	ABI_CHECK(callContractFunction("f(int8,uint8)", u256(0x0), u256(0xFF)), encodeArgs(u256(-1)));
	ABI_CHECK(callContractFunction("f(int8,uint8)", u256(0x0), u256(0x1003)), encodeArgs(u256(-2)));
	ABI_CHECK(callContractFunction("f(int8,uint8)", u256(0x0), u256(0x1004)), encodeArgs(u256(-1)));
	ABI_CHECK(callContractFunction("g(int8,uint8)", u256(0x0), u256(3)), encodeArgs(u256(-2)));
	ABI_CHECK(callContractFunction("g(int8,uint8)", u256(0x0), u256(4)), encodeArgs(u256(-1)));
	ABI_CHECK(callContractFunction("g(int8,uint8)", u256(0x0), u256(0xFF)), encodeArgs(u256(-1)));
	ABI_CHECK(callContractFunction("g(int8,uint8)", u256(0x0), u256(0x1003)), encodeArgs(u256(-2)));
	ABI_CHECK(callContractFunction("g(int8,uint8)", u256(0x0), u256(0x1004)), encodeArgs(u256(-1)));
}

BOOST_AUTO_TEST_CASE(shift_right_uint32)
{
	char const* sourceCode = R"(
		contract C {
			function f(uint32 a, uint32 b) public returns (uint) {
				return a >> b;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f(uint32,uint32)", u256(0x4266), u256(0)), encodeArgs(u256(0x4266)));
	ABI_CHECK(callContractFunction("f(uint32,uint32)", u256(0x4266), u256(8)), encodeArgs(u256(0x42)));
	ABI_CHECK(callContractFunction("f(uint32,uint32)", u256(0x4266), u256(16)), encodeArgs(u256(0)));
	ABI_CHECK(callContractFunction("f(uint32,uint32)", u256(0x4266), u256(17)), encodeArgs(u256(0)));
}

BOOST_AUTO_TEST_CASE(shift_right_uint8)
{
	char const* sourceCode = R"(
		contract C {
			function f(uint8 a, uint8 b) public returns (uint) {
				return a >> b;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f(uint8,uint8)", u256(0x66), u256(0)), encodeArgs(u256(0x66)));
	ABI_CHECK(callContractFunction("f(uint8,uint8)", u256(0x66), u256(8)), encodeArgs(u256(0x0)));
}

BOOST_AUTO_TEST_CASE(shift_right_assignment)
{
	char const* sourceCode = R"(
		contract C {
			function f(uint a, uint b) public returns (uint) {
				a >>= b;
				return a;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f(uint256,uint256)", u256(0x4266), u256(0)), encodeArgs(u256(0x4266)));
	ABI_CHECK(callContractFunction("f(uint256,uint256)", u256(0x4266), u256(8)), encodeArgs(u256(0x42)));
	ABI_CHECK(callContractFunction("f(uint256,uint256)", u256(0x4266), u256(16)), encodeArgs(u256(0)));
	ABI_CHECK(callContractFunction("f(uint256,uint256)", u256(0x4266), u256(17)), encodeArgs(u256(0)));
}

BOOST_AUTO_TEST_CASE(shift_right_assignment_signed)
{
	char const* sourceCode = R"(
			contract C {
				function f(int a, int b) public returns (int) {
					a >>= b;
					return a;
				}
			}
		)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f(int256,int256)", u256(0x4266), u256(0)), encodeArgs(u256(0x4266)));
	ABI_CHECK(callContractFunction("f(int256,int256)", u256(0x4266), u256(8)), encodeArgs(u256(0x42)));
	ABI_CHECK(callContractFunction("f(int256,int256)", u256(0x4266), u256(16)), encodeArgs(u256(0)));
	ABI_CHECK(callContractFunction("f(int256,int256)", u256(0x4266), u256(17)), encodeArgs(u256(0)));
}

BOOST_AUTO_TEST_CASE(shift_right_negative_lvalue)
{
	char const* sourceCode = R"(
		contract C {
			function f(int a, int b) public returns (int) {
				return a >> b;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f(int256,int256)", u256(-4266), u256(0)), encodeArgs(u256(-4266)));
	ABI_CHECK(callContractFunction("f(int256,int256)", u256(-4266), u256(1)), encodeArgs(u256(-2133)));
	ABI_CHECK(callContractFunction("f(int256,int256)", u256(-4266), u256(4)), encodeArgs(u256(-267)));
	ABI_CHECK(callContractFunction("f(int256,int256)", u256(-4266), u256(8)), encodeArgs(u256(-17)));
	ABI_CHECK(callContractFunction("f(int256,int256)", u256(-4266), u256(16)), encodeArgs(u256(-1)));
	ABI_CHECK(callContractFunction("f(int256,int256)", u256(-4266), u256(17)), encodeArgs(u256(-1)));
	ABI_CHECK(callContractFunction("f(int256,int256)", u256(-4267), u256(0)), encodeArgs(u256(-4267)));
	ABI_CHECK(callContractFunction("f(int256,int256)", u256(-4267), u256(1)), encodeArgs(u256(-2134)));
	ABI_CHECK(callContractFunction("f(int256,int256)", u256(-4267), u256(4)), encodeArgs(u256(-267)));
	ABI_CHECK(callContractFunction("f(int256,int256)", u256(-4267), u256(8)), encodeArgs(u256(-17)));
	ABI_CHECK(callContractFunction("f(int256,int256)", u256(-4267), u256(16)), encodeArgs(u256(-1)));
	ABI_CHECK(callContractFunction("f(int256,int256)", u256(-4267), u256(17)), encodeArgs(u256(-1)));
}

BOOST_AUTO_TEST_CASE(shift_right_negative_literal)
{
	char const* sourceCode = R"(
			contract C {
				function f1() public pure returns (bool) {
					return (-4266 >> 0) == -4266;
				}
				function f2() public pure returns (bool) {
					return (-4266 >> 1) == -2133;
				}
				function f3() public pure returns (bool) {
					return (-4266 >> 4) == -267;
				}
				function f4() public pure returns (bool) {
					return (-4266 >> 8) == -17;
				}
				function f5() public pure returns (bool) {
					return (-4266 >> 16) == -1;
				}
				function f6() public pure returns (bool) {
					return (-4266 >> 17) == -1;
				}
				function g1() public pure returns (bool) {
					return (-4267 >> 0) == -4267;
				}
				function g2() public pure returns (bool) {
					return (-4267 >> 1) == -2134;
				}
				function g3() public pure returns (bool) {
					return (-4267 >> 4) == -267;
				}
				function g4() public pure returns (bool) {
					return (-4267 >> 8) == -17;
				}
				function g5() public pure returns (bool) {
					return (-4267 >> 16) == -1;
				}
				function g6() public pure returns (bool) {
					return (-4267 >> 17) == -1;
				}
			}
		)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f1()"), encodeArgs(true));
	ABI_CHECK(callContractFunction("f2()"), encodeArgs(true));
	ABI_CHECK(callContractFunction("f3()"), encodeArgs(true));
	ABI_CHECK(callContractFunction("f4()"), encodeArgs(true));
	ABI_CHECK(callContractFunction("f5()"), encodeArgs(true));
	ABI_CHECK(callContractFunction("f6()"), encodeArgs(true));
	ABI_CHECK(callContractFunction("g1()"), encodeArgs(true));
	ABI_CHECK(callContractFunction("g2()"), encodeArgs(true));
	ABI_CHECK(callContractFunction("g3()"), encodeArgs(true));
	ABI_CHECK(callContractFunction("g4()"), encodeArgs(true));
	ABI_CHECK(callContractFunction("g5()"), encodeArgs(true));
	ABI_CHECK(callContractFunction("g6()"), encodeArgs(true));
}

BOOST_AUTO_TEST_CASE(shift_right_negative_lvalue_int8)
{
	char const* sourceCode = R"(
			contract C {
				function f(int8 a, int8 b) public returns (int) {
					return a >> b;
				}
			}
		)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f(int8,int8)", u256(-66), u256(0)), encodeArgs(u256(-66)));
	ABI_CHECK(callContractFunction("f(int8,int8)", u256(-66), u256(1)), encodeArgs(u256(-33)));
	ABI_CHECK(callContractFunction("f(int8,int8)", u256(-66), u256(4)), encodeArgs(u256(-5)));
	ABI_CHECK(callContractFunction("f(int8,int8)", u256(-66), u256(8)), encodeArgs(u256(-1)));
	ABI_CHECK(callContractFunction("f(int8,int8)", u256(-66), u256(16)), encodeArgs(u256(-1)));
	ABI_CHECK(callContractFunction("f(int8,int8)", u256(-66), u256(17)), encodeArgs(u256(-1)));
	ABI_CHECK(callContractFunction("f(int8,int8)", u256(-67), u256(0)), encodeArgs(u256(-67)));
	ABI_CHECK(callContractFunction("f(int8,int8)", u256(-67), u256(1)), encodeArgs(u256(-34)));
	ABI_CHECK(callContractFunction("f(int8,int8)", u256(-67), u256(4)), encodeArgs(u256(-5)));
	ABI_CHECK(callContractFunction("f(int8,int8)", u256(-67), u256(8)), encodeArgs(u256(-1)));
	ABI_CHECK(callContractFunction("f(int8,int8)", u256(-67), u256(16)), encodeArgs(u256(-1)));
	ABI_CHECK(callContractFunction("f(int8,int8)", u256(-67), u256(17)), encodeArgs(u256(-1)));
}

BOOST_AUTO_TEST_CASE(shift_right_negative_lvalue_signextend_int8)
{
	char const* sourceCode = R"(
			contract C {
				function f(int8 a, int8 b) public returns (int8) {
					return a >> b;
				}
			}
		)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f(int8,int8)", u256(0x99u), u256(0)), encodeArgs(u256(-103)));
	ABI_CHECK(callContractFunction("f(int8,int8)", u256(0x99u), u256(1)), encodeArgs(u256(-52)));
	ABI_CHECK(callContractFunction("f(int8,int8)", u256(0x99u), u256(2)), encodeArgs(u256(-26)));
	ABI_CHECK(callContractFunction("f(int8,int8)", u256(0x99u), u256(4)), encodeArgs(u256(-7)));
	ABI_CHECK(callContractFunction("f(int8,int8)", u256(0x99u), u256(8)), encodeArgs(u256(-1)));
}

BOOST_AUTO_TEST_CASE(shift_right_negative_lvalue_signextend_int16)
{
	char const* sourceCode = R"(
			contract C {
				function f(int16 a, int16 b) public returns (int16) {
					return a >> b;
				}
			}
		)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f(int16,int16)", u256(0xFF99u), u256(0)), encodeArgs(u256(-103)));
	ABI_CHECK(callContractFunction("f(int16,int16)", u256(0xFF99u), u256(1)), encodeArgs(u256(-52)));
	ABI_CHECK(callContractFunction("f(int16,int16)", u256(0xFF99u), u256(2)), encodeArgs(u256(-26)));
	ABI_CHECK(callContractFunction("f(int16,int16)", u256(0xFF99u), u256(4)), encodeArgs(u256(-7)));
	ABI_CHECK(callContractFunction("f(int16,int16)", u256(0xFF99u), u256(8)), encodeArgs(u256(-1)));
}

BOOST_AUTO_TEST_CASE(shift_right_negative_lvalue_signextend_int32)
{
	char const* sourceCode = R"(
			contract C {
				function f(int32 a, int32 b) public returns (int32) {
					return a >> b;
				}
			}
		)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f(int32,int32)", u256(0xFFFFFF99u), u256(0)), encodeArgs(u256(-103)));
	ABI_CHECK(callContractFunction("f(int32,int32)", u256(0xFFFFFF99u), u256(1)), encodeArgs(u256(-52)));
	ABI_CHECK(callContractFunction("f(int32,int32)", u256(0xFFFFFF99u), u256(2)), encodeArgs(u256(-26)));
	ABI_CHECK(callContractFunction("f(int32,int32)", u256(0xFFFFFF99u), u256(4)), encodeArgs(u256(-7)));
	ABI_CHECK(callContractFunction("f(int32,int32)", u256(0xFFFFFF99u), u256(8)), encodeArgs(u256(-1)));
}


BOOST_AUTO_TEST_CASE(shift_right_negative_lvalue_int16)
{
	char const* sourceCode = R"(
			contract C {
				function f(int16 a, int16 b) public returns (int) {
					return a >> b;
				}
			}
		)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f(int16,int16)", u256(-4266), u256(0)), encodeArgs(u256(-4266)));
	ABI_CHECK(callContractFunction("f(int16,int16)", u256(-4266), u256(1)), encodeArgs(u256(-2133)));
	ABI_CHECK(callContractFunction("f(int16,int16)", u256(-4266), u256(4)), encodeArgs(u256(-267)));
	ABI_CHECK(callContractFunction("f(int16,int16)", u256(-4266), u256(8)), encodeArgs(u256(-17)));
	ABI_CHECK(callContractFunction("f(int16,int16)", u256(-4266), u256(16)), encodeArgs(u256(-1)));
	ABI_CHECK(callContractFunction("f(int16,int16)", u256(-4266), u256(17)), encodeArgs(u256(-1)));
	ABI_CHECK(callContractFunction("f(int16,int16)", u256(-4267), u256(0)), encodeArgs(u256(-4267)));
	ABI_CHECK(callContractFunction("f(int16,int16)", u256(-4267), u256(1)), encodeArgs(u256(-2134)));
	ABI_CHECK(callContractFunction("f(int16,int16)", u256(-4267), u256(4)), encodeArgs(u256(-267)));
	ABI_CHECK(callContractFunction("f(int16,int16)", u256(-4267), u256(8)), encodeArgs(u256(-17)));
	ABI_CHECK(callContractFunction("f(int16,int16)", u256(-4267), u256(16)), encodeArgs(u256(-1)));
	ABI_CHECK(callContractFunction("f(int16,int16)", u256(-4267), u256(17)), encodeArgs(u256(-1)));
}

BOOST_AUTO_TEST_CASE(shift_right_negative_lvalue_int32)
{
	char const* sourceCode = R"(
			contract C {
				function f(int32 a, int32 b) public returns (int) {
					return a >> b;
				}
			}
		)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f(int32,int32)", u256(-4266), u256(0)), encodeArgs(u256(-4266)));
	ABI_CHECK(callContractFunction("f(int32,int32)", u256(-4266), u256(1)), encodeArgs(u256(-2133)));
	ABI_CHECK(callContractFunction("f(int32,int32)", u256(-4266), u256(4)), encodeArgs(u256(-267)));
	ABI_CHECK(callContractFunction("f(int32,int32)", u256(-4266), u256(8)), encodeArgs(u256(-17)));
	ABI_CHECK(callContractFunction("f(int32,int32)", u256(-4266), u256(16)), encodeArgs(u256(-1)));
	ABI_CHECK(callContractFunction("f(int32,int32)", u256(-4266), u256(17)), encodeArgs(u256(-1)));
	ABI_CHECK(callContractFunction("f(int32,int32)", u256(-4267), u256(0)), encodeArgs(u256(-4267)));
	ABI_CHECK(callContractFunction("f(int32,int32)", u256(-4267), u256(1)), encodeArgs(u256(-2134)));
	ABI_CHECK(callContractFunction("f(int32,int32)", u256(-4267), u256(4)), encodeArgs(u256(-267)));
	ABI_CHECK(callContractFunction("f(int32,int32)", u256(-4267), u256(8)), encodeArgs(u256(-17)));
	ABI_CHECK(callContractFunction("f(int32,int32)", u256(-4267), u256(16)), encodeArgs(u256(-1)));
	ABI_CHECK(callContractFunction("f(int32,int32)", u256(-4267), u256(17)), encodeArgs(u256(-1)));
}

BOOST_AUTO_TEST_CASE(shift_right_negative_lvalue_assignment)
{
	char const* sourceCode = R"(
		contract C {
			function f(int a, int b) public returns (int) {
				a >>= b;
				return a;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f(int256,int256)", u256(-4266), u256(0)), encodeArgs(u256(-4266)));
	ABI_CHECK(callContractFunction("f(int256,int256)", u256(-4266), u256(1)), encodeArgs(u256(-2133)));
	ABI_CHECK(callContractFunction("f(int256,int256)", u256(-4266), u256(4)), encodeArgs(u256(-267)));
	ABI_CHECK(callContractFunction("f(int256,int256)", u256(-4266), u256(8)), encodeArgs(u256(-17)));
	ABI_CHECK(callContractFunction("f(int256,int256)", u256(-4266), u256(16)), encodeArgs(u256(-1)));
	ABI_CHECK(callContractFunction("f(int256,int256)", u256(-4266), u256(17)), encodeArgs(u256(-1)));
	ABI_CHECK(callContractFunction("f(int256,int256)", u256(-4267), u256(0)), encodeArgs(u256(-4267)));
	ABI_CHECK(callContractFunction("f(int256,int256)", u256(-4267), u256(1)), encodeArgs(u256(-2134)));
	ABI_CHECK(callContractFunction("f(int256,int256)", u256(-4267), u256(4)), encodeArgs(u256(-267)));
	ABI_CHECK(callContractFunction("f(int256,int256)", u256(-4267), u256(8)), encodeArgs(u256(-17)));
	ABI_CHECK(callContractFunction("f(int256,int256)", u256(-4267), u256(16)), encodeArgs(u256(-1)));
	ABI_CHECK(callContractFunction("f(int256,int256)", u256(-4267), u256(17)), encodeArgs(u256(-1)));
}

BOOST_AUTO_TEST_CASE(shift_negative_rvalue)
{
	char const* sourceCode = R"(
		contract C {
			function f(int a, int b) public returns (int) {
				return a << b;
			}
			function g(int a, int b) public returns (int) {
				return a >> b;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f(int256,int256)", u256(1), u256(-1)), encodeArgs());
	ABI_CHECK(callContractFunction("g(int256,int256)", u256(1), u256(-1)), encodeArgs());
}

BOOST_AUTO_TEST_CASE(shift_negative_rvalue_assignment)
{
	char const* sourceCode = R"(
		contract C {
			function f(int a, int b) public returns (int) {
				a <<= b;
				return a;
			}
			function g(int a, int b) public returns (int) {
				a >>= b;
				return a;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f(int256,int256)", u256(1), u256(-1)), encodeArgs());
	ABI_CHECK(callContractFunction("g(int256,int256)", u256(1), u256(-1)), encodeArgs());
}

BOOST_AUTO_TEST_CASE(shift_constant_left_assignment)
{
	char const* sourceCode = R"(
		contract C {
			function f() public returns (uint a) {
				a = 0x42;
				a <<= 8;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(0x4200)));
}

BOOST_AUTO_TEST_CASE(shift_constant_right_assignment)
{
	char const* sourceCode = R"(
		contract C {
			function f() public returns (uint a) {
				a = 0x4200;
				a >>= 8;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(0x42)));
}

BOOST_AUTO_TEST_CASE(shift_cleanup)
{
	char const* sourceCode = R"(
		contract C {
			function f() public returns (uint16 x) {
				x = 0xffff;
				x += 32;
				x <<= 8;
				x >>= 16;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(0x0)));
}

BOOST_AUTO_TEST_CASE(shift_cleanup_garbled)
{
	char const* sourceCode = R"(
		contract C {
			function f() public returns (uint8 x) {
				assembly {
					x := 0xffff
				}
				x >>= 8;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(0x0)));
}

BOOST_AUTO_TEST_CASE(shift_overflow)
{
	char const* sourceCode = R"(
		contract C {
			function leftU(uint8 x, uint8 y) public returns (uint8) {
				return x << y;
			}
			function leftS(int8 x, int8 y) public returns (int8) {
				return x << y;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("leftU(uint8,uint8)", 255, 8), encodeArgs(u256(0)));
	ABI_CHECK(callContractFunction("leftU(uint8,uint8)", 255, 1), encodeArgs(u256(254)));
	ABI_CHECK(callContractFunction("leftU(uint8,uint8)", 255, 0), encodeArgs(u256(255)));

	// Result is -128 and output is sign-extended, not zero-padded.
	ABI_CHECK(callContractFunction("leftS(int8,int8)", 1, 7), encodeArgs(u256(0) - 128));
	ABI_CHECK(callContractFunction("leftS(int8,int8)", 1, 6), encodeArgs(u256(64)));
}

BOOST_AUTO_TEST_CASE(shift_bytes)
{
	char const* sourceCode = R"(
		contract C {
			function left(bytes20 x, uint8 y) public returns (bytes20) {
				return x << y;
			}
			function right(bytes20 x, uint8 y) public returns (bytes20) {
				return x >> y;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("left(bytes20,uint8)", "12345678901234567890", 8 * 8), encodeArgs("901234567890" + string(8, 0)));
	ABI_CHECK(callContractFunction("right(bytes20,uint8)", "12345678901234567890", 8 * 8), encodeArgs(string(8, 0) + "123456789012"));
}

BOOST_AUTO_TEST_CASE(shift_bytes_cleanup)
{
	char const* sourceCode = R"(
		contract C {
			function left(uint8 y) public returns (bytes20) {
				bytes20 x;
				assembly { x := "12345678901234567890abcde" }
				return x << y;
			}
			function right(uint8 y) public returns (bytes20) {
				bytes20 x;
				assembly { x := "12345678901234567890abcde" }
				return x >> y;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("left(uint8)", 8 * 8), encodeArgs("901234567890" + string(8, 0)));
	ABI_CHECK(callContractFunction("right(uint8)", 8 * 8), encodeArgs(string(8, 0) + "123456789012"));
}

BOOST_AUTO_TEST_CASE(exp_cleanup)
{
	char const* sourceCode = R"(
		contract C {
			function f() public pure returns (uint8 x) {
				uint8 y = uint8(2) ** uint8(8);
				return 0 ** y;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(0x1)));
}

BOOST_AUTO_TEST_CASE(exp_cleanup_direct)
{
	char const* sourceCode = R"(
		contract C {
			function f() public pure returns (uint8 x) {
				return uint8(0) ** uint8(uint8(2) ** uint8(8));
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(0x1)));
}

BOOST_AUTO_TEST_CASE(exp_cleanup_nonzero_base)
{
	char const* sourceCode = R"(
		contract C {
			function f() public pure returns (uint8 x) {
				return uint8(0x166) ** uint8(uint8(2) ** uint8(8));
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(0x1)));
}

BOOST_AUTO_TEST_CASE(cleanup_in_compound_assign)
{
	char const* sourceCode = R"(
		contract C {
			function test() public returns (uint, uint) {
				uint32 a = 0xffffffff;
				uint16 x = uint16(a);
				uint16 y = x;
				x /= 0x100;
				y = y / 0x100;
				return (x, y);
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("test()"), encodeArgs(u256(0xff), u256(0xff)));
}

BOOST_AUTO_TEST_CASE(inline_assembly_in_modifiers)
{
	char const* sourceCode = R"(
		contract C {
			modifier m {
				uint a = 1;
				assembly {
					a := 2
				}
				if (a != 2)
					revert();
				_;
			}
			function f() m public returns (bool) {
				return true;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(true));
}

BOOST_AUTO_TEST_CASE(packed_storage_overflow)
{
	char const* sourceCode = R"(
		contract C {
			uint16 x = 0x1234;
			uint16 a = 0xffff;
			uint16 b;
			function f() public returns (uint, uint, uint, uint) {
				a++;
				uint c = b;
				delete b;
				a -= 2;
				return (x, c, b, a);
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(0x1234), u256(0), u256(0), u256(0xfffe)));
}

BOOST_AUTO_TEST_CASE(contracts_separated_with_comment)
{
	char const* sourceCode = R"(
		contract C1 {}
		/**
		**/
		contract C2 {}
	)";
	compileAndRun(sourceCode, 0, "C1");
	compileAndRun(sourceCode, 0, "C2");
}


BOOST_AUTO_TEST_CASE(include_creation_bytecode_only_once)
{
	char const* sourceCode = R"(
		contract D {
			bytes a = hex"1237651237125387136581271652831736512837126583171583712358126123765123712538713658127165283173651283712658317158371235812612376512371253871365812716528317365128371265831715837123581261237651237125387136581271652831736512837126583171583712358126";
			bytes b = hex"1237651237125327136581271252831736512837126583171383712358126123765125712538713658127165253173651283712658357158371235812612376512371a5387136581271652a317365128371265a317158371235812612a765123712538a13658127165a83173651283712a58317158371235a126";
			constructor(uint) public {}
		}
		contract Double {
			function f() public {
				new D(2);
			}
			function g() public {
				new D(3);
			}
		}
		contract Single {
			function f() public {
				new D(2);
			}
		}
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK_LE(
		double(m_compiler.object("Double").bytecode.size()),
		1.1 * double(m_compiler.object("Single").bytecode.size())
	);
}

BOOST_AUTO_TEST_CASE(recursive_structs)
{
	char const* sourceCode = R"(
		contract C {
			struct S {
				S[] x;
			}
			S sstorage;
			function f() public returns (uint) {
				S memory s;
				s.x = new S[](10);
				delete s;
				sstorage.x.length++;
				delete sstorage;
				return 1;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(1)));
}

BOOST_AUTO_TEST_CASE(invalid_instruction)
{
	char const* sourceCode = R"(
		contract C {
			function f() public {
				assembly {
					invalid()
				}
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs());
}

BOOST_AUTO_TEST_CASE(assert_require)
{
	char const* sourceCode = R"(
		contract C {
			function f() public {
				assert(false);
			}
			function g(bool val) public returns (bool) {
				assert(val == true);
				return true;
			}
			function h(bool val) public returns (bool) {
				require(val);
				return true;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs());
	ABI_CHECK(callContractFunction("g(bool)", false), encodeArgs());
	ABI_CHECK(callContractFunction("g(bool)", true), encodeArgs(true));
	ABI_CHECK(callContractFunction("h(bool)", false), encodeArgs());
	ABI_CHECK(callContractFunction("h(bool)", true), encodeArgs(true));
}

BOOST_AUTO_TEST_CASE(revert)
{
	char const* sourceCode = R"(
		contract C {
			uint public a = 42;
			function f() public {
				a = 1;
				revert();
			}
			function g() public {
				a = 1;
				assembly {
					revert(0, 0)
				}
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs());
	ABI_CHECK(callContractFunction("a()"), encodeArgs(u256(42)));
	ABI_CHECK(callContractFunction("g()"), encodeArgs());
	ABI_CHECK(callContractFunction("a()"), encodeArgs(u256(42)));
}

BOOST_AUTO_TEST_CASE(revert_with_cause)
{
	char const* sourceCode = R"(
		contract D {
			string constant msg1 = "test1234567890123456789012345678901234567890";
			string msg2 = "test1234567890123456789012345678901234567890";
			function f() public {
				revert("test123");
			}
			function g() public {
				revert("test1234567890123456789012345678901234567890");
			}
			function h() public {
				revert(msg1);
			}
			function i() public {
				revert(msg2);
			}
			function j() public {
				string memory msg3 = "test1234567890123456789012345678901234567890";
				revert(msg3);
			}
		}
		contract C {
			D d = new D();
			function forward(address target, bytes memory data) internal returns (bool success, bytes memory retval) {
				uint retsize;
				assembly {
					success := call(not(0), target, 0, add(data, 0x20), mload(data), 0, 0)
					retsize := returndatasize()
				}
				retval = new bytes(retsize);
				assembly {
					returndatacopy(add(retval, 0x20), 0, returndatasize())
				}
			}
			function f() public returns (bool, bytes memory) {
				return forward(address(d), msg.data);
			}
			function g() public returns (bool, bytes memory) {
				return forward(address(d), msg.data);
			}
			function h() public returns (bool, bytes memory) {
				return forward(address(d), msg.data);
			}
			function i() public returns (bool, bytes memory) {
				return forward(address(d), msg.data);
			}
			function j() public returns (bool, bytes memory) {
				return forward(address(d), msg.data);
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	bool const haveReturndata = dev::test::Options::get().evmVersion().supportsReturndata();
	bytes const errorSignature = bytes{0x08, 0xc3, 0x79, 0xa0};
	ABI_CHECK(callContractFunction("f()"), haveReturndata ? encodeArgs(0, 0x40, 0x64) + errorSignature + encodeArgs(0x20, 7, "test123") + bytes(28, 0) : bytes());
	ABI_CHECK(callContractFunction("g()"), haveReturndata ? encodeArgs(0, 0x40, 0x84) + errorSignature + encodeArgs(0x20, 44, "test1234567890123456789012345678901234567890") + bytes(28, 0): bytes());
	ABI_CHECK(callContractFunction("h()"), haveReturndata ? encodeArgs(0, 0x40, 0x84) + errorSignature + encodeArgs(0x20, 44, "test1234567890123456789012345678901234567890") + bytes(28, 0): bytes());
	ABI_CHECK(callContractFunction("i()"), haveReturndata ? encodeArgs(0, 0x40, 0x84) + errorSignature + encodeArgs(0x20, 44, "test1234567890123456789012345678901234567890") + bytes(28, 0): bytes());
	ABI_CHECK(callContractFunction("j()"), haveReturndata ? encodeArgs(0, 0x40, 0x84) + errorSignature + encodeArgs(0x20, 44, "test1234567890123456789012345678901234567890") + bytes(28, 0): bytes());
}

BOOST_AUTO_TEST_CASE(require_with_message)
{
	char const* sourceCode = R"(
		contract D {
			bool flag = false;
			string storageError = "abc";
			string constant constantError = "abc";
			function f(uint x) public {
				require(x > 7, "failed");
			}
			function g() public {
				// As a side-effect of internalFun, the flag will be set to true
				// (even if the condition is true),
				// but it will only throw in the next evaluation.
				bool flagCopy = flag;
				require(flagCopy == false, internalFun());
			}
			function internalFun() public returns (string memory) {
				flag = true;
				return "only on second run";
			}
			function h() public {
				require(false, storageError);
			}
			function i() public {
				require(false, constantError);
			}
			function j() public {
				string memory errMsg = "msg";
				require(false, errMsg);
			}
		}
		contract C {
			D d = new D();
			function forward(address target, bytes memory data) internal returns (bool success, bytes memory retval) {
				uint retsize;
				assembly {
					success := call(not(0), target, 0, add(data, 0x20), mload(data), 0, 0)
					retsize := returndatasize()
				}
				retval = new bytes(retsize);
				assembly {
					returndatacopy(add(retval, 0x20), 0, returndatasize())
				}
			}
			function f(uint x) public returns (bool, bytes memory) {
				return forward(address(d), msg.data);
			}
			function g() public returns (bool, bytes memory) {
				return forward(address(d), msg.data);
			}
			function h() public returns (bool, bytes memory) {
				return forward(address(d), msg.data);
			}
			function i() public returns (bool, bytes memory) {
				return forward(address(d), msg.data);
			}
			function j() public returns (bool, bytes memory) {
				return forward(address(d), msg.data);
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	bool const haveReturndata = dev::test::Options::get().evmVersion().supportsReturndata();
	bytes const errorSignature = bytes{0x08, 0xc3, 0x79, 0xa0};
	ABI_CHECK(callContractFunction("f(uint256)", 8), haveReturndata ? encodeArgs(1, 0x40, 0) : bytes());
	ABI_CHECK(callContractFunction("f(uint256)", 5), haveReturndata ? encodeArgs(0, 0x40, 0x64) + errorSignature + encodeArgs(0x20, 6, "failed") + bytes(28, 0) : bytes());
	ABI_CHECK(callContractFunction("g()"), haveReturndata ? encodeArgs(1, 0x40, 0) : bytes());
	ABI_CHECK(callContractFunction("g()"), haveReturndata ? encodeArgs(0, 0x40, 0x64) + errorSignature + encodeArgs(0x20, 18, "only on second run") + bytes(28, 0) : bytes());
	ABI_CHECK(callContractFunction("h()"), haveReturndata ? encodeArgs(0, 0x40, 0x64) + errorSignature + encodeArgs(0x20, 3, "abc") + bytes(28, 0): bytes());
	ABI_CHECK(callContractFunction("i()"), haveReturndata ? encodeArgs(0, 0x40, 0x64) + errorSignature + encodeArgs(0x20, 3, "abc") + bytes(28, 0): bytes());
	ABI_CHECK(callContractFunction("j()"), haveReturndata ? encodeArgs(0, 0x40, 0x64) + errorSignature + encodeArgs(0x20, 3, "msg") + bytes(28, 0): bytes());
}

BOOST_AUTO_TEST_CASE(bubble_up_error_messages)
{
	char const* sourceCode = R"(
		contract D {
			function f() public {
				revert("message");
			}
			function g() public {
				this.f();
			}
		}
		contract C {
			D d = new D();
			function forward(address target, bytes memory data) internal returns (bool success, bytes memory retval) {
				uint retsize;
				assembly {
					success := call(not(0), target, 0, add(data, 0x20), mload(data), 0, 0)
					retsize := returndatasize()
				}
				retval = new bytes(retsize);
				assembly {
					returndatacopy(add(retval, 0x20), 0, returndatasize())
				}
			}
			function f() public returns (bool, bytes memory) {
				return forward(address(d), msg.data);
			}
			function g() public returns (bool, bytes memory) {
				return forward(address(d), msg.data);
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	bool const haveReturndata = dev::test::Options::get().evmVersion().supportsReturndata();
	bytes const errorSignature = bytes{0x08, 0xc3, 0x79, 0xa0};
	ABI_CHECK(callContractFunction("f()"), haveReturndata ? encodeArgs(0, 0x40, 0x64) + errorSignature + encodeArgs(0x20, 7, "message") + bytes(28, 0) : bytes());
	ABI_CHECK(callContractFunction("g()"), haveReturndata ? encodeArgs(0, 0x40, 0x64) + errorSignature + encodeArgs(0x20, 7, "message") + bytes(28, 0) : bytes());
}

BOOST_AUTO_TEST_CASE(bubble_up_error_messages_through_transfer)
{
	char const* sourceCode = R"(
		contract D {
			function() external payable {
				revert("message");
			}
			function f() public {
				address(this).transfer(0);
			}
		}
		contract C {
			D d = new D();
			function forward(address target, bytes memory data) internal returns (bool success, bytes memory retval) {
				uint retsize;
				assembly {
					success := call(not(0), target, 0, add(data, 0x20), mload(data), 0, 0)
					retsize := returndatasize()
				}
				retval = new bytes(retsize);
				assembly {
					returndatacopy(add(retval, 0x20), 0, returndatasize())
				}
			}
			function f() public returns (bool, bytes memory) {
				return forward(address(d), msg.data);
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	bool const haveReturndata = dev::test::Options::get().evmVersion().supportsReturndata();
	bytes const errorSignature = bytes{0x08, 0xc3, 0x79, 0xa0};
	ABI_CHECK(callContractFunction("f()"), haveReturndata ? encodeArgs(0, 0x40, 0x64) + errorSignature + encodeArgs(0x20, 7, "message") + bytes(28, 0) : bytes());
}

BOOST_AUTO_TEST_CASE(bubble_up_error_messages_through_create)
{
	char const* sourceCode = R"(
		contract E {
			constructor() public {
				revert("message");
			}
		}
		contract D {
			function f() public {
				E x = new E();
			}
		}
		contract C {
			D d = new D();
			function forward(address target, bytes memory data) internal returns (bool success, bytes memory retval) {
				uint retsize;
				assembly {
					success := call(not(0), target, 0, add(data, 0x20), mload(data), 0, 0)
					retsize := returndatasize()
				}
				retval = new bytes(retsize);
				assembly {
					returndatacopy(add(retval, 0x20), 0, returndatasize())
				}
			}
			function f() public returns (bool, bytes memory) {
				return forward(address(d), msg.data);
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	bool const haveReturndata = dev::test::Options::get().evmVersion().supportsReturndata();
	bytes const errorSignature = bytes{0x08, 0xc3, 0x79, 0xa0};
	ABI_CHECK(callContractFunction("f()"), haveReturndata ? encodeArgs(0, 0x40, 0x64) + errorSignature + encodeArgs(0x20, 7, "message") + bytes(28, 0) : bytes());
}

BOOST_AUTO_TEST_CASE(negative_stack_height)
{
	// This code was causing negative stack height during code generation
	// because the stack height was not adjusted at the beginning of functions.
	char const* sourceCode = R"(
		contract C {
			mapping(uint => Invoice) public invoices;
			struct Invoice {
				uint AID;
				bool Aboola;
				bool Aboolc;
				bool exists;
			}
			function nredit(uint startindex) public pure returns(uint[500] memory CIDs, uint[500] memory dates, uint[500] memory RIDs, bool[500] memory Cboolas, uint[500] memory amounts){}
			function return500InvoicesByDates(uint begindate, uint enddate, uint startindex) public view returns(uint[500] memory AIDs, bool[500] memory Aboolas, uint[500] memory dates, bytes32[3][500] memory Abytesas, bytes32[3][500] memory bytesbs, bytes32[2][500] memory bytescs, uint[500] memory amounts, bool[500] memory Aboolbs, bool[500] memory Aboolcs){}
			function return500PaymentsByDates(uint begindate, uint enddate, uint startindex) public view returns(uint[500] memory BIDs, uint[500] memory dates, uint[500] memory RIDs, bool[500] memory Bboolas, bytes32[3][500] memory bytesbs,bytes32[2][500] memory bytescs, uint[500] memory amounts, bool[500] memory Bboolbs){}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
}

BOOST_AUTO_TEST_CASE(literal_empty_string)
{
	char const* sourceCode = R"(
		contract C {
			bytes32 public x;
			uint public a;
			function f(bytes32 _x, uint _a) public {
				x = _x;
				a = _a;
			}
			function g() public {
				this.f("", 2);
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("x()"), encodeArgs(u256(0)));
	ABI_CHECK(callContractFunction("a()"), encodeArgs(u256(0)));
	ABI_CHECK(callContractFunction("g()"), encodeArgs());
	ABI_CHECK(callContractFunction("x()"), encodeArgs(u256(0)));
	ABI_CHECK(callContractFunction("a()"), encodeArgs(u256(2)));
}

BOOST_AUTO_TEST_CASE(scientific_notation)
{
	char const* sourceCode = R"(
		contract C {
			function f() public returns (uint) {
				return 2e10 wei;
			}
			function g() public returns (uint) {
				return 200e-2 wei;
			}
			function h() public returns (uint) {
				return 2.5e1;
			}
			function i() public returns (int) {
				return -2e10;
			}
			function j() public returns (int) {
				return -200e-2;
			}
			function k() public returns (int) {
				return -2.5e1;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(20000000000)));
	ABI_CHECK(callContractFunction("g()"), encodeArgs(u256(2)));
	ABI_CHECK(callContractFunction("h()"), encodeArgs(u256(25)));
	ABI_CHECK(callContractFunction("i()"), encodeArgs(u256(-20000000000)));
	ABI_CHECK(callContractFunction("j()"), encodeArgs(u256(-2)));
	ABI_CHECK(callContractFunction("k()"), encodeArgs(u256(-25)));
}

BOOST_AUTO_TEST_CASE(interface_contract)
{
	char const* sourceCode = R"(
		interface I {
			event A();
			function f() external returns (bool);
			function() external payable;
		}

		contract A is I {
			function f() public returns (bool) {
				return g();
			}

			function g() public returns (bool) {
				return true;
			}

			function() external payable {
			}
		}

		contract C {
			function f(address payable _interfaceAddress) public returns (bool) {
				I i = I(_interfaceAddress);
				return i.f();
			}
		}
	)";
	compileAndRun(sourceCode, 0, "A");
	u160 const recipient = m_contractAddress;
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f(address)", recipient), encodeArgs(true));
}

BOOST_AUTO_TEST_CASE(keccak256_assembly)
{
	char const* sourceCode = R"(
		contract C {
			function f() public pure returns (bytes32 ret) {
				assembly {
					ret := keccak256(0, 0)
				}
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), fromHex("0xc5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470"));
}

BOOST_AUTO_TEST_CASE(multi_modifiers)
{
	// This triggered a bug in some version because the variable in the modifier was not
	// unregistered correctly.
	char const* sourceCode = R"(
		contract C {
			uint public x;
			modifier m1 {
				address a1 = msg.sender;
				x++;
				_;
			}
			function f1() m1() public {
				x += 7;
			}
			function f2() m1() public {
				x += 3;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f1()"), bytes());
	ABI_CHECK(callContractFunction("x()"), encodeArgs(u256(8)));
	ABI_CHECK(callContractFunction("f2()"), bytes());
	ABI_CHECK(callContractFunction("x()"), encodeArgs(u256(12)));
}

BOOST_AUTO_TEST_CASE(inlineasm_empty_let)
{
	char const* sourceCode = R"(
		contract C {
			function f() public pure returns (uint a, uint b) {
				assembly {
					let x
					let y, z
					a := x
					b := z
				}
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(0), u256(0)));
}

BOOST_AUTO_TEST_CASE(bare_call_invalid_address)
{
	char const* sourceCode = R"YY(
		contract C {
			/// Calling into non-existant account is successful (creates the account)
			function f() external returns (bool) {
				(bool success,) = address(0x4242).call("");
				return success;
			}
			function h() external returns (bool) {
				(bool success,) = address(0x4242).delegatecall("");
				return success;
			}
		}
	)YY";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(1)));
	ABI_CHECK(callContractFunction("h()"), encodeArgs(u256(1)));

	if (dev::test::Options::get().evmVersion().hasStaticCall())
	{
		char const* sourceCode = R"YY(
			contract C {
				function f() external returns (bool, bytes memory) {
					return address(0x4242).staticcall("");
				}
			}
		)YY";
		compileAndRun(sourceCode, 0, "C");
		ABI_CHECK(callContractFunction("f()"), encodeArgs(u256(1), 0x40, 0x00));
	}
}

BOOST_AUTO_TEST_CASE(bare_call_return_data)
{
	if (dev::test::Options::get().evmVersion().supportsReturndata())
	{
		vector<string> calltypes = {"call", "delegatecall"};
		if (dev::test::Options::get().evmVersion().hasStaticCall())
			calltypes.emplace_back("staticcall");
		for (string const& calltype: calltypes)
		{
			string sourceCode = R"DELIMITER(
				contract A {
					constructor() public {
					}
					function return_bool() public pure returns(bool) {
						return true;
					}
					function return_int32() public pure returns(int32) {
						return -32;
					}
					function return_uint32() public pure returns(uint32) {
						return 0x3232;
					}
					function return_int256() public pure returns(int256) {
						return -256;
					}
					function return_uint256() public pure returns(uint256) {
						return 0x256256;
					}
					function return_bytes4() public pure returns(bytes4) {
						return 0xabcd0012;
					}
					function return_multi() public pure returns(bool, uint32, bytes4) {
						return (false, 0x3232, 0xabcd0012);
					}
					function return_bytes() public pure returns(bytes memory b) {
						b = new bytes(2);
						b[0] = 0x42;
						b[1] = 0x21;
					}
				}
				contract C {
					A addr;
					constructor() public {
						addr = new A();
					}
					function f(string memory signature) public returns (bool, bytes memory) {
						return address(addr).)DELIMITER" + calltype + R"DELIMITER((abi.encodeWithSignature(signature));
					}
					function check_bool() external returns (bool) {
						(bool success, bytes memory data) = f("return_bool()");
						assert(success);
						bool a = abi.decode(data, (bool));
						assert(a);
						return true;
					}
					function check_int32() external returns (bool) {
						(bool success, bytes memory data) = f("return_int32()");
						assert(success);
						int32 a = abi.decode(data, (int32));
						assert(a == -32);
						return true;
					}
					function check_uint32() external returns (bool) {
						(bool success, bytes memory data) = f("return_uint32()");
						assert(success);
						uint32 a = abi.decode(data, (uint32));
						assert(a == 0x3232);
						return true;
					}
					function check_int256() external returns (bool) {
						(bool success, bytes memory data) = f("return_int256()");
						assert(success);
						int256 a = abi.decode(data, (int256));
						assert(a == -256);
						return true;
					}
					function check_uint256() external returns (bool) {
						(bool success, bytes memory data) = f("return_uint256()");
						assert(success);
						uint256 a = abi.decode(data, (uint256));
						assert(a == 0x256256);
						return true;
					}
					function check_bytes4() external returns (bool) {
						(bool success, bytes memory data) = f("return_bytes4()");
						assert(success);
						bytes4 a = abi.decode(data, (bytes4));
						assert(a == 0xabcd0012);
						return true;
					}
					function check_multi() external returns (bool) {
						(bool success, bytes memory data) = f("return_multi()");
						assert(success);
						(bool a, uint32 b, bytes4 c) = abi.decode(data, (bool, uint32, bytes4));
						assert(a == false && b == 0x3232 && c == 0xabcd0012);
						return true;
					}
					function check_bytes() external returns (bool) {
						(bool success, bytes memory data) = f("return_bytes()");
						assert(success);
						(bytes memory d) = abi.decode(data, (bytes));
						assert(d.length == 2 && d[0] == 0x42 && d[1] == 0x21);
						return true;
					}
				}
			)DELIMITER";
			compileAndRun(sourceCode, 0, "C");
			ABI_CHECK(callContractFunction("f(string)", encodeDyn(string("return_bool()"))), encodeArgs(true, 0x40, 0x20, true));
			ABI_CHECK(callContractFunction("f(string)", encodeDyn(string("return_int32()"))), encodeArgs(true, 0x40, 0x20, u256(-32)));
			ABI_CHECK(callContractFunction("f(string)", encodeDyn(string("return_uint32()"))), encodeArgs(true, 0x40, 0x20, u256(0x3232)));
			ABI_CHECK(callContractFunction("f(string)", encodeDyn(string("return_int256()"))), encodeArgs(true, 0x40, 0x20, u256(-256)));
			ABI_CHECK(callContractFunction("f(string)", encodeDyn(string("return_uint256()"))), encodeArgs(true, 0x40, 0x20, u256(0x256256)));
			ABI_CHECK(callContractFunction("f(string)", encodeDyn(string("return_bytes4()"))), encodeArgs(true, 0x40, 0x20, u256(0xabcd0012) << (28*8)));
			ABI_CHECK(callContractFunction("f(string)", encodeDyn(string("return_multi()"))), encodeArgs(true, 0x40, 0x60, false, u256(0x3232), u256(0xabcd0012) << (28*8)));
			ABI_CHECK(callContractFunction("f(string)", encodeDyn(string("return_bytes()"))), encodeArgs(true, 0x40, 0x60, 0x20, 0x02, encode(bytes{0x42,0x21}, false)));
			ABI_CHECK(callContractFunction("check_bool()"), encodeArgs(true));
			ABI_CHECK(callContractFunction("check_int32()"), encodeArgs(true));
			ABI_CHECK(callContractFunction("check_uint32()"), encodeArgs(true));
			ABI_CHECK(callContractFunction("check_int256()"), encodeArgs(true));
			ABI_CHECK(callContractFunction("check_uint256()"), encodeArgs(true));
			ABI_CHECK(callContractFunction("check_bytes4()"), encodeArgs(true));
			ABI_CHECK(callContractFunction("check_multi()"), encodeArgs(true));
			ABI_CHECK(callContractFunction("check_bytes()"), encodeArgs(true));
		}
	}
}

BOOST_AUTO_TEST_CASE(delegatecall_return_value)
{
	if (dev::test::Options::get().evmVersion().supportsReturndata())
	{
		char const* sourceCode = R"DELIMITER(
			contract C {
				uint value;
				function set(uint _value) external {
					value = _value;
				}
				function get() external view returns (uint) {
					return value;
				}
				function get_delegated() external returns (bool, bytes memory) {
					return address(this).delegatecall(abi.encodeWithSignature("get()"));
				}
				function assert0() external view {
					assert(value == 0);
				}
				function assert0_delegated() external returns (bool, bytes memory) {
					return address(this).delegatecall(abi.encodeWithSignature("assert0()"));
				}
			}
		)DELIMITER";
		compileAndRun(sourceCode, 0, "C");
		ABI_CHECK(callContractFunction("get()"), encodeArgs(u256(0)));
		ABI_CHECK(callContractFunction("assert0_delegated()"), encodeArgs(u256(1), 0x40, 0x00));
		ABI_CHECK(callContractFunction("get_delegated()"), encodeArgs(u256(1), 0x40, 0x20, 0x00));
		ABI_CHECK(callContractFunction("set(uint256)", u256(1)), encodeArgs());
		ABI_CHECK(callContractFunction("get()"), encodeArgs(u256(1)));
		ABI_CHECK(callContractFunction("assert0_delegated()"), encodeArgs(u256(0), 0x40, 0x00));
		ABI_CHECK(callContractFunction("get_delegated()"), encodeArgs(u256(1), 0x40, 0x20, 1));
		ABI_CHECK(callContractFunction("set(uint256)", u256(42)), encodeArgs());
		ABI_CHECK(callContractFunction("get()"), encodeArgs(u256(42)));
		ABI_CHECK(callContractFunction("assert0_delegated()"), encodeArgs(u256(0), 0x40, 0x00));
		ABI_CHECK(callContractFunction("get_delegated()"), encodeArgs(u256(1), 0x40, 0x20, 42));
	}
	else
	{
		char const* sourceCode = R"DELIMITER(
			contract C {
				uint value;
				function set(uint _value) external {
					value = _value;
				}
				function get() external view returns (uint) {
					return value;
				}
				function get_delegated() external returns (bool) {
					(bool success,) = address(this).delegatecall(abi.encodeWithSignature("get()"));
					return success;
				}
				function assert0() external view {
					assert(value == 0);
				}
				function assert0_delegated() external returns (bool) {
					(bool success,) = address(this).delegatecall(abi.encodeWithSignature("assert0()"));
					return success;
				}
			}
		)DELIMITER";
		compileAndRun(sourceCode, 0, "C");
		ABI_CHECK(callContractFunction("get()"), encodeArgs(u256(0)));
		ABI_CHECK(callContractFunction("assert0_delegated()"), encodeArgs(u256(1)));
		ABI_CHECK(callContractFunction("get_delegated()"), encodeArgs(u256(1)));
		ABI_CHECK(callContractFunction("set(uint256)", u256(1)), encodeArgs());
		ABI_CHECK(callContractFunction("get()"), encodeArgs(u256(1)));
		ABI_CHECK(callContractFunction("assert0_delegated()"), encodeArgs(u256(0)));
		ABI_CHECK(callContractFunction("get_delegated()"), encodeArgs(u256(1)));
		ABI_CHECK(callContractFunction("set(uint256)", u256(42)), encodeArgs());
		ABI_CHECK(callContractFunction("get()"), encodeArgs(u256(42)));
		ABI_CHECK(callContractFunction("assert0_delegated()"), encodeArgs(u256(0)));
		ABI_CHECK(callContractFunction("get_delegated()"), encodeArgs(u256(1)));
	}
}

BOOST_AUTO_TEST_CASE(function_types_sig)
{
	char const* sourceCode = R"(
		contract C {
			uint public x;
			function f() public pure returns (bytes4) {
				return this.f.selector;
			}
			function g() public returns (bytes4) {
				function () pure external returns (bytes4) fun = this.f;
				return fun.selector;
			}
			function h() public returns (bytes4) {
				function () pure external returns (bytes4) fun = this.f;
				return fun.selector;
			}
			function i() public pure returns (bytes4) {
				return this.x.selector;
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(asString(FixedHash<4>(dev::keccak256("f()")).asBytes())));
	ABI_CHECK(callContractFunction("g()"), encodeArgs(asString(FixedHash<4>(dev::keccak256("f()")).asBytes())));
	ABI_CHECK(callContractFunction("h()"), encodeArgs(asString(FixedHash<4>(dev::keccak256("f()")).asBytes())));
	ABI_CHECK(callContractFunction("i()"), encodeArgs(asString(FixedHash<4>(dev::keccak256("x()")).asBytes())));
}

BOOST_AUTO_TEST_CASE(constant_string)
{
	char const* sourceCode = R"(
		contract C {
			bytes constant a = "\x03\x01\x02";
			bytes constant b = hex"030102";
			string constant c = "hello";
			function f() public returns (bytes memory) {
				return a;
			}
			function g() public returns (bytes memory) {
				return b;
			}
			function h() public returns (bytes memory) {
				return bytes(c);
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeDyn(string("\x03\x01\x02")));
	ABI_CHECK(callContractFunction("g()"), encodeDyn(string("\x03\x01\x02")));
	ABI_CHECK(callContractFunction("h()"), encodeDyn(string("hello")));
}

BOOST_AUTO_TEST_CASE(address_overload_resolution)
{
	char const* sourceCode = R"(
		contract C {
			function balance() public returns (uint) {
				return 1;
			}
			function transfer(uint amount) public returns (uint) {
				return amount;
			}
		}
		contract D {
			function f() public returns (uint) {
				return (new C()).balance();
			}
			function g() public returns (uint) {
				return (new C()).transfer(5);
			}
		}
	)";
	compileAndRun(sourceCode, 0, "D");
	BOOST_CHECK(callContractFunction("f()") == encodeArgs(u256(1)));
	BOOST_CHECK(callContractFunction("g()") == encodeArgs(u256(5)));
}

BOOST_AUTO_TEST_CASE(snark)
{
	char const* sourceCode = R"(
	library Pairing {
		struct G1Point {
			uint X;
			uint Y;
		}
		// Encoding of field elements is: X[0] * z + X[1]
		struct G2Point {
			uint[2] X;
			uint[2] Y;
		}

		/// @return the generator of G1
		function P1() internal returns (G1Point memory) {
			return G1Point(1, 2);
		}

		/// @return the generator of G2
		function P2() internal returns (G2Point memory) {
			return G2Point(
				[11559732032986387107991004021392285783925812861821192530917403151452391805634,
				 10857046999023057135944570762232829481370756359578518086990519993285655852781],
				[4082367875863433681332203403145435568316851327593401208105741076214120093531,
				 8495653923123431417604973247489272438418190587263600148770280649306958101930]
			);
		}

		/// @return the negation of p, i.e. p.add(p.negate()) should be zero.
		function negate(G1Point memory p) internal returns (G1Point memory) {
			// The prime q in the base field F_q for G1
			uint q = 21888242871839275222246405745257275088696311157297823662689037894645226208583;
			if (p.X == 0 && p.Y == 0)
				return G1Point(0, 0);
			return G1Point(p.X, q - (p.Y % q));
		}

		/// @return the sum of two points of G1
		function add(G1Point memory p1, G1Point memory p2) internal returns (G1Point memory r) {
			uint[4] memory input;
			input[0] = p1.X;
			input[1] = p1.Y;
			input[2] = p2.X;
			input[3] = p2.Y;
			bool success;
			assembly {
				success := call(sub(gas, 2000), 6, 0, input, 0xc0, r, 0x60)
				// Use "invalid" to make gas estimation work
				switch success case 0 { invalid() }
			}
			require(success);
		}

		/// @return the product of a point on G1 and a scalar, i.e.
		/// p == p.mul(1) and p.add(p) == p.mul(2) for all points p.
		function mul(G1Point memory p, uint s) internal returns (G1Point memory r) {
			uint[3] memory input;
			input[0] = p.X;
			input[1] = p.Y;
			input[2] = s;
			bool success;
			assembly {
				success := call(sub(gas, 2000), 7, 0, input, 0x80, r, 0x60)
				// Use "invalid" to make gas estimation work
				switch success case 0 { invalid() }
			}
			require(success);
		}

		/// @return the result of computing the pairing check
		/// e(p1[0], p2[0]) *  .... * e(p1[n], p2[n]) == 1
		/// For example pairing([P1(), P1().negate()], [P2(), P2()]) should
		/// return true.
		function pairing(G1Point[] memory p1, G2Point[] memory p2) internal returns (bool) {
			require(p1.length == p2.length);
			uint elements = p1.length;
			uint inputSize = p1.length * 6;
			uint[] memory input = new uint[](inputSize);
			for (uint i = 0; i < elements; i++)
			{
				input[i * 6 + 0] = p1[i].X;
				input[i * 6 + 1] = p1[i].Y;
				input[i * 6 + 2] = p2[i].X[0];
				input[i * 6 + 3] = p2[i].X[1];
				input[i * 6 + 4] = p2[i].Y[0];
				input[i * 6 + 5] = p2[i].Y[1];
			}
			uint[1] memory out;
			bool success;
			assembly {
				success := call(sub(gas, 2000), 8, 0, add(input, 0x20), mul(inputSize, 0x20), out, 0x20)
				// Use "invalid" to make gas estimation work
				switch success case 0 { invalid() }
			}
			require(success);
			return out[0] != 0;
		}
		function pairingProd2(G1Point memory a1, G2Point memory a2, G1Point memory b1, G2Point memory b2) internal returns (bool) {
			G1Point[] memory p1 = new G1Point[](2);
			G2Point[] memory p2 = new G2Point[](2);
			p1[0] = a1;
			p1[1] = b1;
			p2[0] = a2;
			p2[1] = b2;
			return pairing(p1, p2);
		}
		function pairingProd3(
				G1Point memory a1, G2Point memory a2,
				G1Point memory b1, G2Point memory b2,
				G1Point memory c1, G2Point memory c2
		) internal returns (bool) {
			G1Point[] memory p1 = new G1Point[](3);
			G2Point[] memory p2 = new G2Point[](3);
			p1[0] = a1;
			p1[1] = b1;
			p1[2] = c1;
			p2[0] = a2;
			p2[1] = b2;
			p2[2] = c2;
			return pairing(p1, p2);
		}
		function pairingProd4(
				G1Point memory a1, G2Point memory a2,
				G1Point memory b1, G2Point memory b2,
				G1Point memory c1, G2Point memory c2,
				G1Point memory d1, G2Point memory d2
		) internal returns (bool) {
			G1Point[] memory p1 = new G1Point[](4);
			G2Point[] memory p2 = new G2Point[](4);
			p1[0] = a1;
			p1[1] = b1;
			p1[2] = c1;
			p1[3] = d1;
			p2[0] = a2;
			p2[1] = b2;
			p2[2] = c2;
			p2[3] = d2;
			return pairing(p1, p2);
		}
	}

	contract Test {
		using Pairing for *;
		struct VerifyingKey {
			Pairing.G2Point A;
			Pairing.G1Point B;
			Pairing.G2Point C;
			Pairing.G2Point gamma;
			Pairing.G1Point gammaBeta1;
			Pairing.G2Point gammaBeta2;
			Pairing.G2Point Z;
			Pairing.G1Point[] IC;
		}
		struct Proof {
			Pairing.G1Point A;
			Pairing.G1Point A_p;
			Pairing.G2Point B;
			Pairing.G1Point B_p;
			Pairing.G1Point C;
			Pairing.G1Point C_p;
			Pairing.G1Point K;
			Pairing.G1Point H;
		}
		function f() public returns (bool) {
			Pairing.G1Point memory p1;
			Pairing.G1Point memory p2;
			p1.X = 1; p1.Y = 2;
			p2.X = 1; p2.Y = 2;
			Pairing.G1Point memory explict_sum = Pairing.add(p1, p2);
			Pairing.G1Point memory scalar_prod = Pairing.mul(p1, 2);
			return (explict_sum.X == scalar_prod.X &&
					explict_sum.Y == scalar_prod.Y);
		}
		function g() public returns (bool) {
			Pairing.G1Point memory x = Pairing.add(Pairing.P1(), Pairing.negate(Pairing.P1()));
			// should be zero
			return (x.X == 0 && x.Y == 0);
		}
		function testMul() public returns (bool) {
			Pairing.G1Point memory p;
			// @TODO The points here are reported to be not well-formed
			p.X = 14125296762497065001182820090155008161146766663259912659363835465243039841726;
			p.Y = 16229134936871442251132173501211935676986397196799085184804749187146857848057;
			p = Pairing.mul(p, 13986731495506593864492662381614386532349950841221768152838255933892789078521);
			return
				p.X == 18256332256630856740336504687838346961237861778318632856900758565550522381207 &&
				p.Y == 6976682127058094634733239494758371323697222088503263230319702770853579280803;
		}
		function pair() public returns (bool) {
			Pairing.G2Point memory fiveTimesP2 = Pairing.G2Point(
				[4540444681147253467785307942530223364530218361853237193970751657229138047649, 20954117799226682825035885491234530437475518021362091509513177301640194298072],
				[11631839690097995216017572651900167465857396346217730511548857041925508482915, 21508930868448350162258892668132814424284302804699005394342512102884055673846]
			);
			// The prime p in the base field F_p for G1
			uint p = 21888242871839275222246405745257275088696311157297823662689037894645226208583;
			Pairing.G1Point[] memory g1points = new Pairing.G1Point[](2);
			Pairing.G2Point[] memory g2points = new Pairing.G2Point[](2);
			// check e(5 P1, P2)e(-P1, 5 P2) == 1
			g1points[0] = Pairing.P1().mul(5);
			g1points[1] = Pairing.P1().negate();
			g2points[0] = Pairing.P2();
			g2points[1] = fiveTimesP2;
			if (!Pairing.pairing(g1points, g2points))
				return false;
			// check e(P1, P2)e(-P1, P2) == 1
			g1points[0] = Pairing.P1();
			g1points[1] = Pairing.P1();
			g1points[1].Y = p - g1points[1].Y;
			g2points[0] = Pairing.P2();
			g2points[1] = Pairing.P2();
			if (!Pairing.pairing(g1points, g2points))
				return false;
			return true;
		}
		function verifyingKey() internal returns (VerifyingKey memory vk) {
			vk.A = Pairing.G2Point([0x209dd15ebff5d46c4bd888e51a93cf99a7329636c63514396b4a452003a35bf7, 0x04bf11ca01483bfa8b34b43561848d28905960114c8ac04049af4b6315a41678], [0x2bb8324af6cfc93537a2ad1a445cfd0ca2a71acd7ac41fadbf933c2a51be344d, 0x120a2a4cf30c1bf9845f20c6fe39e07ea2cce61f0c9bb048165fe5e4de877550]);
			vk.B = Pairing.G1Point(0x2eca0c7238bf16e83e7a1e6c5d49540685ff51380f309842a98561558019fc02, 0x03d3260361bb8451de5ff5ecd17f010ff22f5c31cdf184e9020b06fa5997db84);
			vk.C = Pairing.G2Point([0x2e89718ad33c8bed92e210e81d1853435399a271913a6520736a4729cf0d51eb, 0x01a9e2ffa2e92599b68e44de5bcf354fa2642bd4f26b259daa6f7ce3ed57aeb3], [0x14a9a87b789a58af499b314e13c3d65bede56c07ea2d418d6874857b70763713, 0x178fb49a2d6cd347dc58973ff49613a20757d0fcc22079f9abd10c3baee24590]);
			vk.gamma = Pairing.G2Point([0x25f83c8b6ab9de74e7da488ef02645c5a16a6652c3c71a15dc37fe3a5dcb7cb1, 0x22acdedd6308e3bb230d226d16a105295f523a8a02bfc5e8bd2da135ac4c245d], [0x065bbad92e7c4e31bf3757f1fe7362a63fbfee50e7dc68da116e67d600d9bf68, 0x06d302580dc0661002994e7cd3a7f224e7ddc27802777486bf80f40e4ca3cfdb]);
			vk.gammaBeta1 = Pairing.G1Point(0x15794ab061441e51d01e94640b7e3084a07e02c78cf3103c542bc5b298669f21, 0x14db745c6780e9df549864cec19c2daf4531f6ec0c89cc1c7436cc4d8d300c6d);
			vk.gammaBeta2 = Pairing.G2Point([0x1f39e4e4afc4bc74790a4a028aff2c3d2538731fb755edefd8cb48d6ea589b5e, 0x283f150794b6736f670d6a1033f9b46c6f5204f50813eb85c8dc4b59db1c5d39], [0x140d97ee4d2b36d99bc49974d18ecca3e7ad51011956051b464d9e27d46cc25e, 0x0764bb98575bd466d32db7b15f582b2d5c452b36aa394b789366e5e3ca5aabd4]);
			vk.Z = Pairing.G2Point([0x217cee0a9ad79a4493b5253e2e4e3a39fc2df38419f230d341f60cb064a0ac29, 0x0a3d76f140db8418ba512272381446eb73958670f00cf46f1d9e64cba057b53c], [0x26f64a8ec70387a13e41430ed3ee4a7db2059cc5fc13c067194bcc0cb49a9855, 0x2fd72bd9edb657346127da132e5b82ab908f5816c826acb499e22f2412d1a2d7]);
			vk.IC = new Pairing.G1Point[](10);
			vk.IC[0] = Pairing.G1Point(0x0aee46a7ea6e80a3675026dfa84019deee2a2dedb1bbe11d7fe124cb3efb4b5a, 0x044747b6e9176e13ede3a4dfd0d33ccca6321b9acd23bf3683a60adc0366ebaf);
			vk.IC[1] = Pairing.G1Point(0x1e39e9f0f91fa7ff8047ffd90de08785777fe61c0e3434e728fce4cf35047ddc, 0x2e0b64d75ebfa86d7f8f8e08abbe2e7ae6e0a1c0b34d028f19fa56e9450527cb);
			vk.IC[2] = Pairing.G1Point(0x1c36e713d4d54e3a9644dffca1fc524be4868f66572516025a61ca542539d43f, 0x042dcc4525b82dfb242b09cb21909d5c22643dcdbe98c4d082cc2877e96b24db);
			vk.IC[3] = Pairing.G1Point(0x17d5d09b4146424bff7e6fb01487c477bbfcd0cdbbc92d5d6457aae0b6717cc5, 0x02b5636903efbf46db9235bbe74045d21c138897fda32e079040db1a16c1a7a1);
			vk.IC[4] = Pairing.G1Point(0x0f103f14a584d4203c27c26155b2c955f8dfa816980b24ba824e1972d6486a5d, 0x0c4165133b9f5be17c804203af781bcf168da7386620479f9b885ecbcd27b17b);
			vk.IC[5] = Pairing.G1Point(0x232063b584fb76c8d07995bee3a38fa7565405f3549c6a918ddaa90ab971e7f8, 0x2ac9b135a81d96425c92d02296322ad56ffb16299633233e4880f95aafa7fda7);
			vk.IC[6] = Pairing.G1Point(0x09b54f111d3b2d1b2fe1ae9669b3db3d7bf93b70f00647e65c849275de6dc7fe, 0x18b2e77c63a3e400d6d1f1fbc6e1a1167bbca603d34d03edea231eb0ab7b14b4);
			vk.IC[7] = Pairing.G1Point(0x0c54b42137b67cc268cbb53ac62b00ecead23984092b494a88befe58445a244a, 0x18e3723d37fae9262d58b548a0575f59d9c3266db7afb4d5739555837f6b8b3e);
			vk.IC[8] = Pairing.G1Point(0x0a6de0e2240aa253f46ce0da883b61976e3588146e01c9d8976548c145fe6e4a, 0x04fbaa3a4aed4bb77f30ebb07a3ec1c7d77a7f2edd75636babfeff97b1ea686e);
			vk.IC[9] = Pairing.G1Point(0x111e2e2a5f8828f80ddad08f9f74db56dac1cc16c1cb278036f79a84cf7a116f, 0x1d7d62e192b219b9808faa906c5ced871788f6339e8d91b83ac1343e20a16b30);
		}
		function verify(uint[] memory input, Proof memory proof) internal returns (uint) {
			VerifyingKey memory vk = verifyingKey();
			require(input.length + 1 == vk.IC.length);
			// Compute the linear combination vk_x
			Pairing.G1Point memory vk_x = Pairing.G1Point(0, 0);
			for (uint i = 0; i < input.length; i++)
				vk_x = Pairing.add(vk_x, Pairing.mul(vk.IC[i + 1], input[i]));
			vk_x = Pairing.add(vk_x, vk.IC[0]);
			if (!Pairing.pairingProd2(proof.A, vk.A, Pairing.negate(proof.A_p), Pairing.P2())) return 1;
			if (!Pairing.pairingProd2(vk.B, proof.B, Pairing.negate(proof.B_p), Pairing.P2())) return 2;
			if (!Pairing.pairingProd2(proof.C, vk.C, Pairing.negate(proof.C_p), Pairing.P2())) return 3;
			if (!Pairing.pairingProd3(
				proof.K, vk.gamma,
				Pairing.negate(Pairing.add(vk_x, Pairing.add(proof.A, proof.C))), vk.gammaBeta2,
				Pairing.negate(vk.gammaBeta1), proof.B
			)) return 4;
			if (!Pairing.pairingProd3(
					Pairing.add(vk_x, proof.A), proof.B,
					Pairing.negate(proof.H), vk.Z,
					Pairing.negate(proof.C), Pairing.P2()
			)) return 5;
			return 0;
		}
		event Verified(string);
		function verifyTx() public returns (bool) {
			uint[] memory input = new uint[](9);
			Proof memory proof;
			proof.A = Pairing.G1Point(12873740738727497448187997291915224677121726020054032516825496230827252793177, 21804419174137094775122804775419507726154084057848719988004616848382402162497);
			proof.A_p = Pairing.G1Point(7742452358972543465462254569134860944739929848367563713587808717088650354556, 7324522103398787664095385319014038380128814213034709026832529060148225837366);
			proof.B = Pairing.G2Point(
				[8176651290984905087450403379100573157708110416512446269839297438960217797614, 15588556568726919713003060429893850972163943674590384915350025440408631945055],
				[15347511022514187557142999444367533883366476794364262773195059233657571533367, 4265071979090628150845437155927259896060451682253086069461962693761322642015]);
			proof.B_p = Pairing.G1Point(2979746655438963305714517285593753729335852012083057917022078236006592638393, 6470627481646078059765266161088786576504622012540639992486470834383274712950);
			proof.C = Pairing.G1Point(6851077925310461602867742977619883934042581405263014789956638244065803308498, 10336382210592135525880811046708757754106524561907815205241508542912494488506);
			proof.C_p = Pairing.G1Point(12491625890066296859584468664467427202390981822868257437245835716136010795448, 13818492518017455361318553880921248537817650587494176379915981090396574171686);
			proof.H = Pairing.G1Point(12091046215835229523641173286701717671667447745509192321596954139357866668225, 14446807589950902476683545679847436767890904443411534435294953056557941441758);
			proof.K = Pairing.G1Point(21341087976609916409401737322664290631992568431163400450267978471171152600502, 2942165230690572858696920423896381470344658299915828986338281196715687693170);
			input[0] = 13986731495506593864492662381614386532349950841221768152838255933892789078521;
			input[1] = 622860516154313070522697309645122400675542217310916019527100517240519630053;
			input[2] = 11094488463398718754251685950409355128550342438297986977413505294941943071569;
			input[3] = 6627643779954497813586310325594578844876646808666478625705401786271515864467;
			input[4] = 2957286918163151606545409668133310005545945782087581890025685458369200827463;
			input[5] = 1384290496819542862903939282897996566903332587607290986044945365745128311081;
			input[6] = 5613571677741714971687805233468747950848449704454346829971683826953541367271;
			input[7] = 9643208548031422463313148630985736896287522941726746581856185889848792022807;
			input[8] = 18066496933330839731877828156604;
			if (verify(input, proof) == 0) {
				emit Verified("Transaction successfully verified.");
				return true;
			} else {
				return false;
			}
		}

	}
	)";
	compileAndRun(sourceCode, 0, "Pairing");
	compileAndRun(sourceCode, 0, "Test", bytes(), map<string, Address>{{"Pairing", m_contractAddress}});
	// Disabled because the point seems to be not well-formed, we need to find another example.
	//BOOST_CHECK(callContractFunction("testMul()") == encodeArgs(true));
	BOOST_CHECK(callContractFunction("f()") == encodeArgs(true));
	BOOST_CHECK(callContractFunction("g()") == encodeArgs(true));
	BOOST_CHECK(callContractFunction("pair()") == encodeArgs(true));
	BOOST_CHECK(callContractFunction("verifyTx()") == encodeArgs(true));
}

BOOST_AUTO_TEST_CASE(abi_encode)
{
	char const* sourceCode = R"(
		contract C {
			function f0() public returns (bytes memory) {
				return abi.encode();
			}
			function f1() public returns (bytes memory) {
				return abi.encode(1, 2);
			}
			function f2() public returns (bytes memory) {
				string memory x = "abc";
				return abi.encode(1, x, 2);
			}
			function f3() public returns (bytes memory r) {
				// test that memory is properly allocated
				string memory x = "abc";
				r = abi.encode(1, x, 2);
				bytes memory y = "def";
				require(y[0] == "d");
				y[0] = "e";
				require(y[0] == "e");
			}
			function f4() public returns (bytes memory) {
				bytes4 x = "abcd";
				return abi.encode(bytes2(x));
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f0()"), encodeArgs(0x20, 0));
	ABI_CHECK(callContractFunction("f1()"), encodeArgs(0x20, 0x40, 1, 2));
	ABI_CHECK(callContractFunction("f2()"), encodeArgs(0x20, 0xa0, 1, 0x60, 2, 3, "abc"));
	ABI_CHECK(callContractFunction("f3()"), encodeArgs(0x20, 0xa0, 1, 0x60, 2, 3, "abc"));
	ABI_CHECK(callContractFunction("f4()"), encodeArgs(0x20, 0x20, "ab"));
}

BOOST_AUTO_TEST_CASE(abi_encode_v2)
{
	char const* sourceCode = R"(
		pragma experimental ABIEncoderV2;
		contract C {
			struct S { uint a; uint[] b; }
			function f0() public pure returns (bytes memory) {
				return abi.encode();
			}
			function f1() public pure returns (bytes memory) {
				return abi.encode(1, 2);
			}
			function f2() public pure returns (bytes memory) {
				string memory x = "abc";
				return abi.encode(1, x, 2);
			}
			function f3() public pure returns (bytes memory r) {
				// test that memory is properly allocated
				string memory x = "abc";
				r = abi.encode(1, x, 2);
				bytes memory y = "def";
				require(y[0] == "d");
				y[0] = "e";
				require(y[0] == "e");
			}
			S s;
			function f4() public returns (bytes memory r) {
				string memory x = "abc";
				s.a = 7;
				s.b.push(2);
				s.b.push(3);
				r = abi.encode(1, x, s, 2);
				bytes memory y = "def";
				require(y[0] == "d");
				y[0] = "e";
				require(y[0] == "e");
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f0()"), encodeArgs(0x20, 0));
	ABI_CHECK(callContractFunction("f1()"), encodeArgs(0x20, 0x40, 1, 2));
	ABI_CHECK(callContractFunction("f2()"), encodeArgs(0x20, 0xa0, 1, 0x60, 2, 3, "abc"));
	ABI_CHECK(callContractFunction("f3()"), encodeArgs(0x20, 0xa0, 1, 0x60, 2, 3, "abc"));
	ABI_CHECK(callContractFunction("f4()"), encodeArgs(0x20, 0x160, 1, 0x80, 0xc0, 2, 3, "abc", 7, 0x40, 2, 2, 3));
}


BOOST_AUTO_TEST_CASE(abi_encodePacked)
{
	char const* sourceCode = R"(
		contract C {
			function f0() public pure returns (bytes memory) {
				return abi.encodePacked();
			}
			function f1() public pure returns (bytes memory) {
				return abi.encodePacked(uint8(1), uint8(2));
			}
			function f2() public pure returns (bytes memory) {
				string memory x = "abc";
				return abi.encodePacked(uint8(1), x, uint8(2));
			}
			function f3() public pure returns (bytes memory r) {
				// test that memory is properly allocated
				string memory x = "abc";
				r = abi.encodePacked(uint8(1), x, uint8(2));
				bytes memory y = "def";
				require(y[0] == "d");
				y[0] = "e";
				require(y[0] == "e");
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f0()"), encodeArgs(0x20, 0));
	ABI_CHECK(callContractFunction("f1()"), encodeArgs(0x20, 2, "\x01\x02"));
	ABI_CHECK(callContractFunction("f2()"), encodeArgs(0x20, 5, "\x01" "abc" "\x02"));
	ABI_CHECK(callContractFunction("f3()"), encodeArgs(0x20, 5, "\x01" "abc" "\x02"));
}

BOOST_AUTO_TEST_CASE(abi_encode_with_selector)
{
	char const* sourceCode = R"(
		contract C {
			function f0() public pure returns (bytes memory) {
				return abi.encodeWithSelector(0x12345678);
			}
			function f1() public pure returns (bytes memory) {
				return abi.encodeWithSelector(0x12345678, "abc");
			}
			function f2() public pure returns (bytes memory) {
				bytes4 x = 0x12345678;
				return abi.encodeWithSelector(x, "abc");
			}
			function f3() public pure returns (bytes memory) {
				bytes4 x = 0x12345678;
				return abi.encodeWithSelector(x, uint(-1));
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f0()"), encodeArgs(0x20, 4, "\x12\x34\x56\x78"));
	bytes expectation;
	expectation = encodeArgs(0x20, 4 + 0x60) + bytes{0x12, 0x34, 0x56, 0x78} + encodeArgs(0x20, 3, "abc") + bytes(0x20 - 4);
	ABI_CHECK(callContractFunction("f1()"), expectation);
	expectation = encodeArgs(0x20, 4 + 0x60) + bytes{0x12, 0x34, 0x56, 0x78} + encodeArgs(0x20, 3, "abc") + bytes(0x20 - 4);
	ABI_CHECK(callContractFunction("f2()"), expectation);
	expectation = encodeArgs(0x20, 4 + 0x20) + bytes{0x12, 0x34, 0x56, 0x78} + encodeArgs(u256(-1)) + bytes(0x20 - 4);
	ABI_CHECK(callContractFunction("f3()"), expectation);
}

BOOST_AUTO_TEST_CASE(abi_encode_with_selectorv2)
{
	char const* sourceCode = R"(
		pragma experimental ABIEncoderV2;
		contract C {
			function f0() public pure returns (bytes memory) {
				return abi.encodeWithSelector(0x12345678);
			}
			function f1() public pure returns (bytes memory) {
				return abi.encodeWithSelector(0x12345678, "abc");
			}
			function f2() public pure returns (bytes memory) {
				bytes4 x = 0x12345678;
				return abi.encodeWithSelector(x, "abc");
			}
			function f3() public pure returns (bytes memory) {
				bytes4 x = 0x12345678;
				return abi.encodeWithSelector(x, uint(-1));
			}
			struct S { uint a; string b; uint16 c; }
			function f4() public pure returns (bytes memory) {
				bytes4 x = 0x12345678;
				S memory s;
				s.a = 0x1234567;
				s.b = "Lorem ipsum dolor sit ethereum........";
				s.c = 0x1234;
				return abi.encodeWithSelector(x, uint(-1), s, uint(3));
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f0()"), encodeArgs(0x20, 4, "\x12\x34\x56\x78"));
	bytes expectation;
	expectation = encodeArgs(0x20, 4 + 0x60) + bytes{0x12, 0x34, 0x56, 0x78} + encodeArgs(0x20, 3, "abc") + bytes(0x20 - 4);
	ABI_CHECK(callContractFunction("f1()"), expectation);
	expectation = encodeArgs(0x20, 4 + 0x60) + bytes{0x12, 0x34, 0x56, 0x78} + encodeArgs(0x20, 3, "abc") + bytes(0x20 - 4);
	ABI_CHECK(callContractFunction("f2()"), expectation);
	expectation = encodeArgs(0x20, 4 + 0x20) + bytes{0x12, 0x34, 0x56, 0x78} + encodeArgs(u256(-1)) + bytes(0x20 - 4);
	ABI_CHECK(callContractFunction("f3()"), expectation);
	expectation =
		encodeArgs(0x20, 4 + 0x120) +
		bytes{0x12, 0x34, 0x56, 0x78} +
		encodeArgs(u256(-1), 0x60, u256(3), 0x1234567, 0x60, 0x1234, 38, "Lorem ipsum dolor sit ethereum........") +
		bytes(0x20 - 4);
	ABI_CHECK(callContractFunction("f4()"), expectation);
}

BOOST_AUTO_TEST_CASE(abi_encode_with_signature)
{
	char const* sourceCode = R"T(
		contract C {
			function f0() public pure returns (bytes memory) {
				return abi.encodeWithSignature("f(uint256)");
			}
			function f1() public pure returns (bytes memory) {
				string memory x = "f(uint256)";
				return abi.encodeWithSignature(x, "abc");
			}
			string xstor;
			function f1s() public returns (bytes memory) {
				xstor = "f(uint256)";
				return abi.encodeWithSignature(xstor, "abc");
			}
			function f2() public pure returns (bytes memory r, uint[] memory ar) {
				string memory x = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
				uint[] memory y = new uint[](4);
				y[0] = uint(-1);
				y[1] = uint(-2);
				y[2] = uint(-3);
				y[3] = uint(-4);
				r = abi.encodeWithSignature(x, y);
				// The hash uses temporary memory. This allocation re-uses the memory
				// and should initialize it properly.
				ar = new uint[](2);
			}
		}
	)T";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f0()"), encodeArgs(0x20, 4, "\xb3\xde\x64\x8b"));
	bytes expectation;
	expectation = encodeArgs(0x20, 4 + 0x60) + bytes{0xb3, 0xde, 0x64, 0x8b} + encodeArgs(0x20, 3, "abc") + bytes(0x20 - 4);
	ABI_CHECK(callContractFunction("f1()"), expectation);
	ABI_CHECK(callContractFunction("f1s()"), expectation);
	expectation =
		encodeArgs(0x40, 0x140, 4 + 0xc0) +
		(bytes{0xe9, 0xc9, 0x21, 0xcd} + encodeArgs(0x20, 4, u256(-1), u256(-2), u256(-3), u256(-4)) + bytes(0x20 - 4)) +
		encodeArgs(2, 0, 0);
	ABI_CHECK(callContractFunction("f2()"), expectation);
}

BOOST_AUTO_TEST_CASE(abi_encode_with_signaturev2)
{
	char const* sourceCode = R"T(
		pragma experimental ABIEncoderV2;
		contract C {
			function f0() public pure returns (bytes memory) {
				return abi.encodeWithSignature("f(uint256)");
			}
			function f1() public pure returns (bytes memory) {
				string memory x = "f(uint256)";
				return abi.encodeWithSignature(x, "abc");
			}
			string xstor;
			function f1s() public returns (bytes memory) {
				xstor = "f(uint256)";
				return abi.encodeWithSignature(xstor, "abc");
			}
			function f2() public pure returns (bytes memory r, uint[] memory ar) {
				string memory x = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
				uint[] memory y = new uint[](4);
				y[0] = uint(-1);
				y[1] = uint(-2);
				y[2] = uint(-3);
				y[3] = uint(-4);
				r = abi.encodeWithSignature(x, y);
				// The hash uses temporary memory. This allocation re-uses the memory
				// and should initialize it properly.
				ar = new uint[](2);
			}
			struct S { uint a; string b; uint16 c; }
			function f4() public pure returns (bytes memory) {
				bytes4 x = 0x12345678;
				S memory s;
				s.a = 0x1234567;
				s.b = "Lorem ipsum dolor sit ethereum........";
				s.c = 0x1234;
				return abi.encodeWithSignature(s.b, uint(-1), s, uint(3));
			}
		}
	)T";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f0()"), encodeArgs(0x20, 4, "\xb3\xde\x64\x8b"));
	bytes expectation;
	expectation = encodeArgs(0x20, 4 + 0x60) + bytes{0xb3, 0xde, 0x64, 0x8b} + encodeArgs(0x20, 3, "abc") + bytes(0x20 - 4);
	ABI_CHECK(callContractFunction("f1()"), expectation);
	ABI_CHECK(callContractFunction("f1s()"), expectation);
	expectation =
		encodeArgs(0x40, 0x140, 4 + 0xc0) +
		(bytes{0xe9, 0xc9, 0x21, 0xcd} + encodeArgs(0x20, 4, u256(-1), u256(-2), u256(-3), u256(-4)) + bytes(0x20 - 4)) +
		encodeArgs(2, 0, 0);
	ABI_CHECK(callContractFunction("f2()"), expectation);
	expectation =
		encodeArgs(0x20, 4 + 0x120) +
		bytes{0x7c, 0x79, 0x30, 0x02} +
		encodeArgs(u256(-1), 0x60, u256(3), 0x1234567, 0x60, 0x1234, 38, "Lorem ipsum dolor sit ethereum........") +
		bytes(0x20 - 4);
	ABI_CHECK(callContractFunction("f4()"), expectation);
}

BOOST_AUTO_TEST_CASE(abi_encode_empty_string)
{
	char const* sourceCode = R"(
		// Tests that this will not end up using a "bytes0" type
		// (which would assert)
		contract C {
			function f() public pure returns (bytes memory, bytes memory) {
				return (abi.encode(""), abi.encodePacked(""));
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(
		0x40, 0xc0,
		0x60, 0x20, 0x00, 0x00,
		0x00
	));
}

BOOST_AUTO_TEST_CASE(abi_encode_empty_string_v2)
{
	char const* sourceCode = R"(
		// Tests that this will not end up using a "bytes0" type
		// (which would assert)
		pragma experimental ABIEncoderV2;
		contract C {
			function f() public pure returns (bytes memory, bytes memory) {
				return (abi.encode(""), abi.encodePacked(""));
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(
		0x40, 0xa0,
		0x40, 0x20, 0x00,
		0x00
	));
}

BOOST_AUTO_TEST_CASE(abi_encode_rational)
{
	char const* sourceCode = R"(
		// Tests that rational numbers (even negative ones) are encoded properly.
		contract C {
			function f() public pure returns (bytes memory) {
				return abi.encode(1, -2);
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(
		0x20,
		0x40, u256(1), u256(-2)
	));
}

BOOST_AUTO_TEST_CASE(abi_encode_rational_v2)
{
	char const* sourceCode = R"(
		// Tests that rational numbers (even negative ones) are encoded properly.
		pragma experimental ABIEncoderV2;
		contract C {
			function f() public pure returns (bytes memory) {
				return abi.encode(1, -2);
			}
		}
	)";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(
		0x20,
		0x40, u256(1), u256(-2)
	));
}

BOOST_AUTO_TEST_CASE(abi_encode_call)
{
	char const* sourceCode = R"T(
		contract C {
			bool x;
			function c(uint a, uint[] memory b) public {
				require(a == 5);
				require(b.length == 2);
				require(b[0] == 6);
				require(b[1] == 7);
				x = true;
			}
			function f() public returns (bool) {
				uint a = 5;
				uint[] memory b = new uint[](2);
				b[0] = 6;
				b[1] = 7;
				(bool success,) = address(this).call(abi.encodeWithSignature("c(uint256,uint256[])", a, b));
				require(success);
				return x;
			}
		}
	)T";
	compileAndRun(sourceCode, 0, "C");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(true));
}

BOOST_AUTO_TEST_CASE(staticcall_for_view_and_pure)
{
	char const* sourceCode = R"(
		contract C {
			uint x;
			function f() public returns (uint) {
				x = 3;
				return 1;
			}
		}
		interface CView {
			function f() view external returns (uint);
		}
		interface CPure {
			function f() pure external returns (uint);
		}
		contract D {
			function f() public returns (uint) {
				return (new C()).f();
			}
			function fview() public returns (uint) {
				return (CView(address(new C()))).f();
			}
			function fpure() public returns (uint) {
				return (CPure(address(new C()))).f();
			}
		}
	)";
	compileAndRun(sourceCode, 0, "D");
	// This should work (called via CALL)
	ABI_CHECK(callContractFunction("f()"), encodeArgs(1));
	if (dev::test::Options::get().evmVersion().hasStaticCall())
	{
		// These should throw (called via STATICCALL)
		ABI_CHECK(callContractFunction("fview()"), encodeArgs());
		ABI_CHECK(callContractFunction("fpure()"), encodeArgs());
	}
	else
	{
		ABI_CHECK(callContractFunction("fview()"), encodeArgs(1));
		ABI_CHECK(callContractFunction("fpure()"), encodeArgs(1));
	}
}

BOOST_AUTO_TEST_CASE(bitwise_shifting_constantinople)
{
	if (!dev::test::Options::get().evmVersion().hasBitwiseShifting())
		return;
	char const* sourceCode = R"(
		contract C {
			function shl(uint a, uint b) public returns (uint c) {
				assembly {
					c := shl(b, a)
				}
			}
			function shr(uint a, uint b) public returns (uint c) {
				assembly {
					c := shr(b, a)
				}
			}
			function sar(uint a, uint b) public returns (uint c) {
				assembly {
					c := sar(b, a)
				}
			}
		}
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callContractFunction("shl(uint256,uint256)", u256(1), u256(2)) == encodeArgs(u256(4)));
	BOOST_CHECK(callContractFunction("shl(uint256,uint256)", u256("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"), u256(1)) == encodeArgs(u256("0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe")));
	BOOST_CHECK(callContractFunction("shl(uint256,uint256)", u256("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"), u256(256)) == encodeArgs(u256(0)));
	BOOST_CHECK(callContractFunction("shr(uint256,uint256)", u256(3), u256(1)) == encodeArgs(u256(1)));
	BOOST_CHECK(callContractFunction("shr(uint256,uint256)", u256("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"), u256(1)) == encodeArgs(u256("0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff")));
	BOOST_CHECK(callContractFunction("shr(uint256,uint256)", u256("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"), u256(255)) == encodeArgs(u256(1)));
	BOOST_CHECK(callContractFunction("shr(uint256,uint256)", u256("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"), u256(256)) == encodeArgs(u256(0)));
	BOOST_CHECK(callContractFunction("sar(uint256,uint256)", u256(3), u256(1)) == encodeArgs(u256(1)));
	BOOST_CHECK(callContractFunction("sar(uint256,uint256)", u256("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"), u256(1)) == encodeArgs(u256("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff")));
	BOOST_CHECK(callContractFunction("sar(uint256,uint256)", u256("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"), u256(255)) == encodeArgs(u256("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff")));
	BOOST_CHECK(callContractFunction("sar(uint256,uint256)", u256("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"), u256(256)) == encodeArgs(u256("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff")));
}

BOOST_AUTO_TEST_CASE(bitwise_shifting_constants_constantinople)
{
	if (!dev::test::Options::get().evmVersion().hasBitwiseShifting())
		return;
	char const* sourceCode = R"(
		contract C {
			function shl_1() public returns (bool) {
				uint c;
				assembly {
					c := shl(2, 1)
				}
				assert(c == 4);
				return true;
			}
			function shl_2() public returns (bool) {
				uint c;
				assembly {
					c := shl(1, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
				}
				assert(c == 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe);
				return true;
			}
			function shl_3() public returns (bool) {
				uint c;
				assembly {
					c := shl(256, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
				}
				assert(c == 0);
				return true;
			}
			function shr_1() public returns (bool) {
				uint c;
				assembly {
					c := shr(1, 3)
				}
				assert(c == 1);
				return true;
			}
			function shr_2() public returns (bool) {
				uint c;
				assembly {
					c := shr(1, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
				}
				assert(c == 0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff);
				return true;
			}
			function shr_3() public returns (bool) {
				uint c;
				assembly {
					c := shr(256, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
				}
				assert(c == 0);
				return true;
			}
		}
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callContractFunction("shl_1()") == encodeArgs(u256(1)));
	BOOST_CHECK(callContractFunction("shl_2()") == encodeArgs(u256(1)));
	BOOST_CHECK(callContractFunction("shl_3()") == encodeArgs(u256(1)));
	BOOST_CHECK(callContractFunction("shr_1()") == encodeArgs(u256(1)));
	BOOST_CHECK(callContractFunction("shr_2()") == encodeArgs(u256(1)));
	BOOST_CHECK(callContractFunction("shr_3()") == encodeArgs(u256(1)));
}

BOOST_AUTO_TEST_CASE(senders_balance)
{
	char const* sourceCode = R"(
		contract C {
			function f() public view returns (uint) {
				return msg.sender.balance;
			}
		}
		contract D {
			C c = new C();
			constructor() public payable { }
			function f() public view returns (uint) {
				return c.f();
			}
		}
	)";
	compileAndRun(sourceCode, 27, "D");
	BOOST_CHECK(callContractFunction("f()") == encodeArgs(u256(27)));
}

BOOST_AUTO_TEST_CASE(abi_decode_trivial)
{
	char const* sourceCode = R"(
		contract C {
			function f(bytes memory data) public pure returns (uint) {
				return abi.decode(data, (uint));
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f(bytes)", 0x20, 0x20, 33), encodeArgs(u256(33)));
}

BOOST_AUTO_TEST_CASE(abi_encode_decode_simple)
{
	char const* sourceCode = R"XX(
		contract C {
			function f() public pure returns (uint, bytes memory) {
				bytes memory arg = "abcdefg";
				return abi.decode(abi.encode(uint(33), arg), (uint, bytes));
			}
		}
	)XX";
	compileAndRun(sourceCode);
	ABI_CHECK(
		callContractFunction("f()"),
		encodeArgs(33, 0x40, 7, "abcdefg")
	);
}

BOOST_AUTO_TEST_CASE(abi_decode_simple)
{
	char const* sourceCode = R"(
		contract C {
			function f(bytes memory data) public pure returns (uint, bytes memory) {
				return abi.decode(data, (uint, bytes));
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(
		callContractFunction("f(bytes)", 0x20, 0x20 * 4, 33, 0x40, 7, "abcdefg"),
		encodeArgs(33, 0x40, 7, "abcdefg")
	);
}

BOOST_AUTO_TEST_CASE(abi_decode_v2)
{
	char const* sourceCode = R"(
		pragma experimental ABIEncoderV2;
		contract C {
			struct S { uint a; uint[] b; }
			function f() public pure returns (S memory) {
				S memory s;
				s.a = 8;
				s.b = new uint[](3);
				s.b[0] = 9;
				s.b[1] = 10;
				s.b[2] = 11;
				return abi.decode(abi.encode(s), (S));
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(
		callContractFunction("f()"),
		encodeArgs(0x20, 8, 0x40, 3, 9, 10, 11)
	);
}

BOOST_AUTO_TEST_CASE(abi_decode_simple_storage)
{
	char const* sourceCode = R"(
		contract C {
			bytes data;
			function f(bytes memory _data) public returns (uint, bytes memory) {
				data = _data;
				return abi.decode(data, (uint, bytes));
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(
		callContractFunction("f(bytes)", 0x20, 0x20 * 4, 33, 0x40, 7, "abcdefg"),
		encodeArgs(33, 0x40, 7, "abcdefg")
	);
}

BOOST_AUTO_TEST_CASE(abi_decode_v2_storage)
{
	char const* sourceCode = R"(
		pragma experimental ABIEncoderV2;
		contract C {
			bytes data;
			struct S { uint a; uint[] b; }
			function f() public returns (S memory) {
				S memory s;
				s.a = 8;
				s.b = new uint[](3);
				s.b[0] = 9;
				s.b[1] = 10;
				s.b[2] = 11;
				data = abi.encode(s);
				return abi.decode(data, (S));
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(
		callContractFunction("f()"),
		encodeArgs(0x20, 8, 0x40, 3, 9, 10, 11)
	);
}

BOOST_AUTO_TEST_CASE(abi_decode_calldata)
{
	char const* sourceCode = R"(
		contract C {
			function f(bytes calldata data) external pure returns (uint, bytes memory r) {
				return abi.decode(data, (uint, bytes));
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(
		callContractFunction("f(bytes)", 0x20, 0x20 * 4, 33, 0x40, 7, "abcdefg"),
		encodeArgs(33, 0x40, 7, "abcdefg")
	);
}

BOOST_AUTO_TEST_CASE(abi_decode_v2_calldata)
{
	char const* sourceCode = R"(
		pragma experimental ABIEncoderV2;
		contract C {
			struct S { uint a; uint[] b; }
			function f(bytes calldata data) external pure returns (S memory) {
				return abi.decode(data, (S));
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(
		callContractFunction("f(bytes)", 0x20, 0x20 * 7, 0x20, 33, 0x40, 3, 10, 11, 12),
		encodeArgs(0x20, 33, 0x40, 3, 10, 11, 12)
	);
}

BOOST_AUTO_TEST_CASE(abi_decode_static_array)
{
	char const* sourceCode = R"(
		contract C {
			function f(bytes calldata data) external pure returns (uint[2][3] memory) {
				return abi.decode(data, (uint[2][3]));
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(
		callContractFunction("f(bytes)", 0x20, 6 * 0x20, 1, 2, 3, 4, 5, 6),
		encodeArgs(1, 2, 3, 4, 5, 6)
	);
}

BOOST_AUTO_TEST_CASE(abi_decode_static_array_v2)
{
	char const* sourceCode = R"(
		pragma experimental ABIEncoderV2;
		contract C {
			function f(bytes calldata data) external pure returns (uint[2][3] memory) {
				return abi.decode(data, (uint[2][3]));
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(
		callContractFunction("f(bytes)", 0x20, 6 * 0x20, 1, 2, 3, 4, 5, 6),
		encodeArgs(1, 2, 3, 4, 5, 6)
	);
}

BOOST_AUTO_TEST_CASE(abi_decode_dynamic_array)
{
	char const* sourceCode = R"(
		contract C {
			function f(bytes calldata data) external pure returns (uint[] memory) {
				return abi.decode(data, (uint[]));
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(
		callContractFunction("f(bytes)", 0x20, 6 * 0x20, 0x20, 4, 3, 4, 5, 6),
		encodeArgs(0x20, 4, 3, 4, 5, 6)
	);
}

BOOST_AUTO_TEST_CASE(write_storage_external)
{
	char const* sourceCode = R"(
		contract C {
			uint public x;
			function f(uint y) public payable {
				x = y;
			}
			function g(uint y) external {
				x = y;
			}
			function h() public {
				this.g(12);
			}
		}
		contract D {
			C c = new C();
			function f() public payable returns (uint) {
				c.g(3);
				return c.x();
			}
			function g() public returns (uint) {
				c.g(8);
				return c.x();
			}
			function h() public returns (uint) {
				c.h();
				return c.x();
			}
		}
	)";
	compileAndRun(sourceCode, 0, "D");
	ABI_CHECK(callContractFunction("f()"), encodeArgs(3));
	ABI_CHECK(callContractFunction("g()"), encodeArgs(8));
	ABI_CHECK(callContractFunction("h()"), encodeArgs(12));
}

BOOST_AUTO_TEST_CASE(test_underscore_in_hex)
{
	char const* sourceCode = R"(
		contract test {
			function f(bool cond) public pure returns (uint) {
				uint32 x = 0x1234_ab;
				uint y = 0x1234_abcd_1234;
				return cond ? x : y;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f(bool)", true), encodeArgs(u256(0x1234ab)));
	ABI_CHECK(callContractFunction("f(bool)", false), encodeArgs(u256(0x1234abcd1234)));
}

BOOST_AUTO_TEST_CASE(flipping_sign_tests)
{
	char const* sourceCode = R"(
		contract test {
			function f() public returns (bool){
				int x = -2**255;
				assert(-x == x);
				return true;
			}
		}
	)";
	compileAndRun(sourceCode);
	ABI_CHECK(callContractFunction("f()"), encodeArgs(true));
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
