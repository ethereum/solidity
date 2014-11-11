
/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Unit tests for the solidity expression compiler, testing the behaviour of the code.
 */

#include <string>
#include <boost/test/unit_test.hpp>
#include <libethereum/State.h>
#include <libethereum/Executive.h>
#include <libsolidity/CompilerStack.h>

using namespace std;

namespace dev
{
namespace solidity
{
namespace test
{

class ExecutionFramework
{
public:
	ExecutionFramework() { g_logVerbosity = 0; }

	bytes const& compileAndRun(std::string const& _sourceCode)
	{
		bytes code = dev::solidity::CompilerStack::staticCompile(_sourceCode);
		sendMessage(code, true);
		BOOST_REQUIRE(!m_output.empty());
		return m_output;
	}

	bytes const& callFunction(byte _index, bytes const& _data)
	{
		sendMessage(bytes(1, _index) + _data, false);
		return m_output;
	}

	bytes const& callFunction(byte _index, u256 const& _argument1)
	{
		return callFunction(_index, toBigEndian(_argument1));
	}

	bool testSolidityAgainstCpp(byte _index, std::function<u256(u256)> const& _cppfun, u256 const& _argument1)
	{
		return toBigEndian(_cppfun(_argument1)) == callFunction(_index, toBigEndian(_argument1));
	}

	bool testSolidityAgainstCpp(byte _index, std::function<u256()> const& _cppfun)
	{
		return toBigEndian(_cppfun()) == callFunction(_index, bytes());
	}

private:
	void sendMessage(bytes const& _data, bool _isCreation)
	{
		eth::Executive executive(m_state);
		eth::Transaction t = _isCreation ? eth::Transaction(0, m_gasPrice, m_gas, _data, 0, KeyPair::create().sec())
										 : eth::Transaction(0, m_gasPrice, m_gas, m_contractAddress, _data, 0, KeyPair::create().sec());
		bytes transactionRLP = t.rlp();
		try
		{
			// this will throw since the transaction is invalid, but it should nevertheless store the transaction
			executive.setup(&transactionRLP);
		}
		catch (...) {}
		if (_isCreation)
		{
			BOOST_REQUIRE(!executive.create(Address(), 0, m_gasPrice, m_gas, &_data, Address()));
			m_contractAddress = executive.newAddress();
			BOOST_REQUIRE(m_contractAddress);
			BOOST_REQUIRE(m_state.addressHasCode(m_contractAddress));
		}
		else
			BOOST_REQUIRE(!executive.call(m_contractAddress, Address(), 0, m_gasPrice, &_data, m_gas, Address()));
		BOOST_REQUIRE(executive.go());
		executive.finalize();
		m_output = executive.out().toVector();
	}

