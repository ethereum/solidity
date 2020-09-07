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

#include <test/libyul/YulOptimizerTest.h>

#include <test/libsolidity/util/SoltestErrors.h>
#include <test/libyul/Common.h>
#include <test/Common.h>

#include <libyul/optimiser/BlockFlattener.h>
#include <libyul/optimiser/VarDeclInitializer.h>
#include <libyul/optimiser/VarNameCleaner.h>
#include <libyul/optimiser/ControlFlowSimplifier.h>
#include <libyul/optimiser/DeadCodeEliminator.h>
#include <libyul/optimiser/Disambiguator.h>
#include <libyul/optimiser/CallGraphGenerator.h>
#include <libyul/optimiser/CircularReferencesPruner.h>
#include <libyul/optimiser/ConditionalUnsimplifier.h>
#include <libyul/optimiser/ConditionalSimplifier.h>
#include <libyul/optimiser/CommonSubexpressionEliminator.h>
#include <libyul/optimiser/NameCollector.h>
#include <libyul/optimiser/EquivalentFunctionCombiner.h>
#include <libyul/optimiser/ExpressionSplitter.h>
#include <libyul/optimiser/FunctionGrouper.h>
#include <libyul/optimiser/FunctionHoister.h>
#include <libyul/optimiser/ExpressionInliner.h>
#include <libyul/optimiser/FullInliner.h>
#include <libyul/optimiser/ForLoopConditionIntoBody.h>
#include <libyul/optimiser/ForLoopConditionOutOfBody.h>
#include <libyul/optimiser/ForLoopInitRewriter.h>
#include <libyul/optimiser/LoadResolver.h>
#include <libyul/optimiser/LoopInvariantCodeMotion.h>
#include <libyul/optimiser/MainFunction.h>
#include <libyul/optimiser/NameDisplacer.h>
#include <libyul/optimiser/Rematerialiser.h>
#include <libyul/optimiser/ExpressionSimplifier.h>
#include <libyul/optimiser/UnusedFunctionParameterPruner.h>
#include <libyul/optimiser/UnusedPruner.h>
#include <libyul/optimiser/ExpressionJoiner.h>
#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/optimiser/SSAReverser.h>
#include <libyul/optimiser/SSATransform.h>
#include <libyul/optimiser/Semantics.h>
#include <libyul/optimiser/RedundantAssignEliminator.h>
#include <libyul/optimiser/StructuralSimplifier.h>
#include <libyul/optimiser/StackCompressor.h>
#include <libyul/optimiser/Suite.h>
#include <libyul/backends/evm/ConstantOptimiser.h>
#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/backends/evm/EVMMetrics.h>
#include <libyul/backends/wasm/WordSizeTransform.h>
#include <libyul/backends/wasm/WasmDialect.h>
#include <libyul/AsmPrinter.h>
#include <libyul/AsmParser.h>
#include <libyul/AsmAnalysis.h>
#include <libyul/AssemblyStack.h>
#include <liblangutil/SourceReferenceFormatter.h>

#include <liblangutil/ErrorReporter.h>
#include <liblangutil/Scanner.h>

#include <libsolutil/AnsiColorized.h>

#include <libsolidity/interface/OptimiserSettings.h>

#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string.hpp>

#include <fstream>
#include <variant>

using namespace solidity;
using namespace solidity::util;
using namespace solidity::langutil;
using namespace solidity::yul;
using namespace solidity::yul::test;
using namespace solidity::frontend;
using namespace solidity::frontend::test;
using namespace std;

YulOptimizerTest::YulOptimizerTest(string const& _filename):
	EVMVersionRestrictedTestCase(_filename)
{
	boost::filesystem::path path(_filename);

	if (path.empty() || std::next(path.begin()) == path.end() || std::next(std::next(path.begin())) == path.end())
		BOOST_THROW_EXCEPTION(runtime_error("Filename path has to contain a directory: \"" + _filename + "\"."));
	m_optimizerStep = std::prev(std::prev(path.end()))->string();

	m_source = m_reader.source();

	auto dialectName = m_reader.stringSetting("dialect", "evm");
	m_dialect = &dialect(dialectName, solidity::test::CommonOptions::get().evmVersion());

	m_expectation = m_reader.simpleExpectations();
}

