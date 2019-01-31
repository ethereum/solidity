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
#include <libyul/optimiser/SSAReverser.h>
#include <libyul/optimiser/SSATransform.h>
#include <libyul/optimiser/StructuralSimplifier.h>
#include <libyul/optimiser/RedundantAssignEliminator.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/AsmData.h>
#include <libyul/AsmPrinter.h>

#include <libdevcore/CommonData.h>

using namespace std;
using namespace dev;
using namespace yul;

void OptimiserSuite::run(
	shared_ptr<Dialect> const& _dialect,
	Block& _ast,
	AsmAnalysisInfo const& _analysisInfo,
	set<YulString> const& _externallyUsedIdentifiers
)
{
	set<YulString> reservedIdentifiers = _externallyUsedIdentifiers;

	Block ast = boost::get<Block>(Disambiguator(*_dialect, _analysisInfo, reservedIdentifiers)(_ast));

	(VarDeclInitializer{})(ast);
	(FunctionHoister{})(ast);
	(BlockFlattener{})(ast);
	(FunctionGrouper{})(ast);
	EquivalentFunctionCombiner::run(ast);
	UnusedPruner::runUntilStabilised(*_dialect, ast, reservedIdentifiers);
	(ForLoopInitRewriter{})(ast);
	(BlockFlattener{})(ast);
	StructuralSimplifier{*_dialect}(ast);

	NameDispenser dispenser{*_dialect, ast};

	for (size_t i = 0; i < 4; i++)
	{
		ExpressionSplitter{*_dialect, dispenser}(ast);
		SSATransform::run(ast, dispenser);
		RedundantAssignEliminator::run(*_dialect, ast);
		RedundantAssignEliminator::run(*_dialect, ast);

		ExpressionSimplifier::run(*_dialect, ast);
		CommonSubexpressionEliminator{*_dialect}(ast);
		StructuralSimplifier{*_dialect}(ast);
		(BlockFlattener{})(ast);
		SSATransform::run(ast, dispenser);
		RedundantAssignEliminator::run(*_dialect, ast);
		RedundantAssignEliminator::run(*_dialect, ast);
		UnusedPruner::runUntilStabilised(*_dialect, ast, reservedIdentifiers);
		CommonSubexpressionEliminator{*_dialect}(ast);
		UnusedPruner::runUntilStabilised(*_dialect, ast, reservedIdentifiers);

		SSAReverser::run(ast);
		CommonSubexpressionEliminator{*_dialect}(ast);
		UnusedPruner::runUntilStabilised(*_dialect, ast, reservedIdentifiers);

		ExpressionJoiner::run(ast);
		ExpressionJoiner::run(ast);
		ExpressionInliner(*_dialect, ast).run();
		UnusedPruner::runUntilStabilised(*_dialect, ast, reservedIdentifiers);

		ExpressionSplitter{*_dialect, dispenser}(ast);
		SSATransform::run(ast, dispenser);
		RedundantAssignEliminator::run(*_dialect, ast);
		RedundantAssignEliminator::run(*_dialect, ast);
		CommonSubexpressionEliminator{*_dialect}(ast);

		(FunctionGrouper{})(ast);
		EquivalentFunctionCombiner::run(ast);
		FullInliner{ast, dispenser}.run();
		(BlockFlattener{})(ast);

		SSATransform::run(ast, dispenser);
		RedundantAssignEliminator::run(*_dialect, ast);
		RedundantAssignEliminator::run(*_dialect, ast);
		ExpressionSimplifier::run(*_dialect, ast);
		StructuralSimplifier{*_dialect}(ast);
		(BlockFlattener{})(ast);
		CommonSubexpressionEliminator{*_dialect}(ast);
		SSATransform::run(ast, dispenser);
		RedundantAssignEliminator::run(*_dialect, ast);
		RedundantAssignEliminator::run(*_dialect, ast);
		UnusedPruner::runUntilStabilised(*_dialect, ast, reservedIdentifiers);
		CommonSubexpressionEliminator{*_dialect}(ast);
	}
	ExpressionJoiner::run(ast);
	Rematerialiser::run(*_dialect, ast);
	UnusedPruner::runUntilStabilised(*_dialect, ast, reservedIdentifiers);
	ExpressionJoiner::run(ast);
	UnusedPruner::runUntilStabilised(*_dialect, ast, reservedIdentifiers);
	ExpressionJoiner::run(ast);
	UnusedPruner::runUntilStabilised(*_dialect, ast, reservedIdentifiers);

	SSAReverser::run(ast);
	CommonSubexpressionEliminator{*_dialect}(ast);
	UnusedPruner::runUntilStabilised(*_dialect, ast, reservedIdentifiers);

	ExpressionJoiner::run(ast);
	Rematerialiser::run(*_dialect, ast);
	UnusedPruner::runUntilStabilised(*_dialect, ast, reservedIdentifiers);

	_ast = std::move(ast);
}
