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

#include <tools/yulPhaser/PairSelections.h>

#include <tools/yulPhaser/Selections.h>
#include <tools/yulPhaser/SimulationRNG.h>

#include <cmath>

using namespace std;
using namespace solidity::phaser;

vector<tuple<size_t, size_t>> RandomPairSelection::materialise(size_t _poolSize) const
{
	if (_poolSize < 2)
		return {};

	auto count = static_cast<size_t>(round(double(_poolSize) * m_selectionSize));

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

vector<tuple<size_t, size_t>> PairsFromRandomSubset::materialise(size_t _poolSize) const
{
	vector<size_t> selectedIndices = RandomSubset(m_selectionChance).materialise(_poolSize);

	if (selectedIndices.size() % 2 != 0)
	{
		if (selectedIndices.size() < _poolSize && SimulationRNG::bernoulliTrial(0.5))
		{
			do
			{
				size_t extraIndex = SimulationRNG::uniformInt(0, selectedIndices.size() - 1);
				if (find(selectedIndices.begin(), selectedIndices.end(), extraIndex) == selectedIndices.end())
					selectedIndices.push_back(extraIndex);
			} while (selectedIndices.size() % 2 != 0);
		}
		else
			selectedIndices.erase(
				selectedIndices.begin() +
				static_cast<ptrdiff_t>(SimulationRNG::uniformInt(0, selectedIndices.size() - 1))
			);
	}
	assert(selectedIndices.size() % 2 == 0);

	vector<tuple<size_t, size_t>> selectedPairs;
	for (size_t i = selectedIndices.size() / 2; i > 0; --i)
	{
		size_t position1 = SimulationRNG::uniformInt(0, selectedIndices.size() - 1);
		size_t value1 = selectedIndices[position1];
		selectedIndices.erase(selectedIndices.begin() + static_cast<ptrdiff_t>(position1));
		size_t position2 = SimulationRNG::uniformInt(0, selectedIndices.size() - 1);
		size_t value2 = selectedIndices[position2];
		selectedIndices.erase(selectedIndices.begin() + static_cast<ptrdiff_t>(position2));

		selectedPairs.emplace_back(value1, value2);
	}
	assert(selectedIndices.empty());

	return selectedPairs;
}

vector<tuple<size_t, size_t>> PairMosaicSelection::materialise(size_t _poolSize) const
{
	if (_poolSize < 2)
		return {};

	size_t count = static_cast<size_t>(round(double(_poolSize) * m_selectionSize));

	vector<tuple<size_t, size_t>> selection;
	for (size_t i = 0; i < count; ++i)
	{
		tuple<size_t, size_t> pair = m_pattern[i % m_pattern.size()];
		selection.emplace_back(min(get<0>(pair), _poolSize - 1), min(get<1>(pair), _poolSize - 1));
	}

	return selection;
}
