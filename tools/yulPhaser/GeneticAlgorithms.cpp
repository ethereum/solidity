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

#include <tools/yulPhaser/GeneticAlgorithms.h>
#include <tools/yulPhaser/Mutations.h>
#include <tools/yulPhaser/Selections.h>
#include <tools/yulPhaser/PairSelections.h>

using namespace std;
using namespace solidity;
using namespace solidity::phaser;

function<Crossover> phaser::buildCrossoverOperator(
	CrossoverChoice _choice,
	optional<double> _uniformCrossoverSwapChance
)
{
	switch (_choice)
	{
		case CrossoverChoice::SinglePoint:
			return randomPointCrossover();
		case CrossoverChoice::TwoPoint:
			return randomTwoPointCrossover();
		case CrossoverChoice::Uniform:
			assert(_uniformCrossoverSwapChance.has_value());
			return uniformCrossover(_uniformCrossoverSwapChance.value());
		default:
			assertThrow(false, solidity::util::Exception, "Invalid CrossoverChoice value.");
	};
}

function<SymmetricCrossover> phaser::buildSymmetricCrossoverOperator(
	CrossoverChoice _choice,
	optional<double> _uniformCrossoverSwapChance
)
{
	switch (_choice)
	{
		case CrossoverChoice::SinglePoint:
			return symmetricRandomPointCrossover();
		case CrossoverChoice::TwoPoint:
			return symmetricRandomTwoPointCrossover();
		case CrossoverChoice::Uniform:
			assert(_uniformCrossoverSwapChance.has_value());
			return symmetricUniformCrossover(_uniformCrossoverSwapChance.value());
		default:
			assertThrow(false, solidity::util::Exception, "Invalid CrossoverChoice value.");
	};
}

Population RandomAlgorithm::runNextRound(Population _population)
{
	RangeSelection elite(0.0, m_options.elitePoolSize);

	Population elitePopulation = _population.select(elite);
	size_t replacementCount = _population.individuals().size() - elitePopulation.individuals().size();

	return
		std::move(elitePopulation) +
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

	RangeSelection elitePool(0.0, elitePoolSize);
	RandomSelection mutationPoolFromElite(m_options.mutationPoolSize / elitePoolSize);
	RandomPairSelection crossoverPoolFromElite(m_options.crossoverPoolSize / elitePoolSize);

	std::function<Mutation> mutationOperator = alternativeMutations(
		m_options.randomisationChance,
		geneRandomisation(m_options.percentGenesToRandomise),
		alternativeMutations(
			m_options.deletionVsAdditionChance,
			geneDeletion(m_options.percentGenesToAddOrDelete),
			geneAddition(m_options.percentGenesToAddOrDelete)
		)
	);
	std::function<Crossover> crossoverOperator = buildCrossoverOperator(
		m_options.crossover,
		m_options.uniformCrossoverSwapChance
	);

	return
		_population.select(elitePool) +
		_population.select(elitePool).mutate(mutationPoolFromElite, mutationOperator) +
		_population.select(elitePool).crossover(crossoverPoolFromElite, crossoverOperator);
}

Population ClassicGeneticAlgorithm::runNextRound(Population _population)
{
	Population elite = _population.select(RangeSelection(0.0, m_options.elitePoolSize));
	Population rest = _population.select(RangeSelection(m_options.elitePoolSize, 1.0));

	Population selectedPopulation = select(_population, rest.individuals().size());

	std::function<SymmetricCrossover> crossoverOperator = buildSymmetricCrossoverOperator(
		m_options.crossover,
		m_options.uniformCrossoverSwapChance
	);

	Population crossedPopulation = Population::combine(
		selectedPopulation.symmetricCrossoverWithRemainder(
			PairsFromRandomSubset(m_options.crossoverChance),
			crossoverOperator
		)
	);

	std::function<Mutation> mutationOperator = mutationSequence({
		geneRandomisation(m_options.mutationChance),
		geneDeletion(m_options.deletionChance),
		geneAddition(m_options.additionChance),
	});

	RangeSelection all(0.0, 1.0);
	Population mutatedPopulation = crossedPopulation.mutate(all, mutationOperator);

	return elite + mutatedPopulation;
}

Population ClassicGeneticAlgorithm::select(Population _population, size_t _selectionSize)
{
	if (_population.individuals().size() == 0)
		return _population;

	size_t maxFitness = 0;
	for (auto const& individual: _population.individuals())
		maxFitness = max(maxFitness, individual.fitness);

	size_t rouletteRange = 0;
	for (auto const& individual: _population.individuals())
		// Add 1 to make sure that every chromosome has non-zero probability of being chosen
		rouletteRange += maxFitness + 1 - individual.fitness;

	vector<Individual> selectedIndividuals;
	for (size_t i = 0; i < _selectionSize; ++i)
	{
		size_t ball = SimulationRNG::uniformInt(0, rouletteRange - 1);

		size_t cumulativeFitness = 0;
		for (auto const& individual: _population.individuals())
		{
			size_t pocketSize = maxFitness + 1 - individual.fitness;
			if (ball < cumulativeFitness + pocketSize)
			{
				selectedIndividuals.push_back(individual);
				break;
			}
			cumulativeFitness += pocketSize;
		}
	}

	assert(selectedIndividuals.size() == _selectionSize);
	return Population(_population.fitnessMetric(), selectedIndividuals);
}
