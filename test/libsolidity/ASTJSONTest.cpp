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

#include <liblangutil/SourceReferenceFormatter.h>
#include <libsolidity/ast/ASTJsonConverter.h>
#include <libsolutil/AnsiColorized.h>
#include <libsolutil/CommonIO.h>

#include <test/Common.h>
#include <test/libsolidity/ASTJSONTest.h>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/throw_exception.hpp>

#include <fstream>
#include <memory>
#include <stdexcept>

using namespace solidity::langutil;
using namespace solidity::frontend;
using namespace solidity::frontend::test;
using namespace solidity::util::formatting;
using namespace solidity::util;
using namespace solidity;
using namespace std;
namespace fs = boost::filesystem;
using namespace boost::unit_test;

namespace
{

string const sourceDelimiter("==== Source: ");

void replaceVersionWithTag(string& _input)
{
	boost::algorithm::replace_all(
		_input,
		"\"" + solidity::test::CommonOptions::get().evmVersion().name() + "\"",
		"%EVMVERSION%"
	);
}

void replaceTagWithVersion(string& _input)
{
	boost::algorithm::replace_all(
		_input,
		"%EVMVERSION%",
		"\"" + solidity::test::CommonOptions::get().evmVersion().name() + "\""
	);
}

}


ASTJSONTest::ASTJSONTest(string const& _filename)
{
	if (!boost::algorithm::ends_with(_filename, ".sol"))
		BOOST_THROW_EXCEPTION(runtime_error("Invalid test contract file name: \"" + _filename + "\"."));

	string_view baseName = _filename;
	baseName.remove_suffix(4);

	m_variants = {
		TestVariant(baseName, CompilerStack::State::Parsed),
		TestVariant(baseName, CompilerStack::State::AnalysisPerformed),
	};

	ifstream file(_filename);
	if (!file)
		BOOST_THROW_EXCEPTION(runtime_error("Cannot open test contract: \"" + _filename + "\"."));
	file.exceptions(ios::badbit);

	string sourceName;
	string source;
	string line;
	string const delimiter("// ----");
	while (getline(file, line))
	{
		if (boost::algorithm::starts_with(line, sourceDelimiter))
		{
			if (!sourceName.empty())
				m_sources.emplace_back(sourceName, source);

			sourceName = line.substr(
				sourceDelimiter.size(),
				line.size() - " ===="s.size() - sourceDelimiter.size()
			);
			source = string();
		}
		else if (!line.empty() && !boost::algorithm::starts_with(line, delimiter))
			source += line + "\n";
	}

	m_sources.emplace_back(sourceName.empty() ? "a" : sourceName, source);
	file.close();

	for (TestVariant& variant: m_variants)
	{
		variant.expectation = readFileAsString(variant.astFilename());
		boost::replace_all(variant.expectation, "\r\n", "\n");
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
		sourceIndices[m_sources[i].first] = static_cast<unsigned>(i + 1);
	}

	bool resultsMatch = true;

	for (TestVariant& variant: m_variants)
	{
		c.reset();
		c.setSources(sources);
		c.setEVMVersion(solidity::test::CommonOptions::get().evmVersion());

		if (!c.parseAndAnalyze(variant.stopAfter))
		{
			// Ignore non-fatal analysis errors, we only want to export.
			if (c.state() > CompilerStack::State::Parsed)
				continue;

			SourceReferenceFormatter formatter(_stream, c, _formatted, false);
			formatter.printErrorInformation(c.errors());
			return TestResult::FatalError;
		}

		resultsMatch = resultsMatch && runTest(
			variant,
			sourceIndices,
			c,
			_stream,
			_linePrefix,
			_formatted
		);
	}

	return resultsMatch ? TestResult::Success : TestResult::Failure;
}

bool ASTJSONTest::runTest(
	TestVariant& _variant,
	map<string, unsigned> const& _sourceIndices,
	CompilerStack& _compiler,
	ostream& _stream,
	string const& _linePrefix,
	bool const _formatted
)
{
	if (m_sources.size() > 1)
		_variant.result += "[\n";

	for (size_t i = 0; i < m_sources.size(); i++)
	{
		ostringstream result;
		ASTJsonConverter(_compiler.state(), _sourceIndices).print(result, _compiler.ast(m_sources[i].first));
		_variant.result += result.str();
		if (i != m_sources.size() - 1)
			_variant.result += ",";
		_variant.result += "\n";
	}

	if (m_sources.size() > 1)
		_variant.result += "]\n";

	replaceTagWithVersion(_variant.expectation);

	if (_variant.expectation != _variant.result)
	{
		string nextIndentLevel = _linePrefix + "  ";
		AnsiColorized(_stream, _formatted, {BOLD, CYAN}) <<
			_linePrefix <<
			"Expected result" <<
			(!_variant.name().empty() ? " (" + _variant.name() + "):" : ":") <<
			endl;
		{
			istringstream stream(_variant.expectation);
			string line;
			while (getline(stream, line))
				_stream << nextIndentLevel << line << endl;
		}
		_stream << endl;

		AnsiColorized(_stream, _formatted, {BOLD, CYAN}) <<
			_linePrefix <<
			"Obtained result" <<
			(!_variant.name().empty() ? " (" + _variant.name() + "):" : ":") <<
			endl;
		{
			istringstream stream(_variant.result);
			string line;
			while (getline(stream, line))
				_stream << nextIndentLevel << line << endl;
		}
		_stream << endl;
		return false;
	}

	return true;
}

void ASTJSONTest::printSource(ostream& _stream, string const& _linePrefix, bool const) const
{
	for (auto const& source: m_sources)
	{
		if (m_sources.size() > 1 || source.first != "a")
			_stream << _linePrefix << sourceDelimiter << source.first << " ====" << endl << endl;
		stringstream stream(source.second);
		string line;
		while (getline(stream, line))
			_stream << _linePrefix << line << endl;
		_stream << endl;
	}
}

void ASTJSONTest::printUpdatedExpectations(std::ostream&, std::string const&) const
{
	for (TestVariant const& variant: m_variants)
		updateExpectation(
			variant.astFilename(),
			variant.result,
			variant.name().empty() ? "" : variant.name() + " "
		);
}

void ASTJSONTest::updateExpectation(string const& _filename, string const& _expectation, string const& _variant) const
{
	ofstream file(_filename.c_str());
	if (!file) BOOST_THROW_EXCEPTION(runtime_error("Cannot write " + _variant + "AST expectation to \"" + _filename + "\"."));
	file.exceptions(ios::badbit);

	string replacedResult = _expectation;
	replaceVersionWithTag(replacedResult);

	file << replacedResult;
	file.flush();
	file.close();
}
