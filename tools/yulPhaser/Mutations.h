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
/**
 * Mutation and crossover operators for use in genetic algorithms.
 */

#pragma once

#include <tools/yulPhaser/Chromosome.h>

#include <functional>

namespace solidity::phaser
{

using Mutation = Chromosome(Chromosome const&);

// MUTATIONS

/// Creates a mutation operator that iterates over all genes in a chromosome and with probability
/// @a _chance replaces a gene with a random one (which could also be the same as the original).
std::function<Mutation> geneRandomisation(double _chance);

/// Creates a mutation operator that iterates over all genes in a chromosome and with probability
/// @a _chance deletes it.
std::function<Mutation> geneDeletion(double _chance);

/// Creates a mutation operator that iterates over all positions in a chromosome (including spots
/// at the beginning and at the end of the sequence) and with probability @a _chance insert a new,
/// randomly chosen gene.
std::function<Mutation> geneAddition(double _chance);

/// Creates a mutation operator that always applies one of the mutations passed to it.
/// The probability that the chosen mutation is the first one is @a _firstMutationChance.
/// randomly chosen gene.
std::function<Mutation> alternativeMutations(
	double _firstMutationChance,
	std::function<Mutation> _mutation1,
	std::function<Mutation> _mutation2
);

}
