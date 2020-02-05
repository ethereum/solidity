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


#include <algorithm>
#include <cassert>
#include <numeric>

using namespace std;
using namespace solidity;
using namespace solidity::langutil;
using namespace solidity::phaser;

namespace solidity::phaser
{

ostream& operator<<(ostream& _stream, Individual const& _individual);
ostream& operator<<(ostream& _stream, Population const& _population);

}

ostream& phaser::operator<<(ostream& _stream, Individual const& _individual)
{
	_stream << "Fitness: " << _individual.fitness;
	_stream << ", optimisations: " << _individual.chromosome;

	return _stream;
}

Population Population::makeRandom(shared_ptr<FitnessMetric const> _fitnessMetric, size_t _size)
{
	vector<Chromosome> chromosomes;
	for (size_t i = 0; i < _size; ++i)
		chromosomes.push_back(Chromosome::makeRandom(randomChromosomeLength()));

	return Population(move(_fitnessMetric), move(chromosomes));
}

void Population::run(optional<size_t> _numRounds, ostream& _outputStream)
{
	for (size_t round = 0; !_numRounds.has_value() || round < _numRounds.value(); ++round)
	{
		doMutation();
		doSelection();

		_outputStream << "---------- ROUND " << round << " ----------" << endl;
		_outputStream << *this;
	}
}

ostream& phaser::operator<<(ostream& _stream, Population const& _population)
{
	auto individual = _population.m_individuals.begin();
	for (; individual != _population.m_individuals.end(); ++individual)
		_stream << *individual << endl;

	return _stream;
}

void Population::doMutation()
{
	// TODO: Implement mutation and crossover
}

void Population::doSelection()
{
	m_individuals = sortIndividuals(move(m_individuals));
	randomizeWorstChromosomes(*m_fitnessMetric, m_individuals, m_individuals.size() / 2);
}

void Population::randomizeWorstChromosomes(
	FitnessMetric const& _fitnessMetric,
	vector<Individual>& _individuals,
	size_t _count
)
{
	assert(_individuals.size() >= _count);
	// ASSUMPTION: _individuals is sorted in ascending order

	auto individual = _individuals.begin() + (_individuals.size() - _count);
	for (; individual != _individuals.end(); ++individual)
	{
		auto chromosome = Chromosome::makeRandom(randomChromosomeLength());
		size_t fitness = _fitnessMetric.evaluate(chromosome);
		*individual = {move(chromosome), fitness};
	}
}

vector<Individual> Population::chromosomesToIndividuals(
	FitnessMetric const& _fitnessMetric,
	vector<Chromosome> _chromosomes
)
{
	vector<Individual> individuals;
	for (auto& chromosome: _chromosomes)
	{
		size_t fitness = _fitnessMetric.evaluate(chromosome);
		individuals.push_back({move(chromosome), fitness});
	}

	return individuals;
}

vector<Individual> Population::sortIndividuals(vector<Individual> _individuals)
{
	sort(
		_individuals.begin(),
		_individuals.end(),
		[](auto const& a, auto const& b){ return a.fitness < b.fitness; }
	);

	return _individuals;
}
