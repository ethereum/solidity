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

#include <tools/yulPhaser/Selections.h>

#include <tools/yulPhaser/SimulationRNG.h>

#include <cmath>
#include <numeric>

using namespace std;
using namespace solidity::phaser;

vector<size_t> RangeSelection::materialise(size_t _poolSize) const
{
	size_t beginIndex = static_cast<size_t>(round(_poolSize * m_startPercent));
	size_t endIndex = static_cast<size_t>(round(_poolSize * m_endPercent));
	vector<size_t> selection;

	for (size_t i = beginIndex; i < endIndex; ++i)
		selection.push_back(i);

	return selection;
}

vector<size_t> MosaicSelection::materialise(size_t _poolSize) const
{
	size_t count = static_cast<size_t>(round(_poolSize * m_selectionSize));

	vector<size_t> selection;
	for (size_t i = 0; i < count; ++i)
		selection.push_back(min(m_pattern[i % m_pattern.size()], _poolSize - 1));

	return selection;
}

vector<size_t> RandomSelection::materialise(size_t _poolSize) const
{
	size_t count = static_cast<size_t>(round(_poolSize * m_selectionSize));

	vector<size_t> selection;
	for (size_t i = 0; i < count; ++i)
		selection.push_back(SimulationRNG::uniformInt(0, _poolSize - 1));

	return selection;
}

vector<size_t> RandomSubset::materialise(size_t _poolSize) const
{
	vector<size_t> selection;
	for (size_t index = 0; index < _poolSize; ++index)
		if (SimulationRNG::bernoulliTrial(m_selectionChance))
			selection.push_back(index);

	return selection;
}
