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

#include <functional>
#include <tuple>

namespace solidity::phaser
{

using ChromsomePair = std::tuple<Chromosome, Chromosome>;

using Mutation = Chromosome(Chromosome const&);
using Crossover = ChromsomePair(Chromosome const&, Chromosome const&);

std::function<Mutation> geneRandomization(double _chance);
std::function<Mutation> geneDeletion(double _chance);
std::function<Mutation> geneAddition(double _chance);
std::function<Mutation> alternativeMutations(
	double _firstMutationChance,
	std::function<Mutation> _mutation1,
	std::function<Mutation> _mutation2
);

std::function<Crossover> singlePointCrossover();

}
