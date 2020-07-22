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

using namespace std;
using namespace solidity;
using namespace solidity::yul;
using namespace solidity::phaser;

namespace solidity::phaser
{

ostream& operator<<(ostream& _stream, Chromosome const& _chromosome);

}

Chromosome::Chromosome(string const& _optimisationSteps)
{
	for (char abbreviation: _optimisationSteps)
		m_optimisationSteps.push_back(OptimiserSuite::stepAbbreviationToNameMap().at(abbreviation));
}

Chromosome Chromosome::makeRandom(size_t _length)
{
	vector<string> steps;
	for (size_t i = 0; i < _length; ++i)
		steps.push_back(randomOptimisationStep());

	return Chromosome(move(steps));
}

ostream& phaser::operator<<(ostream& _stream, Chromosome const& _chromosome)
{
	for (auto const& stepName: _chromosome.m_optimisationSteps)
		_stream << OptimiserSuite::stepNameToAbbreviationMap().at(stepName);

	return _stream;
}

vector<string> Chromosome::allStepNames()
{
	vector<string> stepNames;
	for (auto const& step: OptimiserSuite::allSteps())
		stepNames.push_back(step.first);

	return stepNames;
}

string const& Chromosome::randomOptimisationStep()
{
	static vector<string> stepNames = allStepNames();

	return stepNames[SimulationRNG::uniformInt(0, stepNames.size() - 1)];
}
