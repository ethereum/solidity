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
#include <libsolutil/Visitor.h>

#include <boost/algorithm/string/join.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm/copy.hpp>


using namespace solidity::test::fuzzer;
using namespace solidity::test::fuzzer::mutator;
using namespace solidity::util;
using namespace std;

GeneratorBase::GeneratorBase(std::shared_ptr<SolidityGenerator> _mutator)
{
	mutator = std::move(_mutator);
	state = mutator->testState();
	uRandDist = mutator->uniformRandomDist();
}

string GeneratorBase::visitChildren()
{
	ostringstream os;
	// Randomise visit order
	vector<std::pair<GeneratorPtr, unsigned>> randomisedChildren;
	for (auto const& child: generators)
		randomisedChildren.push_back(child);
	shuffle(randomisedChildren.begin(), randomisedChildren.end(), *uRandDist->randomEngine);
	for (auto const& child: randomisedChildren)
		if (uRandDist->likely(child.second + 1))
			for (unsigned i = 0; i < uRandDist->distributionOneToN(child.second); i++)
				os << std::visit(GenericVisitor{
					[&](auto const& _item) { return _item->generate(); }
				}, child.first);
	return os.str();
}

void SourceState::print(std::ostream& _os) const
{
	for (auto const& import: importedSources)
		_os << "Imports: " << import << std::endl;
}

set<string> TestState::sourceUnitPaths() const
{
	set<string> keys;
	boost::copy(sourceUnitState | boost::adaptors::map_keys, std::inserter(keys, keys.begin()));
	return keys;
}

string TestState::randomPath(set<string> const& _sourceUnitPaths) const
{
	auto it = _sourceUnitPaths.begin();
	/// Advance iterator by n where 0 <= n <= sourceUnitPaths.size() - 1
	size_t increment = uRandDist->distributionOneToN(_sourceUnitPaths.size()) - 1;
	solAssert(
		increment >= 0 && increment < _sourceUnitPaths.size(),
		"Solc custom mutator: Invalid increment"
	);
	advance(it, increment);
	return *it;
}

string TestState::randomPath() const
{
	solAssert(!empty(), "Solc custom mutator: Null test state");
	return randomPath(sourceUnitPaths());
}

void TestState::print(std::ostream& _os) const
{
	_os << "Printing test state" << std::endl;
	for (auto const& item: sourceUnitState)
	{
		_os << "Source path: " << item.first << std::endl;
		item.second->print(_os);
	}
}

string TestState::randomNonCurrentPath() const
{
	/// To obtain a source path that is not the currently visited
	/// source unit itself, we require at least one other source
	/// unit to be previously visited.
	solAssert(size() >= 2, "Solc custom mutator: Invalid test state");

	set<string> filteredSourcePaths;
	string currentPath = currentSourceUnitPath;
	set<string> sourcePaths = sourceUnitPaths();
	copy_if(
		sourcePaths.begin(),
		sourcePaths.end(),
		inserter(filteredSourcePaths, filteredSourcePaths.begin()),
		[currentPath](string const& _item) {
			return _item != currentPath;
		}
	);
	return randomPath(filteredSourcePaths);
}

void TestCaseGenerator::setup()
{
	addGenerators({
		{mutator->generator<SourceUnitGenerator>(), s_maxSourceUnits}
	});
}

string TestCaseGenerator::visit()
{
	return visitChildren();
}

void SourceUnitGenerator::setup()
{
	addGenerators({
		{mutator->generator<ImportGenerator>(), s_maxImports},
		{mutator->generator<PragmaGenerator>(), 1},
		{mutator->generator<ContractGenerator>(), 1},
		{mutator->generator<FunctionGenerator>(), s_maxFreeFunctions}
	});
}

string SourceUnitGenerator::visit()
{
	state->addSource();
	ostringstream os;
	os << "\n"
	   << "==== Source: "
	   << state->currentPath()
	   << " ===="
	   << "\n";
	os << visitChildren();
	return os.str();
}

string PragmaGenerator::visit()
{
	set<string> pragmas;
	// Add preamble
	pragmas.insert(string(s_preamble));
	// Choose either abicoder v1 or v2 but not both.
	pragmas.insert(s_abiPragmas[uRandDist->distributionOneToN(s_abiPragmas.size()) - 1]);
	return boost::algorithm::join(pragmas, "\n") + "\n";
}

string ImportGenerator::visit()
{
	/*
	 * Case 1: No source units defined
	 * Case 2: One source unit defined
	 * Case 3: At least two source units defined
	 */
	ostringstream os;
	string importPath;
	// Import a different source unit if at least
	// two source units available.
	if (state->size() > 1)
		importPath = state->randomNonCurrentPath();
	// Do not reimport already imported source unit
	if (!importPath.empty() && !state->sourceUnitState[state->currentPath()]->sourcePathImported(importPath))
	{
		os << "import "
		   << "\""
		   << importPath
		   << "\";\n";
		state->sourceUnitState[state->currentPath()]->addImportedSourcePath(importPath);
		state->sourceUnitState[state->currentPath()]->resolveImports(
			state->sourceUnitState[importPath]->exports
		);
	}
	return os.str();
}

void ContractGenerator::setup()
{
	addGenerators({
		{mutator->generator<FunctionGenerator>(), s_maxFunctions}
	});
}

