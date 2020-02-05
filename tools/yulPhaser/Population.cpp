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
	_stream << "Fitness: ";
	if (_individual.fitness.has_value())
		_stream << _individual.fitness.value();
	else
		_stream << "<NONE>";
	_stream << ", optimisations: " << _individual.chromosome;

	return _stream;
}

Population Population::makeRandom(shared_ptr<FitnessMetric const> _fitnessMetric, size_t _size)
{
	vector<Individual> individuals;
	for (size_t i = 0; i < _size; ++i)
		individuals.push_back({Chromosome::makeRandom(randomChromosomeLength())});

	return Population(move(_fitnessMetric), move(individuals));
}

void Population::run(optional<size_t> _numRounds, ostream& _outputStream)
{
	doEvaluation();
	for (size_t round = 0; !_numRounds.has_value() || round < _numRounds.value(); ++round)
	{
		doMutation();
		doSelection();
		doEvaluation();

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

void Population::doEvaluation()
{
	for (auto& individual: m_individuals)
		if (!individual.fitness.has_value())
			individual.fitness = m_fitnessMetric->evaluate(individual.chromosome);
}

void Population::doSelection()
{
	m_individuals = sortIndividuals(move(m_individuals));
	randomizeWorstChromosomes(m_individuals, m_individuals.size() / 2);
}

void Population::randomizeWorstChromosomes(
	vector<Individual>& _individuals,
	size_t _count
)
{
	assert(_individuals.size() >= _count);
	// ASSUMPTION: _individuals is sorted in ascending order

	auto individual = _individuals.begin() + (_individuals.size() - _count);
	for (; individual != _individuals.end(); ++individual)
	{
		*individual = {Chromosome::makeRandom(randomChromosomeLength())};
	}
}

vector<Individual> Population::chromosomesToIndividuals(
	vector<Chromosome> _chromosomes
)
{
	vector<Individual> individuals;
	for (auto& chromosome: _chromosomes)
		individuals.push_back({move(chromosome)});

	return individuals;
}

vector<Individual> Population::sortIndividuals(vector<Individual> _individuals)
{
	assert(all_of(_individuals.begin(), _individuals.end(), [](auto& i){ return i.fitness.has_value(); }));

	sort(
		_individuals.begin(),
		_individuals.end(),
		[](auto const& a, auto const& b){ return a.fitness.value() < b.fitness.value(); }
	);

	return _individuals;
}
