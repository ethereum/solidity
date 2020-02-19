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

#include <map>
#include <ostream>
#include <string>
#include <vector>

namespace solidity::phaser
{

/**
 * An object that represents a sequence of optimiser steps that can be applied to a program.
 * Such sequences are used in our genetic algorithm to represent individual members of the
 * population.
 *
 * To calculate the fitness of an individual one must apply its sequence to a specific program.
 * This class does not provide any means to do so. It just stores information.
 *
 * Once created a sequence cannot be changed. The only way to mutate it is to generate a new
 * chromosome based on the old one.
 */
class Chromosome
{
public:
	Chromosome() = default;
	explicit Chromosome(std::vector<std::string> _optimisationSteps):
		m_optimisationSteps(std::move(_optimisationSteps)) {}
	explicit Chromosome(std::string const& _optimisationSteps);
	static Chromosome makeRandom(size_t _length);

	size_t length() const { return m_optimisationSteps.size(); }
	std::vector<std::string> const& optimisationSteps() const { return m_optimisationSteps; }

	friend std::ostream& operator<<(std::ostream& _stream, Chromosome const& _chromosome);

	bool operator==(Chromosome const& _other) const { return m_optimisationSteps == _other.m_optimisationSteps; }
	bool operator!=(Chromosome const& _other) const { return !(*this == _other); }

	static std::string const& randomOptimisationStep();

private:
	static std::vector<std::string> allStepNames();

	std::vector<std::string> m_optimisationSteps;
};

}
