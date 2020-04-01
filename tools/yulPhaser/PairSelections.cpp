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

#include <tools/yulPhaser/PairSelections.h>

#include <tools/yulPhaser/SimulationRNG.h>

#include <cmath>

using namespace std;
using namespace solidity::phaser;

vector<tuple<size_t, size_t>> RandomPairSelection::materialise(size_t _poolSize) const
{
	if (_poolSize < 2)
		return {};

	size_t count = static_cast<size_t>(round(_poolSize * m_selectionSize));

	vector<tuple<size_t, size_t>> selection;
	for (size_t i = 0; i < count; ++i)
	{
		size_t index1 = SimulationRNG::uniformInt(0, _poolSize - 1);
		size_t index2;
		do
		{
			index2 = SimulationRNG::uniformInt(0, _poolSize - 1);
		} while (index1 == index2);

		selection.emplace_back(index1, index2);
	}

	return selection;
}

vector<tuple<size_t, size_t>> PairMosaicSelection::materialise(size_t _poolSize) const
{
	if (_poolSize < 2)
		return {};

	size_t count = static_cast<size_t>(round(_poolSize * m_selectionSize));

	vector<tuple<size_t, size_t>> selection;
	for (size_t i = 0; i < count; ++i)
	{
		tuple<size_t, size_t> pair = m_pattern[i % m_pattern.size()];
		selection.emplace_back(min(get<0>(pair), _poolSize - 1), min(get<1>(pair), _poolSize - 1));
	}

	return selection;
}
