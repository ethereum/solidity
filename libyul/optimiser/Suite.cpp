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
#include <libyul/optimiser/DeadCodeEliminator.h>
#include <libyul/optimiser/FunctionGrouper.h>
#include <libyul/optimiser/FunctionHoister.h>
#include <libyul/optimiser/EquivalentFunctionCombiner.h>
#include <libyul/optimiser/ExpressionSplitter.h>
#include <libyul/optimiser/ExpressionJoiner.h>
#include <libyul/optimiser/ExpressionInliner.h>
#include <libyul/optimiser/FullInliner.h>
#include <libyul/optimiser/ForLoopInitRewriter.h>
#include <libyul/optimiser/Rematerialiser.h>
#include <libyul/optimiser/UnusedPruner.h>
#include <libyul/optimiser/ExpressionSimplifier.h>
#include <libyul/optimiser/CommonSubexpressionEliminator.h>
#include <libyul/optimiser/Semantics.h>
#include <libyul/optimiser/SSAReverser.h>
#include <libyul/optimiser/SSATransform.h>
#include <libyul/optimiser/StackCompressor.h>
#include <libyul/optimiser/StructuralSimplifier.h>
#include <libyul/optimiser/RedundantAssignEliminator.h>
#include <libyul/optimiser/VarNameCleaner.h>
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

	VarDeclInitializer{}(ast);
	FunctionHoister{}(ast);
	BlockFlattener{}(ast);
	ForLoopInitRewriter{}(ast);
	DeadCodeEliminator{_dialect}(ast);
	FunctionGrouper{}(ast);
	EquivalentFunctionCombiner::run(ast);
	UnusedPruner::runUntilStabilised(_dialect, ast, reservedIdentifiers);
	BlockFlattener{}(ast);
	ControlFlowSimplifier{_dialect}(ast);
	StructuralSimplifier{_dialect}(ast);
	ControlFlowSimplifier{_dialect}(ast);
	BlockFlattener{}(ast);

	// None of the above can make stack problems worse.

	NameDispenser dispenser{_dialect, ast, reservedIdentifiers};

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
			ExpressionSplitter{_dialect, dispenser}(ast);
			SSATransform::run(ast, dispenser);
			RedundantAssignEliminator::run(_dialect, ast);
			RedundantAssignEliminator::run(_dialect, ast);

			ExpressionSimplifier::run(_dialect, ast);

			CommonSubexpressionEliminator::run(_dialect, ast);
		}

		{
			// still in SSA, perform structural simplification
			ControlFlowSimplifier{_dialect}(ast);
			StructuralSimplifier{_dialect}(ast);
			ControlFlowSimplifier{_dialect}(ast);
			BlockFlattener{}(ast);
			DeadCodeEliminator{_dialect}(ast);
			UnusedPruner::runUntilStabilised(_dialect, ast, reservedIdentifiers);
		}

		{
			// simplify again
			CommonSubexpressionEliminator::run(_dialect, ast);
			UnusedPruner::runUntilStabilised(_dialect, ast, reservedIdentifiers);
		}

		{
			// reverse SSA
			SSAReverser::run(ast);
			CommonSubexpressionEliminator::run(_dialect, ast);
			UnusedPruner::runUntilStabilised(_dialect, ast, reservedIdentifiers);

			ExpressionJoiner::run(ast);
			ExpressionJoiner::run(ast);
		}

		// should have good "compilability" property here.

		{
			// run functional expression inliner
			ExpressionInliner(_dialect, ast).run();
			UnusedPruner::runUntilStabilised(_dialect, ast, reservedIdentifiers);
		}

		{
			// Turn into SSA again and simplify
			ExpressionSplitter{_dialect, dispenser}(ast);
			SSATransform::run(ast, dispenser);
			RedundantAssignEliminator::run(_dialect, ast);
			RedundantAssignEliminator::run(_dialect, ast);
			CommonSubexpressionEliminator::run(_dialect, ast);
		}

		{
			// run full inliner
			FunctionGrouper{}(ast);
			EquivalentFunctionCombiner::run(ast);
			FullInliner{ast, dispenser}.run();
			BlockFlattener{}(ast);
		}

		{
			// SSA plus simplify
			SSATransform::run(ast, dispenser);
			RedundantAssignEliminator::run(_dialect, ast);
			RedundantAssignEliminator::run(_dialect, ast);
			ExpressionSimplifier::run(_dialect, ast);
			StructuralSimplifier{_dialect}(ast);
			BlockFlattener{}(ast);
			DeadCodeEliminator{_dialect}(ast);
			ControlFlowSimplifier{_dialect}(ast);
			CommonSubexpressionEliminator::run(_dialect, ast);
			SSATransform::run(ast, dispenser);
			RedundantAssignEliminator::run(_dialect, ast);
			RedundantAssignEliminator::run(_dialect, ast);
			UnusedPruner::runUntilStabilised(_dialect, ast, reservedIdentifiers);
			CommonSubexpressionEliminator::run(_dialect, ast);
		}
	}

	// Make source short and pretty.

	ExpressionJoiner::run(ast);
	Rematerialiser::run(_dialect, ast);
	UnusedPruner::runUntilStabilised(_dialect, ast, reservedIdentifiers);
	ExpressionJoiner::run(ast);
	UnusedPruner::runUntilStabilised(_dialect, ast, reservedIdentifiers);
	ExpressionJoiner::run(ast);
	UnusedPruner::runUntilStabilised(_dialect, ast, reservedIdentifiers);

	SSAReverser::run(ast);
	CommonSubexpressionEliminator::run(_dialect, ast);
	UnusedPruner::runUntilStabilised(_dialect, ast, reservedIdentifiers);

	ExpressionJoiner::run(ast);
	Rematerialiser::run(_dialect, ast);
	UnusedPruner::runUntilStabilised(_dialect, ast, reservedIdentifiers);

	// This is a tuning parameter, but actually just prevents infinite loops.
	size_t stackCompressorMaxIterations = 16;
	FunctionGrouper{}(ast);
	// We ignore the return value because we will get a much better error
	// message once we perform code generation.
	StackCompressor::run(
		_dialect,
		_object,
		_optimizeStackAllocation,
		stackCompressorMaxIterations
	);
	BlockFlattener{}(ast);
	DeadCodeEliminator{_dialect}(ast);
	ControlFlowSimplifier{_dialect}(ast);

	FunctionGrouper{}(ast);

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
	VarNameCleaner{ast, _dialect, reservedIdentifiers}(ast);

	*_object.analysisInfo = AsmAnalyzer::analyzeStrictAssertCorrect(_dialect, _object);
}
