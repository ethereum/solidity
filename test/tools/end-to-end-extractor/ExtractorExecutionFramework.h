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

#include <test/Common.h>

#include <libsolidity/interface/CompilerStack.h>
#include <libsolidity/interface/DebugSettings.h>
#include <libsolidity/interface/OptimiserSettings.h>

#include <liblangutil/EVMVersion.h>

#include <libsolutil/FixedHash.h>
#include <libsolutil/Keccak256.h>

#include <test/libsolidity/util/BytesUtils.h>

#include <functional>

#include "ExtractionTask.h"
#include "FakeCompilerStack.hpp"
#include "FakeEvmHost.hpp"

bool operator==(solidity::bytes const &_left, solidity::bytes const &_right);
solidity::bytes operator+(solidity::bytes const &_left, solidity::bytes const &_right);
solidity::bytes operator+(const char *_left, solidity::bytes const &_right);
solidity::bytes operator+(solidity::bytes const &_left, const char *_right);

namespace solidity::test
{
typedef std::map<std::string, ExtractionTask> TestSuite;

using rational = boost::rational<bigint>;
/// An Ethereum address: 20 bytes.
/// @NOTE This is not endian-specific; it's just a bunch of bytes.
using Address = util::h160;

// The various denominations; here for ease of use where needed within code.
static const u256 wei = 1;
static const u256 shannon = u256("1000000000");
static const u256 szabo = shannon * 1000;
static const u256 finney = szabo * 1000;
static const u256 ether = finney * 1000;

class ExtractorExecutionFramework
{
  public:
	ExtractorExecutionFramework();
	explicit ExtractorExecutionFramework(langutil::EVMVersion _evmVersion);
	virtual ~ExtractorExecutionFramework() = default;

	bytes const compileAndRunWithoutCheck(std::string const &_sourceCode,
	                                      u256 const &_value = 0,
	                                      std::string const &_contractName = "",
	                                      bytes const &_arguments = bytes(),
	                                      std::map<std::string, Address> const &_libraryAddresses
	                                      = std::map<std::string, Address>())
	{
		m_current->compileAndRunWithoutCheck(_sourceCode, _value, _contractName, _arguments, _libraryAddresses);
		return bytes();
	}

	bytes const compileAndRun(std::string const &_sourceCode,
	                          u256 const &_value = 0,
	                          std::string const &_contractName = "",
	                          bytes const &_arguments = bytes(),
	                          std::map<std::string, Address> const &_libraryAddresses
	                          = std::map<std::string, Address>())
	{
		return compileAndRunWithoutCheck(_sourceCode, _value, _contractName, _arguments, _libraryAddresses);
	}

	bytes const callFallbackWithValue(u256 const &_value)
	{
		if (_value > 0)
			m_current->extractionNotPossible("sending value is not supported");
		return bytes();
	}

	bytes const callFallback() { return callFallbackWithValue(0); }

	bytes const callLowLevel(bytes const &_data, u256 const &_value)
	{
		(void) _data;

		if (_value > 0)
			m_current->extractionNotPossible("sending value is not supported");

		return bytes();
	}

	bytes const callContractFunctionWithValueNoEncoding(std::string _sig, u256 const &_value, bytes const &_arguments)
	{
		(void) _value;

		if (_value > 0)
			m_current->extractionNotPossible("sending value is not supported");

		return frontend::test::BytesUtils::convertString(_sig + "{" + formatString(_arguments) + "}");
	}

	bytes const callContractFunctionNoEncoding(std::string _sig, bytes const &_arguments)
	{
		return callContractFunctionWithValueNoEncoding(_sig, 0, _arguments);
	}

	template <class... Args>
	bytes const callContractFunctionWithValue(std::string _sig, u256 const &_value, Args const &... _arguments)
	{
		return callContractFunctionWithValueNoEncoding(_sig, _value, extractor_encodeArgs(_arguments...));
	}

	template <class... Args> bytes const callContractFunction(std::string _sig, Args const &... _arguments)
	{
		return callContractFunctionWithValue(_sig, 0, _arguments...);
	}