TestCase::TestResult YulOptimizerTest::run(ostream& _stream, string const& _linePrefix, bool const _formatted)
{
	std::tie(m_object, m_analysisInfo) = parse(_stream, _linePrefix, _formatted, m_source);
	if (!m_object)
		return TestResult::FatalError;

	soltestAssert(m_dialect, "Dialect not set.");

	updateContext();

	if (m_optimizerStep == "disambiguator")
		disambiguate();
	else if (m_optimizerStep == "nameDisplacer")
	{
		disambiguate();
		NameDisplacer{
			*m_nameDispenser,
			{"illegal1"_yulstring, "illegal2"_yulstring, "illegal3"_yulstring, "illegal4"_yulstring, "illegal5"_yulstring}
		}(*m_object->code);
	}
	else if (m_optimizerStep == "blockFlattener")
	{
		disambiguate();
		BlockFlattener::run(*m_context, *m_object->code);
	}
	else if (m_optimizerStep == "constantOptimiser")
	{
		GasMeter meter(dynamic_cast<EVMDialect const&>(*m_dialect), false, 200);
		ConstantOptimiser{dynamic_cast<EVMDialect const&>(*m_dialect), meter}(*m_object->code);
	}
	else if (m_optimizerStep == "varDeclInitializer")
		VarDeclInitializer::run(*m_context, *m_object->code);
	else if (m_optimizerStep == "varNameCleaner")
	{
		disambiguate();
		FunctionGrouper::run(*m_context, *m_object->code);
		VarNameCleaner::run(*m_context, *m_object->code);
	}
	else if (m_optimizerStep == "forLoopConditionIntoBody")
	{
		disambiguate();
		ForLoopConditionIntoBody::run(*m_context, *m_object->code);
	}
	else if (m_optimizerStep == "forLoopInitRewriter")
	{
		disambiguate();
		ForLoopInitRewriter::run(*m_context, *m_object->code);
	}
	else if (m_optimizerStep == "commonSubexpressionEliminator")
	{
		disambiguate();
		CommonSubexpressionEliminator::run(*m_context, *m_object->code);
	}
	else if (m_optimizerStep == "conditionalUnsimplifier")
	{
		disambiguate();
		ConditionalUnsimplifier::run(*m_context, *m_object->code);
	}
	else if (m_optimizerStep == "conditionalSimplifier")
	{
		disambiguate();
		ConditionalSimplifier::run(*m_context, *m_object->code);
	}
	else if (m_optimizerStep == "expressionSplitter")
		ExpressionSplitter::run(*m_context, *m_object->code);
	else if (m_optimizerStep == "expressionJoiner")
	{
		disambiguate();
		ExpressionJoiner::run(*m_context, *m_object->code);
	}
	else if (m_optimizerStep == "splitJoin")
	{
		disambiguate();
		ExpressionSplitter::run(*m_context, *m_object->code);
		ExpressionJoiner::run(*m_context, *m_object->code);
		ExpressionJoiner::run(*m_context, *m_object->code);
	}
	else if (m_optimizerStep == "functionGrouper")
	{
		disambiguate();
		FunctionGrouper::run(*m_context, *m_object->code);
	}
	else if (m_optimizerStep == "functionHoister")
	{
		disambiguate();
		FunctionHoister::run(*m_context, *m_object->code);
	}
	else if (m_optimizerStep == "expressionInliner")
	{
		disambiguate();
		ExpressionInliner::run(*m_context, *m_object->code);
	}
	else if (m_optimizerStep == "fullInliner")
	{
		disambiguate();
		FunctionHoister::run(*m_context, *m_object->code);
		FunctionGrouper::run(*m_context, *m_object->code);
		ExpressionSplitter::run(*m_context, *m_object->code);
		FullInliner::run(*m_context, *m_object->code);
		ExpressionJoiner::run(*m_context, *m_object->code);
	}
	else if (m_optimizerStep == "mainFunction")
	{
		disambiguate();
		FunctionGrouper::run(*m_context, *m_object->code);
		MainFunction::run(*m_context, *m_object->code);
	}
	else if (m_optimizerStep == "rematerialiser")
	{
		disambiguate();
		Rematerialiser::run(*m_context, *m_object->code);
	}
	else if (m_optimizerStep == "expressionSimplifier")
	{
		disambiguate();
		ExpressionSimplifier::run(*m_context, *m_object->code);
		ExpressionSimplifier::run(*m_context, *m_object->code);
		ExpressionSimplifier::run(*m_context, *m_object->code);
	}
	else if (m_optimizerStep == "fullSimplify")
	{
		disambiguate();
		ExpressionSplitter::run(*m_context, *m_object->code);
		ForLoopInitRewriter::run(*m_context, *m_object->code);
		CommonSubexpressionEliminator::run(*m_context, *m_object->code);
		ExpressionSimplifier::run(*m_context, *m_object->code);
		UnusedPruner::run(*m_context, *m_object->code);
		CircularReferencesPruner::run(*m_context, *m_object->code);
		DeadCodeEliminator::run(*m_context, *m_object->code);
		ExpressionJoiner::run(*m_context, *m_object->code);
		ExpressionJoiner::run(*m_context, *m_object->code);
	}
	else if (m_optimizerStep == "unusedFunctionParameterPruner")
	{
		disambiguate();
		FunctionHoister::run(*m_context, *m_object->code);
		LiteralRematerialiser::run(*m_context, *m_object->code);
		UnusedFunctionParameterPruner::run(*m_context, *m_object->code);
	}
	else if (m_optimizerStep == "unusedPruner")
	{
		disambiguate();
		UnusedPruner::run(*m_context, *m_object->code);
	}
	else if (m_optimizerStep == "circularReferencesPruner")
	{
		disambiguate();
		FunctionHoister::run(*m_context, *m_object->code);
		CircularReferencesPruner::run(*m_context, *m_object->code);
	}
	else if (m_optimizerStep == "deadCodeEliminator")
	{
		disambiguate();
		ForLoopInitRewriter::run(*m_context, *m_object->code);
		DeadCodeEliminator::run(*m_context, *m_object->code);
	}
	else if (m_optimizerStep == "ssaTransform")
	{
		disambiguate();
		ForLoopInitRewriter::run(*m_context, *m_object->code);
		SSATransform::run(*m_context, *m_object->code);
	}
	else if (m_optimizerStep == "redundantAssignEliminator")
	{
		disambiguate();
		RedundantAssignEliminator::run(*m_context, *m_object->code);
	}
	else if (m_optimizerStep == "ssaPlusCleanup")
	{
		disambiguate();
		SSATransform::run(*m_context, *m_object->code);
		RedundantAssignEliminator::run(*m_context, *m_object->code);
	}
	else if (m_optimizerStep == "loadResolver")
	{
		disambiguate();
		ForLoopInitRewriter::run(*m_context, *m_object->code);
		ExpressionSplitter::run(*m_context, *m_object->code);
		CommonSubexpressionEliminator::run(*m_context, *m_object->code);
		ExpressionSimplifier::run(*m_context, *m_object->code);

		LoadResolver::run(*m_context, *m_object->code);

		UnusedPruner::run(*m_context, *m_object->code);
		ExpressionJoiner::run(*m_context, *m_object->code);
		ExpressionJoiner::run(*m_context, *m_object->code);
	}
	else if (m_optimizerStep == "loopInvariantCodeMotion")
	{
		disambiguate();
		ForLoopInitRewriter::run(*m_context, *m_object->code);
		LoopInvariantCodeMotion::run(*m_context, *m_object->code);
	}
	else if (m_optimizerStep == "controlFlowSimplifier")
	{
		disambiguate();
		ControlFlowSimplifier::run(*m_context, *m_object->code);
	}
	else if (m_optimizerStep == "structuralSimplifier")
	{
		disambiguate();
		ForLoopInitRewriter::run(*m_context, *m_object->code);
		LiteralRematerialiser::run(*m_context, *m_object->code);
		StructuralSimplifier::run(*m_context, *m_object->code);
	}
	else if (m_optimizerStep == "equivalentFunctionCombiner")
	{
		disambiguate();
		EquivalentFunctionCombiner::run(*m_context, *m_object->code);
	}
	else if (m_optimizerStep == "ssaReverser")
	{
		disambiguate();
		SSAReverser::run(*m_context, *m_object->code);
	}
	else if (m_optimizerStep == "ssaAndBack")
	{
		disambiguate();
		// apply SSA
		SSATransform::run(*m_context, *m_object->code);
		RedundantAssignEliminator::run(*m_context, *m_object->code);
		// reverse SSA
		SSAReverser::run(*m_context, *m_object->code);
		CommonSubexpressionEliminator::run(*m_context, *m_object->code);
		UnusedPruner::run(*m_context, *m_object->code);
	}
	else if (m_optimizerStep == "stackCompressor")
	{
		disambiguate();
		FunctionGrouper::run(*m_context, *m_object->code);
		size_t maxIterations = 16;
		Object obj;
		obj.code = m_object->code;
		StackCompressor::run(*m_dialect, obj, true, maxIterations);
		m_object->code = obj.code;
		BlockFlattener::run(*m_context, *m_object->code);
	}
	else if (m_optimizerStep == "wordSizeTransform")
	{
		disambiguate();
		ExpressionSplitter::run(*m_context, *m_object->code);
		WordSizeTransform::run(*m_dialect, *m_dialect, *m_object->code, *m_nameDispenser);
	}
	else if (m_optimizerStep == "fullSuite")
	{
		GasMeter meter(dynamic_cast<EVMDialect const&>(*m_dialect), false, 200);
		yul::Object obj;
		obj.code = m_object->code;
		obj.analysisInfo = m_analysisInfo;
		OptimiserSuite::run(*m_dialect, &meter, obj, true, solidity::frontend::OptimiserSettings::DefaultYulOptimiserSteps);
	}
	else
	{
		AnsiColorized(_stream, _formatted, {formatting::BOLD, formatting::RED}) << _linePrefix << "Invalid optimizer step: " << m_optimizerStep << endl;
		return TestResult::FatalError;
	}

	auto const printed = (m_object->subObjects.empty() ? AsmPrinter{ *m_dialect }(*m_object->code) : m_object->toString(m_dialect));

	// Re-parse new code for compilability
	// TODO: support for wordSizeTransform which needs different input and output dialects
	if (m_optimizerStep != "wordSizeTransform" && !std::get<0>(parse(_stream, _linePrefix, _formatted, printed)))
	{
		util::AnsiColorized(_stream, _formatted, {util::formatting::BOLD, util::formatting::CYAN})
			<< _linePrefix << "Result after the optimiser:" << endl;
		printIndented(_stream, printed, _linePrefix + "  ");
		return TestResult::FatalError;
	}

	m_obtainedResult = "step: " + m_optimizerStep + "\n\n" + printed + "\n";

	return checkResult(_stream, _linePrefix, _formatted);
}

