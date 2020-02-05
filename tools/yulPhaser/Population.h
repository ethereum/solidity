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

#pragma once

#include <tools/yulPhaser/Chromosome.h>
#include <tools/yulPhaser/FitnessMetrics.h>
#include <tools/yulPhaser/Random.h>

#include <optional>
#include <ostream>
#include <vector>

namespace solidity::phaser
{

/**
 * Information describing the state of an individual member of the population during the course
 * of the genetic algorithm.
 */
struct Individual
{
	Chromosome chromosome;
	size_t fitness;

	friend std::ostream& operator<<(std::ostream& _stream, Individual const& _individual);
};

/**
 * Represents a changing set of individuals undergoing a genetic algorithm.
 * Each round of the algorithm involves mutating existing individuals, evaluating their fitness
 * and selecting the best ones for the next round.
 *
 * An individual is a sequence of optimiser steps represented by a @a Chromosome instance. The whole
 * population is associated with a fixed Yul program. By applying the steps to the @a Program
 * instance the class can compute fitness of the individual.
 */
class Population
{
public:
	static constexpr size_t MaxChromosomeLength = 30;

	explicit Population(
		std::shared_ptr<FitnessMetric const> _fitnessMetric,
		std::vector<Chromosome> _chromosomes = {}
	):
		Population(
			std::move(_fitnessMetric),
			chromosomesToIndividuals(*_fitnessMetric, std::move(_chromosomes))
		) {}
	static Population makeRandom(std::shared_ptr<FitnessMetric const> _fitnessMetric, size_t _size);

	void run(std::optional<size_t> _numRounds, std::ostream& _outputStream);

	std::shared_ptr<FitnessMetric const> fitnessMetric() const { return m_fitnessMetric; }
	std::vector<Individual> const& individuals() const { return m_individuals; }

	static size_t randomChromosomeLength() { return binomialRandomInt(MaxChromosomeLength, 0.5); }

	friend std::ostream& operator<<(std::ostream& _stream, Population const& _population);

private:
	explicit Population(std::shared_ptr<FitnessMetric const> _fitnessMetric, std::vector<Individual> _individuals):
		m_fitnessMetric(std::move(_fitnessMetric)),
		m_individuals{std::move(_individuals)} {}

	void doMutation();
	void doSelection();

	static void randomizeWorstChromosomes(
		FitnessMetric const& _fitnessMetric,
		std::vector<Individual>& _individuals,
		size_t _count
	);
	std::vector<Individual> chromosomesToIndividuals(
		FitnessMetric const& _fitnessMetric,
		std::vector<Chromosome> _chromosomes
	);
	std::vector<Individual> sortIndividuals(std::vector<Individual> _individuals);

	std::shared_ptr<FitnessMetric const> m_fitnessMetric;
	std::vector<Individual> m_individuals;
};

}
