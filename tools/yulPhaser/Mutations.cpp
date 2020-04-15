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

#include <tools/yulPhaser/Mutations.h>

#include <tools/yulPhaser/SimulationRNG.h>

#include <libsolutil/CommonData.h>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <string>
#include <vector>

using namespace std;
using namespace solidity;
using namespace solidity::phaser;

function<Mutation> phaser::geneRandomisation(double _chance)
{
	return [=](Chromosome const& _chromosome)
	{
		vector<string> optimisationSteps;
		for (auto const& step: _chromosome.optimisationSteps())
			optimisationSteps.push_back(
				SimulationRNG::bernoulliTrial(_chance) ?
				Chromosome::randomOptimisationStep() :
				step
			);

		return Chromosome(move(optimisationSteps));
	};
}

function<Mutation> phaser::geneDeletion(double _chance)
{
	return [=](Chromosome const& _chromosome)
	{
		vector<string> optimisationSteps;
		for (auto const& step: _chromosome.optimisationSteps())
			if (!SimulationRNG::bernoulliTrial(_chance))
				optimisationSteps.push_back(step);

		return Chromosome(move(optimisationSteps));
	};
}

function<Mutation> phaser::geneAddition(double _chance)
{
	return [=](Chromosome const& _chromosome)
	{
		vector<string> optimisationSteps;

		if (SimulationRNG::bernoulliTrial(_chance))
			optimisationSteps.push_back(Chromosome::randomOptimisationStep());

		for (auto const& step: _chromosome.optimisationSteps())
		{
			optimisationSteps.push_back(step);
			if (SimulationRNG::bernoulliTrial(_chance))
				optimisationSteps.push_back(Chromosome::randomOptimisationStep());
		}

		return Chromosome(move(optimisationSteps));
	};
}

function<Mutation> phaser::alternativeMutations(
	double _firstMutationChance,
	function<Mutation> _mutation1,
	function<Mutation> _mutation2
)
{
	return [=](Chromosome const& _chromosome)
	{
		if (SimulationRNG::bernoulliTrial(_firstMutationChance))
			return _mutation1(_chromosome);
		else
			return _mutation2(_chromosome);
	};
}

function<Mutation> phaser::mutationSequence(vector<function<Mutation>> _mutations)
{
	return [=](Chromosome const& _chromosome)
	{
		Chromosome mutatedChromosome = _chromosome;
		for (size_t i = 0; i < _mutations.size(); ++i)
			mutatedChromosome = _mutations[i](move(mutatedChromosome));

		return mutatedChromosome;
	};
}

namespace
{

ChromosomePair fixedPointSwap(
	Chromosome const& _chromosome1,
	Chromosome const& _chromosome2,
	size_t _crossoverPoint
)
{
	assert(_crossoverPoint <= _chromosome1.length());
	assert(_crossoverPoint <= _chromosome2.length());

	auto begin1 = _chromosome1.optimisationSteps().begin();
	auto begin2 = _chromosome2.optimisationSteps().begin();
	auto end1 = _chromosome1.optimisationSteps().end();
	auto end2 = _chromosome2.optimisationSteps().end();

	return {
		Chromosome(
			vector<string>(begin1, begin1 + _crossoverPoint) +
			vector<string>(begin2 + _crossoverPoint, end2)
		),
		Chromosome(
			vector<string>(begin2, begin2 + _crossoverPoint) +
			vector<string>(begin1 + _crossoverPoint, end1)
		),
	};
}

}

function<Crossover> phaser::randomPointCrossover()
{
	return [=](Chromosome const& _chromosome1, Chromosome const& _chromosome2)
	{
		size_t minLength = min(_chromosome1.length(), _chromosome2.length());

		// Don't use position 0 (because this just swaps the values) unless it's the only choice.
		size_t minPoint = (minLength > 0? 1 : 0);
		assert(minPoint <= minLength);

		size_t randomPoint = SimulationRNG::uniformInt(minPoint, minLength);
		return get<0>(fixedPointSwap(_chromosome1, _chromosome2, randomPoint));
	};
}

function<SymmetricCrossover> phaser::symmetricRandomPointCrossover()
{
	return [=](Chromosome const& _chromosome1, Chromosome const& _chromosome2)
	{
		size_t minLength = min(_chromosome1.length(), _chromosome2.length());

		// Don't use position 0 (because this just swaps the values) unless it's the only choice.
		size_t minPoint = (minLength > 0? 1 : 0);
		assert(minPoint <= minLength);

		size_t randomPoint = SimulationRNG::uniformInt(minPoint, minLength);
		return fixedPointSwap(_chromosome1, _chromosome2, randomPoint);
	};
}

function<Crossover> phaser::fixedPointCrossover(double _crossoverPoint)
{
	assert(0.0 <= _crossoverPoint && _crossoverPoint <= 1.0);

	return [=](Chromosome const& _chromosome1, Chromosome const& _chromosome2)
	{
		size_t minLength = min(_chromosome1.length(), _chromosome2.length());
		size_t concretePoint = static_cast<size_t>(round(minLength * _crossoverPoint));

		return get<0>(fixedPointSwap(_chromosome1, _chromosome2, concretePoint));
	};
}
