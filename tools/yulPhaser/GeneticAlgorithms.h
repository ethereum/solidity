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

#pragma once

#include <tools/yulPhaser/Population.h>

#include <optional>
#include <ostream>

namespace solidity::phaser
{

class GeneticAlgorithm
{
public:
	GeneticAlgorithm(Population _initialPopulation, std::ostream& _outputStream):
		m_population(std::move(_initialPopulation)),
		m_outputStream(_outputStream) {}

	GeneticAlgorithm(PairSelection const&) = delete;
	GeneticAlgorithm& operator=(GeneticAlgorithm const&) = delete;
	virtual ~GeneticAlgorithm() = default;

	void run(std::optional<size_t> _numRounds = std::nullopt);
	virtual void runNextRound() = 0;

protected:
	Population m_population;
	std::ostream& m_outputStream;
};

}