	template <class CppFunction, class... Args>
	void testContractAgainstCpp(std::string _sig, CppFunction const &_cppFunction, Args const &... _arguments)
	{
		bytes contractResult = callContractFunction(_sig, _arguments...);
		bytes cppResult = callCppAndEncodeResult(_cppFunction, _arguments...);

		std::string sig(formatString(contractResult));
		sig = sig.substr(0, sig.find('{'));

		std::string parameters(formatString(contractResult));
		parameters = parameters.substr(parameters.find('{') + 1, parameters.find('}') - parameters.find('{') - 1);

		std::string result(formatString(cppResult));

		m_current->addExpectation(sig, parameters, result);

		m_current->extractionNotPossible("testing against c++ function");
	}

	template <class CppFunction, class... Args>
	void testContractAgainstCppOnRange(std::string _sig,
	                                   CppFunction const &_cppFunction,
	                                   u256 const &_rangeStart,
	                                   u256 const &_rangeEnd)
	{
		for (u256 argument = _rangeStart; argument < _rangeEnd; ++argument)
		{
			bytes contractResult = callContractFunction(_sig, argument);
			bytes cppResult = callCppAndEncodeResult(_cppFunction, argument);

			std::string sig(formatString(contractResult));
			sig = sig.substr(0, sig.find('{'));

			std::string parameters(formatString(contractResult));
			parameters = parameters.substr(parameters.find('{') + 1, parameters.find('}') - parameters.find('{') - 1);

			std::string result(formatString(cppResult));

			m_current->addExpectation(sig, parameters, result);
		}
		m_current->extractionNotPossible("testing against c++ function");
	}

	static bytes encode(bool _value) { return encode(uint8_t(_value)); }
	static bytes encode(int _value) { return encode(u256(_value)); }
	static bytes encode(size_t _value) { return encode(u256(_value)); }
	static bytes encode(char const *_value) { return encode(std::string(_value)); }
	static bytes encode(uint8_t _value) { return bytes(31, 0) + bytes{_value}; }
	static bytes encode(u256 const &_value) { return util::toBigEndian(_value); }
	/// @returns the fixed-point encoding of a rational number with a given
	/// number of fractional bits.
	static bytes encode(std::pair<rational, int> const &_valueAndPrecision)
	{
		rational const &value = _valueAndPrecision.first;
		int fractionalBits = _valueAndPrecision.second;
		return encode(u256((value.numerator() << fractionalBits) / value.denominator()));
	}
	static bytes encode(util::h256 const &_value) { return _value.asBytes(); }
	static bytes encode(bytes const &_value, bool _padLeft = true)
	{
		bytes padding = bytes((32 - _value.size() % 32) % 32, 0);
		return _padLeft ? padding + _value : _value + padding;
	}
	static bytes encode(std::string const &_value) { return encode(util::asBytes(_value), false); }
	template <class _T> static bytes encode(std::vector<_T> const &_value)
	{
		bytes ret;
		for (auto const &v : _value)
			ret += encode(v);
		return ret;
	}

	template <class FirstArg, class... Args>
	static bytes encodeArgs(FirstArg const &_firstArg, Args const &... _followingArgs)
	{
		return encode(_firstArg) + encodeArgs(_followingArgs...);
	}
	static bytes encodeArgs() { return bytes(); }

	//@todo might be extended in the future
	template <class Arg> static bytes encodeDyn(Arg const &_arg)
	{
		return encodeArgs(u256(0x20), u256(_arg.size()), _arg);
	}

	u256 gasLimit() const;
	u256 gasPrice() const;
	u256 blockHash(u256 const &_blockNumber) const;
	u256 blockNumber() const;

	template <typename Range>
	static bytes encodeArray(bool _dynamicallySized, bool _dynamicallyEncoded, Range const &_elements)
	{
		bytes result;
		if (_dynamicallySized)
			result += encode(u256(_elements.size()));
		if (_dynamicallyEncoded)
		{
			u256 offset = u256(_elements.size()) * 32;
			std::vector<bytes> subEncodings;
			for (auto const &element : _elements)
			{
				result += encode(offset);
				subEncodings.emplace_back(encode(element));
				offset += subEncodings.back().size();
			}
			for (auto const &subEncoding : subEncodings)
				result += subEncoding;
		}
		else
			for (auto const &element : _elements)
				result += encode(element);
		return result;
	}

