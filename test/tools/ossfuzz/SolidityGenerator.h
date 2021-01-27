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

namespace solidity::test::fuzzer
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

struct GenerationProbability
{
	/// @returns an unsigned integer in the range [1, @param _n] chosen
	/// uniformly at random.
	static size_t distributionOneToN(size_t _n, std::shared_ptr<RandomEngine> const& _rand)
	{
		return Distribution(1, _n)(*_rand);
	}
};

struct AddDependenciesVisitor
{
	template <typename T>
	void operator()(T const& _t)
	{
		_t->setup();
	}
};

struct GeneratorVisitor
{
	template <typename T>
	std::string operator()(T const& _t)
	{
		return _t->generate();
	}
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
	/// Returns test fragment created by this generator.
	std::string generate()
	{
		std::string generatedCode = visit();
		endVisit();
		return generatedCode;
	}
	/// Virtual visitor that returns a string representing
	/// the generation of the Solidity grammar element.
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
	/// Random engine shared by Solidity mutators
	std::shared_ptr<RandomEngine> rand;
	/// Set of generators used by this generator.
	std::set<GeneratorPtr> generators;
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
	/// Returns a new source path name that is formed by concatenating
	/// a static prefix @name m_sourceUnitNamePrefix, a monotonically
	/// increasing counter starting from 0 and the postfix (extension)
	/// ".sol".
	[[nodiscard]] std::string path() const
	{
		return m_sourceUnitNamePrefix + std::to_string(m_numSourceUnits) + ".sol";
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

class SolidityGenerator: public std::enable_shared_from_this<SolidityGenerator>
{
public:
	explicit SolidityGenerator(unsigned _seed);

	/// Returns the generator of type @param T.
	template <typename T>
	std::shared_ptr<T> generator();
	/// Returns a shared ptr to underlying random
	/// number generator.
	std::shared_ptr<RandomEngine> randomEngine()
	{
		return m_rand;
	}
	/// Returns a pseudo randomly generated test case.
	std::string generateTestProgram();
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
	/// Random number generator
	std::shared_ptr<RandomEngine> m_rand;
	/// Sub generators
	std::set<GeneratorPtr> m_generators;
};
}
