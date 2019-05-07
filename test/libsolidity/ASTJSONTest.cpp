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

#include <test/libsolidity/ASTJSONTest.h>
#include <test/Options.h>
#include <libdevcore/AnsiColorized.h>
#include <liblangutil/SourceReferenceFormatterHuman.h>
#include <libsolidity/ast/ASTJsonConverter.h>
#include <libsolidity/interface/CompilerStack.h>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/throw_exception.hpp>
#include <fstream>
#include <memory>
#include <stdexcept>

using namespace langutil;
using namespace dev::solidity;
using namespace dev::solidity::test;
using namespace dev::formatting;
using namespace dev;
using namespace std;
namespace fs = boost::filesystem;
using namespace boost::unit_test;

ASTJSONTest::ASTJSONTest(string const& _filename)
{
	if (!boost::algorithm::ends_with(_filename, ".sol"))
		BOOST_THROW_EXCEPTION(runtime_error("Invalid test contract file name: \"" + _filename + "\"."));

	m_astFilename = _filename.substr(0, _filename.size() - 4) + ".json";
	m_legacyAstFilename = _filename.substr(0, _filename.size() - 4) + "_legacy.json";

	ifstream file(_filename);
	if (!file)
		BOOST_THROW_EXCEPTION(runtime_error("Cannot open test contract: \"" + _filename + "\"."));
	file.exceptions(ios::badbit);

	string sourceName;
	string source;
	string line;
	string const sourceDelimiter("// ---- SOURCE: ");
	string const delimiter("// ----");
	while (getline(file, line))
	{
		if (boost::algorithm::starts_with(line, sourceDelimiter))
		{
			if (!sourceName.empty())
				m_sources.emplace_back(sourceName, source);

			sourceName = line.substr(sourceDelimiter.size(), string::npos);
			source = string();
		}
		else if (!line.empty() && !boost::algorithm::starts_with(line, delimiter))
			source += line + "\n";
	}

	m_sources.emplace_back(sourceName.empty() ? "a" : sourceName, source);

	file.close();
	file.open(m_astFilename);
	if (file)
	{
		string line;
		while (getline(file, line))
			m_expectation += line + "\n";
	}

	file.close();
	file.open(m_legacyAstFilename);
	if (file)
	{
		string line;
		while (getline(file, line))
			m_expectationLegacy += line + "\n";
	}
}

TestCase::TestResult ASTJSONTest::run(ostream& _stream, string const& _linePrefix, bool const _formatted)
{
	CompilerStack c;

	StringMap sources;
	map<string, unsigned> sourceIndices;
	for (size_t i = 0; i < m_sources.size(); i++)
	{
		sources[m_sources[i].first] = m_sources[i].second;
		sourceIndices[m_sources[i].first] = i + 1;
	}
	c.setSources(sources);
	c.setEVMVersion(dev::test::Options::get().evmVersion());
	if (c.parse())
		c.analyze();
	else
	{
		SourceReferenceFormatterHuman formatter(_stream, _formatted);
		for (auto const& error: c.errors())
			formatter.printErrorInformation(*error);
		return TestResult::FatalError;
	}

	for (size_t i = 0; i < m_sources.size(); i++)
	{
		ostringstream result;
		ASTJsonConverter(false, sourceIndices).print(result, c.ast(m_sources[i].first));
		m_result += result.str();
		if (i != m_sources.size() - 1)
			m_result += ",";
		m_result += "\n";
	}

	bool resultsMatch = true;

	if (m_expectation != m_result)
	{
		string nextIndentLevel = _linePrefix + "  ";
		AnsiColorized(_stream, _formatted, {BOLD, CYAN}) << _linePrefix << "Expected result:" << endl;
		{
			istringstream stream(m_expectation);
			string line;
			while (getline(stream, line))
				_stream << nextIndentLevel << line << endl;
		}
		_stream << endl;
		AnsiColorized(_stream, _formatted, {BOLD, CYAN}) << _linePrefix << "Obtained result:" << endl;
		{
			istringstream stream(m_result);
			string line;
			while (getline(stream, line))
				_stream << nextIndentLevel << line << endl;
		}
		_stream << endl;
		resultsMatch = false;
	}

	for (size_t i = 0; i < m_sources.size(); i++)
	{
		ostringstream result;
		ASTJsonConverter(true, sourceIndices).print(result, c.ast(m_sources[i].first));
		m_resultLegacy = result.str();
		if (i != m_sources.size() - 1)
			m_resultLegacy += ",";
		m_resultLegacy += "\n";
	}

	if (m_expectationLegacy != m_resultLegacy)
	{
		string nextIndentLevel = _linePrefix + "  ";
		AnsiColorized(_stream, _formatted, {BOLD, CYAN}) << _linePrefix << "Expected result (legacy):" << endl;
		{
			istringstream stream(m_expectationLegacy);
			string line;
			while (getline(stream, line))
				_stream << nextIndentLevel << line << endl;
		}
		_stream << endl;
		AnsiColorized(_stream, _formatted, {BOLD, CYAN}) << _linePrefix << "Obtained result (legacy):" << endl;
		{
			istringstream stream(m_resultLegacy);
			string line;
			while (getline(stream, line))
				_stream << nextIndentLevel << line << endl;
		}
		_stream << endl;
		resultsMatch = false;
	}

	return resultsMatch ? TestResult::Success : TestResult::Failure;
}

void ASTJSONTest::printSource(ostream& _stream, string const& _linePrefix, bool const) const
{
	for (auto const& source: m_sources)
	{
		if (m_sources.size() > 1 || source.first != "a")
			_stream << _linePrefix << "// ---- SOURCE: " << source.first << endl << endl;
		stringstream stream(source.second);
		string line;
		while (getline(stream, line))
			_stream << _linePrefix << line << endl;
		_stream << endl;
	}
}

void ASTJSONTest::printUpdatedExpectations(std::ostream&, std::string const&) const
{
	ofstream file(m_astFilename.c_str());
	if (!file) BOOST_THROW_EXCEPTION(runtime_error("Cannot write AST expectation to \"" + m_astFilename + "\"."));
	file.exceptions(ios::badbit);
	file << m_result;
	file.flush();
	file.close();
	file.open(m_legacyAstFilename.c_str());
	if (!file) BOOST_THROW_EXCEPTION(runtime_error("Cannot write legacy AST expectation to \"" + m_legacyAstFilename + "\"."));
	file << m_resultLegacy;
	file.flush();
	file.close();
}
