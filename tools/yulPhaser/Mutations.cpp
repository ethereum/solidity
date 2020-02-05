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