string ContractGenerator::visit()
{
	ScopeGuard reset([&]() {
		mutator->generator<FunctionGenerator>()->scope(true);
		state->unindent();
	});
	auto set = [&]() {
		state->indent();
		mutator->generator<FunctionGenerator>()->scope(false);
	};
	ostringstream os;
	string inheritance;
	if (state->sourceUnitState[state->currentPath()]->contractType())
		inheritance = state->sourceUnitState[state->currentPath()]->randomContract();
	string name = state->newContract();
	state->updateContract(name);
	os << "contract " << name;
	if (!inheritance.empty())
		os << " is " << inheritance;
	os << " {" << endl;
	set();
	os << visitChildren();
	os << "}" << endl;
	return os.str();
}

string FunctionType::toString()
{
	auto typeString = [](std::vector<SolidityTypePtr>& _types)
	{
		std::string sep;
		std::string typeStr;
		for (auto const& i: _types)
		{
			typeStr += sep + std::visit(GenericVisitor{
						[&](auto const& _item) { return _item->toString(); }
					}, i);
			if (sep.empty())
				sep = ",";
		}
		return typeStr;
	};

	std::string retString = std::string("function ") + "(" + typeString(inputs) + ")";
	if (outputs.empty())
		return retString + " external pure";
	else
		return retString + " external pure returns (" + typeString(outputs) +	")";
}

string FunctionState::params(Params _p)
{
	vector<string> params = (_p == Params::INPUT ? inputs : outputs) |
		ranges::views::transform(
		[](auto& _item) -> string
		{
			return visit(
				GenericVisitor{[](auto const& _item) {
					return _item->toString();
				}}, _item.first) +
				" " +
				_item.second;
		}) |
		ranges::to<vector<string>>();
	return "(" + boost::algorithm::join(params, ",") + ")";
}

string BlockStmtGenerator::visit()
{
	return "\n" + indentation() + "{}\n";
}

void FunctionGenerator::setup()
{
	addGenerators({{mutator->generator<BlockStmtGenerator>(), 1}});
}

string FunctionGenerator::visit()
{
	string visibility;
	string name = state->newFunction();
	state->updateFunction(name);
	if (!m_freeFunction)
		visibility = "external";

	// Add I/O
	if (uRandDist->likely(s_maxInputs + 1))
		for (unsigned i = 0; i < uRandDist->distributionOneToN(s_maxInputs); i++)
			state->currentFunctionState()->addInput(TypeProvider{state}.type());

	if (uRandDist->likely(s_maxOutputs + 1))
		for (unsigned i = 0; i < uRandDist->distributionOneToN(s_maxOutputs); i++)
			state->currentFunctionState()->addOutput(TypeProvider{state}.type());

	ostringstream function;
	function << indentation()
		<< "function "
		<< name
		<< state->currentFunctionState()->params(FunctionState::Params::INPUT)
		<< " "
		<< visibility
		<< " pure";
	if (!state->currentFunctionState()->outputs.empty())
		function << " returns"
			<< state->currentFunctionState()->params(FunctionState::Params::OUTPUT);
	function << generator<BlockStmtGenerator>()->visit();
	return function.str();
}

optional<SolidityTypePtr> TypeProvider::type(SolidityTypePtr _type)
{
	vector<SolidityTypePtr> matchingTypes = state->currentFunctionState()->inputs |
		ranges::views::filter([&_type](auto& _item) {
			return _item.first == _type;
		}) |
		ranges::views::transform([](auto& _item) { return _item.first; }) |
		ranges::to<vector<SolidityTypePtr>>();

	if (matchingTypes.empty())
		return nullopt;
	else
		return matchingTypes[state->uRandDist->distributionOneToN(matchingTypes.size()) - 1];
}

SolidityTypePtr TypeProvider::type()
{
	switch (randomTypeCategory())
	{
	case Type::INTEGER:
	{
		IntegerType::Bits b = static_cast<IntegerType::Bits>(
			state->uRandDist->distributionOneToN(
				static_cast<size_t>(IntegerType::Bits::B256)
			)
		);
		// Choose signed/unsigned type with probability of 1/2 = 0.5
		bool signedType = state->uRandDist->probable(2);
		return make_shared<IntegerType>(b, signedType);
	}
	case Type::BOOL:
		return make_shared<BoolType>();
	case Type::FIXEDBYTES:
	{
		FixedBytesType::Bytes w = static_cast<FixedBytesType::Bytes>(
			state->uRandDist->distributionOneToN(
				static_cast<size_t>(FixedBytesType::Bytes::W32)
			)
		);
		return make_shared<FixedBytesType>(w);
	}
	case Type::BYTES:
		return make_shared<BytesType>();
	case Type::ADDRESS:
		return make_shared<AddressType>();
	case Type::FUNCTION:
		return make_shared<FunctionType>();
	case Type::CONTRACT:
		if (state->sourceUnitState[state->currentPath()]->contractType())
			return state->sourceUnitState[state->currentPath()]->randomContractType();
		return make_shared<BoolType>();
	default:
		solAssert(false, "");
	}
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
	m_generators = {};
	m_urd = make_shared<UniformRandomDistribution>(make_unique<RandomEngine>(_seed));
	m_state = make_shared<TestState>(m_urd);
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
		std::visit(GenericVisitor{
			[&](auto const& _item) { return _item->setup(); }
		}, g);
	string program = generator<TestCaseGenerator>()->generate();
	destroyGenerators();
	return program;
}
