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
 * Unit tests for the solidity compiler ABI JSON Interface output.
 */

#include <test/libsolidity/ABIJsonTest.h>

#include <test/Options.h>

#include <libsolidity/interface/CompilerStack.h>
#include <libdevcore/JSON.h>
#include <libdevcore/AnsiColorized.h>

#include <boost/algorithm/string.hpp>

#include <fstream>

using namespace std;
using namespace dev;
using namespace dev::solidity;
using namespace dev::solidity::test;

ABIJsonTest::ABIJsonTest(string const& _filename)
{
	ifstream file(_filename);
	if (!file)
		BOOST_THROW_EXCEPTION(runtime_error("Cannot open test contract: \"" + _filename + "\"."));
	file.exceptions(ios::badbit);

	m_source = parseSourceAndSettings(file);
	m_expectation = parseSimpleExpectations(file);
}

TestCase::TestResult ABIJsonTest::run(ostream& _stream, string const& _linePrefix, bool _formatted)
{
	CompilerStack compiler;

	compiler.setSources({{"", "pragma solidity >=0.0;\n" + m_source}});
	compiler.setEVMVersion(dev::test::Options::get().evmVersion());
	compiler.setOptimiserSettings(dev::test::Options::get().optimize);
	if (!compiler.parseAndAnalyze())
		BOOST_THROW_EXCEPTION(runtime_error("Parsing contract failed"));

	m_obtainedResult.clear();
	bool first = true;
	for (string const& contractName: compiler.contractNames())
	{
		if (!first)
			m_obtainedResult += "\n\n";
		m_obtainedResult += "    " + contractName + "\n";
		m_obtainedResult += jsonPrettyPrint(compiler.contractABI(contractName)) + "\n";
		first = false;
	}
	if (m_expectation != m_obtainedResult)
	{
		string nextIndentLevel = _linePrefix + "  ";
		AnsiColorized(_stream, _formatted, {formatting::BOLD, formatting::CYAN}) << _linePrefix << "Expected result:" << endl;
		printIndented(_stream, m_expectation, nextIndentLevel);
		AnsiColorized(_stream, _formatted, {formatting::BOLD, formatting::CYAN}) << _linePrefix << "Obtained result:" << endl;
		printIndented(_stream, m_obtainedResult, nextIndentLevel);
		return TestResult::Failure;
	}
	return TestResult::Success;
}


void ABIJsonTest::printSource(ostream& _stream, string const& _linePrefix, bool const) const
{
	printIndented(_stream, m_source, _linePrefix);
}

void ABIJsonTest::printUpdatedExpectations(ostream& _stream, string const& _linePrefix) const
{
	printIndented(_stream, m_obtainedResult, _linePrefix);
}

void ABIJsonTest::printIndented(ostream& _stream, string const& _output, string const& _linePrefix) const
{
	stringstream output(_output);
	string line;
	while (getline(output, line))
		if (line.empty())
			// Avoid trailing spaces.
			_stream << boost::trim_right_copy(_linePrefix) << endl;
		else
			_stream << _linePrefix << line << endl;
}

