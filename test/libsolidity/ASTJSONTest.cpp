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
#include <libsolidity/ast/ASTJsonExporter.h>
#include <libsolutil/AnsiColorized.h>
#include <libsolutil/CommonIO.h>
#include <libsolutil/JSON.h>

#include <test/Common.h>
#include <test/libsolidity/ASTJSONTest.h>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/throw_exception.hpp>

#include <fstream>
#include <memory>
#include <stdexcept>

using namespace solidity;
using namespace solidity::langutil;
using namespace solidity::frontend;
using namespace solidity::frontend::test;
using namespace solidity::util::formatting;
using namespace solidity::util;
namespace fs = boost::filesystem;
using namespace boost::unit_test;
using namespace std::string_literals;

namespace
{

std::string const sourceDelimiter("==== Source: ");

std::string compilerStateToString(CompilerStack::State _state)
{
	switch (_state)
	{
		case CompilerStack::State::Empty: return "Empty";
		case CompilerStack::State::SourcesSet: return "SourcesSet";
		case CompilerStack::State::Parsed: return "Parsed";
		case CompilerStack::State::ParsedAndImported: return "ParsedAndImported";
		case CompilerStack::State::AnalysisSuccessful: return "AnalysisSuccessful";
		case CompilerStack::State::CompilationSuccessful: return "CompilationSuccessful";
	}
	soltestAssert(false, "Unexpected value of state parameter");
}

CompilerStack::State stringToCompilerState(const std::string& _state)
{
	for (unsigned int i = CompilerStack::State::Empty; i <= CompilerStack::State::CompilationSuccessful; ++i)
	{
		if (_state == compilerStateToString(CompilerStack::State(i)))
			return CompilerStack::State(i);
	}
	BOOST_THROW_EXCEPTION(std::runtime_error("Unsupported compiler state (" + _state + ") in test contract file"));
}

void replaceVersionWithTag(std::string& _input)
{
	boost::algorithm::replace_all(
		_input,
		"\"" + solidity::test::CommonOptions::get().evmVersion().name() + "\"",
		"%EVMVERSION%"
	);
}

void replaceTagWithVersion(std::string& _input)
{
	boost::algorithm::replace_all(
		_input,
		"%EVMVERSION%",
		"\"" + solidity::test::CommonOptions::get().evmVersion().name() + "\""
	);
}

}

void ASTJSONTest::generateTestVariants(std::string const& _filename)
{
	std::string_view baseName = _filename;
	baseName.remove_suffix(4);

	const std::vector<CompilerStack::State> variantCompileStates = {
		CompilerStack::State::Parsed,
		CompilerStack::State::AnalysisSuccessful,
	};

	for (const auto state: variantCompileStates)
	{
		auto variant = TestVariant(baseName, state);
		if (boost::filesystem::exists(variant.astFilename()))
		{
			variant.expectation = readFileAsString(variant.astFilename());
			boost::replace_all(variant.expectation, "\r\n", "\n");
			m_variants.push_back(variant);
		}
	}
}

void ASTJSONTest::fillSources(std::string const& _filename)
{
	std::ifstream file(_filename);
	if (!file)
		BOOST_THROW_EXCEPTION(std::runtime_error("Cannot open test contract: \"" + _filename + "\"."));
	file.exceptions(std::ios::badbit);

	std::string sourceName;
	std::string source;
	std::string line;
	std::string const delimiter("// ----");
	std::string const failMarker("// failAfter:");
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
			source = std::string();
		}
		else if (boost::algorithm::starts_with(line, failMarker))
		{
			std::string state = line.substr(failMarker.size());
			boost::algorithm::trim(state);
			if (m_expectedFailAfter.has_value())
				BOOST_THROW_EXCEPTION(std::runtime_error("Duplicated \"failAfter\" directive"));
			m_expectedFailAfter = stringToCompilerState(state);

		}
		else if (!line.empty() && !boost::algorithm::starts_with(line, delimiter))
			source += line + "\n";
	}
	m_sources.emplace_back(sourceName.empty() ? "a" : sourceName, source);
	file.close();
}

