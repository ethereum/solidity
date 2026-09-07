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

#include "TestCase.h"
#include "libsolidity/AnalysisFramework.h"
#include "libsolidity/interface/CompilerStack.h"
#include <test/libsolidity/SyntaxTest.h>

#include <test/libsolidity/util/Common.h>
#include <test/Common.h>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/throw_exception.hpp>
#include <range/v3/algorithm/find_if.hpp>
#include <fstream>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>

using namespace solidity;
using namespace solidity::util;
using namespace solidity::util::formatting;
using namespace solidity::langutil;
using namespace solidity::frontend;
using namespace solidity::frontend::test;
using namespace boost::unit_test;
namespace fs = boost::filesystem;

SyntaxTestSettings SyntaxTestSettings::fromReader(TestCaseReader& _reader)
{
	SyntaxTestSettings settings;

	settings.compileViaYul = _reader.enumSetting<CompileViaYul>(
		"compileViaYul",
		{
			{"true", CompileViaYul::True},
			{"false", CompileViaYul::False},
			{"also", CompileViaYul::Also}
		},
		"also"
	);
	settings.optimizeYul = _reader.boolSetting("optimize-yul", true);
	settings.compileViaSSACFG = _reader.boolSetting("compileViaSSACFG", true);
	settings.experimental = _reader.boolSetting("experimental", settings.compileViaSSACFG);
	settings.stopAfter = _reader.enumSetting<PipelineStage>(
		"stopAfter",
		{
			{"parsing", PipelineStage::Parsing},
			{"analysis", PipelineStage::Analysis},
			{"compilation", PipelineStage::Compilation}
		},
		"compilation"
	);

	return settings;
}

void SyntaxTest::setupCompiler(CompilerStack& _compiler)
{
	AnalysisFramework::setupCompiler(_compiler);

	_compiler.setEVMVersion(m_compilerInput.evmVersion);
	_compiler.setOptimiserSettings(m_compilerInput.optimiserSettings);
	_compiler.setViaIR(m_compilerInput.viaIR);
	_compiler.setViaSSACFG(m_compilerInput.viaSSACFG);
	_compiler.setExperimental(m_compilerInput.experimental);
	_compiler.setMetadataFormat(m_compilerInput.metadataFormat);
	_compiler.setMetadataHash(m_compilerInput.metadataHash);
}

void SyntaxTest::parseAndAnalyze()
{
	m_errorList.clear();

	m_compilerInput.experimental = m_settings.experimental;
	m_compilerInput.optimiserSettings = m_settings.optimizeYul ?
		OptimiserSettings::full() :
		OptimiserSettings::minimal();
	m_compilerInput.metadataFormat = CompilerStack::MetadataFormat::NoMetadata;
	m_compilerInput.metadataHash = CompilerStack::MetadataHash::None;

	runFramework(withPreamble(m_compilerInput.sources), m_settings.stopAfter);
	if (stageSuccessful(PipelineStage::Analysis) && !pipelineSuccessful())
		reportUnexpectedErrors();
	filterObtainedErrors();
}

TestCase::TestResult SyntaxTest::run(
	std::ostream& _stream,
	std::string const& _linePrefix,
	bool _formatted
)
{
	bool compileLegacy =
		m_settings.compileViaYul == CompileViaYul::False ||
		m_settings.compileViaYul == CompileViaYul::Also;
	bool compileViaYul =
		m_settings.compileViaYul == CompileViaYul::True ||
		m_settings.compileViaYul == CompileViaYul::Also;
	bool compileViaSSACFG = m_settings.compileViaSSACFG;

	parseCustomExpectations(m_reader.stream());

	auto result = TestResult::Success;
	if (compileLegacy)
	{
		m_compilerInput.viaIR = false;
		m_compilerInput.viaSSACFG = false;
		parseAndAnalyze();
		result = conclude(_stream, _linePrefix, _formatted);
	}
	if (compileViaYul && result == TestResult::Success)
	{
		m_compilerInput.viaIR = true;
		m_compilerInput.viaSSACFG = false;
		parseAndAnalyze();
		result = conclude(_stream, _linePrefix, _formatted);
	}
	if (compileViaYul && compileViaSSACFG && result == TestResult::Success)
	{
		m_compilerInput.viaIR = true;
		m_compilerInput.viaSSACFG = true;
		parseAndAnalyze();
		result = conclude(_stream, _linePrefix, _formatted);
	}

	if (result != TestResult::Success)
	{
		solidity::test::CommonOptions::get().printSelectedOptions(
			_stream,
			_linePrefix,
			{"evmVersion", "optimize", "useABIEncoderV1", "batch"}
		);
	}

	return result;
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
			if(m_compilerInput.sources.count(sourceName) == 1)
			{
				int preambleSize =
						static_cast<int>(compiler().charStream(sourceName).size()) -
						static_cast<int>(m_compilerInput.sources[sourceName].size());
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

void SyntaxTest::reportUnexpectedErrors()
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

