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

#include <tools/yulPhaser/FitnessMetrics.h>

#include <libsolutil/CommonIO.h>

#include <cmath>

using namespace std;
using namespace solidity::util;
using namespace solidity::yul;
using namespace solidity::phaser;

Program const& ProgramBasedMetric::program() const
{
	if (m_programCache == nullptr)
		return m_program.value();
	else
		return m_programCache->program();
}

Program ProgramBasedMetric::optimisedProgram(Chromosome const& _chromosome)
{
	if (m_programCache == nullptr)
		return optimisedProgramNoCache(_chromosome);

	return m_programCache->optimiseProgram(
		toString(_chromosome),
		m_repetitionCount
	);
}

Program ProgramBasedMetric::optimisedProgramNoCache(Chromosome const& _chromosome) const
{
	Program programCopy = program();
	for (size_t i = 0; i < m_repetitionCount; ++i)
		programCopy.optimise(_chromosome.optimisationSteps());

	return programCopy;
}

size_t ProgramSize::evaluate(Chromosome const& _chromosome)
{
	return optimisedProgram(_chromosome).codeSize(codeWeights());
}

size_t RelativeProgramSize::evaluate(Chromosome const& _chromosome)
{
	double const scalingFactor = pow(10, m_fixedPointPrecision);

	size_t unoptimisedSize = optimisedProgram(Chromosome("")).codeSize(codeWeights());
	if (unoptimisedSize == 0)
		return static_cast<size_t>(scalingFactor);

	size_t optimisedSize = optimisedProgram(_chromosome).codeSize(codeWeights());

	return static_cast<size_t>(round(
		double(optimisedSize) / double(unoptimisedSize) * scalingFactor
	));
}

size_t FitnessMetricAverage::evaluate(Chromosome const& _chromosome)
{
	assert(m_metrics.size() > 0);

	size_t total = m_metrics[0]->evaluate(_chromosome);
	for (size_t i = 1; i < m_metrics.size(); ++i)
		total += m_metrics[i]->evaluate(_chromosome);

	return total / m_metrics.size();
}

size_t FitnessMetricSum::evaluate(Chromosome const& _chromosome)
{
	assert(m_metrics.size() > 0);

	size_t total = m_metrics[0]->evaluate(_chromosome);
	for (size_t i = 1; i < m_metrics.size(); ++i)
		total += m_metrics[i]->evaluate(_chromosome);

	return total;
}

size_t FitnessMetricMaximum::evaluate(Chromosome const& _chromosome)
{
	assert(m_metrics.size() > 0);

	size_t maximum = m_metrics[0]->evaluate(_chromosome);
	for (size_t i = 1; i < m_metrics.size(); ++i)
		maximum = max(maximum, m_metrics[i]->evaluate(_chromosome));

	return maximum;
}

size_t FitnessMetricMinimum::evaluate(Chromosome const& _chromosome)
{
	assert(m_metrics.size() > 0);

	size_t minimum = m_metrics[0]->evaluate(_chromosome);
	for (size_t i = 1; i < m_metrics.size(); ++i)
		minimum = min(minimum, m_metrics[i]->evaluate(_chromosome));

	return minimum;
}