std::pair<std::shared_ptr<Object>, std::shared_ptr<AsmAnalysisInfo>> YulOptimizerTest::parse(
	ostream& _stream,
	string const& _linePrefix,
	bool const _formatted,
	string const& _source
)
{
	ErrorList errors;
	soltestAssert(m_dialect, "");
	shared_ptr<Object> object;
	shared_ptr<AsmAnalysisInfo> analysisInfo;
	std::tie(object, analysisInfo) = yul::test::parse(_source, *m_dialect, errors);
	if (!object || !analysisInfo || !Error::containsOnlyWarnings(errors))
	{
		AnsiColorized(_stream, _formatted, {formatting::BOLD, formatting::RED}) << _linePrefix << "Error parsing source." << endl;
		printErrors(_stream, errors);
		return {};
	}
	return {std::move(object), std::move(analysisInfo)};
}

void YulOptimizerTest::disambiguate()
{
	*m_object->code = std::get<Block>(Disambiguator(*m_dialect, *m_analysisInfo)(*m_object->code));
	m_analysisInfo.reset();
	updateContext();
}

void YulOptimizerTest::updateContext()
{
	m_nameDispenser = make_unique<NameDispenser>(*m_dialect, *m_object->code, m_reservedIdentifiers);
	m_context = make_unique<OptimiserStepContext>(OptimiserStepContext{
		*m_dialect,
		*m_nameDispenser,
		m_reservedIdentifiers
	});
}

void YulOptimizerTest::printErrors(ostream& _stream, ErrorList const& _errors)
{
	SourceReferenceFormatter formatter(_stream);

	for (auto const& error: _errors)
		formatter.printErrorInformation(*error);
}
