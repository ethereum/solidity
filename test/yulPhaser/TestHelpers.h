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
/**
 * Miscellaneous utilities for use in yul-phaser's test cases.
 *
 * - Generic code that's only used in these particular tests.
 * - Convenience functions and wrappers to make tests more concise.
 * - Mocks and dummy objects/functions used in multiple test suites.
 *
 * Note that the code included here may be not as generic, robust and/or complete as it could be
 * since it's not meant for production use. If some utility seems useful enough to be moved to
 * the normal code base, you should review its implementation before doing so.
 */

#pragma once

#include <tools/yulPhaser/Chromosome.h>
#include <tools/yulPhaser/FitnessMetrics.h>
#include <tools/yulPhaser/Mutations.h>
#include <tools/yulPhaser/Population.h>

#include <boost/test/tools/detail/print_helper.hpp>

#include <cassert>
#include <functional>
#include <map>
#include <string>
#include <tuple>
#include <vector>

// OPERATORS FOR BOOST::TEST

/// Output operator for arbitrary two-element tuples.
/// Necessary to make BOOST_TEST() work with such tuples.
template<typename T1, typename T2>
std::ostream& operator<<(std::ostream& _output, std::tuple<T1, T2> const& _tuple)
{
	_output << "(" << std::get<0>(_tuple) << ", " << std::get<1>(_tuple) << ")";
	return _output;
}

namespace boost::test_tools::tt_detail
{

// Boost won't find find the << operator unless we put it in the std namespace which is illegal.
// The recommended solution is to overload print_log_value<> struct and make it use our global operator.
template<typename T1,typename T2>
struct print_log_value<std::tuple<T1, T2>>
{
	void operator()(std::ostream& _output, std::tuple<T1, T2> const& _tuple) { ::operator<<(_output, _tuple); }
};

}

namespace solidity::phaser::test
{

/**
 * Fitness metric that only takes into account the number of optimisation steps in the chromosome.
 * Recommended for use in tests because it's much faster than ProgramSize metric and it's very
 * easy to guess the result at a glance.
 */
class ChromosomeLengthMetric: public FitnessMetric
{
public:
	using FitnessMetric::FitnessMetric;
	size_t evaluate(Chromosome const& _chromosome) override { return _chromosome.length(); }
};

// MUTATIONS

/// Mutation that always replaces the whole chromosome with the one specified in the parameter.
std::function<Mutation> wholeChromosomeReplacement(Chromosome _newChromosome);

/// Mutation that always replaces the optimisation step at position @a _geneIndex with @a _geneValue.
///
/// The chromosome must be long enough for this position to exist.
std::function<Mutation> geneSubstitution(size_t _geneIndex, std::string _geneValue);

// CHROMOSOME AND POPULATION HELPERS

/// Returns a vector containing lengths of all chromosomes in the population (in the same order).
std::vector<size_t> chromosomeLengths(Population const& _population);

/// Returns the number of genes that differ between two chromosomes.
/// If the chromnosomes have different lengths, the positions that are present in only one of them
/// are counted as mismatches.
size_t countDifferences(Chromosome const& _chromosome1, Chromosome const& _chromosome2);

/// Assigns indices from 0 to N to all optimisation steps available in the OptimiserSuite.
/// This is a convenience helper to make it easier to test their distribution with tools made for
/// integers.
std::map<std::string, size_t> enumerateOptmisationSteps();

// FILESYSTEM UTILITIES

/**
 * An object that creates a unique temporary directory and automatically deletes it and its
 * content upon being destroyed.
 *
 * The directory is guaranteed to be newly created and empty. Directory names are generated
 * randomly. If a directory with the same name already exists (very unlikely but possible) the
 * object won't reuse it and will fail with an exception instead.
 */
class TemporaryDirectory
{
public:
	TemporaryDirectory(std::string const& _prefix = "yul-phaser-test-");
	~TemporaryDirectory();

	std::string const& path() const { return m_path; }

	/// Converts a path relative to the directory held by the object into an absolute one.
	std::string memberPath(std::string const& _relativePath) const;

private:
	std::string m_path;
};

// STRING UTILITIES

/// Returns the input string with all the whitespace characters (spaces, line endings, etc.) removed.
std::string stripWhitespace(std::string const& input);

/// Counts the number of times one strinng can be found inside another. Only non-overlapping
/// occurrences are counted.
size_t countSubstringOccurrences(std::string const& _inputString, std::string const& _substring);

// STATISTICAL UTILITIES

/// Calculates the mean value of a series of samples given in a vector.
///
/// Supports any integer and real type as a convenience but the precision of the result is limited
/// to the precision of type @a double as all the values are internally converted to it.
///
/// This is a very simple, naive implementation that's more than enough for tests where we usually
/// deal with relatively short sequences of small, positive integers. It's not numerically stable
/// in more complicated cases. Don't use in production.
template <typename T>
double mean(std::vector<T> const& _samples)
{
	assert(_samples.size() > 0);

	double sum = 0;
	for (T const& sample: _samples)
		sum += static_cast<double>(sample);

	return sum / _samples.size();
}

/// Calculates the sum of squared differences between @a _expectedValue and the values of a series
/// of samples given in a vector.
///
/// Supports any integer and real type as a convenience but the precision of the result is limited
/// to the precision of type @a double as all the values are internally converted to it.
///
/// This is a very simple, naive implementation that's more than enough for tests where we usually
/// deal with relatively short sequences of small, positive integers. It's not numerically stable
/// in more complicated cases. Don't use in production.
template <typename T>
double meanSquaredError(std::vector<T> const& _samples, double _expectedValue)
{
	assert(_samples.size() > 0);

	double sumOfSquaredDifferences = 0;
	for (T const& sample: _samples)
		sumOfSquaredDifferences += (sample - _expectedValue) * (sample - _expectedValue);

	return sumOfSquaredDifferences / _samples.size();
}

}
