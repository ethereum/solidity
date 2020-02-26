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

#include <tools/yulPhaser/ProgramCache.h>

#include <libyul/optimiser/Suite.h>

using namespace std;
using namespace solidity::yul;
using namespace solidity::phaser;

Program ProgramCache::optimiseProgram(
	string const& _abbreviatedOptimisationSteps,
	size_t _repetitionCount
)
{
	string targetOptimisations = _abbreviatedOptimisationSteps;
	for (size_t i = 1; i < _repetitionCount; ++i)
		targetOptimisations += _abbreviatedOptimisationSteps;

	size_t prefixSize = 0;
	for (size_t i = 1; i <= targetOptimisations.size(); ++i)
	{
		auto const& pair = m_entries.find(targetOptimisations.substr(0, i));
		if (pair != m_entries.end())
		{
			pair->second.roundNumber = m_currentRound;
			++prefixSize;
		}
		else
			break;
	}

	Program intermediateProgram = (
		prefixSize == 0 ?
		m_program :
		m_entries.at(targetOptimisations.substr(0, prefixSize)).program
	);

	for (size_t i = prefixSize + 1; i <= targetOptimisations.size(); ++i)
	{
		string stepName = OptimiserSuite::stepAbbreviationToNameMap().at(targetOptimisations[i - 1]);
		intermediateProgram.optimise({stepName});

		m_entries.insert({targetOptimisations.substr(0, i), {intermediateProgram, m_currentRound}});
	}

	return intermediateProgram;
}

void ProgramCache::startRound(size_t _roundNumber)
{
	assert(_roundNumber > m_currentRound);
	m_currentRound = _roundNumber;

	for (auto pair = m_entries.begin(); pair != m_entries.end();)
	{
		assert(pair->second.roundNumber < m_currentRound);

		if (pair->second.roundNumber < m_currentRound - 1)
			m_entries.erase(pair++);
		else
			++pair;
	}
}

void ProgramCache::clear()
{
	m_entries.clear();
	m_currentRound = 0;
}

Program const* ProgramCache::find(string const& _abbreviatedOptimisationSteps) const
{
	auto const& pair = m_entries.find(_abbreviatedOptimisationSteps);
	if (pair == m_entries.end())
		return nullptr;

	return &(pair->second.program);
}
