
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
 * Framework for executing Solidity contracts and testing them against C++ implementation.
 */

#pragma once

#include <string>
#include <tuple>
#include <boost/test/unit_test.hpp>
#include <libethereum/State.h>
#include <libethereum/Executive.h>
#include <libsolidity/CompilerStack.h>

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

	bytes const& compileAndRun(std::string const& _sourceCode, u256 const& _value = 0, std::string const& _contractName = "")
	{
		dev::solidity::CompilerStack compiler(m_addStandardSources);
		try
		{
			compiler.addSource("", _sourceCode);
			compiler.compile(m_optimize);
		}
		catch(boost::exception const& _e)
		{
			auto msg = std::string("Compiling contract failed with: ") + boost::diagnostic_information(_e);
			BOOST_FAIL(msg);
		}

		bytes code = compiler.getBytecode(_contractName);
		sendMessage(code, true, _value);
		BOOST_REQUIRE(!m_output.empty());
		return m_output;
	}

	template <class... Args>
	bytes const& callContractFunctionWithValue(std::string _sig, u256 const& _value,
											   Args const&... _arguments)
	{
		FixedHash<4> hash(dev::sha3(_sig));
		sendMessage(hash.asBytes() + encodeArgs(_arguments...), false, _value);
		return m_output;
	}

	template <class... Args>
	bytes const& callContractFunction(std::string _sig, Args const&... _arguments)
	{
		return callContractFunctionWithValue(_sig, 0, _arguments...);
	}

	template <class CppFunction, class... Args>
	void testSolidityAgainstCpp(std::string _sig, CppFunction const& _cppFunction, Args const&... _arguments)
	{
		bytes solidityResult = callContractFunction(_sig, _arguments...);
		bytes cppResult = callCppAndEncodeResult(_cppFunction, _arguments...);
		BOOST_CHECK_MESSAGE(solidityResult == cppResult, "Computed values do not match."
							"\nSolidity: " + toHex(solidityResult) + "\nC++:      " + toHex(cppResult));
	}

	template <class CppFunction, class... Args>
	void testSolidityAgainstCppOnRange(std::string _sig, CppFunction const& _cppFunction,
									   u256 const& _rangeStart, u256 const& _rangeEnd)
	{
		for (u256 argument = _rangeStart; argument < _rangeEnd; ++argument)
		{
			bytes solidityResult = callContractFunction(_sig, argument);
			bytes cppResult = callCppAndEncodeResult(_cppFunction, argument);
			BOOST_CHECK_MESSAGE(solidityResult == cppResult, "Computed values do not match."
								"\nSolidity: " + toHex(solidityResult) + "\nC++:      " + toHex(cppResult) +
								"\nArgument: " + toHex(encode(argument)));
		}
	}

	static bytes encode(bool _value) { return encode(byte(_value)); }
	static bytes encode(int _value) { return encode(u256(_value)); }
	static bytes encode(char const* _value) { return encode(std::string(_value)); }
	static bytes encode(byte _value) { return bytes(31, 0) + bytes{_value}; }
	static bytes encode(u256 const& _value) { return toBigEndian(_value); }
	static bytes encode(h256 const& _value) { return _value.asBytes(); }
	static bytes encode(bytes const& _value, bool _padLeft = true)
	{
		bytes padding = bytes((32 - _value.size() % 32) % 32, 0);
		return _padLeft ? padding + _value : _value + padding;
	}
	static bytes encode(std::string const& _value) { return encode(asBytes(_value), false); }

	template <class FirstArg, class... Args>
	static bytes encodeArgs(FirstArg const& _firstArg, Args const&... _followingArgs)
	{
		return encode(_firstArg) + encodeArgs(_followingArgs...);
	}
	static bytes encodeArgs()
	{
		return bytes();
	}

private:
	template <class CppFunction, class... Args>
	auto callCppAndEncodeResult(CppFunction const& _cppFunction, Args const&... _arguments)
	-> typename std::enable_if<std::is_void<decltype(_cppFunction(_arguments...))>::value, bytes>::type
	{
		_cppFunction(_arguments...);
		return bytes();
	}
	template <class CppFunction, class... Args>
	auto callCppAndEncodeResult(CppFunction const& _cppFunction, Args const&... _arguments)
	-> typename std::enable_if<!std::is_void<decltype(_cppFunction(_arguments...))>::value, bytes>::type
	{
		return encode(_cppFunction(_arguments...));
	}

protected:
	void sendMessage(bytes const& _data, bool _isCreation, u256 const& _value = 0)
	{
		m_state.addBalance(m_sender, _value); // just in case
		eth::Executive executive(m_state, eth::LastHashes(), 0);
		eth::Transaction t = _isCreation ? eth::Transaction(_value, m_gasPrice, m_gas, _data, 0, KeyPair::create().sec())
										 : eth::Transaction(_value, m_gasPrice, m_gas, m_contractAddress, _data, 0, KeyPair::create().sec());
		bytes transactionRLP = t.rlp();
		try
		{
			// this will throw since the transaction is invalid, but it should nevertheless store the transaction
			executive.setup(&transactionRLP);
		}
		catch (...) {}
		if (_isCreation)
		{
			BOOST_REQUIRE(!executive.create(m_sender, _value, m_gasPrice, m_gas, &_data, m_sender));
			m_contractAddress = executive.newAddress();
			BOOST_REQUIRE(m_contractAddress);
			BOOST_REQUIRE(m_state.addressHasCode(m_contractAddress));
		}
		else
		{
			BOOST_REQUIRE(m_state.addressHasCode(m_contractAddress));
			BOOST_REQUIRE(!executive.call(m_contractAddress, m_contractAddress, m_sender, _value, m_gasPrice, &_data, m_gas, m_sender));
		}
		BOOST_REQUIRE(executive.go());
		m_state.noteSending(m_sender);
		executive.finalize();
		m_output = executive.out().toVector();
		m_logs = executive.logs();
	}

	bool m_optimize = false;
	bool m_addStandardSources = false;
	Address m_sender;
	Address m_contractAddress;
	eth::State m_state;
	u256 const m_gasPrice = 100 * eth::szabo;
	u256 const m_gas = 1000000;
	bytes m_output;
	eth::LogEntries m_logs;
};

}
}
} // end namespaces

