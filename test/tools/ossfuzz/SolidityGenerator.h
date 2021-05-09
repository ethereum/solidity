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
// SPDX-License-Identifier: GPL-3.0
/**
 * Implements generators for synthesizing mostly syntactically valid
 * Solidity test programs.
 */

#pragma once

#include <test/tools/ossfuzz/Generators.h>

#include <liblangutil/Exceptions.h>

#include <memory>
#include <random>
#include <set>
#include <variant>

namespace solidity::test::fuzzer::mutator
{
/// Forward declarations
class SolidityGenerator;

/// Type declarations
#define SEMICOLON() ;
#define FORWARDDECLAREGENERATORS(G) class G
GENERATORLIST(FORWARDDECLAREGENERATORS, SEMICOLON(), SEMICOLON())
#undef FORWARDDECLAREGENERATORS
#undef SEMICOLON

#define COMMA() ,
using GeneratorPtr = std::variant<
#define VARIANTOFSHARED(G) std::shared_ptr<G>
GENERATORLIST(VARIANTOFSHARED, COMMA(), )
>;
#undef VARIANTOFSHARED
using Generator = std::variant<
#define VARIANTOFGENERATOR(G) G
GENERATORLIST(VARIANTOFGENERATOR, COMMA(), )
>;
#undef VARIANTOFGENERATOR
#undef COMMA
using RandomEngine = std::mt19937_64;
using Distribution = std::uniform_int_distribution<size_t>;

struct UniformRandomDistribution
{
	explicit UniformRandomDistribution(std::unique_ptr<RandomEngine> _randomEngine):
		randomEngine(std::move(_randomEngine))
	{}

