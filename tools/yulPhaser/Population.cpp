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

#include <tools/yulPhaser/Population.h>

#include <tools/yulPhaser/PairSelections.h>
#include <tools/yulPhaser/Selections.h>

#include <libsolutil/CommonData.h>
#include <libsolutil/CommonIO.h>

#include <algorithm>
#include <cassert>
#include <numeric>

using namespace std;
using namespace solidity;
using namespace solidity::langutil;
using namespace solidity::util;
using namespace solidity::phaser;

namespace solidity::phaser
{

ostream& operator<<(ostream& _stream, Individual const& _individual);
ostream& operator<<(ostream& _stream, Population const& _population);

}

ostream& phaser::operator<<(ostream& _stream, Individual const& _individual)
{
	_stream << _individual.fitness << " " << _individual.chromosome;

	return _stream;
}

bool phaser::isFitter(Individual const& a, Individual const& b)
{
	return (
		(a.fitness < b.fitness) ||
		(a.fitness == b.fitness && a.chromosome.length() < b.chromosome.length()) ||
		(a.fitness == b.fitness && a.chromosome.length() == b.chromosome.length() && toString(a.chromosome) < toString(b.chromosome))
	);
}

Population Population::makeRandom(
	shared_ptr<FitnessMetric> _fitnessMetric,
	size_t _size,
	function<size_t()> _chromosomeLengthGenerator
)
{
	vector<Chromosome> chromosomes;
	for (size_t i = 0; i < _size; ++i)
		chromosomes.push_back(Chromosome::makeRandom(_chromosomeLengthGenerator()));

	return Population(move(_fitnessMetric), move(chromosomes));
}

Population Population::makeRandom(
	shared_ptr<FitnessMetric> _fitnessMetric,
	size_t _size,
	size_t _minChromosomeLength,
	size_t _maxChromosomeLength
)
{
	return makeRandom(
		move(_fitnessMetric),
		_size,
		std::bind(uniformChromosomeLength, _minChromosomeLength, _maxChromosomeLength)
	);
}

Population Population::select(Selection const& _selection) const
{
	vector<Individual> selectedIndividuals;
	for (size_t i: _selection.materialise(m_individuals.size()))
		selectedIndividuals.emplace_back(m_individuals[i]);

	return Population(m_fitnessMetric, selectedIndividuals);
}

Population Population::mutate(Selection const& _selection, function<Mutation> _mutation) const
{
	vector<Individual> mutatedIndividuals;
	for (size_t i: _selection.materialise(m_individuals.size()))
		mutatedIndividuals.emplace_back(_mutation(m_individuals[i].chromosome), *m_fitnessMetric);

	return Population(m_fitnessMetric, mutatedIndividuals);
}

Population Population::crossover(PairSelection const& _selection, function<Crossover> _crossover) const
{
	vector<Individual> crossedIndividuals;
	for (auto const& [i, j]: _selection.materialise(m_individuals.size()))
	{
		auto childChromosome = _crossover(
			m_individuals[i].chromosome,
			m_individuals[j].chromosome
		);
		crossedIndividuals.emplace_back(move(childChromosome), *m_fitnessMetric);
	}

	return Population(m_fitnessMetric, crossedIndividuals);
}

tuple<Population, Population> Population::symmetricCrossoverWithRemainder(
	PairSelection const& _selection,
	function<SymmetricCrossover> _symmetricCrossover
) const
{
	vector<int> indexSelected(m_individuals.size(), false);

	vector<Individual> crossedIndividuals;
	for (auto const& [i, j]: _selection.materialise(m_individuals.size()))
	{
		auto children = _symmetricCrossover(
			m_individuals[i].chromosome,
			m_individuals[j].chromosome
		);
		crossedIndividuals.emplace_back(move(get<0>(children)), *m_fitnessMetric);
		crossedIndividuals.emplace_back(move(get<1>(children)), *m_fitnessMetric);
		indexSelected[i] = true;
		indexSelected[j] = true;
	}

	vector<Individual> remainder;
	for (size_t i = 0; i < indexSelected.size(); ++i)
		if (!indexSelected[i])
			remainder.emplace_back(m_individuals[i]);

	return {
		Population(m_fitnessMetric, crossedIndividuals),
		Population(m_fitnessMetric, remainder),
	};
}

namespace solidity::phaser
{

Population operator+(Population _a, Population _b)
{
	// This operator is meant to be used only with populations sharing the same metric (and, to make
	// things simple, "the same" here means the same exact object in memory).
	assert(_a.m_fitnessMetric == _b.m_fitnessMetric);

	using ::operator+; // Import the std::vector concat operator from CommonData.h
	return Population(_a.m_fitnessMetric, move(_a.m_individuals) + move(_b.m_individuals));
}

}

Population Population::combine(std::tuple<Population, Population> _populationPair)
{
	return get<0>(_populationPair) + get<1>(_populationPair);
}

bool Population::operator==(Population const& _other) const
{
	// We consider populations identical only if they share the same exact instance of the metric.
	// It might be possible to define some notion of equality for metric objects but it would
	// be an overkill since mixing populations using different metrics is not a common use case.
	return m_individuals == _other.m_individuals && m_fitnessMetric == _other.m_fitnessMetric;
}

ostream& phaser::operator<<(ostream& _stream, Population const& _population)
{
	auto individual = _population.m_individuals.begin();
	for (; individual != _population.m_individuals.end(); ++individual)
		_stream << *individual << endl;

	return _stream;
}

vector<Individual> Population::chromosomesToIndividuals(
	FitnessMetric& _fitnessMetric,
	vector<Chromosome> _chromosomes
)
{
	vector<Individual> individuals;
	for (auto& chromosome: _chromosomes)
		individuals.emplace_back(move(chromosome), _fitnessMetric);

	return individuals;
}

vector<Individual> Population::sortedIndividuals(vector<Individual> _individuals)
{
	sort(_individuals.begin(), _individuals.end(), isFitter);
	return _individuals;
}
