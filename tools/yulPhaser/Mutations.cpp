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
		size_t minPoint = (minLength > 0 ? 1 : 0);
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
		size_t minPoint = (minLength > 0 ? 1 : 0);
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

namespace
{

ChromosomePair fixedTwoPointSwap(
	Chromosome const& _chromosome1,
	Chromosome const& _chromosome2,
	size_t _crossoverPoint1,
	size_t _crossoverPoint2
)
{
	assert(_crossoverPoint1 <= _chromosome1.length());
	assert(_crossoverPoint1 <= _chromosome2.length());
	assert(_crossoverPoint2 <= _chromosome1.length());
	assert(_crossoverPoint2 <= _chromosome2.length());

	size_t lowPoint = min(_crossoverPoint1, _crossoverPoint2);
	size_t highPoint = max(_crossoverPoint1, _crossoverPoint2);

	auto begin1 = _chromosome1.optimisationSteps().begin();
	auto begin2 = _chromosome2.optimisationSteps().begin();
	auto end1 = _chromosome1.optimisationSteps().end();
	auto end2 = _chromosome2.optimisationSteps().end();

	return {
		Chromosome(
			vector<string>(begin1, begin1 + lowPoint) +
			vector<string>(begin2 + lowPoint, begin2 + highPoint) +
			vector<string>(begin1 + highPoint, end1)
		),
		Chromosome(
			vector<string>(begin2, begin2 + lowPoint) +
			vector<string>(begin1 + lowPoint, begin1 + highPoint) +
			vector<string>(begin2 + highPoint, end2)
		),
	};
}

}

function<Crossover> phaser::randomTwoPointCrossover()
{
	return [=](Chromosome const& _chromosome1, Chromosome const& _chromosome2)
	{
		size_t minLength = min(_chromosome1.length(), _chromosome2.length());

		// Don't use position 0 (because this just swaps the values) unless it's the only choice.
		size_t minPoint = (minLength > 0 ? 1 : 0);
		assert(minPoint <= minLength);

		size_t randomPoint1 = SimulationRNG::uniformInt(minPoint, minLength);
		size_t randomPoint2 = SimulationRNG::uniformInt(randomPoint1, minLength);
		return get<0>(fixedTwoPointSwap(_chromosome1, _chromosome2, randomPoint1, randomPoint2));
	};
}

function<SymmetricCrossover> phaser::symmetricRandomTwoPointCrossover()
{
	return [=](Chromosome const& _chromosome1, Chromosome const& _chromosome2)
	{
		size_t minLength = min(_chromosome1.length(), _chromosome2.length());

		// Don't use position 0 (because this just swaps the values) unless it's the only choice.
		size_t minPoint = (minLength > 0 ? 1 : 0);
		assert(minPoint <= minLength);

		size_t randomPoint1 = SimulationRNG::uniformInt(minPoint, minLength);
		size_t randomPoint2 = SimulationRNG::uniformInt(randomPoint1, minLength);
		return fixedTwoPointSwap(_chromosome1, _chromosome2, randomPoint1, randomPoint2);
	};
}

namespace
{

ChromosomePair uniformSwap(Chromosome const& _chromosome1, Chromosome const& _chromosome2, double _swapChance)
{
	vector<string> steps1;
	vector<string> steps2;

	size_t minLength = min(_chromosome1.length(), _chromosome2.length());
	for (size_t i = 0; i < minLength; ++i)
		if (SimulationRNG::bernoulliTrial(_swapChance))
		{
			steps1.push_back(_chromosome2.optimisationSteps()[i]);
			steps2.push_back(_chromosome1.optimisationSteps()[i]);
		}
		else
		{
			steps1.push_back(_chromosome1.optimisationSteps()[i]);
			steps2.push_back(_chromosome2.optimisationSteps()[i]);
		}

	auto begin1 = _chromosome1.optimisationSteps().begin();
	auto begin2 = _chromosome2.optimisationSteps().begin();
	auto end1 = _chromosome1.optimisationSteps().end();
	auto end2 = _chromosome2.optimisationSteps().end();

	bool swapTail = SimulationRNG::bernoulliTrial(_swapChance);
	if (_chromosome1.length() > minLength)
	{
		if (swapTail)
			steps2.insert(steps2.end(), begin1 + minLength, end1);
		else
			steps1.insert(steps1.end(), begin1 + minLength, end1);
	}

	if (_chromosome2.length() > minLength)
	{
		if (swapTail)
			steps1.insert(steps1.end(), begin2 + minLength, end2);
		else
			steps2.insert(steps2.end(), begin2 + minLength, end2);
	}

	return {Chromosome(steps1), Chromosome(steps2)};
}

}

function<Crossover> phaser::uniformCrossover(double _swapChance)
{
	return [=](Chromosome const& _chromosome1, Chromosome const& _chromosome2)
	{
		return get<0>(uniformSwap(_chromosome1, _chromosome2, _swapChance));
	};
}

function<SymmetricCrossover> phaser::symmetricUniformCrossover(double _swapChance)
{
	return [=](Chromosome const& _chromosome1, Chromosome const& _chromosome2)
	{
		return uniformSwap(_chromosome1, _chromosome2, _swapChance);
	};
}
