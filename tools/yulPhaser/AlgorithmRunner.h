// SPDX-License-Identifier: GPL-3.0
/**
 * Contains the implementation of a class that manages the execution of a genetic algorithm.
 */

#pragma once

#include <tools/yulPhaser/GeneticAlgorithms.h>
#include <tools/yulPhaser/Population.h>
#include <tools/yulPhaser/ProgramCache.h>

#include <ctime>
#include <optional>
#include <ostream>

namespace solidity::phaser
{

/**
 * Manages a population and executes a genetic algorithm on it. It's independent of the
 * implementation details of a specific algorithm which is pluggable via @a GeneticAlgorithm class.
 *
 * The class is also responsible for providing text feedback on the execution of the algorithm
 * to the associated output stream.
 */
class AlgorithmRunner
{
public:
	struct Options
	{
		std::optional<size_t> maxRounds = std::nullopt;
		std::optional<std::string> populationAutosaveFile = std::nullopt;
		bool randomiseDuplicates = false;
		std::optional<size_t> minChromosomeLength = std::nullopt;
		std::optional<size_t> maxChromosomeLength = std::nullopt;
		bool showInitialPopulation = false;
		bool showOnlyTopChromosome = false;
		bool showRoundInfo = true;
		bool showCacheStats = false;
	};

	AlgorithmRunner(
		Population _initialPopulation,
		std::vector<std::shared_ptr<ProgramCache>> _programCaches,
		Options _options,
		std::ostream& _outputStream
	):
		m_population(std::move(_initialPopulation)),
		m_programCaches(std::move(_programCaches)),
		m_options(std::move(_options)),
		m_outputStream(_outputStream) {}

	void run(GeneticAlgorithm& _algorithm);

	Options const& options() const { return m_options; }
	Population const& population() const { return m_population; }

private:
	void printRoundSummary(
		size_t _round,
		clock_t _roundTimeStart,
		clock_t _totalTimeStart
	) const;
	void printInitialPopulation() const;
	void printCacheStats() const;
	void populationAutosave() const;
	void randomiseDuplicates();
	void cacheClear();
	void cacheStartRound(size_t _roundNumber);

	static Population randomiseDuplicates(
		Population _population,
		size_t _minChromosomeLength,
		size_t _maxChromosomeLength
	);

	Population m_population;
	std::vector<std::shared_ptr<ProgramCache>> m_programCaches;
	Options m_options;
	std::ostream& m_outputStream;
};

}