	// preserve original type
	template <class T> T extractor_asString(T _b) {
		return _b;
	}

	static bytes extractor_fromHex(std::string const& _hex) {
		std::string prefix{"0x"};
		if (_hex.size() >= 2 && _hex[0] == '0' && _hex[1] == 'x')
			prefix.clear();
		return frontend::test::BytesUtils::convertString(prefix + _hex);
	}

	//@todo might be extended in the future
	template <class Arg> static bytes extractor_encodeDyn(Arg const &_arg)
	{
		return extractor_encodeArgs(u256(0x20), u256(_arg.size()), _arg);
	}

	template <class... Args>
	static bytes extractor_encodeArgs(std::string const &_firstArg, Args const &... _followingArgs)
	{
		std::stringstream o;
		o << "\"" << formatString(frontend::test::BytesUtils::convertString(_firstArg)) << "\"";
		if (sizeof...(_followingArgs))
			o << ", ";
		bytes result = frontend::test::BytesUtils::convertString(o.str()) + extractor_encodeArgs(_followingArgs...);
		return result;
	}

	template <class... Args> static bytes extractor_encodeArgs(const char *_firstArg, Args const &... _followingArgs)
	{
		return extractor_encodeArgs(std::string(_firstArg)) + extractor_encodeArgs(_followingArgs...);
	}

	template <class... Args>
	static bytes extractor_encodeArgs(util::h256 const &_firstArg, Args const &... _followingArgs)
	{
		std::stringstream o;
		o << "0x";
		o << std::setw(2) << std::setfill('0') << std::hex << _firstArg;
		if (sizeof...(_followingArgs))
			o << ", ";
		bytes result = frontend::test::BytesUtils::convertString(o.str()) + extractor_encodeArgs(_followingArgs...);
		return result;
	}

	template <class... Args> static bytes extractor_encodeArgs(u256 const &_firstArg, Args const &... _followingArgs)
	{
		std::stringstream o;
		o << "0x";
		o << std::setw(2) << std::setfill('0') << std::hex << _firstArg;
		if (sizeof...(_followingArgs))
			o << ", ";
		bytes result = frontend::test::BytesUtils::convertString(o.str()) + extractor_encodeArgs(_followingArgs...);
		return result;
	}

	template <class... Args> static bytes extractor_encodeArgs(bytes const &_firstArg, Args const &... _followingArgs)
	{
		std::stringstream o;
		o << frontend::test::BytesUtils::formatHexString(_firstArg);
		if (sizeof...(_followingArgs))
			o << ", ";
		bytes result = frontend::test::BytesUtils::convertString(o.str()) + extractor_encodeArgs(_followingArgs...);
		return result;
	}

	template <class T, class... Args>
	static bytes extractor_encodeArgs(std::vector<T> const &_firstArg, Args const &... _followingArgs)
	{
		std::stringstream o;
		if (!_firstArg.empty())
		{
			for (auto const &a : _firstArg)
			{
				o << "0x" << std::setw(2) << std::setfill('0') << std::hex << a;
				if (a != *_firstArg.rbegin())
					o << ", ";
			}
			if (sizeof...(_followingArgs))
				o << ", ";
		}
		bytes result = frontend::test::BytesUtils::convertString(o.str()) + extractor_encodeArgs(_followingArgs...);
		return result;
	}

	template <class FirstArg, class... Args>
	static bytes extractor_encodeArgs(FirstArg const &_firstArg, Args const &... _followingArgs)
	{
		std::stringstream o;
		o << "0x" << std::hex << _firstArg;
		if (sizeof...(_followingArgs))
			o << ", ";
		bytes result = frontend::test::BytesUtils::convertString(o.str()) + extractor_encodeArgs(_followingArgs...);
		return result;
	}

	static bytes extractor_encodeArgs() { return bytes(); }

