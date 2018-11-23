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
#include <libyul/optimiser/FunctionGrouper.h>
#include <libyul/optimiser/FunctionHoister.h>
#include <libyul/optimiser/ExpressionSplitter.h>
#include <libyul/optimiser/ExpressionJoiner.h>
#include <libyul/optimiser/ExpressionInliner.h>
#include <libyul/optimiser/FullInliner.h>
#include <libyul/optimiser/ForLoopInitRewriter.h>
#include <libyul/optimiser/Rematerialiser.h>
#include <libyul/optimiser/UnusedPruner.h>
#include <libyul/optimiser/ExpressionSimplifier.h>
#include <libyul/optimiser/CommonSubexpressionEliminator.h>
#include <libyul/optimiser/SSATransform.h>
#include <libyul/optimiser/RedundantAssignEliminator.h>
#include <libyul/optimiser/VarDeclPropagator.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/AsmData.h>
#include <libyul/AsmPrinter.h>

#include <libdevcore/CommonData.h>

using namespace std;
using namespace dev;
using namespace dev::yul;

void OptimiserSuite::run(
	Block& _ast,
	solidity::assembly::AsmAnalysisInfo const& _analysisInfo,
	set<YulString> const& _externallyUsedIdentifiers
)
{
	set<YulString> reservedIdentifiers = _externallyUsedIdentifiers;

	Block ast = boost::get<Block>(Disambiguator(_analysisInfo, reservedIdentifiers)(_ast));

	(FunctionHoister{})(ast);
	(FunctionGrouper{})(ast);
	(ForLoopInitRewriter{})(ast);

	NameDispenser dispenser{ast};

	for (size_t i = 0; i < 4; i++)
	{
		ExpressionSplitter{dispenser}(ast);
		SSATransform::run(ast, dispenser);
		RedundantAssignEliminator::run(ast);
		VarDeclPropagator{}(ast);
		RedundantAssignEliminator::run(ast);

		CommonSubexpressionEliminator{}(ast);
		ExpressionSimplifier::run(ast);
		SSATransform::run(ast, dispenser);
		RedundantAssignEliminator::run(ast);
		RedundantAssignEliminator::run(ast);
		UnusedPruner::runUntilStabilised(ast, reservedIdentifiers);
		CommonSubexpressionEliminator{}(ast);
		UnusedPruner::runUntilStabilised(ast, reservedIdentifiers);
		SSATransform::run(ast, dispenser);
		RedundantAssignEliminator::run(ast);
		RedundantAssignEliminator::run(ast);

		ExpressionJoiner::run(ast);
		ExpressionJoiner::run(ast);
		ExpressionInliner(ast).run();
		UnusedPruner::runUntilStabilised(ast);

		ExpressionSplitter{dispenser}(ast);
		SSATransform::run(ast, dispenser);
		RedundantAssignEliminator::run(ast);
		RedundantAssignEliminator::run(ast);
		CommonSubexpressionEliminator{}(ast);
		FullInliner{ast, dispenser}.run();
		VarDeclPropagator{}(ast);
		SSATransform::run(ast, dispenser);
		RedundantAssignEliminator::run(ast);
		VarDeclPropagator{}(ast);
		RedundantAssignEliminator::run(ast);
		ExpressionSimplifier::run(ast);
		CommonSubexpressionEliminator{}(ast);
		SSATransform::run(ast, dispenser);
		RedundantAssignEliminator::run(ast);
		VarDeclPropagator{}(ast);
		RedundantAssignEliminator::run(ast);
		UnusedPruner::runUntilStabilised(ast, reservedIdentifiers);
	}
	ExpressionJoiner::run(ast);
	VarDeclPropagator{}(ast);
	UnusedPruner::runUntilStabilised(ast);
	ExpressionJoiner::run(ast);
	UnusedPruner::runUntilStabilised(ast);
	ExpressionJoiner::run(ast);
	VarDeclPropagator{}(ast);
	UnusedPruner::runUntilStabilised(ast);
	ExpressionJoiner::run(ast);
	UnusedPruner::runUntilStabilised(ast);

	_ast = std::move(ast);
}
