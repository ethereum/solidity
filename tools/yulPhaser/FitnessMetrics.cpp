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

#include <tools/yulPhaser/FitnessMetrics.h>

using namespace std;
using namespace solidity::phaser;

size_t ProgramSize::evaluate(Chromosome const& _chromosome) const
{
	Program programCopy = m_program;
	for (size_t i = 0; i < m_repetitionCount; ++i)
		programCopy.optimise(_chromosome.optimisationSteps());

	return programCopy.codeSize();
}
