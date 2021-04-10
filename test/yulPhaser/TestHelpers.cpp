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

#include <test/yulPhaser/TestHelpers.h>

#include <libyul/optimiser/Suite.h>

#include <regex>

using namespace std;
using namespace solidity;
using namespace solidity::yul;
using namespace solidity::phaser;
using namespace solidity::phaser::test;

function<Mutation> phaser::test::wholeChromosomeReplacement(Chromosome _newChromosome)
{
	return [_newChromosome = move(_newChromosome)](Chromosome const&) { return _newChromosome; };
}

function<Mutation> phaser::test::geneSubstitution(size_t _geneIndex, string _geneValue)
{
	return [=](Chromosome const& _chromosome)
	{
		vector<string> newGenes = _chromosome.optimisationSteps();
		assert(_geneIndex < newGenes.size());
		newGenes[_geneIndex] = _geneValue;

		return Chromosome(newGenes);
	};
}

vector<size_t> phaser::test::chromosomeLengths(Population const& _population)
{
	vector<size_t> lengths;
	for (auto const& individual: _population.individuals())
		lengths.push_back(individual.chromosome.length());

	return lengths;
}

map<string, size_t> phaser::test::enumerateOptmisationSteps()
{
	map<string, size_t> stepIndices;
	size_t i = 0;
	for (auto const& nameAndAbbreviation: OptimiserSuite::stepNameToAbbreviationMap())
		stepIndices.insert({nameAndAbbreviation.first, i++});

	return stepIndices;
}

size_t phaser::test::countDifferences(Chromosome const& _chromosome1, Chromosome const& _chromosome2)
{
	size_t count = 0;
	for (size_t i = 0; i < min(_chromosome1.length(), _chromosome2.length()); ++i)
		if (_chromosome1.optimisationSteps()[i] != _chromosome2.optimisationSteps()[i])
			++count;

	return count + static_cast<size_t>(abs(
		static_cast<long>(_chromosome1.length()) -
		static_cast<long>(_chromosome2.length())
	));
}

string phaser::test::stripWhitespace(string const& input)
{
	regex whitespaceRegex("\\s+");
	return regex_replace(input, whitespaceRegex, "");
}

size_t phaser::test::countSubstringOccurrences(string const& _inputString, string const& _substring)
{
	assert(_substring.size() > 0);

	size_t count = 0;
	size_t lastOccurrence = 0;
	while ((lastOccurrence = _inputString.find(_substring, lastOccurrence)) != string::npos)
	{
		++count;
		lastOccurrence += _substring.size();
	}

	return count;
}
