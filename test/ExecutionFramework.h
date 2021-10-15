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
// SPDX-License-Identifier: GPL-3.0
/**
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Framework for executing contracts and testing them using RPC.
 */

#pragma once

#include <test/Common.h>
#include <test/EVMHost.h>

#include <libsolidity/interface/OptimiserSettings.h>
#include <libsolidity/interface/DebugSettings.h>

#include <liblangutil/EVMVersion.h>

#include <libsolutil/FixedHash.h>
#include <libsolutil/Keccak256.h>
#include <libsolutil/ErrorCodes.h>

#include <functional>

#include <boost/test/unit_test.hpp>

namespace solidity::frontend::test
{
struct LogRecord;
} // namespace solidity::frontend::test

namespace solidity::test
{
using rational = boost::rational<bigint>;

// The ether and gwei denominations; here for ease of use where needed within code.
static u256 const gwei = u256(1) << 9;
static u256 const ether = u256(1) << 18;
class ExecutionFramework
{

public:
	ExecutionFramework();
	ExecutionFramework(langutil::EVMVersion _evmVersion, std::vector<boost::filesystem::path> const& _vmPaths);
	virtual ~ExecutionFramework() = default;

	virtual bytes const& compileAndRunWithoutCheck(
		std::map<std::string, std::string> const& _sourceCode,
		u256 const& _value = 0,
		std::string const& _contractName = "",
		bytes const& _arguments = {},
		std::map<std::string, util::h160> const& _libraryAddresses = {},
		std::optional<std::string> const& _sourceName = std::nullopt
	) = 0;

	bytes const& compileAndRun(
		std::string const& _sourceCode,
		u256 const& _value = 0,
		std::string const& _contractName = "",
		bytes const& _arguments = {},
		std::map<std::string, util::h160> const& _libraryAddresses = {}
	)
	{
		compileAndRunWithoutCheck(
			{{"", _sourceCode}},
			_value,
			_contractName,
			_arguments,
			_libraryAddresses
		);
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

	bytes const& callLowLevel(bytes const& _data, u256 const& _value)
	{
		sendMessage(_data, false, _value);
		return m_output;
	}

	bytes const& callContractFunctionWithValueNoEncoding(std::string _sig, u256 const& _value, bytes const& _arguments)
	{
		util::FixedHash<4> hash(util::keccak256(_sig));
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
				util::toHex(contractResult) +
				"\nC++:      " +
				util::toHex(cppResult)
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
					util::toHex(contractResult) +
					"\nC++:      " +
					util::toHex(cppResult) +
					"\nArgument: " +
					util::toHex(encode(argument))
			);
		}
	}

	static std::pair<bool, std::string> compareAndCreateMessage(bytes const& _result, bytes const& _expectation);

	static bytes encode(bool _value) { return encode(uint8_t(_value)); }
	static bytes encode(int _value) { return encode(u256(_value)); }
	static bytes encode(size_t _value) { return encode(u256(_value)); }
	static bytes encode(char const* _value) { return encode(std::string(_value)); }
	static bytes encode(uint8_t _value) { return bytes(31, 0) + bytes{_value}; }
	static bytes encode(u256 const& _value) { return toBigEndian(_value); }
	/// @returns the fixed-point encoding of a rational number with a given
	/// number of fractional bits.
	static bytes encode(std::pair<rational, int> const& _valueAndPrecision)
	{
		rational const& value = _valueAndPrecision.first;
		int fractionalBits = _valueAndPrecision.second;
		return encode(u256((value.numerator() << fractionalBits) / value.denominator()));
	}
	static bytes encode(util::h256 const& _value) { return _value.asBytes(); }
	static bytes encode(util::h160 const& _value) { return encode(util::h256(_value, util::h256::AlignRight)); }
	static bytes encode(bytes const& _value, bool _padLeft = true)
	{
		bytes padding = bytes((32 - _value.size() % 32) % 32, 0);
		return _padLeft ? padding + _value : _value + padding;
	}
	static bytes encode(std::string const& _value) { return encode(util::asBytes(_value), false); }
	template <class T>
	static bytes encode(std::vector<T> const& _value)
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
	/// @returns error returndata corresponding to the Panic(uint256) error code,
	/// if REVERT is supported by the current EVM version and the empty string otherwise.
	bytes panicData(util::PanicCode _code);

	//@todo might be extended in the future
	template <class Arg>
	static bytes encodeDyn(Arg const& _arg)
	{
		return encodeArgs(u256(0x20), u256(_arg.size()), _arg);
	}

	u256 gasLimit() const;
	u256 gasPrice() const;
	u256 blockHash(u256 const& _blockNumber) const;
	u256 blockNumber() const;

	template<typename Range>
	static bytes encodeArray(bool _dynamicallySized, bool _dynamicallyEncoded, Range const& _elements)
	{
		bytes result;
		if (_dynamicallySized)
			result += encode(u256(_elements.size()));
		if (_dynamicallyEncoded)
		{
			u256 offset = u256(_elements.size()) * 32;
			std::vector<bytes> subEncodings;
			for (auto const& element: _elements)
			{
				result += encode(offset);
				subEncodings.emplace_back(encode(element));
				offset += subEncodings.back().size();
			}
			for (auto const& subEncoding: subEncodings)
				result += subEncoding;
		}
		else
			for (auto const& element: _elements)
				result += encode(element);
		return result;
	}

	util::h160 setAccount(size_t _accountNumber)
	{
		m_sender = account(_accountNumber);
		return m_sender;
	}

	size_t numLogs() const;
	size_t numLogTopics(size_t _logIdx) const;
	util::h256 logTopic(size_t _logIdx, size_t _topicIdx) const;
	util::h160 logAddress(size_t _logIdx) const;
	bytes logData(size_t _logIdx) const;

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
	void selectVM(evmc_capabilities _cap = evmc_capabilities::EVMC_CAPABILITY_EVM1);
	void reset();

	void sendMessage(bytes const& _data, bool _isCreation, u256 const& _value = 0);
	void sendEther(util::h160 const& _to, u256 const& _value);
	size_t currentTimestamp();
	size_t blockTimestamp(u256 _number);

	/// @returns the (potentially newly created) _ith address.
	util::h160 account(size_t _i);

	u256 balanceAt(util::h160 const& _addr) const;
	bool storageEmpty(util::h160 const& _addr) const;
	bool addressHasCode(util::h160 const& _addr) const;

	std::vector<frontend::test::LogRecord> recordedLogs() const;

	langutil::EVMVersion m_evmVersion;
	solidity::frontend::RevertStrings m_revertStrings = solidity::frontend::RevertStrings::Default;
	solidity::frontend::OptimiserSettings m_optimiserSettings = solidity::frontend::OptimiserSettings::minimal();
	bool m_showMessages = false;
	bool m_supportsEwasm = false;
	std::unique_ptr<EVMHost> m_evmcHost;

	std::vector<boost::filesystem::path> m_vmPaths;

	bool m_transactionSuccessful = true;
	util::h160 m_sender = account(0);
	util::h160 m_contractAddress;
	u256 const m_gasPrice = 10 * gwei;
	u256 const m_gas = 100000000;
	bytes m_output;
	u256 m_gasUsed;
};

#define ABI_CHECK(result, expectation) do { \
	auto abiCheckResult = ExecutionFramework::compareAndCreateMessage((result), (expectation)); \
	BOOST_CHECK_MESSAGE(abiCheckResult.first, abiCheckResult.second); \
} while (0)


} // end namespaces
