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

#include <tools/yulPhaser/Population.h>

#include <cassert>
#include <map>
#include <string>
#include <vector>

namespace solidity::phaser::test
{

// CHROMOSOME AND POPULATION HELPERS

/// Returns a vector containing lengths of all chromosomes in the population (in the same order).
std::vector<size_t> chromosomeLengths(Population const& _population);

/// Assigns indices from 0 to N to all optimisation steps available in the OptimiserSuite.
/// This is a convenience helper to make it easier to test their distribution with tools made for
/// integers.
std::map<std::string, size_t> enumerateOptmisationSteps();

// STRING UTILITIES

/// Returns the input string with all the whitespace characters (spaces, line endings, etc.) removed.
std::string stripWhitespace(std::string const& input);

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
