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
			test.call.arguments.rawBytes
		);

		if ((m_transactionSuccessful != test.call.expectations.status) || (output != test.call.expectations.rawBytes))
			success = false;

		string resultOutput;
		if (m_transactionSuccessful)
			resultOutput = "-> " + ExpectationParser::bytesToString(output, test.call.expectations.format);
		else
			resultOutput = "REVERT";

		test.status = m_transactionSuccessful;
		test.rawBytes = std::move(output);
		test.output = std::move(resultOutput);
	}

	if (!success)
	{
		string nextIndentLevel = _linePrefix + "  ";
		FormattedScope(_stream, _formatted, {BOLD, CYAN}) << _linePrefix << "Expected result:" << endl;
		for (auto const& test: m_tests)
		{
			printFunctionCall(_stream, test.call, _linePrefix);
			printFunctionCallTest(_stream, test, true, _linePrefix, _formatted);
		}

		FormattedScope(_stream, _formatted, {BOLD, CYAN}) << _linePrefix << "Obtained result:" << endl;
		for (auto const& test: m_tests)
		{
			printFunctionCall(_stream, test.call, _linePrefix);
			printFunctionCallTest(_stream, test, false, _linePrefix, _formatted);
		}
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
	{
		printFunctionCall(_stream, test.call, _linePrefix);
		printFunctionCallTest(_stream, test, false, _linePrefix);
	}
}


void SemanticTest::parseExpectations(istream& _stream)
{
	ExpectationParser parser{_stream};
	for (auto const& call: parser.parseFunctionCalls())
		m_tests.emplace_back(FunctionCallTest{std::move(call), false, bytes{}, string{}});
}

bool SemanticTest::deploy(string const& _contractName, u256 const& _value, bytes const& _arguments)
{
	auto output = compileAndRunWithoutCheck(m_source, _value, _contractName, _arguments, m_libraryAddresses);
	return !output.empty() && m_transactionSuccessful;
}

void SemanticTest::printFunctionCall(ostream& _stream, FunctionCall const& _call, string const& _linePrefix) const
{
	_stream << _linePrefix << _call.signature;
	if (_call.value > u256(0))
		_stream << "[" << _call.value << "]";
	if (!_call.arguments.raw.empty())
		_stream << ": " << boost::algorithm::trim_copy(_call.arguments.raw);
	if (!_call.arguments.comment.empty())
		_stream << " # " << _call.arguments.comment;
	_stream << endl;
}

void SemanticTest::printFunctionCallTest(
	ostream& _stream,
	FunctionCallTest const& _test,
	bool _expected,
	string const& _linePrefix,
	bool const _formatted
) const
{
	_stream << _linePrefix;
	if (_formatted && !_test.matchesExpectation())
		_stream << formatting::RED_BACKGROUND;
	string output = _expected ? _test.call.expectations.output : _test.output;
	_stream << boost::algorithm::trim_copy(output);
	if (_formatted && !_test.matchesExpectation())
		_stream << formatting::RESET;
	if (!_test.call.expectations.comment.empty())
		_stream << " # " << _test.call.expectations.comment;
	_stream << endl;
}