	Address m_contractAddress;
	eth::State m_state;
	u256 const m_gasPrice = 100 * eth::szabo;
	u256 const m_gas = 1000000;
	bytes m_output;
};

BOOST_FIXTURE_TEST_SUITE(SolidityCompilerEndToEndTest, ExecutionFramework)

BOOST_AUTO_TEST_CASE(smoke_test)
{
	char const* sourceCode = "contract test {\n"
							 "  function f(uint a) returns(uint d) { return a * 7; }\n"
							 "}\n";
	compileAndRun(sourceCode);
	u256 a = 0x200030004;
	BOOST_CHECK(callFunction(0, a) == toBigEndian(a * 7));
}

BOOST_AUTO_TEST_CASE(empty_contract)
{
	char const* sourceCode = "contract test {\n"
							 "}\n";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFunction(0, bytes()).empty());
}

BOOST_AUTO_TEST_CASE(recursive_calls)
{
	char const* sourceCode = "contract test {\n"
							 "  function f(uint n) returns(uint nfac) {\n"
							 "    if (n <= 1) return 1;\n"
							 "    else return n * f(n - 1);\n"
							 "  }\n"
							 "}\n";
	compileAndRun(sourceCode);
	std::function<u256(u256)> recursive_calls_cpp = [&recursive_calls_cpp](u256 const& n) -> u256
	{
		if (n <= 1)
			return 1;
		else
			return n * recursive_calls_cpp(n - 1);
	};

	BOOST_CHECK(testSolidityAgainstCpp(0, recursive_calls_cpp, u256(0)));
	BOOST_CHECK(testSolidityAgainstCpp(0, recursive_calls_cpp, u256(1)));
	BOOST_CHECK(testSolidityAgainstCpp(0, recursive_calls_cpp, u256(2)));
	BOOST_CHECK(testSolidityAgainstCpp(0, recursive_calls_cpp, u256(3)));
	BOOST_CHECK(testSolidityAgainstCpp(0, recursive_calls_cpp, u256(4)));
}

BOOST_AUTO_TEST_CASE(while_loop)
{
	char const* sourceCode = "contract test {\n"
							 "  function f(uint n) returns(uint nfac) {\n"
							 "    nfac = 1;\n"
							 "    var i = 2;\n"
							 "    while (i <= n) nfac *= i++;\n"
							 "  }\n"
							 "}\n";
	compileAndRun(sourceCode);

	auto while_loop_cpp = [](u256 const& n) -> u256
	{
		u256 nfac = 1;
		u256 i = 2;
		while (i <= n)
			nfac *= i++;

		return nfac;
	};

	BOOST_CHECK(testSolidityAgainstCpp(0, while_loop_cpp, u256(0)));
	BOOST_CHECK(testSolidityAgainstCpp(0, while_loop_cpp, u256(1)));
	BOOST_CHECK(testSolidityAgainstCpp(0, while_loop_cpp, u256(2)));
	BOOST_CHECK(testSolidityAgainstCpp(0, while_loop_cpp, u256(3)));
	BOOST_CHECK(testSolidityAgainstCpp(0, while_loop_cpp, u256(4)));
}

BOOST_AUTO_TEST_CASE(break_outside_loop)
{
	// break and continue outside loops should be simply ignored
	char const* sourceCode = "contract test {\n"
							 "  function f(uint x) returns(uint y) {\n"
							 "    break; continue; return 2;\n"
							 "  }\n"
							 "}\n";
	ExecutionFramework framework;
	framework.compileAndRun(sourceCode);
	BOOST_CHECK(framework.callFunction(0, u256(0)) == toBigEndian(u256(2)));
}

BOOST_AUTO_TEST_CASE(nested_loops)
{
	// tests that break and continue statements in nested loops jump to the correct place
	char const* sourceCode = "contract test {\n"
							 "  function f(uint x) returns(uint y) {\n"
							 "    while (x > 1) {\n"
							 "      if (x == 10) break;\n"
							 "      while (x > 5) {\n"
							 "        if (x == 8) break;\n"
							 "        x--;\n"
							 "        if (x == 6) continue;\n"
							 "        return x;\n"
							 "      }\n"
							 "      x--;\n"
							 "      if (x == 3) continue;\n"
							 "      break;\n"
							 "    }\n"
							 "    return x;\n"
							 "  }\n"
							 "}\n";
	ExecutionFramework framework;
	framework.compileAndRun(sourceCode);

	auto nested_loops_cpp = [](u256  n) -> u256
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

	BOOST_CHECK(framework.testSolidityAgainstCpp(0, nested_loops_cpp, u256(0)));
	BOOST_CHECK(framework.testSolidityAgainstCpp(0, nested_loops_cpp, u256(1)));
	BOOST_CHECK(framework.testSolidityAgainstCpp(0, nested_loops_cpp, u256(2)));
	BOOST_CHECK(framework.testSolidityAgainstCpp(0, nested_loops_cpp, u256(3)));
	BOOST_CHECK(framework.testSolidityAgainstCpp(0, nested_loops_cpp, u256(4)));
	BOOST_CHECK(framework.testSolidityAgainstCpp(0, nested_loops_cpp, u256(5)));
	BOOST_CHECK(framework.testSolidityAgainstCpp(0, nested_loops_cpp, u256(6)));
	BOOST_CHECK(framework.testSolidityAgainstCpp(0, nested_loops_cpp, u256(7)));
	BOOST_CHECK(framework.testSolidityAgainstCpp(0, nested_loops_cpp, u256(8)));
	BOOST_CHECK(framework.testSolidityAgainstCpp(0, nested_loops_cpp, u256(9)));
	BOOST_CHECK(framework.testSolidityAgainstCpp(0, nested_loops_cpp, u256(10)));
	BOOST_CHECK(framework.testSolidityAgainstCpp(0, nested_loops_cpp, u256(11)));
}

BOOST_AUTO_TEST_CASE(calling_other_functions)
{
	// note that the index of a function is its index in the sorted sequence of functions
	char const* sourceCode = "contract collatz {\n"
							 "  function run(uint x) returns(uint y) {\n"
							 "    while ((y = x) > 1) {\n"
							 "      if (x % 2 == 0) x = evenStep(x);\n"
							 "      else x = oddStep(x);\n"
							 "    }\n"
							 "  }\n"
							 "  function evenStep(uint x) returns(uint y) {\n"
							 "    return x / 2;\n"
							 "  }\n"
							 "  function oddStep(uint x) returns(uint y) {\n"
							 "    return 3 * x + 1;\n"
							 "  }\n"
							 "}\n";
	compileAndRun(sourceCode);

	auto evenStep_cpp = [](u256 const& n) -> u256
	{
		return n / 2;
	};

	auto oddStep_cpp = [](u256 const& n) -> u256
	{
		return 3 * n + 1;
	};

	auto collatz_cpp = [&evenStep_cpp, &oddStep_cpp] (u256 n) -> u256 {
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

	BOOST_CHECK(testSolidityAgainstCpp(2, collatz_cpp, u256(0)));
	BOOST_CHECK(testSolidityAgainstCpp(2, collatz_cpp, u256(1)));
	BOOST_CHECK(testSolidityAgainstCpp(2, collatz_cpp, u256(2)));
	BOOST_CHECK(testSolidityAgainstCpp(2, collatz_cpp, u256(8)));
	BOOST_CHECK(testSolidityAgainstCpp(2, collatz_cpp, u256(127)));
}

BOOST_AUTO_TEST_CASE(many_local_variables)
{
	char const* sourceCode = "contract test {\n"
							 "  function run(uint x1, uint x2, uint x3) returns(uint y) {\n"
							 "    var a = 0x1; var b = 0x10; var c = 0x100;\n"
							 "    y = a + b + c + x1 + x2 + x3;\n"
							 "    y += b + x2;\n"
							 "  }\n"
							 "}\n";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFunction(0, toBigEndian(u256(0x1000)) + toBigEndian(u256(0x10000)) + toBigEndian(u256(0x100000)))
				== toBigEndian(u256(0x121121)));
}

BOOST_AUTO_TEST_CASE(packing_unpacking_types)
{
	char const* sourceCode = "contract test {\n"
							 "  function run(bool a, uint32 b, uint64 c) returns(uint256 y) {\n"
							 "    if (a) y = 1;\n"
							 "    y = y * 0x100000000 | ~b;\n"
							 "    y = y * 0x10000000000000000 | ~c;\n"
							 "  }\n"
							 "}\n";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFunction(0, fromHex("01""0f0f0f0f""f0f0f0f0f0f0f0f0"))
				== fromHex("00000000000000000000000000000000000000""01""f0f0f0f0""0f0f0f0f0f0f0f0f"));
}

BOOST_AUTO_TEST_CASE(multiple_return_values)
{
	char const* sourceCode = "contract test {\n"
							 "  function run(bool x1, uint x2) returns(uint y1, bool y2, uint y3) {\n"
							 "    y1 = x2; y2 = x1;\n"
							 "  }\n"
							 "}\n";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFunction(0, bytes(1, 1) + toBigEndian(u256(0xcd)))
				== toBigEndian(u256(0xcd)) + bytes(1, 1) + toBigEndian(u256(0)));
}

