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

#include <tools/yulPhaser/GeneticAlgorithms.h>
#include <tools/yulPhaser/Mutations.h>
#include <tools/yulPhaser/Selections.h>
#include <tools/yulPhaser/PairSelections.h>

using namespace std;
using namespace solidity::phaser;

Population RandomAlgorithm::runNextRound(Population _population)
{
	RangeSelection elite(0.0, m_options.elitePoolSize);

	Population elitePopulation = _population.select(elite);
	size_t replacementCount = _population.individuals().size() - elitePopulation.individuals().size();

	return
		move(elitePopulation) +
		Population::makeRandom(
			_population.fitnessMetric(),
			replacementCount,
			m_options.minChromosomeLength,
			m_options.maxChromosomeLength
		);
}

Population GenerationalElitistWithExclusivePools::runNextRound(Population _population)
{
	double elitePoolSize = 1.0 - (m_options.mutationPoolSize + m_options.crossoverPoolSize);
	RangeSelection elite(0.0, elitePoolSize);

	return
		_population.select(elite) +
		_population.select(elite).mutate(
			RandomSelection(m_options.mutationPoolSize / elitePoolSize),
			alternativeMutations(
				m_options.randomisationChance,
				geneRandomisation(m_options.percentGenesToRandomise),
				alternativeMutations(
					m_options.deletionVsAdditionChance,
					geneDeletion(m_options.percentGenesToAddOrDelete),
					geneAddition(m_options.percentGenesToAddOrDelete)
				)
			)
		) +
		_population.select(elite).crossover(
			RandomPairSelection(m_options.crossoverPoolSize / elitePoolSize),
			randomPointCrossover()
		);
}
