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
 * Optimiser suite that combines all steps and also provides the settings for the heuristics.
 */

#include <libyul/optimiser/Suite.h>

#include <libyul/optimiser/Disambiguator.h>
#include <libyul/optimiser/VarDeclInitializer.h>
#include <libyul/optimiser/BlockFlattener.h>
#include <libyul/optimiser/CallGraphGenerator.h>
#include <libyul/optimiser/ControlFlowSimplifier.h>
#include <libyul/optimiser/ConditionalSimplifier.h>
#include <libyul/optimiser/ConditionalUnsimplifier.h>
#include <libyul/optimiser/DeadCodeEliminator.h>
#include <libyul/optimiser/FunctionGrouper.h>
#include <libyul/optimiser/FunctionHoister.h>
#include <libyul/optimiser/EquivalentFunctionCombiner.h>
#include <libyul/optimiser/ExpressionSplitter.h>
#include <libyul/optimiser/ExpressionJoiner.h>
#include <libyul/optimiser/ExpressionInliner.h>
#include <libyul/optimiser/FullInliner.h>
#include <libyul/optimiser/ForLoopConditionIntoBody.h>
#include <libyul/optimiser/ForLoopConditionOutOfBody.h>
#include <libyul/optimiser/ForLoopInitRewriter.h>
#include <libyul/optimiser/ForLoopConditionIntoBody.h>
#include <libyul/optimiser/Rematerialiser.h>
#include <libyul/optimiser/UnusedPruner.h>
#include <libyul/optimiser/ExpressionSimplifier.h>
#include <libyul/optimiser/CommonSubexpressionEliminator.h>
#include <libyul/optimiser/Semantics.h>
#include <libyul/optimiser/SSAReverser.h>
#include <libyul/optimiser/SSATransform.h>
#include <libyul/optimiser/StackCompressor.h>
#include <libyul/optimiser/StructuralSimplifier.h>
#include <libyul/optimiser/SyntacticalEquality.h>
#include <libyul/optimiser/RedundantAssignEliminator.h>
#include <libyul/optimiser/VarNameCleaner.h>
#include <libyul/optimiser/LoadResolver.h>
#include <libyul/optimiser/Metrics.h>
#include <libyul/backends/evm/ConstantOptimiser.h>
#include <libyul/AsmAnalysis.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/AsmData.h>
#include <libyul/AsmPrinter.h>
#include <libyul/Object.h>

#include <libyul/backends/wasm/WasmDialect.h>
#include <libyul/backends/evm/NoOutputAssembly.h>

#include <libdevcore/CommonData.h>

using namespace std;
using namespace dev;
using namespace yul;

