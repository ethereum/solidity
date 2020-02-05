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
#include <tools/yulPhaser/Program.h>
#include <tools/yulPhaser/SimulationRNG.h>

#include <optional>
#include <ostream>
#include <vector>

namespace solidity::phaser
{

class Population;

}

// This operator+ must be declared in the global namespace. Otherwise it would shadow global
// operator+ overloads from CommonData.h (e.g. the one for vector) in the namespace it was declared in.
solidity::phaser::Population operator+(solidity::phaser::Population _a, solidity::phaser::Population _b);

namespace solidity::phaser
{

/**
 * Information describing the state of an individual member of the population during the course
 * of the genetic algorithm.
 */
struct Individual
{
	Chromosome chromosome;
	std::optional<size_t> fitness = std::nullopt;

	bool operator==(Individual const& _other) const { return fitness == _other.fitness && chromosome == _other.chromosome; }
	bool operator!=(Individual const& _other) const { return !(*this == _other); }

	friend std::ostream& operator<<(std::ostream& _stream, Individual const& _individual);
};

/// Determines which individual is better by comparing fitness values. If fitness is the same
/// takes into account all the other properties of the individual to make the comparison
/// deterministic as long as the individuals are not equal.
bool isFitter(Individual const& a, Individual const& b);

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

	explicit Population(Program _program, std::vector<Chromosome> _chromosomes = {}):
		Population(
			std::move(_program),
			chromosomesToIndividuals(std::move(_chromosomes))
		) {}

	static Population makeRandom(
		Program _program,
		size_t _size,
		std::function<size_t()> _chromosomeLengthGenerator
	);
	static Population makeRandom(
		Program _program,
		size_t _size,
		size_t _minChromosomeLength,
		size_t _maxChromosomeLength
	);

	void run(std::optional<size_t> _numRounds, std::ostream& _outputStream);
	friend Population (::operator+)(Population _a, Population _b);

	std::vector<Individual> const& individuals() const { return m_individuals; }

	static size_t uniformChromosomeLength(size_t _min, size_t _max) { return SimulationRNG::uniformInt(_min, _max); }
	static size_t binomialChromosomeLength(size_t _max) { return SimulationRNG::binomialInt(_max, 0.5); }
	static size_t measureFitness(Chromosome const& _chromosome, Program const& _program);

	bool operator==(Population const& _other) const;
	bool operator!=(Population const& _other) const { return !(*this == _other); }

	friend std::ostream& operator<<(std::ostream& _stream, Population const& _population);

private:
	explicit Population(Program _program, std::vector<Individual> _individuals):
		m_program{std::move(_program)},
		m_individuals{std::move(_individuals)} {}

	void doMutation();
	void doEvaluation();
	void doSelection();

	static void randomizeWorstChromosomes(
		std::vector<Individual>& _individuals,
		size_t _count
	);
	static std::vector<Individual> chromosomesToIndividuals(
		std::vector<Chromosome> _chromosomes
	);
	static std::vector<Individual> sortedIndividuals(std::vector<Individual> _individuals);

	Program m_program;
	std::vector<Individual> m_individuals;
};

}
