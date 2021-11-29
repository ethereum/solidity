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
 * Contains an abstract base class representing a fitness metric and its concrete implementations.
 */

#pragma once

#include <tools/yulPhaser/Chromosome.h>
#include <tools/yulPhaser/Program.h>
#include <tools/yulPhaser/ProgramCache.h>

#include <libyul/optimiser/Metrics.h>

#include <cstddef>
#include <optional>

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

	virtual size_t evaluate(Chromosome const& _chromosome) = 0;
};

/**
 * Abstract base class for fitness metrics that return values based on program size.
 *
 * The class provides utilities for optimising programs according to the information stored in
 * chromosomes. Allows using @a ProgramCache.
 *
 * It can also store weights for the @a CodeSize metric. It does not do anything with
 * them because it does not actually compute the code size but they are readily available for use
 * by derived classes.
 */
class ProgramBasedMetric: public FitnessMetric
{
public:
	explicit ProgramBasedMetric(
		std::optional<Program> _program,
		std::shared_ptr<ProgramCache> _programCache,
		yul::CodeWeights const& _codeWeights,
		size_t _repetitionCount = 1
	):
		m_program(std::move(_program)),
		m_programCache(std::move(_programCache)),
		m_codeWeights(_codeWeights),
		m_repetitionCount(_repetitionCount)
	{
		assert(m_program.has_value() == (m_programCache == nullptr));
	}

	Program const& program() const;
	ProgramCache const* programCache() const { return m_programCache.get(); }
	yul::CodeWeights const& codeWeights() const { return m_codeWeights; }
	size_t repetitionCount() const { return m_repetitionCount; }

	Program optimisedProgram(Chromosome const& _chromosome);
	Program optimisedProgramNoCache(Chromosome const& _chromosome) const;

private:
	std::optional<Program> m_program;
	std::shared_ptr<ProgramCache> m_programCache;
	yul::CodeWeights m_codeWeights;
	size_t m_repetitionCount;
};

/**
 * Fitness metric based on the size of a specific program after applying the optimisations from the
 * chromosome to it.
 */
class ProgramSize: public ProgramBasedMetric
{
public:
	using ProgramBasedMetric::ProgramBasedMetric;
	size_t evaluate(Chromosome const& _chromosome) override;
};

/**
 * Fitness metric based on the size of a specific program after applying the optimisations from the
 * chromosome to it in relation to the original, unoptimised program.
 *
 * Since metric values are integers, the class multiplies the ratio by 10^@a _fixedPointPrecision
 * before rounding it.
 */
class RelativeProgramSize: public ProgramBasedMetric
{
public:
	explicit RelativeProgramSize(
		std::optional<Program> _program,
		std::shared_ptr<ProgramCache> _programCache,
		size_t _fixedPointPrecision,
		yul::CodeWeights const& _weights,
		size_t _repetitionCount = 1
	):
		ProgramBasedMetric(std::move(_program), std::move(_programCache), _weights, _repetitionCount),
		m_fixedPointPrecision(_fixedPointPrecision) {}

	size_t fixedPointPrecision() const { return m_fixedPointPrecision; }

	size_t evaluate(Chromosome const& _chromosome) override;

private:
	size_t m_fixedPointPrecision;
};

/**
 * Abstract base class for fitness metrics that compute their value based on values of multiple
 * other, nested metrics.
 */
class FitnessMetricCombination: public FitnessMetric
{
public:
	explicit FitnessMetricCombination(std::vector<std::shared_ptr<FitnessMetric>> _metrics):
		m_metrics(std::move(_metrics)) {}

	std::vector<std::shared_ptr<FitnessMetric>> const& metrics() const { return m_metrics; }

protected:
	std::vector<std::shared_ptr<FitnessMetric>> m_metrics;
};

/**
 * Fitness metric that returns the average of values of its nested metrics.
 */
class FitnessMetricAverage: public FitnessMetricCombination
{
public:
	using FitnessMetricCombination::FitnessMetricCombination;
	size_t evaluate(Chromosome const& _chromosome) override;
};

/**
 * Fitness metric that returns the sum of values of its nested metrics.
 */
class FitnessMetricSum: public FitnessMetricCombination
{
public:
	using FitnessMetricCombination::FitnessMetricCombination;
	size_t evaluate(Chromosome const& _chromosome) override;
};

/**
 * Fitness metric that returns the highest of values of its nested metrics.
 */
class FitnessMetricMaximum: public FitnessMetricCombination
{
public:
	using FitnessMetricCombination::FitnessMetricCombination;
	size_t evaluate(Chromosome const& _chromosome) override;
};

/**
 * Fitness metric that returns the lowest of values of its nested metrics.
 */
class FitnessMetricMinimum: public FitnessMetricCombination
{
public:
	using FitnessMetricCombination::FitnessMetricCombination;
	size_t evaluate(Chromosome const& _chromosome) override;
};

}
