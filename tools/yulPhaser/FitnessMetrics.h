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

#include <tools/yulPhaser/Chromosome.h>
#include <tools/yulPhaser/Program.h>

#include <cstddef>

namespace solidity::phaser
{

class FitnessMetric
{
public:
	FitnessMetric() = default;
	FitnessMetric(FitnessMetric const&) = delete;
	FitnessMetric& operator=(FitnessMetric const&) = delete;
	virtual ~FitnessMetric() = default;

	virtual size_t evaluate(Chromosome const& _chromosome) const = 0;
};

class ProgramSize: public FitnessMetric
{
public:
	ProgramSize(Program _program): m_program(std::move(_program)) {}

	size_t evaluate(Chromosome const& _chromosome) const override;

private:
	Program m_program;
};

}
