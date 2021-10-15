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

#include <test/libyul/YulOptimizerTestCommon.h>

#include <libyul/optimiser/BlockFlattener.h>
#include <libyul/optimiser/VarDeclInitializer.h>
#include <libyul/optimiser/VarNameCleaner.h>
#include <libyul/optimiser/ControlFlowSimplifier.h>
#include <libyul/optimiser/DeadCodeEliminator.h>
#include <libyul/optimiser/Disambiguator.h>
#include <libyul/optimiser/CircularReferencesPruner.h>
#include <libyul/optimiser/ConditionalUnsimplifier.h>
#include <libyul/optimiser/ConditionalSimplifier.h>
#include <libyul/optimiser/CommonSubexpressionEliminator.h>
#include <libyul/optimiser/EquivalentFunctionCombiner.h>
#include <libyul/optimiser/ExpressionSplitter.h>
#include <libyul/optimiser/FunctionGrouper.h>
#include <libyul/optimiser/FunctionHoister.h>
#include <libyul/optimiser/FunctionSpecializer.h>
#include <libyul/optimiser/ExpressionInliner.h>
#include <libyul/optimiser/FullInliner.h>
#include <libyul/optimiser/ForLoopConditionIntoBody.h>
#include <libyul/optimiser/ForLoopInitRewriter.h>
#include <libyul/optimiser/LoadResolver.h>
#include <libyul/optimiser/LoopInvariantCodeMotion.h>
#include <libyul/optimiser/MainFunction.h>
#include <libyul/optimiser/StackLimitEvader.h>
#include <libyul/optimiser/NameDisplacer.h>
#include <libyul/optimiser/Rematerialiser.h>
#include <libyul/optimiser/ExpressionSimplifier.h>
#include <libyul/optimiser/UnusedFunctionParameterPruner.h>
#include <libyul/optimiser/UnusedPruner.h>
#include <libyul/optimiser/ExpressionJoiner.h>
#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/optimiser/ReasoningBasedSimplifier.h>
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
#include <libyul/AsmAnalysis.h>
#include <libyul/CompilabilityChecker.h>

#include <libsolidity/interface/OptimiserSettings.h>

#include <random>

using namespace solidity;
using namespace solidity::util;
using namespace solidity::langutil;
using namespace solidity::yul;
using namespace solidity::yul::test;
using namespace solidity::frontend;
using namespace std;

