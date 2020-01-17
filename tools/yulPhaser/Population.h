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
	std::optional<size_t> fitness = std::nullopt;

	friend std::ostream& operator<<(std::ostream& _stream, Individual const& _individual);
};

/**
 * Represents a changing set of individuals undergoing a genetic algorithm.
 * Each round of the algorithm involves mutating existing individuals, evaluating their fitness
 * and selecting the best ones for the next round.
 *
 * An individual is a sequence of optimiser steps represented by a @a Chromosome instance. The whole
 * population is associated with a fixed Yul program. By loading the source code into a @a Program
 * instance the class can compute fitness of the individual.
 */
class Population
{
public:
	static constexpr size_t MaxChromosomeLength = 30;

	explicit Population(std::string const& _sourcePath, std::vector<Chromosome> const& _chromosomes = {});
	static Population makeRandom(std::string const& _sourcePath, size_t _size);

	void run(std::optional<size_t> _numRounds, std::ostream& _outputStream);

	std::vector<Individual> const& individuals() const { return m_individuals; }

	static size_t randomChromosomeLength() { return binomialRandomInt(MaxChromosomeLength, 0.5); }
	static size_t measureFitness(Chromosome const& _chromosome, std::string const& _sourcePath);

	friend std::ostream& operator<<(std::ostream& _stream, Population const& _population);

private:
	explicit Population(std::string const& _sourcePath, std::vector<Individual> _individuals = {}):
		m_sourcePath{_sourcePath},
		m_individuals{std::move(_individuals)} {}

	void doMutation();
	void doEvaluation();
	void doSelection();

	static void randomizeWorstChromosomes(
		std::vector<Individual>& _individuals,
		size_t _count
	);

	std::string m_sourcePath;

	std::vector<Individual> m_individuals;
};

}
