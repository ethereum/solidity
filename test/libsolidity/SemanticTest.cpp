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

SemanticTest::SemanticTest(string const& _filename):
	SolidityExecutionFramework(ipcPath)
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
	FormattedScope(_stream, _formatted, {BOLD, CYAN}) << _linePrefix << "Expected result:" << endl;

	if (!deploy("", 0, bytes()))
		BOOST_THROW_EXCEPTION(runtime_error("Failed to deploy contract."));

	bool success = true;
	m_results.clear();

	for (auto const& test: m_calls)
	{
		bytes output = callContractFunctionWithValueNoEncoding(
			test.signature,
			test.etherValue,
			test.argumentBytes
		);

		if ((m_transactionSuccessful != test.expectedStatus) || (output != test.expectedBytes))
			success = false;

		m_results.emplace_back(m_transactionSuccessful, std::move(output));
	}

	if (!success)
	{
		string nextIndentLevel = _linePrefix + "  ";
		FormattedScope(_stream, _formatted, {BOLD, CYAN}) << _linePrefix << "Expected result:" << endl;
		printCalls(false, _stream, nextIndentLevel, _formatted);
		FormattedScope(_stream, _formatted, {BOLD, CYAN}) << _linePrefix << "Obtained result:" << endl;
		printCalls(true, _stream, nextIndentLevel, _formatted);
		return false;
	}

	return true;
}

void SemanticTest::printSource(ostream& _stream, string const& _linePrefix, bool const _formatted) const
{
	if (_formatted)
		_stream << _linePrefix;
}

void SemanticTest::printUpdatedExpectations(ostream& _stream, string const& _linePrefix) const
{
	_stream << _linePrefix;
}


void SemanticTest::parseExpectations(istream& _stream)
{
	string line;
	bool isPreamble = true;
	while (getline(_stream, line))
	{
		auto it = line.begin();

		skipSlashes(it, line.end());
		skipWhitespace(it, line.end());

		if (it == line.end())
			continue;


		if (isPreamble)
		{

		}

		FunctionCall call;

		auto signatureBegin = it;
		while (it != line.end() && *it != ')')
			++it;
		expect(it, line.end(), ')');

		call.signature = string(signatureBegin, it);

		m_calls.emplace_back(std::move(call));
	}
}

bool SemanticTest::deploy(string const& _contractName, u256 const& _value, bytes const& _arguments)
{
	auto output = compileAndRunWithoutCheck(m_source, _value, _contractName, _arguments, m_libraryAddresses);
	return !output.empty() && m_transactionSuccessful;
}

void SemanticTest::printCalls(
	bool _actualResults,
	ostream& _stream,
	string const& _linePrefix,
	bool const _formatted
) const
{
	solAssert(m_calls.size() == m_results.size(), "");
	for (size_t i = 0; i < m_calls.size(); i++)
	{
		auto const& call = m_calls[i];
		_stream << _linePrefix << call.signature;
		if (call.etherValue > u256(0))
			_stream << "[" << call.etherValue << "]";
		if (!call.arguments.empty())
			_stream << ": " << boost::algorithm::trim_copy(call.arguments);
		if (!call.argumentComment.empty())
			_stream << " # " << call.argumentComment;
		_stream << endl;

		if(_actualResults && _formatted)
			_stream << endl;

//		string result;
//		std::vector<ByteRangeFormat> formatList;
//		auto expectedBytes = stringToBytes(call.expectedResult, &formatList);
//		if (_actualResults)
//		{
//			if (m_results[i].first)
//				result = "-> " + bytesToString(m_results[i].second, formatList);
//			else
//				result = "REVERT";
//		}
//		else
//		{
//			if (call.expectedStatus)
//				result = "-> " + call.expectedResult;
//			else
//				result = "REVERT";
//		}

//		bool expectationsMatch = (m_results[i].first == call.expectedStatus) && (m_results[i].second == expectedBytes);

//		_stream << _linePrefix;
//		if (_formatted && !expectationsMatch)
//			_stream << formatting::RED_BACKGROUND;
//		_stream << boost::algorithm::trim_copy(result);
//		if (_formatted && !expectationsMatch)
//			_stream << formatting::RESET;
//		if (!call.resultComment.empty())
//			_stream << " # " << call.resultComment;
//		_stream << endl;
	}
}


string SemanticTest::ipcPath;