BOOST_AUTO_TEST_CASE(short_circuiting)
{
	char const* sourceCode = "contract test {\n"
							 "  function run(uint x) returns(uint y) {\n"
							 "    x == 0 || ((x = 8) > 0);\n"
							 "    return x;"
							 "  }\n"
							 "}\n";
	compileAndRun(sourceCode);

	auto short_circuiting_cpp = [](u256 n) -> u256
	{
		n == 0 || (n = 8) > 0;
		return n;
	};

	BOOST_CHECK(testSolidityAgainstCpp(0, short_circuiting_cpp, u256(0)));
	BOOST_CHECK(testSolidityAgainstCpp(0, short_circuiting_cpp, u256(1)));
}

BOOST_AUTO_TEST_CASE(high_bits_cleaning)
{
	char const* sourceCode = "contract test {\n"
							 "  function run() returns(uint256 y) {\n"
							 "    uint32 x = uint32(0xffffffff) + 10;\n"
							 "    if (x >= 0xffffffff) return 0;\n"
							 "    return x;"
							 "  }\n"
							 "}\n";
	compileAndRun(sourceCode);
	auto high_bits_cleaning_cpp = []() -> u256
	{
		uint32_t x = uint32_t(0xffffffff) + 10;
		if (x >= 0xffffffff)
			return 0;
		return x;
	};
	BOOST_CHECK(testSolidityAgainstCpp(0, high_bits_cleaning_cpp));
}