void ASTJSONTest::validateTestConfiguration() const
{
	if (m_variants.empty())
		BOOST_THROW_EXCEPTION(std::runtime_error("No file with expected result found."));

	if (m_expectedFailAfter.has_value())
	{
		auto unexpectedTestVariant = std::find_if(
			m_variants.begin(), m_variants.end(),
			[failAfter = m_expectedFailAfter](TestVariant v) { return v.stopAfter > failAfter; }
		);

		if (unexpectedTestVariant != m_variants.end())
			BOOST_THROW_EXCEPTION(
				std::runtime_error(
					std::string("Unexpected JSON file: ") + unexpectedTestVariant->astFilename() +
					" in \"failAfter: " +
					compilerStateToString(m_expectedFailAfter.value()) + "\" scenario."
				)
			);
	}
}

ASTJSONTest::ASTJSONTest(std::string const& _filename):
	EVMVersionRestrictedTestCase(_filename)
{
	if (!boost::algorithm::ends_with(_filename, ".sol"))
		BOOST_THROW_EXCEPTION(std::runtime_error("Invalid test contract file name: \"" + _filename + "\"."));

	generateTestVariants(_filename);
	fillSources(_filename);
	validateTestConfiguration();
}

TestCase::TestResult ASTJSONTest::run(std::ostream& _stream, std::string const& _linePrefix, bool const _formatted)
{
	CompilerStack c;

	StringMap sources;
	std::map<std::string, unsigned> sourceIndices;
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
			if (!m_expectedFailAfter.has_value() || m_expectedFailAfter.value() + 1 != c.state())
			{
				SourceReferenceFormatter formatter(_stream, c, _formatted, false);
				formatter.printErrorInformation(c.errors());
				return TestResult::FatalError;
			}
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
	std::map<std::string, unsigned> const& _sourceIndices,
	CompilerStack& _compiler,
	std::ostream& _stream,
	std::string const& _linePrefix,
	bool const _formatted
)
{
	if (m_sources.size() > 1)
		_variant.result += "[\n";

	for (size_t i = 0; i < m_sources.size(); i++)
	{
		std::ostringstream result;
		ASTJsonExporter(_compiler.state(), _sourceIndices).print(result, _compiler.ast(m_sources[i].first), JsonFormat{ JsonFormat::Pretty });
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
		std::string nextIndentLevel = _linePrefix + "  ";
		AnsiColorized(_stream, _formatted, {BOLD, CYAN}) <<
			_linePrefix <<
			"Expected result" <<
			(!_variant.name().empty() ? " (" + _variant.name() + "):" : ":") <<
			std::endl;
		printPrefixed(_stream, _variant.expectation, nextIndentLevel);
		_stream << std::endl;

		AnsiColorized(_stream, _formatted, {BOLD, CYAN}) <<
			_linePrefix <<
			"Obtained result" <<
			(!_variant.name().empty() ? " (" + _variant.name() + "):" : ":") <<
			std::endl;
		printPrefixed(_stream, _variant.result, nextIndentLevel);
		_stream << std::endl;
		return false;
	}

	return true;
}

void ASTJSONTest::printSource(std::ostream& _stream, std::string const& _linePrefix, bool const) const
{
	for (auto const& source: m_sources)
	{
		if (m_sources.size() > 1 || source.first != "a")
			printPrefixed(_stream, sourceDelimiter + source.first + " ====\n", _linePrefix);
		printPrefixed(_stream, source.second, _linePrefix);
		_stream << std::endl;
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

void ASTJSONTest::updateExpectation(std::string const& _filename, std::string const& _expectation, std::string const& _variant) const
{
	std::ofstream file(_filename.c_str());
	if (!file) BOOST_THROW_EXCEPTION(std::runtime_error("Cannot write " + _variant + "AST expectation to \"" + _filename + "\"."));
	file.exceptions(std::ios::badbit);

	std::string replacedResult = _expectation;
	replaceVersionWithTag(replacedResult);

	file << replacedResult;
	file.flush();
	file.close();
}
