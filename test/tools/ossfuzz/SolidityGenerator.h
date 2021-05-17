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
#include <test/tools/ossfuzz/Types.h>

#include <liblangutil/Exceptions.h>

#include <memory>
#include <random>
#include <set>
#include <variant>

#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/transform.hpp>

namespace solidity::test::fuzzer::mutator
{
/// Forward declarations
class SolidityGenerator;

/// Type declarations
#define SEMICOLON() ;
#define FORWARDDECLARE(G) class G
GENERATORLIST(FORWARDDECLARE, SEMICOLON(), SEMICOLON())
TYPELIST(FORWARDDECLARE, SEMICOLON(), SEMICOLON())
#undef FORWARDDECLARE
#undef SEMICOLON

#define COMMA() ,
using Generator = std::variant<
#define VARIANTOFGENERATOR(G) G
GENERATORLIST(VARIANTOFGENERATOR, COMMA(), )
>;
using GeneratorPtr = std::variant<
#define VARIANTOFSHARED(G) std::shared_ptr<G>
GENERATORLIST(VARIANTOFSHARED, COMMA(), )
>;
using SolidityTypePtr = std::variant<
TYPELIST(VARIANTOFSHARED, COMMA(), )
>;
#undef VARIANTOFSHARED
#undef COMMA
using RandomEngine = std::minstd_rand;
using Distribution = std::uniform_int_distribution<size_t>;

struct UniformRandomDistribution
{
	explicit UniformRandomDistribution(std::unique_ptr<RandomEngine> _randomEngine):
		randomEngine(std::move(_randomEngine))
	{}

	~UniformRandomDistribution()
	{
		randomEngine.reset();
	}

	/// @returns an unsigned integer in the range [1, @param _n] chosen
	/// uniformly at random.
	[[nodiscard]] size_t distributionOneToN(size_t _n) const
	{
		solAssert(_n > 0, "");
		return Distribution(1, _n)(*randomEngine);
	}
	/// @returns true with a probability of 1/(@param _n), false otherwise.
	/// @param _n > 1.
	[[nodiscard]] bool probable(size_t _n) const
	{
		solAssert(_n > 1, "");
		return distributionOneToN(_n) == 1;
	}
	/// @returns true with a probability of 1 - 1/(@param _n),
	/// false otherwise.
	/// @param _n > 1.
	[[nodiscard]] bool likely(size_t _n) const
	{
		solAssert(_n > 1, "");
		return !probable(_n);
	}
	/// @returns a subset whose elements are of type @param T
	/// created from the set @param _container using
	/// uniform selection.
	template <typename T>
	std::set<T> subset(std::set<T> const& _container)
	{
		size_t s = _container.size();
		solAssert(s > 1, "");
		std::set<T> subContainer;
		for (auto const& item: _container)
			if (probable(s))
				subContainer.insert(item);
		return subContainer;
	}
	std::unique_ptr<RandomEngine> randomEngine;
};

class SolType
{
public:
	virtual ~SolType() = default;
	virtual std::string toString() = 0;
};

class IntegerType: public SolType
{
public:
	enum class Bits: size_t
	{
		B8 = 1,
		B16,
		B24,
		B32,
		B40,
		B48,
		B56,
		B64,
		B72,
		B80,
		B88,
		B96,
		B104,
		B112,
		B120,
		B128,
		B136,
		B144,
		B152,
		B160,
		B168,
		B176,
		B184,
		B192,
		B200,
		B208,
		B216,
		B224,
		B232,
		B240,
		B248,
		B256
	};

	IntegerType(
		Bits _bits,
		bool _signed
	):
		signedType(_signed),
		numBits(static_cast<size_t>(_bits) * 8)
	{}
	bool operator>=(IntegerType const& _rhs)
	{
		return this->signedType == _rhs.signedType &&
			this->numBits >= _rhs.numBits;
	}
	std::string toString() override
	{
		return (signedType ? "int" : "uint") + std::to_string(numBits);
	}
	bool signedType;
	size_t numBits;
};

class BoolType: public SolType
{
public:
	std::string toString() override
	{
		return "bool";
	}
	bool operator>=(BoolType const&)
	{
		return true;
	}
};

class AddressType: public SolType
{
public:
	// TODO: Implement address payable
	std::string toString() override
	{
		return "address";
	}
	bool operator>=(AddressType const&)
	{
		return true;
	}
};

class FixedBytesType: public SolType
{
public:
	enum class Bytes: size_t
	{
		W1 = 1,
		W2,
		W3,
		W4,
		W5,
		W6,
		W7,
		W8,
		W9,
		W10,
		W11,
		W12,
		W13,
		W14,
		W15,
		W16,
		W17,
		W18,
		W19,
		W20,
		W21,
		W22,
		W23,
		W24,
		W25,
		W26,
		W27,
		W28,
		W29,
		W30,
		W31,
		W32
	};
	FixedBytesType(Bytes _width): numBytes(static_cast<size_t>(_width))
	{}

