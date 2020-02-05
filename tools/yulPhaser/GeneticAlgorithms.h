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
/**
 * Contains an abstract base class representing a genetic algorithm and its concrete implementations.
 */

#pragma once

#include <tools/yulPhaser/Population.h>

#include <optional>
#include <ostream>

namespace solidity::phaser
{

/**
 * Abstract base class for genetic algorithms.
 *
 * The main feature is the @a run() method that executes the algorithm, updating the internal
 * population during each round and printing the results to the stream provided to the constructor.
 *
 * Derived classes can provide specific methods for updating the population by implementing
 * the @a runNextRound() method.
 */
class GeneticAlgorithm
{
public:
	GeneticAlgorithm(Population _initialPopulation, std::ostream& _outputStream):
		m_population(std::move(_initialPopulation)),
		m_outputStream(_outputStream) {}

	GeneticAlgorithm(GeneticAlgorithm const&) = delete;
	GeneticAlgorithm& operator=(GeneticAlgorithm const&) = delete;
	virtual ~GeneticAlgorithm() = default;

	Population const& population() const { return m_population; }

	void run(std::optional<size_t> _numRounds = std::nullopt);

	/// The method that actually implements the algorithm. Should use @a m_population as input and
	/// replace it with the updated state after the round.
	virtual void runNextRound() = 0;

protected:
	Population m_population;

private:
	std::ostream& m_outputStream;
};

}
