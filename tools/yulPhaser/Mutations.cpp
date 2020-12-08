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
		string genes;
		for (char gene: _chromosome.genes())
			genes.push_back(
				SimulationRNG::bernoulliTrial(_chance) ?
				Chromosome::randomGene() :
				gene
			);

		return Chromosome(move(genes));
	};
}

function<Mutation> phaser::geneDeletion(double _chance)
{
	return [=](Chromosome const& _chromosome)
	{
		string genes;
		for (char gene: _chromosome.genes())
			if (!SimulationRNG::bernoulliTrial(_chance))
				genes.push_back(gene);

		return Chromosome(move(genes));
	};
}

function<Mutation> phaser::geneAddition(double _chance)
{
	return [=](Chromosome const& _chromosome)
	{
		string genes;

		if (SimulationRNG::bernoulliTrial(_chance))
			genes.push_back(Chromosome::randomGene());

		for (char gene: _chromosome.genes())
		{
			genes.push_back(gene);
			if (SimulationRNG::bernoulliTrial(_chance))
				genes.push_back(Chromosome::randomGene());
		}

		return Chromosome(move(genes));
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

	return {
		Chromosome(
			_chromosome1.genes().substr(0, _crossoverPoint) +
			_chromosome2.genes().substr(_crossoverPoint, _chromosome2.length() - _crossoverPoint)
		),
		Chromosome(
			_chromosome2.genes().substr(0, _crossoverPoint) +
			_chromosome1.genes().substr(_crossoverPoint, _chromosome1.length() - _crossoverPoint)
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
		size_t concretePoint = static_cast<size_t>(round(double(minLength) * _crossoverPoint));

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

	return {
		Chromosome(
			_chromosome1.genes().substr(0, lowPoint) +
			_chromosome2.genes().substr(lowPoint, highPoint - lowPoint) +
			_chromosome1.genes().substr(highPoint, _chromosome1.length() - highPoint)
		),
		Chromosome(
			_chromosome2.genes().substr(0, lowPoint) +
			_chromosome1.genes().substr(lowPoint, highPoint - lowPoint) +
			_chromosome2.genes().substr(highPoint, _chromosome2.length() - highPoint)
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
	string steps1;
	string steps2;

	size_t minLength = min(_chromosome1.length(), _chromosome2.length());
	for (size_t i = 0; i < minLength; ++i)
		if (SimulationRNG::bernoulliTrial(_swapChance))
		{
			steps1.push_back(_chromosome2.genes()[i]);
			steps2.push_back(_chromosome1.genes()[i]);
		}
		else
		{
			steps1.push_back(_chromosome1.genes()[i]);
			steps2.push_back(_chromosome2.genes()[i]);
		}

	bool swapTail = SimulationRNG::bernoulliTrial(_swapChance);
	if (_chromosome1.length() > minLength)
	{
		if (swapTail)
			steps2 += _chromosome1.genes().substr(minLength, _chromosome1.length() - minLength);
		else
			steps1 += _chromosome1.genes().substr(minLength, _chromosome1.length() - minLength);
	}

	if (_chromosome2.length() > minLength)
	{
		if (swapTail)
			steps1 += _chromosome2.genes().substr(minLength, _chromosome2.length() - minLength);
		else
			steps2 += _chromosome2.genes().substr(minLength, _chromosome2.length() - minLength);
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
