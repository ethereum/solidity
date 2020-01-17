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

#include <tools/yulPhaser/Chromosome.h>

#include <tools/yulPhaser/Random.h>

#include <libyul/optimiser/Suite.h>
#include <libsolutil/CommonData.h>

#include <sstream>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::yul;
using namespace solidity::phaser;

namespace solidity::phaser
{

ostream& operator<<(ostream& _stream, Chromosome const& _chromosome);

}

Chromosome Chromosome::makeRandom(size_t _length)
{
	vector<string> steps;
	for (size_t i = 0; i < _length; ++i)
		steps.push_back(randomOptimisationStep());

	return Chromosome(move(steps));
}

ostream& phaser::operator<<(ostream& _stream, Chromosome const& _chromosome)
{
	for (auto const& stepName: _chromosome.m_optimisationSteps)
		_stream << OptimiserSuite::stepNameToAbbreviationMap().at(stepName);

	return _stream;
}

vector<string> Chromosome::allStepNames()
{
	vector<string> stepNames;
	for (auto const& step: OptimiserSuite::allSteps())
		stepNames.push_back(step.first);

	return stepNames;
}

vector<string> Chromosome::allStepNamesExcept(vector<string> const& _excludedStepNames)
{
	// This is not very efficient but vectors are small and the caller will cache the results anyway.
	// What matters a bit more is that using vector rather than a set gives us O(1) access to
	// random elements in other functions.
	return convertContainer<vector<string>>(
		convertContainer<set<string>>(allStepNames()) -
		convertContainer<set<string>>(_excludedStepNames)
	);
}

string const& Chromosome::randomOptimisationStep()
{
	static vector<string> stepNames = allStepNamesExcept({
		// All possible steps, listed and commented-out for easy tweaking.
		// The uncommented ones are not used (possibly because they fail).
		//{BlockFlattener::name},
		//{CommonSubexpressionEliminator::name},
		//{ConditionalSimplifier::name},
		//{ConditionalUnsimplifier::name},
		//{ControlFlowSimplifier::name},
		//{DeadCodeEliminator::name},
		//{EquivalentFunctionCombiner::name},
		//{ExpressionInliner::name},
		//{ExpressionJoiner::name},
		//{ExpressionSimplifier::name},
		//{ExpressionSplitter::name},
		//{ForLoopConditionIntoBody::name},
		//{ForLoopConditionOutOfBody::name},
		//{ForLoopInitRewriter::name},
		//{FullInliner::name},
		//{FunctionGrouper::name},
		//{FunctionHoister::name},
		//{LiteralRematerialiser::name},
		//{LoadResolver::name},
		//{LoopInvariantCodeMotion::name},
		//{RedundantAssignEliminator::name},
		//{Rematerialiser::name},
		//{SSAReverser::name},
		//{SSATransform::name},
		//{StructuralSimplifier::name},
		//{UnusedPruner::name},
		//{VarDeclInitializer::name},
	});

	return stepNames[uniformRandomInt(0, stepNames.size() - 1)];
}
