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
	if (!deploy("", 0, bytes()))
		BOOST_THROW_EXCEPTION(runtime_error("Failed to deploy contract."));

	bool success = true;
	m_results.clear();

	for (auto const& test: m_calls)
	{
		bytes output = callContractFunctionWithValueNoEncoding(
			test.signature,
			test.costs,
			test.arguments.input
		);
		if ((m_transactionSuccessful != test.result.status) || (output != test.result.output))
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
	ExpectationParser parser{_stream};
	m_calls = parser.parseFunctionCalls();
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
		if (call.costs > u256(0))
			_stream << "[" << call.costs << "]";
		if (!call.arguments.raw.empty())
			_stream << ": " << boost::algorithm::trim_copy(call.arguments.raw);
		if (!call.arguments.comment.empty())
			_stream << " # " << call.arguments.comment;
		_stream << endl;

		string result;
		auto expectedBytes = ExpectationParser::stringToBytes(call.result.raw);
		if (_actualResults)
		{
			if (m_results[i].first)
				result = "-> " + ExpectationParser::bytesToString(m_results[i].second);
			else
				result = "REVERT";
		}
		else
		{
			if (call.result.status)
				result = "-> " + call.result.raw;
			else
				result = "REVERT";
		}

		bool expectationsMatch = (m_results[i].first == call.result.status) && (m_results[i].second == expectedBytes);

		_stream << _linePrefix;
		if (_formatted && !expectationsMatch)
			_stream << formatting::RED_BACKGROUND;
		_stream << boost::algorithm::trim_copy(result);
		if (_formatted && !expectationsMatch)
			_stream << formatting::RESET;
		if (!call.result.comment.empty())
			_stream << " # " << call.result.comment;
		_stream << endl;
	}
}

string SemanticTest::ipcPath;
