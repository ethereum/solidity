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

#include <test/tools/ossfuzz/SolidityGenerator.h>

#include <libsolutil/Whiskers.h>

using namespace solidity::test::fuzzer;
using namespace solidity::util;
using namespace std;
using PrngUtil = solidity::test::fuzzer::GenerationProbability;

GeneratorBase::GeneratorBase(std::shared_ptr<SolidityGenerator> _mutator)
{
	mutator = std::move(_mutator);
	rand = mutator->randomEngine();
}

string GeneratorBase::visitChildren()
{
	ostringstream os;
	// Randomise visit order
	vector<GeneratorPtr> randomisedChildren;
	for (auto child: generators)
		randomisedChildren.push_back(child);
	shuffle(randomisedChildren.begin(), randomisedChildren.end(), *rand);
	for (auto child: randomisedChildren)
		os << std::visit(GeneratorVisitor{}, child);
	return os.str();
}

void TestCaseGenerator::setup()
{
	addGenerators({
		mutator->generator<SourceUnitGenerator>()
	});
}

string TestCaseGenerator::visit()
{
	ostringstream os;
	for (unsigned i = 0; i < PrngUtil{}.distributionOneToN(s_maxSourceUnits, rand); i++)
	{
		string sourcePath = path();
		os << "\n"
			<< "==== Source: "
			<< sourcePath
			<< " ===="
	        << "\n";
		m_numSourceUnits++;
		os << visitChildren();
	}
	return os.str();
}

void SourceUnitGenerator::setup()
{
	addGenerators({
		mutator->generator<PragmaGenerator>(),
	});
}

string SourceUnitGenerator::visit()
{
	return visitChildren();
}

string PragmaGenerator::visit()
{
	static constexpr const char* preamble = R"(
		pragma solidity >= 0.0.0;
		pragma experimental SMTChecker;
	)";
	// Choose equally at random from coder v1 and v2
	string abiPragma = "pragma abicoder v" +
		to_string(PrngUtil{}.distributionOneToN(2, rand)) +
		";\n";
	return preamble + abiPragma;
}

template <typename T>
shared_ptr<T> SolidityGenerator::generator()
{
	for (auto& g: m_generators)
		if (holds_alternative<shared_ptr<T>>(g))
			return get<shared_ptr<T>>(g);
	solAssert(false, "");
}

SolidityGenerator::SolidityGenerator(unsigned _seed)
{
	m_rand = make_shared<RandomEngine>(_seed);
	m_generators = {};
}

template <size_t I>
void SolidityGenerator::createGenerators()
{
	if constexpr (I < std::variant_size_v<Generator>)
	{
		createGenerator<std::variant_alternative_t<I, Generator>>();
		createGenerators<I + 1>();
	}
}

string SolidityGenerator::generateTestProgram()
{
	createGenerators();
	for (auto& g: m_generators)
		std::visit(AddDependenciesVisitor{}, g);
	string program = generator<TestCaseGenerator>()->generate();
	destroyGenerators();
	return program;
}