void OptimiserSuite::run(
	Dialect const& _dialect,
	GasMeter const* _meter,
	Object& _object,
	bool _optimizeStackAllocation,
	set<YulString> const& _externallyUsedIdentifiers
)
{
	set<YulString> reservedIdentifiers = _externallyUsedIdentifiers;
	reservedIdentifiers += _dialect.fixedFunctionNames();

	*_object.code = boost::get<Block>(Disambiguator(
		_dialect,
		*_object.analysisInfo,
		reservedIdentifiers
	)(*_object.code));
	Block& ast = *_object.code;

	OptimiserSuite suite(_dialect, reservedIdentifiers, Debug::None, ast);

	suite.runSequence({
		VarDeclInitializer::name,
		FunctionHoister::name,
		BlockFlattener::name,
		ForLoopInitRewriter::name,
		DeadCodeEliminator::name,
		FunctionGrouper::name,
		EquivalentFunctionCombiner::name,
		UnusedPruner::name,
		BlockFlattener::name,
		ControlFlowSimplifier::name,
		LiteralRematerialiser::name,
		ConditionalUnsimplifier::name,
		StructuralSimplifier::name,
		ControlFlowSimplifier::name,
		ForLoopConditionIntoBody::name,
		BlockFlattener::name
	}, ast);

	// None of the above can make stack problems worse.

	size_t codeSize = 0;
	for (size_t rounds = 0; rounds < 12; ++rounds)
	{
		{
			size_t newSize = CodeSize::codeSizeIncludingFunctions(ast);
			if (newSize == codeSize)
				break;
			codeSize = newSize;
		}

		{
			// Turn into SSA and simplify
			suite.runSequence({
				ExpressionSplitter::name,
				SSATransform::name,
				RedundantAssignEliminator::name,
				RedundantAssignEliminator::name,
				ExpressionSimplifier::name,
				CommonSubexpressionEliminator::name,
				LoadResolver::name
			}, ast);
		}

		{
			// perform structural simplification
			suite.runSequence({
				CommonSubexpressionEliminator::name,
				ConditionalSimplifier::name,
				LiteralRematerialiser::name,
				ConditionalUnsimplifier::name,
				StructuralSimplifier::name,
				ForLoopConditionOutOfBody::name,
				ControlFlowSimplifier::name,
				StructuralSimplifier::name,
				ControlFlowSimplifier::name,
				BlockFlattener::name,
				DeadCodeEliminator::name,
				ForLoopConditionIntoBody::name,
				UnusedPruner::name
			}, ast);
		}

		{
			// simplify again
			suite.runSequence({
				LoadResolver::name,
				CommonSubexpressionEliminator::name,
				UnusedPruner::name,
			}, ast);
		}

		{
			// reverse SSA
			suite.runSequence({
				SSAReverser::name,
				CommonSubexpressionEliminator::name,
				UnusedPruner::name,

				ExpressionJoiner::name,
				ExpressionJoiner::name,
			}, ast);
		}

		// should have good "compilability" property here.

		{
			// run functional expression inliner
			suite.runSequence({
				ExpressionInliner::name,
				UnusedPruner::name,
			}, ast);
		}

		{
			// Turn into SSA again and simplify
			suite.runSequence({
				ExpressionSplitter::name,
				SSATransform::name,
				RedundantAssignEliminator::name,
				RedundantAssignEliminator::name,
				CommonSubexpressionEliminator::name,
				LoadResolver::name,
			}, ast);
		}

		{
			// run full inliner
			suite.runSequence({
				FunctionGrouper::name,
				EquivalentFunctionCombiner::name,
				FullInliner::name,
				BlockFlattener::name
			}, ast);
		}

		{
			// SSA plus simplify
			suite.runSequence({
				ConditionalSimplifier::name,
				LiteralRematerialiser::name,
				ConditionalUnsimplifier::name,
				CommonSubexpressionEliminator::name,
				SSATransform::name,
				RedundantAssignEliminator::name,
				RedundantAssignEliminator::name,
				LoadResolver::name,
				ExpressionSimplifier::name,
				ForLoopConditionOutOfBody::name,
				StructuralSimplifier::name,
				BlockFlattener::name,
				DeadCodeEliminator::name,
				ControlFlowSimplifier::name,
				CommonSubexpressionEliminator::name,
				SSATransform::name,
				RedundantAssignEliminator::name,
				RedundantAssignEliminator::name,
				ForLoopConditionIntoBody::name,
				UnusedPruner::name,
				CommonSubexpressionEliminator::name,
			}, ast);
		}
	}

	// Make source short and pretty.

	suite.runSequence({
		ExpressionJoiner::name,
		Rematerialiser::name,
		UnusedPruner::name,
		ExpressionJoiner::name,
		UnusedPruner::name,
		ExpressionJoiner::name,
		UnusedPruner::name,

		SSAReverser::name,
		CommonSubexpressionEliminator::name,
		LiteralRematerialiser::name,
		ForLoopConditionOutOfBody::name,
		CommonSubexpressionEliminator::name,
		UnusedPruner::name,

		ExpressionJoiner::name,
		Rematerialiser::name,
		UnusedPruner::name,
	}, ast);

	// This is a tuning parameter, but actually just prevents infinite loops.
	size_t stackCompressorMaxIterations = 16;
	suite.runSequence({
		FunctionGrouper::name
	}, ast);
	// We ignore the return value because we will get a much better error
	// message once we perform code generation.
	StackCompressor::run(
		_dialect,
		_object,
		_optimizeStackAllocation,
		stackCompressorMaxIterations
	);
	suite.runSequence({
		BlockFlattener::name,
		DeadCodeEliminator::name,
		ControlFlowSimplifier::name,
		LiteralRematerialiser::name,
		ForLoopConditionOutOfBody::name,
		CommonSubexpressionEliminator::name,

		FunctionGrouper::name,
	}, ast);

	if (EVMDialect const* dialect = dynamic_cast<EVMDialect const*>(&_dialect))
	{
		yulAssert(_meter, "");
		ConstantOptimiser{*dialect, *_meter}(ast);
	}
	else if (dynamic_cast<WasmDialect const*>(&_dialect))
	{
		// If the first statement is an empty block, remove it.
		// We should only have function definitions after that.
		if (ast.statements.size() > 1 && boost::get<Block>(ast.statements.front()).statements.empty())
			ast.statements.erase(ast.statements.begin());
	}
	suite.runSequence({
		VarNameCleaner::name
	}, ast);

	*_object.analysisInfo = AsmAnalyzer::analyzeStrictAssertCorrect(_dialect, _object);
}

