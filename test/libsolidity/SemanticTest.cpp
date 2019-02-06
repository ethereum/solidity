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

#include <test/libsolidity/SemanticTest.h>
#include <test/Options.h>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/throw_exception.hpp>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <memory>
#include <stdexcept>

using namespace dev;
using namespace solidity;
using namespace dev::solidity::test;
using namespace dev::solidity::test::formatting;
using namespace std;
namespace fs = boost::filesystem;
using namespace boost;
using namespace boost::algorithm;
using namespace boost::unit_test;

namespace
{
	using ParamList = dev::solidity::test::ParameterList;
	using FunctionCallTest = dev::solidity::test::SemanticTest::FunctionCallTest;
	using FunctionCall = dev::solidity::test::FunctionCall;

	string formatBytes(bytes const& _bytes, ParamList const& _params, bool const _formatInvalid = false)
	{
		stringstream resultStream;
		if (_bytes.empty())
			resultStream.str();
		auto it = _bytes.begin();
		for (auto const& param: _params)
		{
			bytes byteRange{it, it + param.abiType.size};
			// FIXME Check range
			// TODO Check range
			switch (param.abiType.type)
			{
			case ABIType::SignedDec:
				if (*byteRange.begin() & 0x80)
					resultStream << u2s(fromBigEndian<u256>(byteRange));
				else
					resultStream << fromBigEndian<u256>(byteRange);
				break;
			case ABIType::UnsignedDec:
				// Check if the detected type was wrong and if this could
				// be signed. If an unsigned was detected in the expectations,
				// but the actual result returned a signed, it would be formatted
				// incorrectly.
				if (*byteRange.begin() & 0x80)
					resultStream << u2s(fromBigEndian<u256>(byteRange));
				else
					resultStream << fromBigEndian<u256>(byteRange);
				break;
			case ABIType::Failure:
				// If expectations are empty, the encoding type is invalid.
				// In order to still print the actual result even if
				// empty expectations were detected, it must be forced.
				if (_formatInvalid)
					resultStream << fromBigEndian<u256>(byteRange);
				break;
			case ABIType::None:
				// If expectations are empty, the encoding type is NONE.
				if (_formatInvalid)
					resultStream << fromBigEndian<u256>(byteRange);
				break;
			}
			it += param.abiType.size;
			if (it != _bytes.end() && !(param.abiType.type == ABIType::None))
				resultStream << ", ";
		}
		return resultStream.str();
	}

	string formatFunctionCallTest(
		FunctionCallTest const& _test,
		string const& _linePrefix = "",
		bool const _renderResult = false,
		bool const _higlight = false
	)
	{
		stringstream _stream;
		FunctionCall call = _test.call;
		bool hightlight = !_test.matchesExpectation() && _higlight;

		auto formatOutput = [&](bool const _singleLine)
		{
			_stream << _linePrefix << "// " << call.signature;
			if (call.value > u256(0))
				_stream << TestFileParser::formatToken(SoltToken::Comma)
						<< call.value << " "
						<< TestFileParser::formatToken(SoltToken::Ether);
			if (!call.arguments.rawBytes().empty())
				_stream << ": "
						<< formatBytes(call.arguments.rawBytes(), call.arguments.parameters);
			if (!_singleLine)
				_stream << endl << _linePrefix << "// ";
			if (_singleLine)
				_stream << " ";
			_stream << "-> ";
			if (!_singleLine)
				_stream << endl << _linePrefix << "// ";
			if (hightlight)
				_stream << formatting::RED_BACKGROUND;
			bytes output;
			if (_renderResult)
				output = call.expectations.rawBytes();
			else
				output = _test.rawBytes;
			if (!output.empty())
				_stream << formatBytes(output, call.expectations.result);
			if (hightlight)
				_stream << formatting::RESET;
		};

		if (call.displayMode == FunctionCall::DisplayMode::SingleLine)
			formatOutput(true);
		else
			formatOutput(false);
		_stream << endl;

		return _stream.str();
	}
}

SemanticTest::SemanticTest(string const& _filename, string const& _ipcPath):
	SolidityExecutionFramework(_ipcPath)
{
	ifstream file(_filename);
	if (!file)
		BOOST_THROW_EXCEPTION(runtime_error("Cannot open test contract: \"" + _filename + "\"."));
	file.exceptions(ios::badbit);

	m_source = parseSource(file);
	parseExpectations(file);
}

bool SemanticTest::run(ostream& _stream, string const& _linePrefix, bool const _formatted)
{
	if (!deploy("", 0, bytes()))
		BOOST_THROW_EXCEPTION(runtime_error("Failed to deploy contract."));

	bool success = true;
	for (auto& test: m_tests)
		test.reset();

	for (auto& test: m_tests)
	{
		bytes output = callContractFunctionWithValueNoEncoding(
			test.call.signature,
			test.call.value,
			test.call.arguments.rawBytes()
		);

		if ((m_transactionSuccessful == test.call.expectations.failure) || (output != test.call.expectations.rawBytes()))
			success = false;

		test.failure = !m_transactionSuccessful;
		test.rawBytes = std::move(output);
	}

	if (!success)
	{
		FormattedScope(_stream, _formatted, {BOLD, CYAN}) << _linePrefix << "Expected result:" << endl;
		for (auto const& test: m_tests)
			_stream << formatFunctionCallTest(test, _linePrefix, false, true);

		FormattedScope(_stream, _formatted, {BOLD, CYAN}) << _linePrefix << "Obtained result:" << endl;
		for (auto const& test: m_tests)
			_stream << formatFunctionCallTest(test, _linePrefix, true, true);

		FormattedScope(_stream, _formatted, {BOLD, RED}) << _linePrefix
			<< "Attention: Updates on the test will apply the detected format displayed." << endl;
		return false;
	}
	return true;
}

void SemanticTest::printSource(ostream& _stream, string const& _linePrefix, bool const) const
{
	stringstream stream(m_source);
	string line;
	while (getline(stream, line))
		_stream << _linePrefix << line << endl;
}

void SemanticTest::printUpdatedExpectations(ostream& _stream, string const& _linePrefix) const
{
	for (auto const& test: m_tests)
		_stream << formatFunctionCallTest(test, _linePrefix, false, false);
}

void SemanticTest::parseExpectations(istream& _stream)
{
	TestFileParser parser{_stream};
	for (auto const& call: parser.parseFunctionCalls())
		m_tests.emplace_back(FunctionCallTest{call, bytes{}, string{}});
}

bool SemanticTest::deploy(string const& _contractName, u256 const& _value, bytes const& _arguments)
{
	auto output = compileAndRunWithoutCheck(m_source, _value, _contractName, _arguments);
	return !output.empty() && m_transactionSuccessful;
}
