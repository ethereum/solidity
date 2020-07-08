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
