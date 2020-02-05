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

/**
 * Completely random genetic algorithm,
 *
 * The algorithm simply replaces the worst chromosomes with entirely new ones, generated
 * randomly and not based on any member of the current population. Only a constant proportion of the
 * chromosomes (the elite) is preserved in each round.
 *
 * Preserves the size of the population. You can use @a elitePoolSize to make the algorithm
 * generational (replacing most members in each round) or steady state (replacing only one member).
 * Both versions are equivalent in terms of the outcome but the generational one converges in a
 * smaller number of rounds while the steady state one does less work per round. This may matter
 * in case of metrics that take a long time to compute though in case of this particular
 * algorithm the same result could also be achieved by simply making the population smaller.
 */
class RandomAlgorithm: public GeneticAlgorithm
{
public:
	struct Options
	{
		double elitePoolSize;        ///< Percentage of the population treated as the elite
		size_t minChromosomeLength;  ///< Minimum length of newly generated chromosomes
		size_t maxChromosomeLength;  ///< Maximum length of newly generated chromosomes

		bool isValid() const
		{
			return (
				0 <= elitePoolSize && elitePoolSize <= 1.0 &&
				minChromosomeLength <= maxChromosomeLength
			);
		}
	};

	explicit RandomAlgorithm(
		Population _initialPopulation,
		std::ostream& _outputStream,
		Options const& _options
	):
		GeneticAlgorithm(_initialPopulation, _outputStream),
		m_options(_options)
	{
		assert(_options.isValid());
	}

	void runNextRound() override;

private:
	Options m_options;
};

}
