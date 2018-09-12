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

#include <test/Options.h>
#include <test/RPCSession.h>

#include <libsolidity/interface/EVMVersion.h>

#include <libdevcore/FixedHash.h>
#include <libdevcore/SHA3.h>

#include <functional>

namespace dev
{
namespace test
{
	using rational = boost::rational<dev::bigint>;
	/// An Ethereum address: 20 bytes.
	/// @NOTE This is not endian-specific; it's just a bunch of bytes.
	using Address = h160;

	// The various denominations; here for ease of use where needed within code.
	static const u256 wei = 1;
	static const u256 shannon = u256("1000000000");
	static const u256 szabo = shannon * 1000;
	static const u256 finney = szabo * 1000;
	static const u256 ether = finney * 1000;

class ExecutionFramework
{

public:
	ExecutionFramework();
	virtual ~ExecutionFramework() = default;

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
		BOOST_REQUIRE(m_transactionSuccessful);
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

	bytes const& callContractFunctionWithValueNoEncoding(std::string _sig, u256 const& _value, bytes const& _arguments)
	{
		FixedHash<4> hash(dev::keccak256(_sig));
		sendMessage(hash.asBytes() + _arguments, false, _value);
		return m_output;
	}

	bytes const& callContractFunctionNoEncoding(std::string _sig, bytes const& _arguments)
	{
		return callContractFunctionWithValueNoEncoding(_sig, 0, _arguments);
	}

	template <class... Args>
	bytes const& callContractFunctionWithValue(std::string _sig, u256 const& _value, Args const&... _arguments)
	{
		return callContractFunctionWithValueNoEncoding(_sig, _value, encodeArgs(_arguments...));
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

	static std::pair<bool, std::string> compareAndCreateMessage(bytes const& _result, bytes const& _expectation);

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

	u256 gasLimit() const;
	u256 gasPrice() const;
	u256 blockHash(u256 const& _blockNumber) const;
	u256 const& blockNumber() const {
		return m_blockNumber;
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
	void sendMessage(bytes const& _data, bool _isCreation, u256 const& _value = 0);
	void sendEther(Address const& _to, u256 const& _value);
	size_t currentTimestamp();
	size_t blockTimestamp(u256 _number);

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

	solidity::EVMVersion m_evmVersion;
	unsigned m_optimizeRuns = 200;
	bool m_optimize = false;
	bool m_showMessages = false;
	bool m_transactionSuccessful = true;
	Address m_sender;
	Address m_contractAddress;
	u256 m_blockNumber;
	u256 const m_gasPrice = 100 * szabo;
	u256 const m_gas = 100000000;
	bytes m_output;
	std::vector<LogEntry> m_logs;
	u256 m_gasUsed;
};

#define ABI_CHECK(result, expectation) do { \
	auto abiCheckResult = ExecutionFramework::compareAndCreateMessage((result), (expectation)); \
	BOOST_CHECK_MESSAGE(abiCheckResult.first, abiCheckResult.second); \
} while (0)


}
} // end namespaces

