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

#include <test/libsolidity/SyntaxTest.h>

#include <test/Common.h>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/throw_exception.hpp>
#include <fstream>
#include <memory>
#include <stdexcept>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::util::formatting;
using namespace solidity::langutil;
using namespace solidity::frontend;
using namespace solidity::frontend::test;
using namespace boost::unit_test;
namespace fs = boost::filesystem;

SyntaxTest::SyntaxTest(string const& _filename, langutil::EVMVersion _evmVersion, bool _parserErrorRecovery): CommonSyntaxTest(_filename, _evmVersion)
{
	m_optimiseYul = m_reader.boolSetting("optimize-yul", true);
	m_parserErrorRecovery = _parserErrorRecovery;
}

TestCase::TestResult SyntaxTest::run(ostream& _stream, string const& _linePrefix, bool _formatted)
{
	setupCompiler();
	parseAndAnalyze();
	filterObtainedErrors();

	return conclude(_stream, _linePrefix, _formatted);
}

string SyntaxTest::addPreamble(string const& _sourceCode)
{
	// Silence compiler version warning
	string preamble = "pragma solidity >=0.0;\n";
	// NOTE: this check is intentionally loose to match weird cases.
	// We can manually adjust a test case where this causes problem.
	if (_sourceCode.find("SPDX-License-Identifier:") == string::npos)
		preamble += "// SPDX-License-Identifier: GPL-3.0\n";
	return preamble + _sourceCode;
}

void SyntaxTest::setupCompiler()
{
	compiler().reset();
	auto sourcesWithPragma = m_sources.sources;
	for (auto& source: sourcesWithPragma)
		source.second = addPreamble(source.second);
	compiler().setSources(sourcesWithPragma);
	compiler().setEVMVersion(m_evmVersion);
	compiler().setParserErrorRecovery(m_parserErrorRecovery);
	compiler().setOptimiserSettings(
		m_optimiseYul ?
		OptimiserSettings::full() :
		OptimiserSettings::minimal()
	);
	compiler().setMetadataFormat(CompilerStack::MetadataFormat::NoMetadata);
	compiler().setMetadataHash(CompilerStack::MetadataHash::None);
}

void SyntaxTest::parseAndAnalyze()
{
	if (compiler().parse() && compiler().analyze())
		try
		{
			if (!compiler().compile())
			{
				ErrorList const& errors = compiler().errors();
				auto codeGeneretionErrorCount = count_if(errors.cbegin(), errors.cend(), [](auto const& error) {
					return error->type() == Error::Type::CodeGenerationError;
				});
				auto errorCount = count_if(errors.cbegin(), errors.cend(), [](auto const& error) {
					return Error::isError(error->type());
				});
				// failing compilation after successful analysis is a rare case,
				// it assumes that errors contain exactly one error, and the error is of type Error::Type::CodeGenerationError
				if (codeGeneretionErrorCount != 1 || errorCount != 1)
					BOOST_THROW_EXCEPTION(runtime_error("Compilation failed even though analysis was successful."));
			}
		}
		catch (UnimplementedFeatureError const& _e)
		{
			m_errorList.emplace_back(SyntaxTestError{
				"UnimplementedFeatureError",
				nullopt,
				errorMessage(_e),
				"",
				-1,
				-1
			});
		}
}

void SyntaxTest::filterObtainedErrors()
{
	for (auto const& currentError: filterErrors(compiler().errors(), true))
	{
		int locationStart = -1;
		int locationEnd = -1;
		string sourceName;
		if (SourceLocation const* location = currentError->sourceLocation())
		{
			solAssert(location->sourceName, "");
			sourceName = *location->sourceName;
			solAssert(m_sources.sources.count(sourceName) == 1, "");

			int preambleSize =
				static_cast<int>(compiler().charStream(sourceName).size()) -
				static_cast<int>(m_sources.sources[sourceName].size());
			solAssert(preambleSize >= 0, "");

			// ignore the version & license pragma inserted by the testing tool when calculating locations.
			if (location->start != -1)
			{
				solAssert(location->start >= preambleSize, "");
				locationStart = location->start - preambleSize;
			}
			if (location->end != -1)
			{
				solAssert(location->end >= preambleSize, "");
				locationEnd = location->end - preambleSize;
			}
		}
		m_errorList.emplace_back(SyntaxTestError{
			Error::formatErrorType(currentError->type()),
			currentError->errorId(),
			errorMessage(*currentError),
			sourceName,
			locationStart,
			locationEnd
		});
	}
}