YulOptimizerTestCommon::YulOptimizerTestCommon(
	shared_ptr<Object> _obj,
	Dialect const& _dialect
)
{
	m_object = _obj;
	m_ast = m_object->code;
	m_analysisInfo = m_object->analysisInfo;
	m_dialect = &_dialect;

	m_namedSteps = {
		{"disambiguator", [&]() { disambiguate(); }},
		{"nameDisplacer", [&]() {
			disambiguate();
			NameDisplacer{
				*m_nameDispenser,
				{"illegal1"_yulstring, "illegal2"_yulstring, "illegal3"_yulstring, "illegal4"_yulstring, "illegal5"_yulstring}
			}(*m_ast);
		}},
		{"blockFlattener", [&]() {
			disambiguate();
			BlockFlattener::run(*m_context, *m_ast);
		}},
		{"constantOptimiser", [&]() {
			GasMeter meter(dynamic_cast<EVMDialect const&>(*m_dialect), false, 200);
			ConstantOptimiser{dynamic_cast<EVMDialect const&>(*m_dialect), meter}(*m_ast);
		}},
		{"varDeclInitializer", [&]() { VarDeclInitializer::run(*m_context, *m_ast); }},
		{"varNameCleaner", [&]() {
			disambiguate();
			FunctionHoister::run(*m_context, *m_ast);
			FunctionGrouper::run(*m_context, *m_ast);
			VarNameCleaner::run(*m_context, *m_ast);
		}},
		{"forLoopConditionIntoBody", [&]() {
			disambiguate();
			ForLoopConditionIntoBody::run(*m_context, *m_ast);
		}},
		{"forLoopInitRewriter", [&]() {
			disambiguate();
			ForLoopInitRewriter::run(*m_context, *m_ast);
		}},
		{"commonSubexpressionEliminator", [&]() {
			disambiguate();
			ForLoopInitRewriter::run(*m_context, *m_ast);
			FunctionHoister::run(*m_context, *m_ast);
			CommonSubexpressionEliminator::run(*m_context, *m_ast);
		}},
		{"conditionalUnsimplifier", [&]() {
			disambiguate();
			ConditionalUnsimplifier::run(*m_context, *m_ast);
		}},
		{"conditionalSimplifier", [&]() {
			disambiguate();
			ConditionalSimplifier::run(*m_context, *m_ast);
		}},
		{"expressionSplitter", [&]() { ExpressionSplitter::run(*m_context, *m_ast); }},
		{"expressionJoiner", [&]() {
			disambiguate();
			ExpressionJoiner::run(*m_context, *m_ast);
		}},
		{"splitJoin", [&]() {
			disambiguate();
			ExpressionSplitter::run(*m_context, *m_ast);
			ExpressionJoiner::run(*m_context, *m_ast);
			ExpressionJoiner::run(*m_context, *m_ast);
		}},
		{"functionGrouper", [&]() {
			disambiguate();
			FunctionGrouper::run(*m_context, *m_ast);
		}},
		{"functionHoister", [&]() {
			disambiguate();
			FunctionHoister::run(*m_context, *m_ast);
		}},
		{"functionSpecializer", [&]() {
			disambiguate();
			FunctionHoister::run(*m_context, *m_object->code);
			FunctionSpecializer::run(*m_context, *m_object->code);
		}},
		{"expressionInliner", [&]() {
			disambiguate();
			ExpressionInliner::run(*m_context, *m_ast);
		}},
		{"fullInliner", [&]() {
			disambiguate();
			FunctionHoister::run(*m_context, *m_ast);
			FunctionGrouper::run(*m_context, *m_ast);
			ExpressionSplitter::run(*m_context, *m_ast);
			FullInliner::run(*m_context, *m_ast);
			ExpressionJoiner::run(*m_context, *m_ast);
		}},
		{"mainFunction", [&]() {
			disambiguate();
			FunctionGrouper::run(*m_context, *m_ast);
			MainFunction::run(*m_context, *m_ast);
		}},
		{"rematerialiser", [&]() {
			disambiguate();
			ForLoopInitRewriter::run(*m_context, *m_ast);
			FunctionHoister::run(*m_context, *m_ast);
			Rematerialiser::run(*m_context, *m_ast);
		}},
		{"expressionSimplifier", [&]() {
			disambiguate();
			ForLoopInitRewriter::run(*m_context, *m_ast);
			FunctionHoister::run(*m_context, *m_ast);
			ExpressionSplitter::run(*m_context, *m_ast);
			CommonSubexpressionEliminator::run(*m_context, *m_ast);
			ExpressionSimplifier::run(*m_context, *m_ast);
			ExpressionSimplifier::run(*m_context, *m_ast);
			ExpressionSimplifier::run(*m_context, *m_ast);
			UnusedPruner::run(*m_context, *m_ast);
			ExpressionJoiner::run(*m_context, *m_ast);
			ExpressionJoiner::run(*m_context, *m_ast);
		}},
		{"fullSimplify", [&]() {
			disambiguate();
			ExpressionSplitter::run(*m_context, *m_ast);
			ForLoopInitRewriter::run(*m_context, *m_ast);
			FunctionHoister::run(*m_context, *m_ast);
			CommonSubexpressionEliminator::run(*m_context, *m_ast);
			ExpressionSimplifier::run(*m_context, *m_ast);
			UnusedPruner::run(*m_context, *m_ast);
			CircularReferencesPruner::run(*m_context, *m_ast);
			DeadCodeEliminator::run(*m_context, *m_ast);
			ExpressionJoiner::run(*m_context, *m_ast);
			ExpressionJoiner::run(*m_context, *m_ast);
		}},
		{"unusedFunctionParameterPruner", [&]() {
			disambiguate();
			ForLoopInitRewriter::run(*m_context, *m_ast);
			FunctionHoister::run(*m_context, *m_object->code);
			LiteralRematerialiser::run(*m_context, *m_object->code);
			UnusedFunctionParameterPruner::run(*m_context, *m_object->code);
		}},
		{"unusedPruner", [&]() {
			disambiguate();
			UnusedPruner::run(*m_context, *m_ast);
		}},
		{"circularReferencesPruner", [&]() {
			disambiguate();
			FunctionHoister::run(*m_context, *m_ast);
			CircularReferencesPruner::run(*m_context, *m_ast);
		}},
		{"deadCodeEliminator", [&]() {
			disambiguate();
			ForLoopInitRewriter::run(*m_context, *m_ast);
			DeadCodeEliminator::run(*m_context, *m_ast);
		}},
		{"ssaTransform", [&]() {
			disambiguate();
			ForLoopInitRewriter::run(*m_context, *m_ast);
			SSATransform::run(*m_context, *m_ast);
		}},
		{"redundantAssignEliminator", [&]() {
			disambiguate();
			ForLoopInitRewriter::run(*m_context, *m_ast);
			RedundantAssignEliminator::run(*m_context, *m_ast);
		}},
		{"ssaPlusCleanup", [&]() {
			disambiguate();
			ForLoopInitRewriter::run(*m_context, *m_ast);
			SSATransform::run(*m_context, *m_ast);
			RedundantAssignEliminator::run(*m_context, *m_ast);
		}},
		{"loadResolver", [&]() {
			disambiguate();
			ForLoopInitRewriter::run(*m_context, *m_ast);
			FunctionHoister::run(*m_context, *m_ast);
			ExpressionSplitter::run(*m_context, *m_ast);
			CommonSubexpressionEliminator::run(*m_context, *m_ast);
			ExpressionSimplifier::run(*m_context, *m_ast);

			LoadResolver::run(*m_context, *m_ast);

			UnusedPruner::run(*m_context, *m_ast);
			ExpressionJoiner::run(*m_context, *m_ast);
			ExpressionJoiner::run(*m_context, *m_ast);
		}},
		{"loopInvariantCodeMotion", [&]() {
			disambiguate();
			ForLoopInitRewriter::run(*m_context, *m_ast);
			FunctionHoister::run(*m_context, *m_ast);
			LoopInvariantCodeMotion::run(*m_context, *m_ast);
		}},
		{"controlFlowSimplifier", [&]() {
			disambiguate();
			ForLoopInitRewriter::run(*m_context, *m_ast);
			ControlFlowSimplifier::run(*m_context, *m_ast);
		}},
		{"structuralSimplifier", [&]() {
			disambiguate();
			ForLoopInitRewriter::run(*m_context, *m_ast);
			FunctionHoister::run(*m_context, *m_ast);
			LiteralRematerialiser::run(*m_context, *m_ast);
			StructuralSimplifier::run(*m_context, *m_ast);
		}},
		{"reasoningBasedSimplifier", [&]() {
			disambiguate();
			ReasoningBasedSimplifier::run(*m_context, *m_object->code);
		}},
		{"equivalentFunctionCombiner", [&]() {
			disambiguate();
			ForLoopInitRewriter::run(*m_context, *m_ast);
			FunctionHoister::run(*m_context, *m_ast);
			EquivalentFunctionCombiner::run(*m_context, *m_ast);
		}},
		{"ssaReverser", [&]() {
			disambiguate();
			SSAReverser::run(*m_context, *m_ast);
		}},
		{"ssaAndBack", [&]() {
			disambiguate();
			ForLoopInitRewriter::run(*m_context, *m_ast);
			// apply SSA
			SSATransform::run(*m_context, *m_ast);
			RedundantAssignEliminator::run(*m_context, *m_ast);
			// reverse SSA
			SSAReverser::run(*m_context, *m_ast);
			FunctionHoister::run(*m_context, *m_ast);
			CommonSubexpressionEliminator::run(*m_context, *m_ast);
			UnusedPruner::run(*m_context, *m_ast);
		}},
		{"stackCompressor", [&]() {
			disambiguate();
			ForLoopInitRewriter::run(*m_context, *m_ast);
			FunctionHoister::run(*m_context, *m_ast);
			FunctionGrouper::run(*m_context, *m_ast);
			size_t maxIterations = 16;
			StackCompressor::run(*m_dialect, *m_object, true, maxIterations);
			BlockFlattener::run(*m_context, *m_ast);
		}},
		{"wordSizeTransform", [&]() {
			disambiguate();
			ExpressionSplitter::run(*m_context, *m_ast);
			WordSizeTransform::run(*m_dialect, *m_dialect, *m_ast, *m_nameDispenser);
		}},
		{"fullSuite", [&]() {
			GasMeter meter(dynamic_cast<EVMDialect const&>(*m_dialect), false, 200);
			OptimiserSuite::run(
				*m_dialect,
				&meter,
				*m_object,
				true,
				frontend::OptimiserSettings::DefaultYulOptimiserSteps,
				frontend::OptimiserSettings::standard().expectedExecutionsPerDeployment
			);
		}},
		{"stackLimitEvader", [&]() {
			disambiguate();
			StackLimitEvader::run(*m_context, *m_object, CompilabilityChecker{
				*m_dialect,
				*m_object,
				true
			}.unreachableVariables);
		}},
		{"fakeStackLimitEvader", [&]() {
			disambiguate();
			// Mark all variables with a name starting with "$" for escalation to memory.
			struct FakeUnreachableGenerator: ASTWalker
			{
				map<YulString, set<YulString>> fakeUnreachables;
				using ASTWalker::operator();
				void operator()(FunctionDefinition const& _function) override
				{
					YulString originalFunctionName = m_currentFunction;
					m_currentFunction = _function.name;
					for (TypedName const& _argument: _function.parameters)
						visitVariableName(_argument.name);
					for (TypedName const& _argument: _function.returnVariables)
						visitVariableName(_argument.name);
					ASTWalker::operator()(_function);
					m_currentFunction = originalFunctionName;
				}
				void visitVariableName(YulString _var)
				{
					if (!_var.empty() && _var.str().front() == '$')
						fakeUnreachables[m_currentFunction].insert(_var);
				}
				void operator()(VariableDeclaration const& _varDecl) override
				{
					for (auto const& var: _varDecl.variables)
						visitVariableName(var.name);
					ASTWalker::operator()(_varDecl);
				}
				void operator()(Identifier const& _identifier) override
				{
					visitVariableName(_identifier.name);
					ASTWalker::operator()(_identifier);
				}
				YulString m_currentFunction = YulString{};
			};
			FakeUnreachableGenerator fakeUnreachableGenerator;
			fakeUnreachableGenerator(*m_ast);
			StackLimitEvader::run(*m_context, *m_object, fakeUnreachableGenerator.fakeUnreachables);
		}}
	};
}