	bool operator>=(FixedBytesType const& _rhs)
	{
		return this->numBytes == _rhs.numBytes;
	}
	std::string toString() override
	{
		return "bytes" + std::to_string(numBytes);
	}
	size_t numBytes;
};

class BytesType: public SolType
{
public:
	std::string toString() override
	{
		return "bytes memory";
	}
	bool operator>=(BytesType const&)
	{
		return true;
	}
};

class ContractType: public SolType
{
public:
	ContractType(std::string _name): contractName(_name)
	{}
	std::string toString() override
	{
		return name();
	}
	std::string name() const
	{
		return contractName;
	}
	bool operator>=(ContractType const& _rhs)
	{
		return _rhs.name() == this->name();
	}
	std::string contractName;
};

class FunctionType: public SolType
{
public:
	FunctionType(bool _freeFunction)
	{
		freeFunction = _freeFunction;
	}
	~FunctionType() override
	{
		inputs.clear();
		outputs.clear();
	}

	void addInput(SolidityTypePtr _input)
	{
		inputs.emplace_back(_input);
	}

	void addOutput(SolidityTypePtr _output)
	{
		outputs.emplace_back(_output);
	}

	bool functionScope()
	{
		return freeFunction;
	}

	std::string toString() override;
	bool operator>=(FunctionType const& _rhs)
	{
		if (_rhs.inputs.size() != this->inputs.size() || _rhs.outputs.size() != this->outputs.size())
			return false;
		if (!std::equal(_rhs.inputs.begin(), _rhs.inputs.end(), this->inputs.begin()) ||
			!std::equal(_rhs.outputs.begin(), _rhs.outputs.end(), this->outputs.begin())
		)
			return false;
		return true;
	}

	std::vector<SolidityTypePtr> inputs;
	std::vector<SolidityTypePtr> outputs;
	bool freeFunction;
};

/// Forward declaration
struct TestState;
struct FunctionState;

struct SourceState
{
	explicit SourceState(
		std::shared_ptr<UniformRandomDistribution>& _urd,
		std::string _sourceName
	):
		uRandDist(_urd),
		importedSources({}),
		sourceName(_sourceName)
	{}
	void addFreeFunction(std::string& _functionName)
	{
		exports[std::make_shared<FunctionType>(true)] = _functionName;
	}
	void addFreeFunction(std::shared_ptr<FunctionState> _state)
	{
		freeFunctions.emplace(_state);
	}
	bool freeFunction(std::string const& _functionName)
	{
		return !(exports | ranges::views::filter([&_functionName](auto& _p) { return _p.second == _functionName; })).empty();
	}
	bool contractType()
	{
		return !(exports | ranges::views::filter([](auto& _i) {
			return std::holds_alternative<std::shared_ptr<ContractType>>(_i.first);
		})).empty();
	}
	std::string randomContract()
	{
		auto contracts = exports |
			ranges::views::filter([](auto& _item) -> bool {
				return std::holds_alternative<std::shared_ptr<ContractType>>(
					_item.first
				);
			}) |
			ranges::views::transform([](auto& _item) -> std::string {
				return _item.second;
			}) | ranges::to<std::vector<std::string>>;
		return contracts[uRandDist->distributionOneToN(contracts.size()) - 1];
	}
	std::shared_ptr<ContractType> randomContractType()
	{
		auto contracts = exports |
			ranges::views::filter([](auto& _item) -> bool {
				return std::holds_alternative<std::shared_ptr<ContractType>>(_item.first);
			}) |
			ranges::views::transform([](auto& _item) -> std::shared_ptr<ContractType> {
				return std::get<std::shared_ptr<ContractType>>(_item.first);
			}) |
			ranges::to<std::vector<std::shared_ptr<ContractType>>>;
		return contracts[uRandDist->distributionOneToN(contracts.size()) - 1];
	}
	void addImportedSourcePath(std::string& _sourcePath)
	{
		importedSources.emplace(_sourcePath);
	}
	void resolveImports(std::map<SolidityTypePtr, std::string> _importedSymbols);
	void mergeFunctionState(std::set<std::shared_ptr<FunctionState>> _importedFreeFunctions);

