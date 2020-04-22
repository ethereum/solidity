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

#include <tools/yulPhaser/AlgorithmRunner.h>

#include <tools/yulPhaser/Exceptions.h>

#include <libsolutil/Assertions.h>

#include <cerrno>
#include <cstring>
#include <fstream>

using namespace std;
using namespace solidity::phaser;

void AlgorithmRunner::run(GeneticAlgorithm& _algorithm)
{
	populationAutosave();
	printInitialPopulation();
	cacheClear();

	clock_t totalTimeStart = clock();
	for (size_t round = 0; !m_options.maxRounds.has_value() || round < m_options.maxRounds.value(); ++round)
	{
		clock_t roundTimeStart = clock();
		cacheStartRound(round + 1);

		m_population = _algorithm.runNextRound(m_population);
		randomiseDuplicates();

		printRoundSummary(round, roundTimeStart, totalTimeStart);
		printCacheStats();
		populationAutosave();
	}
}

void AlgorithmRunner::printRoundSummary(
	size_t _round,
	clock_t _roundTimeStart,
	clock_t _totalTimeStart
) const
{
	clock_t now = clock();
	double roundTime = static_cast<double>(now - _roundTimeStart) / CLOCKS_PER_SEC;
	double totalTime = static_cast<double>(now - _totalTimeStart) / CLOCKS_PER_SEC;

	if (!m_options.showOnlyTopChromosome)
	{
		if (m_options.showRoundInfo)
		{
			m_outputStream << "---------- ROUND " << _round + 1;
			m_outputStream << " [round: " << fixed << setprecision(1) << roundTime << " s,";
			m_outputStream << " total: " << fixed << setprecision(1) << totalTime << " s]";
			m_outputStream << " ----------" << endl;
		}
		else if (m_population.individuals().size() > 0)
			m_outputStream << endl;

		m_outputStream << m_population;
	}
	else if (m_population.individuals().size() > 0)
	{
		if (m_options.showRoundInfo)
		{
			m_outputStream << setw(5) << _round + 1 << " | ";
			m_outputStream << setw(5) << fixed << setprecision(1) << totalTime << " | ";
		}

		m_outputStream << m_population.individuals()[0] << endl;
	}
}

void AlgorithmRunner::printInitialPopulation() const
{
	if (!m_options.showInitialPopulation)
		return;

	m_outputStream << "---------- INITIAL POPULATION ----------" << endl;
	m_outputStream << m_population;
}

void AlgorithmRunner::printCacheStats() const
{
	if (!m_options.showCacheStats)
		return;

	CacheStats totalStats{};
	size_t disabledCacheCount = 0;
	for (size_t i = 0; i < m_programCaches.size(); ++i)
		if (m_programCaches[i] != nullptr)
			totalStats += m_programCaches[i]->gatherStats();
		else
			++disabledCacheCount;

	m_outputStream << "---------- CACHE STATS ----------" << endl;

	if (disabledCacheCount < m_programCaches.size())
	{
		for (auto& [round, count]: totalStats.roundEntryCounts)
			m_outputStream << "Round " << round << ": " << count << " entries" << endl;
		m_outputStream << "Total hits: " << totalStats.hits << endl;
		m_outputStream << "Total misses: " << totalStats.misses << endl;
		m_outputStream << "Size of cached code: " << totalStats.totalCodeSize << endl;
	}

	if (disabledCacheCount == m_programCaches.size())
		m_outputStream << "Program cache disabled" << endl;
	else if (disabledCacheCount > 0)
	{
		m_outputStream << "Program cache disabled for " << disabledCacheCount << " out of ";
		m_outputStream << m_programCaches.size() << " programs" << endl;
	}
}

void AlgorithmRunner::populationAutosave() const
{
	if (!m_options.populationAutosaveFile.has_value())
		return;

	ofstream outputStream(m_options.populationAutosaveFile.value(), ios::out | ios::trunc);
	assertThrow(
		outputStream.is_open(),
		FileOpenError,
		"Could not open file '" + m_options.populationAutosaveFile.value() + "': " + strerror(errno)
	);

	for (auto& individual: m_population.individuals())
		outputStream << individual.chromosome << endl;

	assertThrow(
		!outputStream.bad(),
		FileWriteError,
		"Error while writing to file '" + m_options.populationAutosaveFile.value() + "': " + strerror(errno)
	);
}

void AlgorithmRunner::cacheClear()
{
	for (auto& cache: m_programCaches)
		if (cache != nullptr)
			cache->clear();
}

void AlgorithmRunner::cacheStartRound(size_t _roundNumber)
{
	for (auto& cache: m_programCaches)
		if (cache != nullptr)
			cache->startRound(_roundNumber);
}

void AlgorithmRunner::randomiseDuplicates()
{
	if (m_options.randomiseDuplicates)
	{
		assert(m_options.minChromosomeLength.has_value());
		assert(m_options.maxChromosomeLength.has_value());

		m_population = randomiseDuplicates(
			m_population,
			m_options.minChromosomeLength.value(),
			m_options.maxChromosomeLength.value()
		);
	}
}

Population AlgorithmRunner::randomiseDuplicates(
	Population _population,
	size_t _minChromosomeLength,
	size_t _maxChromosomeLength
)
{
	if (_population.individuals().size() == 0)
		return _population;

	vector<Individual> individuals{_population.individuals()[0]};
	size_t duplicateCount = 0;
	for (size_t i = 1; i < _population.individuals().size(); ++i)
		if (_population.individuals()[i].chromosome == _population.individuals()[i - 1].chromosome)
			++duplicateCount;
		else
			individuals.push_back(_population.individuals()[i]);

	return (
		Population(_population.fitnessMetric(), individuals) +
		Population::makeRandom(_population.fitnessMetric(), duplicateCount, _minChromosomeLength, _maxChromosomeLength)
	);
}