void YulOptimizerTestCommon::setStep(string const& _optimizerStep)
{
	m_optimizerStep = _optimizerStep;
}

bool YulOptimizerTestCommon::runStep()
{
	yulAssert(m_dialect, "Dialect not set.");

	updateContext();

	if (m_namedSteps.count(m_optimizerStep))
		m_namedSteps[m_optimizerStep]();
	else
		return false;

	return true;
}

string YulOptimizerTestCommon::randomOptimiserStep(unsigned _seed)
{
	std::mt19937 prng(_seed);
	std::uniform_int_distribution<size_t> dist(1, m_namedSteps.size());
	size_t idx = dist(prng);
	size_t count = 1;
	for (auto &step: m_namedSteps)
	{
		if (count == idx)
		{
			string optimiserStep = step.first;
			// Do not fuzz mainFunction and wordSizeTransform
			// because they do not preserve yul code semantics.
			// Do not fuzz reasoning based simplifier because
			// it can sometimes drain memory.
			if (
				optimiserStep == "mainFunction"	||
				optimiserStep == "wordSizeTransform" ||
				optimiserStep == "reasoningBasedSimplifier"
			)
				// "Fullsuite" is fuzzed roughly four times more frequently than
				// other steps because of the filtering in place above.
				return "fullSuite";
			else
				return optimiserStep;
		}
		count++;
	}
	yulAssert(false, "Optimiser step selection failed.");
}

shared_ptr<Block> YulOptimizerTestCommon::run()
{
	return runStep() ? m_ast : nullptr;
}

void YulOptimizerTestCommon::disambiguate()
{
	*m_object->code = std::get<Block>(Disambiguator(*m_dialect, *m_analysisInfo)(*m_object->code));
	m_analysisInfo.reset();
	updateContext();
}

void YulOptimizerTestCommon::updateContext()
{
	m_nameDispenser = make_unique<NameDispenser>(*m_dialect, *m_object->code, m_reservedIdentifiers);
	m_context = make_unique<OptimiserStepContext>(OptimiserStepContext{
		*m_dialect,
		*m_nameDispenser,
		m_reservedIdentifiers,
		frontend::OptimiserSettings::standard().expectedExecutionsPerDeployment
	});
}
