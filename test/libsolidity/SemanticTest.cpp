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
using namespace dev::formatting;
using namespace std;
using namespace boost;
using namespace boost::algorithm;
using namespace boost::unit_test;
namespace fs = boost::filesystem;

namespace
{
	using FunctionCallTest = SemanticTest::FunctionCallTest;
	using FunctionCall = dev::solidity::test::FunctionCall;
	using ParamList = dev::solidity::test::ParameterList;


	string formatBytes(bytes const& _bytes, ParamList const& _params)
	{
		stringstream resultStream;
		if (_bytes.empty())
			return {};
		auto it = _bytes.begin();
		for (auto const& param: _params)
		{
			long offset = static_cast<long>(param.abiType.size);
			auto offsetIter = it + offset;
			soltestAssert(offsetIter <= _bytes.end(), "Byte range can not be extended past the end of given bytes.");

			bytes byteRange{it, offsetIter};
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
				break;
			case ABIType::None:
				break;
			}
			it += offset;
			if (it != _bytes.end() && !(param.abiType.type == ABIType::None))
				resultStream << ", ";
		}
		soltestAssert(it == _bytes.end(), "Parameter encoding too short for the given byte range.");
		return resultStream.str();
	}

	string formatRawArguments(ParamList const& _params, string const& _linePrefix = "")
	{
		stringstream resultStream;
		for (auto const& param: _params)
		{
			if (param.format.newline)
				resultStream << endl << _linePrefix << "//";
			resultStream << " " << param.rawString;
			if (&param != &_params.back())
				resultStream << ",";
		}
		return resultStream.str();
	}

	string formatFunctionCallTest(
		FunctionCallTest const& _test,
		string const& _linePrefix = "",
		bool const _renderResult = false,
		bool const _highlight = false
	)
	{
		using namespace soltest;
		using Token = soltest::Token;

		stringstream _stream;
		FunctionCall call = _test.call;
		bool highlight = !_test.matchesExpectation() && _highlight;

		auto formatOutput = [&](bool const _singleLine)
		{
			string ws = " ";
			string arrow = formatToken(Token::Arrow);
			string colon = formatToken(Token::Colon);
			string comma = formatToken(Token::Comma);
			string comment = formatToken(Token::Comment);
			string ether = formatToken(Token::Ether);
			string newline = formatToken(Token::Newline);
			string failure = formatToken(Token::Failure);

			/// Prints the function signature. This is the same independent from the display-mode.
			_stream << _linePrefix << newline << ws << call.signature;
			if (call.value > u256(0))
				_stream << comma << ws << call.value << ws << ether;
			if (!call.arguments.rawBytes().empty())
			{
				string output = formatRawArguments(call.arguments.parameters, _linePrefix);
				_stream << colon << output;
			}

			/// Prints comments on the function parameters and the arrow taking
			/// the display-mode into account.
			if (_singleLine)
			{
				if (!call.arguments.comment.empty())
					_stream << ws << comment << call.arguments.comment << comment;
				_stream << ws << arrow << ws;
			}
			else
			{
				_stream << endl << _linePrefix << newline << ws;
				if (!call.arguments.comment.empty())
				{
					 _stream << comment << call.arguments.comment << comment;
					 _stream << endl << _linePrefix << newline << ws;
				}
				_stream << arrow << ws;
			}

			/// Print either the expected output or the actual result output
			string result;
			if (!_renderResult)
			{
				bytes output = call.expectations.rawBytes();
				bool const isFailure = call.expectations.failure;
				result = isFailure ? failure : formatBytes(output, call.expectations.result);
			}
			else
			{
				bytes output = _test.rawBytes;
				bool const isFailure = _test.failure;
				result = isFailure ? failure : formatBytes(output, call.expectations.result);
			}
			AnsiColorized(_stream, highlight, {RED_BACKGROUND}) << result;

			/// Print comments on expectations taking the display-mode into account.
			if (_singleLine)
			{
				if (!call.expectations.comment.empty())
					_stream << ws << comment << call.expectations.comment << comment;
			}
			else
			{
				if (!call.expectations.comment.empty())
				{
					_stream << endl << _linePrefix << newline << ws;
					_stream << comment << call.expectations.comment << comment;
				}
			}
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
	soltestAssert(file, "Cannot open test contract: \"" + _filename + "\".");
	file.exceptions(ios::badbit);

	m_source = parseSource(file);
	parseExpectations(file);
}

bool SemanticTest::run(ostream& _stream, string const& _linePrefix, bool const _formatted)
{
	soltestAssert(deploy("", 0, bytes()), "Failed to deploy contract.");

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
		AnsiColorized(_stream, _formatted, {BOLD, CYAN}) << _linePrefix << "Expected result:" << endl;
		for (auto const& test: m_tests)
			_stream << formatFunctionCallTest(test, _linePrefix, false, true & _formatted);

		AnsiColorized(_stream, _formatted, {BOLD, CYAN}) << _linePrefix << "Obtained result:" << endl;
		for (auto const& test: m_tests)
			_stream << formatFunctionCallTest(test, _linePrefix, true, true & _formatted);

		AnsiColorized(_stream, _formatted, {BOLD, RED}) << _linePrefix
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

void SemanticTest::printUpdatedExpectations(ostream& _stream, string const&) const
{
	for (auto const& test: m_tests)
		_stream << formatFunctionCallTest(test, "", true, false);
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
