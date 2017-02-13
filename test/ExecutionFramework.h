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
 * @date 2014
 * Framework for executing contracts and testing them using RPC.
 */

#pragma once

#include <functional>

#include "TestHelper.h"
#include "RPCSession.h"

#include <libdevcore/ABI.h>
#include <libdevcore/FixedHash.h>

namespace dev
{
namespace test
{
	using rational = boost::rational<dev::bigint>;
	/// An Ethereum address: 20 bytes.
	/// @NOTE This is not endian-specific; it's just a bunch of bytes.
	using Address = h160;

	// The various denominations; here for ease of use where needed within code.
	static const u256 ether = exp10<18>();
	static const u256 finney = exp10<15>();
	static const u256 szabo = exp10<12>();
	static const u256 shannon = exp10<9>();
	static const u256 wei = exp10<0>();

class ExecutionFramework
{

public:
	ExecutionFramework();

	virtual bytes const& compileAndRunWithoutCheck(
		std::string const& _sourceCode,
		u256 const& _value = 0,
		std::string const& _contractName = "",
		bytes const& _arguments = bytes(),
		std::map<std::string, Address> const& _libraryAddresses = std::map<std::string, Address>()
	) = 0;

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

	bytes const& callFallbackWithValue(u256 const& _value)
	{
		sendMessage(bytes(), false, _value);
		return m_output;
	}

	bytes const & callFallback()
	{
		return callFallbackWithValue(0);
	}

	template <class... Args>
	bytes const& callContractFunctionWithValue(std::string _sig, u256 const& _value, Args const&... _arguments)
	{
		FixedHash<4> hash(dev::keccak256(_sig));
		sendMessage(hash.asBytes() + encodeArgs(_arguments...), false, _value);
		return m_output;
	}

	template <class... Args>
	bytes const& callContractFunction(std::string _sig, Args const&... _arguments)
	{
		return callContractFunctionWithValue(_sig, 0, _arguments...);
	}

	template <class CppFunction, class... Args>
	void testContractAgainstCpp(std::string _sig, CppFunction const& _cppFunction, Args const&... _arguments)
	{
		bytes contractResult = callContractFunction(_sig, _arguments...);
		bytes cppResult = callCppAndEncodeResult(_cppFunction, _arguments...);
		BOOST_CHECK_MESSAGE(
			contractResult == cppResult,
			"Computed values do not match.\nContract: " +
				toHex(contractResult) +
				"\nC++:      " +
				toHex(cppResult)
		);
	}

	template <class CppFunction, class... Args>
	void testContractAgainstCppOnRange(std::string _sig, CppFunction const& _cppFunction, u256 const& _rangeStart, u256 const& _rangeEnd)
	{
		for (u256 argument = _rangeStart; argument < _rangeEnd; ++argument)
		{
			bytes contractResult = callContractFunction(_sig, argument);
			bytes cppResult = callCppAndEncodeResult(_cppFunction, argument);
			BOOST_CHECK_MESSAGE(
				contractResult == cppResult,
				"Computed values do not match.\nContract: " +
					toHex(contractResult) +
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
	/// @returns the fixed-point encoding of a rational number with a given
	/// number of fractional bits.
	static bytes encode(std::pair<rational, int> const& _valueAndPrecision)
	{
		rational const& value = _valueAndPrecision.first;
		int fractionalBits = _valueAndPrecision.second;
		return encode(u256((value.numerator() << fractionalBits) / value.denominator()));
	}
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
	void sendMessage(bytes const& _data, bool _isCreation, u256 const& _value = 0);
	void sendEther(Address const& _to, u256 const& _value);
	size_t currentTimestamp();
	size_t blockTimestamp(u256 number);

	/// @returns the (potentially newly created) _ith address.
	Address account(size_t _i);

	u256 balanceAt(Address const& _addr);
	bool storageEmpty(Address const& _addr);
	bool addressHasCode(Address const& _addr);

	RPCSession& m_rpc;

	struct LogEntry
	{
		Address address;
		std::vector<h256> topics;
		bytes data;
	};

	unsigned m_optimizeRuns = 200;
	bool m_optimize = false;
	bool m_showMessages = false;
	Address m_sender;
	Address m_contractAddress;
	u256 m_blockNumber;
	u256 const m_gasPrice = 100 * szabo;
	u256 const m_gas = 100000000;
	bytes m_output;
	std::vector<LogEntry> m_logs;
	u256 m_gasUsed;
};

}
} // end namespaces

