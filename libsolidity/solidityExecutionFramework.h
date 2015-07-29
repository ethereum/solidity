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
#include "../TestHelper.h"
#include <libethereum/State.h>
#include <libethereum/Executive.h>
#include <libsolidity/CompilerStack.h>
#include <libsolidity/Exceptions.h>

namespace dev
{

namespace solidity
{
namespace test
{

class ExecutionFramework
{
public:
	ExecutionFramework()
	{
		g_logVerbosity = 0;
		m_state.resetCurrent();
	}

	bytes const& compileAndRunWithoutCheck(
		std::string const& _sourceCode,
		u256 const& _value = 0,
		std::string const& _contractName = "",
		bytes const& _arguments = bytes()
	)
	{
		m_compiler.reset(false, m_addStandardSources);
		m_compiler.addSource("", _sourceCode);
		ETH_TEST_REQUIRE_NO_THROW(m_compiler.compile(m_optimize, m_optimizeRuns), "Compiling contract failed");
		bytes code = m_compiler.getBytecode(_contractName);
		sendMessage(code + _arguments, true, _value);
		return m_output;
	}

	template <class Exceptiontype>
	void compileRequireThrow(std::string const& _sourceCode)
	{
		m_compiler.reset(false, m_addStandardSources);
		m_compiler.addSource("", _sourceCode);
		BOOST_REQUIRE_THROW(m_compiler.compile(m_optimize, m_optimizeRuns), Exceptiontype);
	}

	bytes const& compileAndRun(
		std::string const& _sourceCode,
		u256 const& _value = 0,
		std::string const& _contractName = "",
		bytes const& _arguments = bytes()
	)
	{
		compileAndRunWithoutCheck(_sourceCode, _value, _contractName, _arguments);
		BOOST_REQUIRE(!m_output.empty());
		return m_output;
	}

	template <class... Args>
	bytes const& callContractFunctionWithValue(std::string _sig, u256 const& _value, Args const&... _arguments)
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
		BOOST_CHECK_MESSAGE(
			solidityResult == cppResult,
			"Computed values do not match.\nSolidity: " +
				toHex(solidityResult) +
				"\nC++:      " +
				toHex(cppResult));
	}

	template <class CppFunction, class... Args>
	void testSolidityAgainstCppOnRange(std::string _sig, CppFunction const& _cppFunction, u256 const& _rangeStart, u256 const& _rangeEnd)
	{
		for (u256 argument = _rangeStart; argument < _rangeEnd; ++argument)
		{
			bytes solidityResult = callContractFunction(_sig, argument);
			bytes cppResult = callCppAndEncodeResult(_cppFunction, argument);
			BOOST_CHECK_MESSAGE(
				solidityResult == cppResult,
				"Computed values do not match.\nSolidity: " +
					toHex(solidityResult) +
					"\nC++:      " +
					toHex(cppResult) +
					"\nArgument: " +
					toHex(encode(argument))
			);
		}
	}

	static bytes encode(bool _value) { return encode(byte(_value)); }
	static bytes encode(int _value) { return encode(u256(_value)); }
	static bytes encode(size_t _value) { return encode(u256(_value)); }
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
	template <class _T>
	static bytes encode(std::vector<_T> const& _value)
	{
		bytes ret;
		for (auto const& v: _value)
			ret += encode(v);
		return ret;
	}

	template <class FirstArg, class... Args>
	static bytes encodeArgs(FirstArg const& _firstArg, Args const&... _followingArgs)
	{
		return encode(_firstArg) + encodeArgs(_followingArgs...);
	}
	static bytes encodeArgs()
	{
		return bytes();
	}
	//@todo might be extended in the future
	template <class Arg>
	static bytes encodeDyn(Arg const& _arg)
	{
		return encodeArgs(u256(0x20), u256(_arg.size()), _arg);
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
		eth::ExecutionResult res;
		executive.setResultRecipient(res);
		eth::Transaction t =
			_isCreation ?
				eth::Transaction(_value, m_gasPrice, m_gas, _data, 0, KeyPair::create().sec()) :
				eth::Transaction(_value, m_gasPrice, m_gas, m_contractAddress, _data, 0, KeyPair::create().sec());
		bytes transactionRLP = t.rlp();
		try
		{
			// this will throw since the transaction is invalid, but it should nevertheless store the transaction
			executive.initialize(&transactionRLP);
			executive.execute();
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
			BOOST_REQUIRE(!executive.call(m_contractAddress, m_sender, _value, m_gasPrice, &_data, m_gas));
		}
		BOOST_REQUIRE(executive.go(/* DEBUG eth::Executive::simpleTrace() */));
		m_state.noteSending(m_sender);
		executive.finalize();
		m_gasUsed = res.gasUsed;
		m_output = std::move(res.output);
		m_logs = executive.logs();
	}

	size_t m_optimizeRuns = 200;
	bool m_optimize = false;
	bool m_addStandardSources = false;
	dev::solidity::CompilerStack m_compiler;
	Address m_sender;
	Address m_contractAddress;
	eth::State m_state;
	u256 const m_gasPrice = 100 * eth::szabo;
	u256 const m_gas = 100000000;
	bytes m_output;
	eth::LogEntries m_logs;
	u256 m_gasUsed;
};

}
}
} // end namespaces