namespace
{


template <class... Step>
map<string, unique_ptr<OptimiserStep>> optimiserStepCollection()
{
	map<string, unique_ptr<OptimiserStep>> ret;
	for (unique_ptr<OptimiserStep>& s: make_vector<unique_ptr<OptimiserStep>>(
		(make_unique<OptimiserStepInstance<Step>>())...
	))
	{
		yulAssert(!ret.count(s->name), "");
		ret[s->name] = std::move(s);
	}
	return ret;
}

}

map<string, unique_ptr<OptimiserStep>> const& OptimiserSuite::allSteps()
{
	static map<string, unique_ptr<OptimiserStep>> instance;
	if (instance.empty())
		instance = optimiserStepCollection<
			BlockFlattener,
			CommonSubexpressionEliminator,
			ConditionalSimplifier,
			ConditionalUnsimplifier,
			ControlFlowSimplifier,
			DeadCodeEliminator,
			EquivalentFunctionCombiner,
			ExpressionInliner,
			ExpressionJoiner,
			ExpressionSimplifier,
			ExpressionSplitter,
			ForLoopConditionIntoBody,
			ForLoopConditionOutOfBody,
			ForLoopInitRewriter,
			FullInliner,
			FunctionGrouper,
			FunctionHoister,
			LiteralRematerialiser,
			LoadResolver,
			RedundantAssignEliminator,
			Rematerialiser,
			SSAReverser,
			SSATransform,
			StructuralSimplifier,
			UnusedPruner,
			VarDeclInitializer,
			VarNameCleaner
		>();
	return instance;
}

void OptimiserSuite::runSequence(std::vector<string> const& _steps, Block& _ast)
{
	unique_ptr<Block> copy;
	if (m_debug == Debug::PrintChanges)
		copy = make_unique<Block>(boost::get<Block>(ASTCopier{}(_ast)));
	for (string const& step: _steps)
	{
		if (m_debug == Debug::PrintStep)
			cout << "Running " << step << endl;
		allSteps().at(step)->run(m_context, _ast);
		if (m_debug == Debug::PrintChanges)
		{
			// TODO should add switch to also compare variable names!
			if (SyntacticallyEqual{}.statementEqual(_ast, *copy))
				cout << "== Running " << step << " did not cause changes." << endl;
			else
			{
				cout << "== Running " << step << " changed the AST." << endl;
				cout << AsmPrinter{}(_ast) << endl;
				copy = make_unique<Block>(boost::get<Block>(ASTCopier{}(_ast)));
			}
		}
	}
}