	/// @returns an unsigned integer in the range [1, @param _n] chosen
	/// uniformly at random.
	[[nodiscard]] size_t distributionOneToN(size_t _n) const
	{
		return Distribution(1, _n)(*randomEngine);
	}
	/// @returns true with a probability of 1/(@param _n), false otherwise.
	/// @param _n must be non zero.
	[[nodiscard]] bool probable(size_t _n) const
	{
		solAssert(_n > 0, "");
		return distributionOneToN(_n) == 1;
	}
	std::unique_ptr<RandomEngine> randomEngine;
};

struct TestState
{
	explicit TestState(std::shared_ptr<UniformRandomDistribution> _urd):
		sourceUnitPaths({}),
		currentSourceUnitPath({}),
		uRandDist(std::move(_urd))
	{}
	/// Adds @param _path to @name sourceUnitPaths updates
	/// @name currentSourceUnitPath.
	void addSourceUnit(std::string const& _path)
	{
		sourceUnitPaths.insert(_path);
		currentSourceUnitPath = _path;
	}
	/// @returns true if @name sourceUnitPaths is empty,
	/// false otherwise.
	[[nodiscard]] bool empty() const
	{
		return sourceUnitPaths.empty();
	}
	/// @returns the number of items in @name sourceUnitPaths.
	[[nodiscard]] size_t size() const
	{
		return sourceUnitPaths.size();
	}
	/// Prints test state to @param _os.
	void print(std::ostream& _os) const;
	/// @returns a randomly chosen path from @param _sourceUnitPaths.
	[[nodiscard]] std::string randomPath(std::set<std::string> const& _sourceUnitPaths) const;
	/// @returns a randomly chosen path from @name sourceUnitPaths.
	[[nodiscard]] std::string randomPath() const;
	/// @returns a randomly chosen non current source unit path.
	[[nodiscard]] std::string randomNonCurrentPath() const;
	/// List of source paths in test input.
	std::set<std::string> sourceUnitPaths;
	/// Source path being currently visited.
	std::string currentSourceUnitPath;
	/// Uniform random distribution.
	std::shared_ptr<UniformRandomDistribution> uRandDist;
};

struct GeneratorBase
{
	explicit GeneratorBase(std::shared_ptr<SolidityGenerator> _mutator);
	template <typename T>
	std::shared_ptr<T> generator()
	{
		for (auto& g: generators)
			if (std::holds_alternative<std::shared_ptr<T>>(g))
				return std::get<std::shared_ptr<T>>(g);
		solAssert(false, "");
	}
	/// @returns test fragment created by this generator.
	std::string generate()
	{
		std::string generatedCode = visit();
		endVisit();
		return generatedCode;
	}
	/// @returns a string representing the generation of
	/// the Solidity grammar element.
	virtual std::string visit() = 0;
	/// Method called after visiting this generator. Used
	/// for clearing state if necessary.
	virtual void endVisit() {}
	/// Visitor that invokes child grammar elements of
	/// this grammar element returning their string
	/// representations.
	std::string visitChildren();
	/// Adds generators for child grammar elements of
	/// this grammar element.
	void addGenerators(std::set<GeneratorPtr> _generators)
	{
		generators += _generators;
	}
	/// Virtual method to obtain string name of generator.
	virtual std::string name() = 0;
	/// Virtual method to add generators that this grammar
	/// element depends on. If not overridden, there are
	/// no dependencies.
	virtual void setup() {}
	virtual ~GeneratorBase()
	{
		generators.clear();
	}
	/// Shared pointer to the mutator instance
	std::shared_ptr<SolidityGenerator> mutator;
	/// Set of generators used by this generator.
	std::set<GeneratorPtr> generators;
	/// Shared ptr to global test state.
	std::shared_ptr<TestState> state;
	/// Uniform random distribution
	std::shared_ptr<UniformRandomDistribution> uRandDist;
};

class TestCaseGenerator: public GeneratorBase
{
public:
	explicit TestCaseGenerator(std::shared_ptr<SolidityGenerator> _mutator):
		GeneratorBase(std::move(_mutator)),
		m_numSourceUnits(0)
	{}
	void setup() override;
	std::string visit() override;
	std::string name() override
	{
		return "Test case generator";
	}
private:
	/// @returns a new source path name that is formed by concatenating
	/// a static prefix @name m_sourceUnitNamePrefix, a monotonically
	/// increasing counter starting from 0 and the postfix (extension)
	/// ".sol".
	[[nodiscard]] std::string path() const
	{
		return m_sourceUnitNamePrefix + std::to_string(m_numSourceUnits) + ".sol";
	}
	/// Adds @param _path to list of source paths in global test
	/// state and increments @name m_numSourceUnits.
	void updateSourcePath(std::string const& _path)
	{
		state->addSourceUnit(_path);
		m_numSourceUnits++;
	}
	/// Number of source units in test input
	size_t m_numSourceUnits;
	/// String prefix of source unit names
	std::string const m_sourceUnitNamePrefix = "su";
	/// Maximum number of source units per test input
	static constexpr unsigned s_maxSourceUnits = 3;
};

class SourceUnitGenerator: public GeneratorBase
{
public:
	explicit SourceUnitGenerator(std::shared_ptr<SolidityGenerator> _mutator):
		GeneratorBase(std::move(_mutator))
	{}
	void setup() override;
	std::string visit() override;
	std::string name() override { return "Source unit generator"; }
};

class PragmaGenerator: public GeneratorBase
{
public:
	explicit PragmaGenerator(std::shared_ptr<SolidityGenerator> _mutator):
		GeneratorBase(std::move(_mutator))
	{}
	std::string visit() override;
	std::string name() override { return "Pragma generator"; }
};

class ImportGenerator: public GeneratorBase
{
public:
	explicit ImportGenerator(std::shared_ptr<SolidityGenerator> _mutator):
	       GeneratorBase(std::move(_mutator))
	{}
	std::string visit() override;
	std::string name() override { return "Import generator"; }
private:
	/// Inverse probability with which a source unit
	/// imports itself. Keeping this at 17 seems to
	/// produce self imported source units with a
	/// frequency small enough so that it does not
	/// consume too many fuzzing cycles but large
	/// enough so that the fuzzer generates self
	/// import statements every once in a while.
	static constexpr size_t s_selfImportInvProb = 17;
};

class SolidityGenerator: public std::enable_shared_from_this<SolidityGenerator>
{
public:
	explicit SolidityGenerator(unsigned _seed);

	/// @returns the generator of type @param T.
	template <typename T>
	std::shared_ptr<T> generator();
	/// @returns a shared ptr to underlying random
	/// number distribution.
	std::shared_ptr<UniformRandomDistribution> uniformRandomDist()
	{
		return m_urd;
	}
	/// @returns a pseudo randomly generated test case.
	std::string generateTestProgram();
	/// @returns shared ptr to global test state.
	std::shared_ptr<TestState> testState()
	{
		return m_state;
	}
private:
	template <typename T>
	void createGenerator()
	{
		m_generators.insert(
			std::make_shared<T>(shared_from_this())
		);
	}
	template <std::size_t I = 0>
	void createGenerators();
	void destroyGenerators()
	{
		m_generators.clear();
	}
	/// Sub generators
	std::set<GeneratorPtr> m_generators;
	/// Shared global test state
	std::shared_ptr<TestState> m_state;
	/// Uniform random distribution
	std::shared_ptr<UniformRandomDistribution> m_urd;
};
}