BOOST_AUTO_TEST_CASE(sign_extension)
{
	char const* sourceCode = "contract test {\n"
							 "  function run() returns(uint256 y) {\n"
							 "    int64 x = -int32(0xff);\n"
							 "    if (x >= 0xff) return 0;\n"
							 "    return -uint256(x);"
							 "  }\n"
							 "}\n";
	compileAndRun(sourceCode);
	auto sign_extension_cpp = []() -> u256
	{
		int64_t x = -int32_t(0xff);
		if (x >= 0xff)
			return 0;
		return u256(x) * -1;
	};
	BOOST_CHECK(testSolidityAgainstCpp(0, sign_extension_cpp));
}

BOOST_AUTO_TEST_CASE(small_unsigned_types)
{
	char const* sourceCode = "contract test {\n"
							 "  function run() returns(uint256 y) {\n"
							 "    uint32 x = uint32(0xffffff) * 0xffffff;\n"
							 "    return x / 0x100;"
							 "  }\n"
							 "}\n";
	compileAndRun(sourceCode);
	auto small_unsigned_types_cpp = []() -> u256
	{
		uint32_t x = uint32_t(0xffffff) * 0xffffff;
		return x / 0x100;
	};
	BOOST_CHECK(testSolidityAgainstCpp(0, small_unsigned_types_cpp));
}

BOOST_AUTO_TEST_CASE(small_signed_types)
{
	char const* sourceCode = "contract test {\n"
							 "  function run() returns(int256 y) {\n"
							 "    return -int32(10) * -int64(20);\n"
							 "  }\n"
							 "}\n";
	compileAndRun(sourceCode);
	auto small_signed_types_cpp = []() -> u256
	{
		return -int32_t(10) * -int64_t(20);
	};
	BOOST_CHECK(testSolidityAgainstCpp(0, small_signed_types_cpp));
}

BOOST_AUTO_TEST_CASE(state_smoke_test)
{
	char const* sourceCode = "contract test {\n"
							 "  uint256 value1;\n"
							 "  uint256 value2;\n"
							 "  function get(uint8 which) returns (uint256 value) {\n"
							 "    if (which == 0) return value1;\n"
							 "    else return value2;\n"
							 "  }\n"
							 "  function set(uint8 which, uint256 value) {\n"
							 "    if (which == 0) value1 = value;\n"
							 "    else value2 = value;\n"
							 "  }\n"
							 "}\n";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFunction(0, bytes(1, 0x00)) == toBigEndian(u256(0)));
	BOOST_CHECK(callFunction(0, bytes(1, 0x01)) == toBigEndian(u256(0)));
	BOOST_CHECK(callFunction(1, bytes(1, 0x00) + toBigEndian(u256(0x1234))) == bytes());
	BOOST_CHECK(callFunction(1, bytes(1, 0x01) + toBigEndian(u256(0x8765))) == bytes());
	BOOST_CHECK(callFunction(0, bytes(1, 0x00)) == toBigEndian(u256(0x1234)));
	BOOST_CHECK(callFunction(0, bytes(1, 0x01)) == toBigEndian(u256(0x8765)));
	BOOST_CHECK(callFunction(1, bytes(1, 0x00) + toBigEndian(u256(0x3))) == bytes());
	BOOST_CHECK(callFunction(0, bytes(1, 0x00)) == toBigEndian(u256(0x3)));
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces

