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

#include <tools/yulPhaser/Chromosome.h>

#include <tools/yulPhaser/SimulationRNG.h>

#include <libyul/optimiser/Suite.h>
#include <libsolutil/CommonData.h>

#include <sstream>

using namespace solidity;
using namespace solidity::yul;
using namespace solidity::phaser;

namespace solidity::phaser
{

std::ostream& operator<<(std::ostream& _stream, Chromosome const& _chromosome);

}

Chromosome Chromosome::makeRandom(size_t _length)
{
	std::vector<std::string> steps;
	for (size_t i = 0; i < _length; ++i)
		steps.push_back(randomOptimisationStep());

	return Chromosome(std::move(steps));
}

std::ostream& phaser::operator<<(std::ostream& _stream, Chromosome const& _chromosome)
{
	return _stream << _chromosome.m_genes;
}

std::vector<std::string> Chromosome::allStepNames()
{
	std::vector<std::string> stepNames;
	for (auto const& step: OptimiserSuite::allSteps())
		stepNames.push_back(step.first);

	return stepNames;
}

std::string const& Chromosome::randomOptimisationStep()
{
	static std::vector<std::string> stepNames = allStepNames();

	return stepNames[SimulationRNG::uniformInt(0, stepNames.size() - 1)];
}

char Chromosome::randomGene()
{
	return OptimiserSuite::stepNameToAbbreviationMap().at(randomOptimisationStep());
}

std::string Chromosome::stepsToGenes(std::vector<std::string> const& _optimisationSteps)
{
	std::string genes;
	for (std::string const& stepName: _optimisationSteps)
		genes.push_back(OptimiserSuite::stepNameToAbbreviationMap().at(stepName));

	return genes;
}

std::vector<std::string> Chromosome::genesToSteps(std::string const& _genes)
{
	std::vector<std::string> steps;
	for (char abbreviation: _genes)
		steps.push_back(OptimiserSuite::stepAbbreviationToNameMap().at(abbreviation));

	return steps;
}
