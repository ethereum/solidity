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
#include <tuple>

namespace solidity::phaser
{

using ChromosomePair = std::tuple<Chromosome, Chromosome>;

using Mutation = Chromosome(Chromosome const&);
using Crossover = Chromosome(Chromosome const&, Chromosome const&);
using SymmetricCrossover = ChromosomePair(Chromosome const&, Chromosome const&);

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

/// Creates a mutation operator that sequentially applies all the operators given in @a _mutations.
std::function<Mutation> mutationSequence(std::vector<std::function<Mutation>> _mutations);

// CROSSOVER

/// Creates a crossover operator that randomly selects a number between 0 and 1 and uses it as the
/// position at which to perform perform @a fixedPointCrossover.
std::function<Crossover> randomPointCrossover();

/// Symmetric version of @a randomPointCrossover(). Creates an operator that returns a pair
/// containing both possible results for the same crossover point.
std::function<SymmetricCrossover> symmetricRandomPointCrossover();

/// Creates a crossover operator that always chooses a point that lies at @a _crossoverPoint
/// percent of the length of the shorter chromosome. Then creates a new chromosome by
/// splitting both inputs at the crossover point and stitching output from the first half or first
/// input and the second half of the second input.
///
/// Avoids selecting position 0 (since this just produces a chromosome identical to the second one)
/// unless there is no other choice (i.e. one of the chromosomes is empty).
std::function<Crossover> fixedPointCrossover(double _crossoverPoint);

/// Creates a crossover operator that randomly selects two points between 0 and 1 and swaps genes
/// from the resulting interval. The interval may be empty in which case no genes are swapped.
std::function<Crossover> randomTwoPointCrossover();

/// Symmetric version of @a randomTwoPointCrossover(). Creates an operator that returns a pair
/// containing both possible results for the same crossover points.
std::function<SymmetricCrossover> symmetricRandomTwoPointCrossover();

/// Creates a crossover operator that goes over the length of the shorter chromosomes and for
/// each gene independently decides whether to swap it or not (with probability given by
/// @a _swapChance). The tail of the longer chromosome (the part that's past the length of the
/// shorter one) is treated as a single gene and can potentially be swapped too.
std::function<Crossover> uniformCrossover(double _swapChance);

/// Symmetric version of @a uniformCrossover(). Creates an operator that returns a pair
/// containing both possible results for the same set or swap decisions.
std::function<SymmetricCrossover> symmetricUniformCrossover(double _swapChance);

}
