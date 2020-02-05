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
 * Contains an abstract base class representing a fitness metric and its concrete implementations.
 */

#pragma once

#include <tools/yulPhaser/Chromosome.h>
#include <tools/yulPhaser/Program.h>

#include <cstddef>

namespace solidity::phaser
{

/**
 * Abstract base class for fitness metrics.
 *
 * The main feature is the @a evaluate() method that can tell how good a given chromosome is.
 * The lower the value, the better the fitness is. The result should be deterministic and depend
 * only on the chromosome and metric's state (which is constant).
 */
class FitnessMetric
{
public:
	FitnessMetric() = default;
	FitnessMetric(FitnessMetric const&) = delete;
	FitnessMetric& operator=(FitnessMetric const&) = delete;
	virtual ~FitnessMetric() = default;

	virtual size_t evaluate(Chromosome const& _chromosome) const = 0;
};

/**
 * Fitness metric based on the size of a specific program after applying the optimisations from the
 * chromosome to it.
 */
class ProgramSize: public FitnessMetric
{
public:
	explicit ProgramSize(Program _program, size_t _repetitionCount = 1):
		m_program(std::move(_program)),
		m_repetitionCount(_repetitionCount) {}

	size_t evaluate(Chromosome const& _chromosome) const override;

private:
	Program m_program;
	size_t m_repetitionCount;
};

}
