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

#include <test/yulPhaser/TestHelpers.h>

#include <libyul/optimiser/Suite.h>

#include <boost/filesystem.hpp>

#include <regex>
#include <iostream>

using namespace std;
using namespace solidity;
using namespace solidity::yul;
using namespace solidity::phaser;
using namespace solidity::phaser::test;

namespace fs = boost::filesystem;

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
		count += static_cast<int>(_chromosome1.optimisationSteps()[i] != _chromosome2.optimisationSteps()[i]);

	return count + abs(static_cast<int>(_chromosome1.length() - _chromosome2.length()));
}

TemporaryDirectory::TemporaryDirectory(std::string const& _prefix):
	m_path((fs::temp_directory_path() / fs::unique_path(_prefix + "%%%%-%%%%-%%%%-%%%%")).string())
{
	// Prefix should just be a file name and not contain anything that would make us step out of /tmp.
	assert(fs::path(_prefix) == fs::path(_prefix).stem());

	fs::create_directory(m_path);
}

TemporaryDirectory::~TemporaryDirectory()
{
	// A few paranoid sanity checks just to be extra sure we're not deleting someone's homework.
	assert(m_path.find(fs::temp_directory_path().string()) == 0);
	assert(fs::path(m_path) != fs::temp_directory_path());
	assert(fs::path(m_path) != fs::path(m_path).root_path());
	assert(!fs::path(m_path).empty());

	boost::system::error_code errorCode;
	uintmax_t numRemoved = fs::remove_all(m_path, errorCode);
	if (errorCode.value() != boost::system::errc::success)
	{
		cerr << "Failed to completely remove temporary directory '" << m_path << "'. ";
		cerr << "Only " << numRemoved << " files were actually removed." << endl;
		cerr << "Reason: " << errorCode.message() << endl;
	}
}

string TemporaryDirectory::memberPath(string const& _relativePath) const
{
	assert(fs::path(_relativePath).is_relative());

	return (fs::path(m_path) / _relativePath).string();
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
