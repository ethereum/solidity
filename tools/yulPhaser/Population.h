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

#pragma once

#include <tools/yulPhaser/Chromosome.h>
#include <tools/yulPhaser/FitnessMetrics.h>
#include <tools/yulPhaser/Mutations.h>
#include <tools/yulPhaser/SimulationRNG.h>

#include <cstddef>
#include <optional>
#include <ostream>
#include <vector>

namespace solidity::phaser
{

class PairSelection;
class Selection;

/**
 * Information describing the state of an individual member of the population during the course
 * of the genetic algorithm.
 */
struct Individual
{
	Chromosome chromosome;
	size_t fitness;

	Individual(Chromosome _chromosome, size_t _fitness):
		chromosome(std::move(_chromosome)),
		fitness(_fitness) {}
	Individual(Chromosome _chromosome, FitnessMetric& _fitnessMetric):
		chromosome(std::move(_chromosome)),
		fitness(_fitnessMetric.evaluate(chromosome)) {}

	bool operator==(Individual const& _other) const { return fitness == _other.fitness && chromosome == _other.chromosome; }
	bool operator!=(Individual const& _other) const { return !(*this == _other); }

	friend std::ostream& operator<<(std::ostream& _stream, Individual const& _individual);
};

/// Determines which individual is better by comparing fitness values. If fitness is the same
/// takes into account all the other properties of the individual to make the comparison
/// deterministic as long as the individuals are not equal.
bool isFitter(Individual const& a, Individual const& b);

/**
 * Represents a snapshot of a population undergoing a genetic algorithm. Consists of a set of
 * chromosomes with associated fitness values.
 *
 * An individual is a sequence of optimiser steps represented by a @a Chromosome instance.
 * Individuals are always ordered by their fitness (based on @_fitnessMetric and @a isFitter()).
 * The fitness is computed using the metric as soon as an individual is inserted into the population.
 *
 * The population is immutable. Selections, mutations and crossover work by producing a new
 * instance and copying the individuals.
 */
class Population
{
public:
	explicit Population(
		std::shared_ptr<FitnessMetric> _fitnessMetric,
		std::vector<Chromosome> _chromosomes = {}
	):
		Population(
			_fitnessMetric,
			chromosomesToIndividuals(*_fitnessMetric, std::move(_chromosomes))
		) {}
	explicit Population(std::shared_ptr<FitnessMetric> _fitnessMetric, std::vector<Individual> _individuals):
		m_fitnessMetric(std::move(_fitnessMetric)),
		m_individuals{sortedIndividuals(std::move(_individuals))} {}

	static Population makeRandom(
		std::shared_ptr<FitnessMetric> _fitnessMetric,
		size_t _size,
		std::function<size_t()> _chromosomeLengthGenerator
	);
	static Population makeRandom(
		std::shared_ptr<FitnessMetric> _fitnessMetric,
		size_t _size,
		size_t _minChromosomeLength,
		size_t _maxChromosomeLength
	);

	Population select(Selection const& _selection) const;
	Population mutate(Selection const& _selection, std::function<Mutation> _mutation) const;
	Population crossover(PairSelection const& _selection, std::function<Crossover> _crossover) const;
	std::tuple<Population, Population> symmetricCrossoverWithRemainder(
		PairSelection const& _selection,
		std::function<SymmetricCrossover> _symmetricCrossover
	) const;

	friend Population operator+(Population _a, Population _b);
	static Population combine(std::tuple<Population, Population> _populationPair);

	std::shared_ptr<FitnessMetric> fitnessMetric() { return m_fitnessMetric; }
	std::vector<Individual> const& individuals() const { return m_individuals; }

	static size_t uniformChromosomeLength(size_t _min, size_t _max) { return SimulationRNG::uniformInt(_min, _max); }
	static size_t binomialChromosomeLength(size_t _max) { return SimulationRNG::binomialInt(_max, 0.5); }

	bool operator==(Population const& _other) const;
	bool operator!=(Population const& _other) const { return !(*this == _other); }

	friend std::ostream& operator<<(std::ostream& _stream, Population const& _population);

private:
	static std::vector<Individual> chromosomesToIndividuals(
		FitnessMetric& _fitnessMetric,
		std::vector<Chromosome> _chromosomes
	);
	static std::vector<Individual> sortedIndividuals(std::vector<Individual> _individuals);

	std::shared_ptr<FitnessMetric> m_fitnessMetric;
	std::vector<Individual> m_individuals;
};

}
