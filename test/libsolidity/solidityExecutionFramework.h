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
#include <fstream>
#include "../TestHelper.h"
#include "../IPCSocket.h"
#include <libethcore/ABI.h>
#include <libethcore/SealEngine.h>
#include <libethereum/State.h>
#include <libethereum/Executive.h>
#include <libethereum/ChainParams.h>
#include <libsolidity/interface/CompilerStack.h>
#include <libsolidity/interface/Exceptions.h>
#include <libethcore/BasicAuthority.h>
#include <libethcore/SealEngine.h>

namespace dev
{

namespace solidity
{
namespace test
{

class ExecutionFramework
{

public:
	ExecutionFramework():
		m_state(0),
		m_socket("/tmp/test/geth.ipc")
	{
		eth::NoProof::init();
		m_sealEngine.reset(eth::ChainParams().createSealEngine());
		if (g_logVerbosity != -1)
			g_logVerbosity = 0;

		string account = m_socket.personal_newAccount("qwerty");
		m_socket.test_setChainParams(
			"0x1000000000000000000000000000000000000000",
			 account,
			"1000000000000000000000000000000000000000000000"
		);
		m_socket.personal_unlockAccount(account, "qwerty", 10000);
		m_sender = Address(account);
	}	

	bytes const& compileAndRunWithoutCheck(
		std::string const& _sourceCode,
		u256 const& _value = 0,
		std::string const& _contractName = "",
		bytes const& _arguments = bytes(),
		std::map<std::string, Address> const& _libraryAddresses = std::map<std::string, Address>()
	)
	{
		m_compiler.reset(false, m_addStandardSources);
		m_compiler.addSource("", _sourceCode);
		ETH_TEST_REQUIRE_NO_THROW(m_compiler.compile(m_optimize, m_optimizeRuns), "Compiling contract failed");
		eth::LinkerObject obj = m_compiler.object(_contractName);
		obj.link(_libraryAddresses);
		BOOST_REQUIRE(obj.linkReferences.empty());
		sendMessage(obj.bytecode + _arguments, true, _value);
		return m_output;
	}

	bytes const& compileAndRun(
		std::string const& _sourceCode,
		u256 const& _value = 0,
		std::string const& _contractName = "",
		bytes const& _arguments = bytes(),
		std::map<std::string, Address> const& _libraryAddresses = std::map<std::string, Address>()
	)
	{
		compileAndRunWithoutCheck(_sourceCode, _value, _contractName, _arguments, _libraryAddresses);
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
				toHex(cppResult)
		);
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

	class ContractInterface
	{
	public:
		ContractInterface(ExecutionFramework& _framework): m_framework(_framework) {}

		void setNextValue(u256 const& _value) { m_nextValue = _value; }

	protected:
		template <class... Args>
		bytes const& call(std::string const& _sig, Args const&... _arguments)
		{
			auto const& ret = m_framework.callContractFunctionWithValue(_sig, m_nextValue, _arguments...);
			m_nextValue = 0;
			return ret;
		}

		void callString(std::string const& _name, std::string const& _arg)
		{
			BOOST_CHECK(call(_name + "(string)", u256(0x20), _arg.length(), _arg).empty());
		}

		void callStringAddress(std::string const& _name, std::string const& _arg1, u160 const& _arg2)
		{
			BOOST_CHECK(call(_name + "(string,address)", u256(0x40), _arg2, _arg1.length(), _arg1).empty());
		}

		void callStringAddressBool(std::string const& _name, std::string const& _arg1, u160 const& _arg2, bool _arg3)
		{
			BOOST_CHECK(call(_name + "(string,address,bool)", u256(0x60), _arg2, _arg3, _arg1.length(), _arg1).empty());
		}

		void callStringBytes32(std::string const& _name, std::string const& _arg1, h256 const& _arg2)
		{
			BOOST_CHECK(call(_name + "(string,bytes32)", u256(0x40), _arg2, _arg1.length(), _arg1).empty());
		}

		u160 callStringReturnsAddress(std::string const& _name, std::string const& _arg)
		{
			bytes const& ret = call(_name + "(string)", u256(0x20), _arg.length(), _arg);
			BOOST_REQUIRE(ret.size() == 0x20);
			BOOST_CHECK(std::count(ret.begin(), ret.begin() + 12, 0) == 12);
			return eth::abiOut<u160>(ret);
		}

		std::string callAddressReturnsString(std::string const& _name, u160 const& _arg)
		{
			bytesConstRef ret = ref(call(_name + "(address)", _arg));
			BOOST_REQUIRE(ret.size() >= 0x20);
			u256 offset = eth::abiOut<u256>(ret);
			BOOST_REQUIRE_EQUAL(offset, 0x20);
			u256 len = eth::abiOut<u256>(ret);
			BOOST_REQUIRE_EQUAL(ret.size(), ((len + 0x1f) / 0x20) * 0x20);
			return ret.cropped(0, size_t(len)).toString();
		}

		h256 callStringReturnsBytes32(std::string const& _name, std::string const& _arg)
		{
			bytes const& ret = call(_name + "(string)", u256(0x20), _arg.length(), _arg);
			BOOST_REQUIRE(ret.size() == 0x20);
			return eth::abiOut<h256>(ret);
		}

	private:
		u256 m_nextValue;
		ExecutionFramework& m_framework;
	};

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
		RPCRequest::transactionData d;
		d.data = "0x" + toHex(_data);
		d.from = "0x" + toString(m_sender);
		d.gas = toHex(m_gas, HexPrefix::Add);
		d.gasPrice = toHex(m_gasPrice, HexPrefix::Add);
		d.value = toHex(_value, HexPrefix::Add);
		if (_isCreation)
			d.to = "";
		else
			d.to = dev::toString(m_contractAddress);

		string code = m_socket.eth_getCode(d.to, "latest");
		string output = m_socket.eth_call(d, "latest");
		string hash = m_socket.eth_sendTransaction(d);
		m_socket.test_mineBlocks(1);
		RPCRequest::transactionReceipt receipt;
		receipt = m_socket.eth_getTransactionReceipt(hash);

		if (_isCreation)
		{
			m_contractAddress = Address(receipt.contractAddress);
			BOOST_REQUIRE(m_contractAddress);
			string code = m_socket.eth_getCode(receipt.contractAddress, "latest");
			BOOST_REQUIRE(code.size() > 2);
		}
		else	
			BOOST_REQUIRE(code.size() > 2);

		m_gasUsed = u256(receipt.gasUsed);
		m_output = fromHex(output, WhenError::Throw);
		m_logs.clear();
	}

	std::unique_ptr<eth::SealEngineFace> m_sealEngine;
	size_t m_optimizeRuns = 200;
	bool m_optimize = false;
	bool m_addStandardSources = false;
	dev::solidity::CompilerStack m_compiler;
	Address m_sender;
	Address m_contractAddress;
	eth::EnvInfo m_envInfo;
	eth::State m_state;
	u256 const m_gasPrice = 100 * eth::szabo;
	u256 const m_gas = 100000000;
	bytes m_output;
	eth::LogEntries m_logs;
	u256 m_gasUsed;

	RPCRequest m_socket;
};

}
}
} // end namespaces