	static std::string formatString(bytes const &_bytes)
	{
		std::vector<std::string> parameters;
		bool wasPrintable{false};

		std::string current;
		for (size_t i = 0; i < _bytes.size(); ++i)
		{
			auto const v = _bytes[i];
			if (isprint(v))
			{
				wasPrintable = true;
				current += (char) v;
			}
			else
			{
				if (!current.empty())
					parameters.push_back(current);

				current.clear();
				wasPrintable = false;
				std::stringstream os;
				os << "0x" << std::setw(2) << std::setfill('0') << std::hex << (int) v;
				parameters.emplace_back(os.str());
			}
		}
		if (!current.empty())
			parameters.push_back(current);

		std::string result;
		for (auto const &item : parameters)
		{
			result += item;
			if (item != *parameters.rbegin())
				result += ", ";
		}
		return result;
	}

	std::shared_ptr<FakeEvmHost> extractor_m_evmHost()
	{
		m_current->extractionNotPossible("Accessing m_evmHost");
		return m_evmHost;
	}

	bytes extractor_m_output()
	{
		m_current->extractionNotPossible("Accessing m_output");
		return m_output;
	}

	Address extractor_m_contractAddress()
	{
		m_current->extractionNotPossible("Accessing m_contractAddress");
		return m_contractAddress;
	}

	FakeCompilerStack &extractor_m_compiler()
	{
		m_current->extractionNotPossible("Accessing m_compiler");
		return m_compiler;
	}

	static ExtractionTask *m_current;

	void prepareTest(std::string const &name, TestSuite &suite) { m_current = &suite[name]; }

	void ABI_CHECK(bytes const &_result, bytes const &_expectation)
	{
		std::string sig(formatString(_result));
		sig = sig.substr(0, sig.find('{'));

		std::string parameters(formatString(_result));
		parameters = parameters.substr(parameters.find('{') + 1, parameters.find('}') - parameters.find('{') - 1);

		std::string result(formatString(_expectation));

		m_current->addExpectation(sig, parameters, result);
	}

  private:
	template <class CppFunction, class... Args>
	auto callCppAndEncodeResult(CppFunction const &_cppFunction, Args const &... _arguments) ->
	    typename std::enable_if<std::is_void<decltype(_cppFunction(_arguments...))>::value, bytes>::type
	{
		_cppFunction(_arguments...);
		return bytes();
	}
	template <class CppFunction, class... Args>
	auto callCppAndEncodeResult(CppFunction const &_cppFunction, Args const &... _arguments) ->
	    typename std::enable_if<!std::is_void<decltype(_cppFunction(_arguments...))>::value, bytes>::type
	{
		return extractor_encodeArgs(_cppFunction(_arguments...));
	}

  protected:
	void sendMessage(bytes const &_data, bool _isCreation, u256 const &_value = 0);
	void sendEther(Address const &_to, u256 const &_value);
	size_t currentTimestamp();
	size_t blockTimestamp(u256 _number);

	/// @returns the (potentially newly created) _ith address.
	Address account(size_t _i);

	u256 balanceAt(Address const &_addr);
	bool storageEmpty(Address const &_addr);
	bool addressHasCode(Address const &_addr);

	size_t numLogs() const;
	size_t numLogTopics(size_t _logIdx) const;
	util::h256 logTopic(size_t _logIdx, size_t _topicIdx) const;
	Address logAddress(size_t _logIdx) const;
	bytes logData(size_t _logIdx) const;

	langutil::EVMVersion m_evmVersion;
	solidity::frontend::RevertStrings m_revertStrings = solidity::frontend::RevertStrings::Default;
	solidity::frontend::OptimiserSettings m_optimiserSettings = solidity::frontend::OptimiserSettings::minimal();
	bool m_showMessages = false;
	std::shared_ptr<FakeEvmHost> m_evmHost;

	bool m_transactionSuccessful = true;
	Address m_sender = account(0);
	Address m_contractAddress;
	u256 const m_gasPrice = 100 * szabo;
	u256 const m_gas = 100000000;
	u256 m_gasUsed;
	bytes m_output;

	FakeCompilerStack m_compiler;
};

} // namespace solidity::test