	[[nodiscard]] bool sourcePathImported(std::string const& _sourcePath) const
	{
		return importedSources.count(_sourcePath);
	}
	~SourceState()
	{
		importedSources.clear();
		freeFunctions.clear();
		exports.clear();
		uRandDist.reset();
	}
	/// Prints source state to @param _os.
	void print(std::ostream& _os) const;
	std::shared_ptr<UniformRandomDistribution>& uRandDist;
	std::set<std::string> importedSources;
	std::map<SolidityTypePtr, std::string> exports;
	std::set<std::shared_ptr<FunctionState>> freeFunctions;
	std::string sourceName;
};

struct BlockScope
{
	BlockScope() = default;
	~BlockScope()
	{
		variables.clear();
	}
	std::vector<std::pair<SolidityTypePtr, std::string>> variables;
};

struct FunctionState
{
	enum class Params
	{
		INPUT,
		OUTPUT
	};
	FunctionState(std::string _functionName, bool _freeFunction):
		numInputs(0),
		numOutpus(0),
		numLocals(0),
		name(_functionName)
	{
		type = std::make_shared<FunctionType>(_freeFunction);
	}
	~FunctionState()
	{
		inputs.clear();
		outputs.clear();
		scopes.clear();
		type.reset();
	}
	void addInput(SolidityTypePtr _input)
	{
		type->addInput(_input);
		inputs.emplace_back(std::move(_input), "i" + std::to_string(numInputs++));
	}
	void addOutput(SolidityTypePtr _output)
	{
		type->addOutput(_output);
		outputs.emplace_back(std::move(_output), "o" + std::to_string(numOutpus++));
	}
	void addLocal(SolidityTypePtr _local)
	{
		scopes.back()->variables.emplace_back(std::move(_local), "l" + std::to_string(numLocals++));
	}
	std::string params(Params _p);

	std::vector<std::pair<SolidityTypePtr, std::string>> inputs;
	std::vector<std::pair<SolidityTypePtr, std::string>> outputs;
	std::vector<std::shared_ptr<BlockScope>> scopes;
	std::shared_ptr<FunctionType> type;
	unsigned numInputs;
	unsigned numOutpus;
	unsigned numLocals;
	std::string name;
};

struct ContractState
{
	explicit ContractState(
		std::shared_ptr<UniformRandomDistribution>& _urd,
		std::string _contractName
	):
		uRandDist(_urd),
		name(_contractName)
	{}
	~ContractState()
	{
		functions.clear();
		uRandDist.reset();
	}
	void addFunction(std::shared_ptr<FunctionState> _function)
	{
		functions.emplace(_function);
	}

	std::set<std::shared_ptr<FunctionState>> functions;
	std::shared_ptr<UniformRandomDistribution>& uRandDist;
	std::string name;
};

struct TestState
{
	explicit TestState(std::shared_ptr<UniformRandomDistribution>& _urd):
		sourceUnitState({}),
		contractState({}),
		currentSourceUnitPath({}),
		currentContract({}),
		currentFunction({}),
		uRandDist(_urd),
		numSourceUnits(0),
		numContracts(0),
		numFunctions(0),
		indentationLevel(0),
		insideContract(false)
	{}
	/// Adds @param _path to @name sourceUnitPaths updates
	/// @name currentSourceUnitPath.
	void addSourceUnit(std::string const& _path)
	{
		auto sourceState = std::make_shared<SourceState>(uRandDist, _path);
		sourceUnitState.emplace(_path, std::move(sourceState));
		currentSourceUnitPath = _path;
	}
	/// Adds @param _name to @name contractState updates
	/// @name currentContract.
	void addContract(std::string const& _name)
	{
		auto newContractState = std::make_shared<ContractState>(uRandDist, _name);
		contractState.emplace(_name, std::move(newContractState));
		auto newContractType = std::make_shared<ContractType>(_name);
		sourceUnitState[currentSourceUnitPath]->exports[std::move(newContractType)] = _name;
		currentContract = _name;
	}
	void addFunction(std::string const& _name, bool _freeFunction)
	{
		auto fState = std::make_shared<FunctionState>(_name, _freeFunction);
		functionState.emplace(_name, std::move(fState));
		currentFunction = _name;
	}
	std::shared_ptr<FunctionState> currentFunctionState()
	{
		std::string function = currentFunctionName();
		return functionState[function];
	}
	std::shared_ptr<SourceState> currentSourceState()
	{
		std::string currentSource = currentPath();
		return sourceUnitState[currentSource];
	}
	std::shared_ptr<ContractState> currentContractState()
	{
		std::string contract = currentContractName();
		return contractState[contract];
	}
	/// Returns true if @name sourceUnitPaths is empty,
	/// false otherwise.
	[[nodiscard]] bool empty() const
	{
		return sourceUnitState.empty();
	}
	/// Returns the number of items in @name sourceUnitPaths.
	[[nodiscard]] size_t size() const
	{
		return sourceUnitState.size();
	}
	/// Returns a new source path name that is formed by concatenating
	/// a static prefix @name m_sourceUnitNamePrefix, a monotonically
	/// increasing counter starting from 0 and the postfix (extension)
	/// ".sol".
	[[nodiscard]] std::string newPath() const
	{
		return sourceUnitNamePrefix + std::to_string(numSourceUnits) + ".sol";
	}
	[[nodiscard]] std::string newContract() const
	{
		return contractPrefix + std::to_string(numContracts);
	}
	[[nodiscard]] std::string newFunction() const
	{
		return functionPrefix + std::to_string(numFunctions);
	}
	[[nodiscard]] std::string currentPath() const
	{
		solAssert(numSourceUnits > 0, "");
		return currentSourceUnitPath;
	}
	std::string currentFunctionName() const
	{
		solAssert(numFunctions > 0, "");
		return currentFunction;
	}
	std::string currentContractName() const
	{
		solAssert(numContracts > 0, "");
		return currentContract;
	}
	/// Adds @param _path to list of source paths in global test
	/// state and increments @name m_numSourceUnits.
	void updateSourcePath(std::string const& _path)
	{
		addSourceUnit(_path);
		numSourceUnits++;
	}
	/// Adds @param _contract to list of contracts in global test state and
	/// increments @name numContracts
	void updateContract(std::string const& _name)
	{
		addContract(_name);
		numContracts++;
	}
	void updateFunction(std::string const& _name, bool _freeFunction)
	{
		addFunction(_name, _freeFunction);
		numFunctions++;
	}
	void addSource()
	{
		std::string path = newPath();
		updateSourcePath(path);
	}
	/// Increments indentation level globally.
	void indent()
	{
		++indentationLevel;
	}
	/// Decrements indentation level globally.
	void unindent()
	{
		--indentationLevel;
	}
	void enterContract()
	{
		insideContract = true;
	}
	void exitContract()
	{
		insideContract = false;
	}
	~TestState()
	{
		sourceUnitState.clear();
		contractState.clear();
		functionState.clear();
		uRandDist.reset();
	}
	/// Prints test state to @param _os.
	void print(std::ostream& _os) const;
	/// Returns a randomly chosen path from @param _sourceUnitPaths.
	[[nodiscard]] std::string randomPath(std::set<std::string> const& _sourceUnitPaths) const;
	[[nodiscard]] std::set<std::string> sourceUnitPaths() const;
	/// Returns a randomly chosen path from @name sourceUnitPaths.
	[[nodiscard]] std::string randomPath() const;
	/// Returns a randomly chosen non current source unit path.
	[[nodiscard]] std::string randomNonCurrentPath() const;
	/// Map of source name -> state
	std::map<std::string, std::shared_ptr<SourceState>> sourceUnitState;
	/// Map of contract name -> state
	std::map<std::string, std::shared_ptr<ContractState>> contractState;
	/// Map of function name -> state
	std::map<std::string, std::shared_ptr<FunctionState>> functionState;
	/// Source path being currently visited.
	std::string currentSourceUnitPath;
	/// Current contract
	std::string currentContract;
	/// Current function
	std::string currentFunction;
	/// Uniform random distribution.
	std::shared_ptr<UniformRandomDistribution>& uRandDist;
	/// Number of source units in test input
	size_t numSourceUnits;
	/// Number of contracts in test input
	size_t numContracts;
	/// Number of functions in test input
	size_t numFunctions;
	/// Indentation level
	unsigned indentationLevel;
	/// Contract scope
	bool insideContract;
	/// Source name prefix
	std::string const sourceUnitNamePrefix = "su";
	/// Contract name prefix
	std::string const contractPrefix = "C";
	/// Function name prefix
	std::string const functionPrefix = "f";
};

struct TypeProvider
{
	TypeProvider(std::shared_ptr<TestState>& _state): state(_state)
	{}

	enum class Type: size_t
	{
		INTEGER = 1,
		BOOL,
		FIXEDBYTES,
		BYTES,
		ADDRESS,
		FUNCTION,
		CONTRACT,
		TYPEMAX
	};

	SolidityTypePtr type();
	std::optional<SolidityTypePtr> type(SolidityTypePtr _typePtr);

	Type randomTypeCategory()
	{
		return static_cast<Type>(state->uRandDist->distributionOneToN(static_cast<size_t>(Type::TYPEMAX) - 1));
	}

	std::shared_ptr<TestState>& state;
};

struct TypeComparator
{
	template <typename T>
	bool operator()(T _i1, T _i2)
	{
		return *_i1 >= *_i2;
	}
	template <typename T1, typename T2>
	bool operator()(T1 _i1, T2 _i2)
	{
		if (std::is_same_v<T1, T2>)
			return this->template operator()(_i1, _i2);
		return false;
	}
};

struct LiteralGenerator
{
	explicit LiteralGenerator(std::shared_ptr<TestState>& _state): state(_state)
	{}

	std::string operator()(std::shared_ptr<AddressType> const& _type);
	std::string operator()(std::shared_ptr<BoolType> const& _type);
	std::string operator()(std::shared_ptr<BytesType> const& _type);
	std::string operator()(std::shared_ptr<ContractType> const& _type);
	std::string operator()(std::shared_ptr<FixedBytesType> const& _type);
	std::string operator()(std::shared_ptr<FunctionType> const& _type);
	std::string operator()(std::shared_ptr<IntegerType> const& _type);

	std::shared_ptr<TestState>& state;
};

struct ExpressionGenerator
{
	ExpressionGenerator(std::shared_ptr<TestState>& _state): state(_state)
	{}

	enum class RLValueExpr: size_t
	{
		VARREF = 1,
		PINC,
		PDEC,
		SINC,
		SDEC,
		NOT,
		BITNOT,
		USUB,
		EXP,
		MUL,
		DIV,
		MOD,
		ADD,
		BSUB,
		SHL,
		SHR,
		BITAND,
		BITXOR,
		BITOR,
		LT,
		GT,
		LTE,
		GTE,
		EQ,
		NEQ,
		AND,
		OR,
		LIT,
		RLMAX
	};

	std::optional<std::pair<SolidityTypePtr, std::string>> rOrLValueExpression(
		std::pair<SolidityTypePtr, std::string> _typeName
	);
	std::optional<std::pair<SolidityTypePtr, std::string>> literal(SolidityTypePtr _type);
	std::optional<std::pair<SolidityTypePtr, std::string>> randomLValueExpression();
	std::optional<std::pair<SolidityTypePtr, std::string>> lValueExpression(
		std::pair<SolidityTypePtr, std::string> _typeName
	);
	std::vector<std::pair<SolidityTypePtr, std::string>> liveVariables();
	std::vector<std::pair<SolidityTypePtr, std::string>> liveVariables(
		std::pair<SolidityTypePtr, std::string> _typeName
	);
	std::optional<std::pair<SolidityTypePtr, std::string>> unaryExpression(
		std::pair<SolidityTypePtr, std::string>& _typeName,
		std::string const& _op
	);
	std::optional<std::pair<SolidityTypePtr, std::string>> binaryExpression(
		std::pair<SolidityTypePtr, std::string>& _typeName,
		std::string const& _op
	);
	std::optional<std::pair<SolidityTypePtr, std::string>> incDecOperation(
		std::pair<SolidityTypePtr, std::string>& _typeName,
		std::string const& _op,
		bool _prefixOp
	);
	std::optional<std::pair<SolidityTypePtr, std::string>> rLValueOrLiteral(
		std::pair<SolidityTypePtr, std::string>& _typeName
	);


	void incrementNestingDepth()
	{
		nestingDepth++;
	}
	void resetNestingDepth()
	{
		nestingDepth = 0;
	}
	bool deeplyNested()
	{
		return nestingDepth > s_maxNestingDepth;
	}
	std::shared_ptr<TestState>& state;
	unsigned nestingDepth;
	static constexpr unsigned s_maxNestingDepth = 30;
};

class SolidityGenerator
{
public:
	explicit SolidityGenerator(unsigned _seed);

	~SolidityGenerator()
	{
		m_generators.clear();
		m_urd.reset();
		m_state.reset();
	}

	/// @returns the generator of type @param T.
	template <typename T>
	std::shared_ptr<T> generator();
	/// @returns a shared ptr to underlying random
	/// number distribution.
	std::shared_ptr<UniformRandomDistribution>& uniformRandomDist()
	{
		return m_urd;
	}
	/// @returns a pseudo randomly generated test case.
	std::string generateTestProgram();
	/// @returns shared ptr to global test state.
	std::shared_ptr<TestState>& testState()
	{
		return m_state;
	}
private:
	template <typename T>
	void createGenerator()
	{
		auto generator = std::make_shared<T>(this);
		m_generators.insert(std::move(generator));
	}
	template <std::size_t I = 0>
	void createGenerators();
	/// Sub generators
	std::set<GeneratorPtr> m_generators;
	/// Shared global test state
	std::shared_ptr<TestState> m_state;
	/// Uniform random distribution
	std::shared_ptr<UniformRandomDistribution> m_urd;
};

struct GeneratorBase
{
	explicit GeneratorBase(SolidityGenerator* _mutator);
	template <typename T>
	std::shared_ptr<T> generator()
	{
		for (auto& g: generators)
			if (std::holds_alternative<std::shared_ptr<T>>(g.first))
				return std::get<std::shared_ptr<T>>(g.first);
		solAssert(false, "");
	}
	/// @returns test fragment created by this generator.
	std::string generate()
	{
		std::string generatedCode = visit();
		endVisit();
		return generatedCode;
	}
	/// @returns current indentation as string. Each indentation level comprises
	/// two whitespace characters.
	std::string indentation()
	{
		return std::string(state->indentationLevel * 2, ' ');
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
	void addGenerators(std::set<std::pair<GeneratorPtr, unsigned>> _generators)
	{
		generators += std::move(_generators);
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
	std::shared_ptr<UniformRandomDistribution>& uRandDist()
	{
		return mutator->uniformRandomDist();
	}

	/// Shared pointer to the mutator instance
	SolidityGenerator* mutator;
	/// Set of generators used by this generator.
	std::set<std::pair<GeneratorPtr, unsigned>> generators;
	/// Shared ptr to global test state.
	std::shared_ptr<TestState>& state;
};

class TestCaseGenerator: public GeneratorBase
{
public:
	explicit TestCaseGenerator(SolidityGenerator* _mutator):
		GeneratorBase(std::move(_mutator))
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
	explicit SourceUnitGenerator(SolidityGenerator* _mutator):
		GeneratorBase(std::move(_mutator))
	{}
	void setup() override;
	std::string visit() override;
	std::string name() override { return "Source unit generator"; }
private:
	static unsigned constexpr s_maxImports = 2;
	static unsigned constexpr s_maxFreeFunctions = 2;
};

class PragmaGenerator: public GeneratorBase
{
public:
	explicit PragmaGenerator(SolidityGenerator* _mutator):
		GeneratorBase(std::move(_mutator))
	{}
	std::string visit() override;
	std::string name() override { return "Pragma generator"; }
private:
	static constexpr char const* s_preamble = R"(pragma solidity >= 0.0.0;)";
	std::vector<std::string> const s_abiPragmas = {
		R"(pragma abicoder v1;)",
		R"(pragma abicoder v2;)"
	};
};

class ImportGenerator: public GeneratorBase
{
public:
	explicit ImportGenerator(SolidityGenerator* _mutator):
	       GeneratorBase(std::move(_mutator))
	{}
	std::string visit() override;
	std::string name() override { return "Import generator"; }
};

class ContractGenerator: public GeneratorBase
{
public:
	explicit ContractGenerator(SolidityGenerator* _mutator):
		GeneratorBase(std::move(_mutator))
	{}
	void setup() override;
	std::string visit() override;
	std::string name() override { return "Contract generator"; }
private:
	static unsigned constexpr s_maxFunctions = 4;
};

class FunctionGenerator: public GeneratorBase
{
public:
	explicit FunctionGenerator(SolidityGenerator* _mutator):
		GeneratorBase(std::move(_mutator)),
		m_freeFunction(true)
	{}
	std::string visit() override;
	std::string name() override { return "Function generator"; }
	void endVisit() override;
	void setup() override;
	/// Sets @name m_freeFunction to @param _freeFunction.
	void scope(bool _freeFunction)
	{
		m_freeFunction = _freeFunction;
	}
private:
	bool m_freeFunction;
	static constexpr unsigned s_maxInputs = 4;
	static constexpr unsigned s_maxOutputs = 4;
	static constexpr unsigned s_maxStatements = 5;
};

class StatementGenerator: public GeneratorBase
{
public:
	explicit StatementGenerator(SolidityGenerator* _mutator):
		GeneratorBase(std::move(_mutator))
	{}
	void setup() override;
	std::string visit() override;
	std::string name() override { return "Statement generator"; }
private:
	static constexpr unsigned s_uncheckedBlockInvProb = 37;
};

class AssignmentStmtGenerator: public GeneratorBase
{
public:
	enum class AssignOp: size_t
	{
		ASSIGN = 1,
		ASSIGNBITOR,
		ASSIGNBITXOR,
		ASSIGNBITAND,
		ASSIGNSHL,
		ASSIGNSAR,
		ASSIGNSHR,
		ASSIGNADD,
		ASSIGNSUB,
		ASSIGNMUL,
		ASSIGNDIV,
		ASSIGNMOD,
		ASSIGNMAX
	};
	explicit AssignmentStmtGenerator(SolidityGenerator* _mutator):
		GeneratorBase(std::move(_mutator))
	{}
	std::string visit() override;
	std::string name() override { return "Assignment statement generator"; }
private:
	AssignOp assignOp(SolidityTypePtr _type);
	std::string assignOp(AssignOp _op);
};

class BlockStmtGenerator: public GeneratorBase
{
public:
	explicit BlockStmtGenerator(SolidityGenerator* _mutator):
		GeneratorBase(std::move(_mutator)),
		m_nestingDepth(0),
		m_unchecked(false),
		m_inUnchecked(false)
	{}
	void endVisit() override
	{
		decrementNestingDepth();
	}
	void incrementNestingDepth()
	{
		++m_nestingDepth;
	}
	void decrementNestingDepth()
	{
		--m_nestingDepth;
	}
	void resetNestingDepth()
	{
		m_nestingDepth = 0;
	}
	bool nestingTooDeep()
	{
		return m_nestingDepth > s_maxNestingDepth;
	}
	void setup() override;
	std::string visit() override;
	std::string name() override { return "Block statement generator"; }
	void unchecked(bool _unchecked)
	{
		m_unchecked = _unchecked;
	}
	bool unchecked()
	{
		return m_unchecked;
	}
	void resetInUnchecked()
	{
		m_inUnchecked = false;
	}
private:
	size_t m_nestingDepth;
	bool m_unchecked;
	bool m_inUnchecked;
	static constexpr unsigned s_maxStatements = 4;
	static constexpr unsigned s_maxNestingDepth = 1;
	static constexpr size_t s_uncheckedInvProb = 13;
};

class FunctionCallGenerator: public GeneratorBase
{
public:
	FunctionCallGenerator(SolidityGenerator* _mutator):
		GeneratorBase(std::move(_mutator))
	{}
	std::string visit() override;
	std::string name() override
	{
		return "Function call generator";
	}
private:
	std::string lhs(std::vector<std::pair<SolidityTypePtr, std::string>> _functionReturnTypeNames);
	std::optional<std::string> rhs(std::vector<std::pair<SolidityTypePtr, std::string>> _functionInputTypeNames);
	std::string callStmt(std::shared_ptr<FunctionState> _callee);
};
}
