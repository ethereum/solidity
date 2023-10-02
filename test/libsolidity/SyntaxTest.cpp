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

#include <test/libsolidity/util/Common.h>
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

SyntaxTest::SyntaxTest(
	string const& _filename,
	langutil::EVMVersion _evmVersion,
	Error::Severity _minSeverity
):
	CommonSyntaxTest(_filename, _evmVersion),
	m_minSeverity(_minSeverity)
{
	m_optimiseYul = m_reader.boolSetting("optimize-yul", true);
}

void SyntaxTest::setupCompiler(CompilerStack& _compiler)
{
	AnalysisFramework::setupCompiler(_compiler);

	_compiler.setEVMVersion(m_evmVersion);
	_compiler.setOptimiserSettings(
		m_optimiseYul ?
		OptimiserSettings::full() :
		OptimiserSettings::minimal()
	);
	_compiler.setMetadataFormat(CompilerStack::MetadataFormat::NoMetadata);
	_compiler.setMetadataHash(CompilerStack::MetadataHash::None);
}

void SyntaxTest::parseAndAnalyze()
{
	try
	{
		runFramework(withPreamble(m_sources.sources), PipelineStage::Compilation);
		if (!pipelineSuccessful() && stageSuccessful(PipelineStage::Analysis))
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

	filterObtainedErrors();
}

void SyntaxTest::filterObtainedErrors()
{
	for (auto const& currentError: filteredErrors())
	{
		if (currentError->severity() < m_minSeverity)
			continue;

		int locationStart = -1;
		int locationEnd = -1;
		string sourceName;
		if (SourceLocation const* location = currentError->sourceLocation())
		{
			locationStart = location->start;
			locationEnd = location->end;
			solAssert(location->sourceName, "");
			sourceName = *location->sourceName;
			if(m_sources.sources.count(sourceName) == 1)
			{
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
