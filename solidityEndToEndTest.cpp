
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

	bytes compileAndRun(std::string const& _sourceCode)
	{
		bytes code = dev::solidity::CompilerStack::compile(_sourceCode);
		sendMessage(code, true);
		return m_output;
	}

	bytes callFunction(byte _index, bytes const& _data)
	{
		sendMessage(bytes(1, _index) + _data, false);
		return m_output;
	}

	bytes callFunction(byte _index, u256 const& _argument1)
	{
		callFunction(_index, toBigEndian(_argument1));
		return m_output;
	}

private:
	void sendMessage(bytes const& _data, bool _isCreation)
	{
		eth::Executive executive(m_state);
		eth::Transaction t = _isCreation ? eth::Transaction(0, m_gasPrice, m_gas, _data)
										 : eth::Transaction(0, m_gasPrice, m_gas, m_contractAddress, _data);
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
			BOOST_REQUIRE(m_state.addressHasCode(m_contractAddress));
		}
		else
			BOOST_REQUIRE(!executive.call(m_contractAddress, Address(), 0, m_gasPrice, &_data, m_gas, Address()));
		BOOST_REQUIRE(executive.go());
		executive.finalize();
		m_output = executive.out().toBytes();
	}

	Address m_contractAddress;
	eth::State m_state;
	u256 const m_gasPrice = 100 * eth::szabo;
	u256 const m_gas = 1000000;
	bytes m_output;
};

BOOST_AUTO_TEST_SUITE(SolidityCompilerEndToEndTest)

BOOST_AUTO_TEST_CASE(smoke_test)
{
	char const* sourceCode = "contract test {\n"
							 "  function f(uint a) returns(uint d) { return a * 7; }\n"
							 "}\n";
	ExecutionFramework framework;
	framework.compileAndRun(sourceCode);
	u256 a = 0x200030004;
	bytes result = framework.callFunction(0, a);
	BOOST_CHECK(result == toBigEndian(a * 7));
}

BOOST_AUTO_TEST_CASE(empty_contract)
{
	char const* sourceCode = "contract test {\n"
							 "}\n";
	ExecutionFramework framework;
	framework.compileAndRun(sourceCode);
	BOOST_CHECK(framework.callFunction(0, bytes()).empty());
}

BOOST_AUTO_TEST_CASE(recursive_calls)
{
	char const* sourceCode = "contract test {\n"
							 "  function f(uint n) returns(uint nfac) {\n"
							 "    if (n <= 1) return 1;\n"
							 "    else return n * f(n - 1);\n"
							 "  }\n"
							 "}\n";
	ExecutionFramework framework;
	framework.compileAndRun(sourceCode);
	BOOST_CHECK(framework.callFunction(0, u256(0)) == toBigEndian(u256(1)));
	BOOST_CHECK(framework.callFunction(0, u256(1)) == toBigEndian(u256(1)));
	BOOST_CHECK(framework.callFunction(0, u256(2)) == toBigEndian(u256(2)));
	BOOST_CHECK(framework.callFunction(0, u256(3)) == toBigEndian(u256(6)));
	BOOST_CHECK(framework.callFunction(0, u256(4)) == toBigEndian(u256(24)));
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
	ExecutionFramework framework;
	framework.compileAndRun(sourceCode);
	BOOST_CHECK(framework.callFunction(0, u256(0)) == toBigEndian(u256(1)));
	BOOST_CHECK(framework.callFunction(0, u256(1)) == toBigEndian(u256(1)));
	BOOST_CHECK(framework.callFunction(0, u256(2)) == toBigEndian(u256(2)));
	BOOST_CHECK(framework.callFunction(0, u256(3)) == toBigEndian(u256(6)));
	BOOST_CHECK(framework.callFunction(0, u256(4)) == toBigEndian(u256(24)));
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
	ExecutionFramework framework;
	framework.compileAndRun(sourceCode);
	BOOST_CHECK(framework.callFunction(2, u256(0)) == toBigEndian(u256(0)));
	BOOST_CHECK(framework.callFunction(2, u256(1)) == toBigEndian(u256(1)));
	BOOST_CHECK(framework.callFunction(2, u256(2)) == toBigEndian(u256(1)));
	BOOST_CHECK(framework.callFunction(2, u256(8)) == toBigEndian(u256(1)));
	BOOST_CHECK(framework.callFunction(2, u256(127)) == toBigEndian(u256(1)));
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
	ExecutionFramework framework;
	framework.compileAndRun(sourceCode);
	BOOST_CHECK(framework.callFunction(0, toBigEndian(u256(0x1000)) + toBigEndian(u256(0x10000)) + toBigEndian(u256(0x100000)))
				== toBigEndian(u256(0x121121)));
}

BOOST_AUTO_TEST_CASE(multiple_return_values)
{
	char const* sourceCode = "contract test {\n"
							 "  function run(bool x1, uint x2) returns(uint y1, bool y2, uint y3) {\n"
							 "    y1 = x2; y2 = x1;\n"
							 "  }\n"
							 "}\n";
	ExecutionFramework framework;
	framework.compileAndRun(sourceCode);
	BOOST_CHECK(framework.callFunction(0, bytes(1, 1) + toBigEndian(u256(0xcd)))
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
	ExecutionFramework framework;
	framework.compileAndRun(sourceCode);
	BOOST_CHECK(framework.callFunction(0, u256(0)) == toBigEndian(u256(0)));
	BOOST_CHECK(framework.callFunction(0, u256(1)) == toBigEndian(u256(8)));
}

//@todo test smaller types

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces

