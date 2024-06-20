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
#include <range/v3/algorithm/find_if.hpp>
#include <fstream>
#include <memory>
#include <stdexcept>

using namespace solidity;
using namespace solidity::util;
using namespace solidity::util::formatting;
using namespace solidity::langutil;
using namespace solidity::frontend;
using namespace solidity::frontend::test;
using namespace boost::unit_test;
namespace fs = boost::filesystem;

SyntaxTest::SyntaxTest(
	std::string const& _filename,
	langutil::EVMVersion _evmVersion,
	Error::Severity _minSeverity
):
	CommonSyntaxTest(_filename, _evmVersion),
	m_minSeverity(_minSeverity)
{
	static std::set<std::string> const compileViaYulAllowedValues{"true", "false"};

	m_compileViaYul = m_reader.stringSetting("compileViaYul", "false");
	if (!util::contains(compileViaYulAllowedValues, m_compileViaYul))
		BOOST_THROW_EXCEPTION(std::runtime_error("Invalid compileViaYul value: " + m_compileViaYul + "."));
	m_optimiseYul = m_reader.boolSetting("optimize-yul", true);

	static std::map<std::string, PipelineStage> const pipelineStages = {
		{"parsing", PipelineStage::Parsing},
		{"analysis", PipelineStage::Analysis},
		{"compilation", PipelineStage::Compilation}
	};
	std::string stopAfter = m_reader.stringSetting("stopAfter", "compilation");
	if (!pipelineStages.count(stopAfter))
		BOOST_THROW_EXCEPTION(std::runtime_error("Invalid stopAfter value: " + stopAfter + "."));
	m_stopAfter = pipelineStages.at(stopAfter);
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
	_compiler.setViaIR(m_compileViaYul == "true");
	_compiler.setMetadataFormat(CompilerStack::MetadataFormat::NoMetadata);
	_compiler.setMetadataHash(CompilerStack::MetadataHash::None);
}

void SyntaxTest::parseAndAnalyze()
{
	runFramework(withPreamble(m_sources.sources), m_stopAfter);
	if (!pipelineSuccessful() && stageSuccessful(PipelineStage::Analysis) && !compiler().isExperimentalAnalysis())
	{
		ErrorList const& errors = compiler().errors();
		static auto isInternalError = [](std::shared_ptr<Error const> const& _error) {
			return
				Error::isError(_error->type()) &&
				_error->type() != Error::Type::CodeGenerationError &&
				_error->type() != Error::Type::UnimplementedFeatureError
			;
		};
		// Most errors are detected during analysis, and should not happen during code generation.
		// There are some exceptions, e.g. unimplemented features or stack too deep, but anything else at this stage
		// is an internal error that signals a bug in the compiler (rather than in user's code).
		if (
			auto error = ranges::find_if(errors, isInternalError);
			error != ranges::end(errors)
		)
			BOOST_THROW_EXCEPTION(std::runtime_error(
				"Unexpected " + Error::formatErrorType((*error)->type()) + " at compilation stage."
				" This error should NOT be encoded as expectation and should be fixed instead."
			));
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
		std::string sourceName;
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
			currentError->type(),
			currentError->errorId(),
			errorMessage(*currentError),
			sourceName,
			locationStart,
			locationEnd
		});
	}
}
