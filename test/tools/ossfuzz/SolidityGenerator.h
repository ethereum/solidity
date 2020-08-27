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

#include <libsolutil/Whiskers.h>

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
	enum class NumberLiteral
	{
		DECIMAL,
		HEX
	};

	/// @returns an unsigned integer in the range [1, @param _n] chosen
	/// uniformly at random.
	static size_t distributionOneToN(size_t _n, std::shared_ptr<RandomEngine> _rand)
	{
		return Distribution(1, _n)(*_rand);
	}
	static bool chooseOneOfN(size_t _n, std::shared_ptr<RandomEngine> _rand)
	{
		return distributionOneToN(_n, _rand) == 1;
	}
	static std::string chooseOneOfNStrings(
		std::vector<std::string> const& _list,
		std::shared_ptr<RandomEngine> _rand
	)
	{
		return _list[GenerationProbability{}.distributionOneToN(_list.size(), _rand) - 1];
	}
	static std::string generateRandomAsciiString(size_t _length, std::shared_ptr<RandomEngine> _rand);
	static std::string generateRandomHexString(size_t _length, std::shared_ptr<RandomEngine> _rand);
	static std::pair<NumberLiteral, std::string> generateRandomNumberLiteral(size_t _length, std::shared_ptr<RandomEngine> _rand);
};

struct AddDependenciesVisitor
{
	template<typename T>
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

struct NameVisitor
{

	template <typename T>
	std::string operator()(T const& _t)
	{
		return _t->name();
	}
};


/// Forward declarations
#define SEMICOLON() ;
#define COMMA() ,
#define EMPTY()
#define FORWARDDECLAREGENERATORS(G) class G
GENERATORLIST(FORWARDDECLAREGENERATORS, SEMICOLON(), SEMICOLON())
#undef FORWARDDECLAREGENERATORS
class SolidityGenerator;
struct TestState;

/// Type declarations
using GeneratorPtr = std::variant<
#define VARIANTOFSHARED(G) std::shared_ptr<G>
GENERATORLIST(VARIANTOFSHARED, COMMA(), EMPTY())
>;
#undef VARIANTOFSHARED

using Generator = std::variant<
#define VARIANTOFGENERATOR(G) G
GENERATORLIST(VARIANTOFGENERATOR, COMMA(), EMPTY())
>;
#undef VARIANTOFGENERATOR
#undef EMPTY
#undef COMMA
#undef SEMICOLON

struct GeneratorBase
{
	GeneratorBase(std::shared_ptr<SolidityGenerator> _mutator);
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
	GeneratorPtr randomGenerator();
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
	std::shared_ptr<TestState> state;
};

//class FunctionTypeGenerator: public GeneratorBase
//{
//public:
//	FunctionTypeGenerator(std::shared_ptr<SolidityGenerator> _mutator):
//		GeneratorBase(std::move(_mutator))
//	{}
//	void setup() override;
//	void reset() override {}
//	std::string name() override
//	{
//		return "Function type generator";
//	}
//	std::string visit() override;
//private:
//	static const std::vector<std::string> s_visibility;
//	std::string const m_functionTypeTemplate =
//		std::string(R"(function (<paramList>) )") +
//		R"(<visibility> <stateMutability>)" +
//		R"(<?return> returns (<retParamList>)</return>)";
//};
//
//class UserDefinedTypeGenerator: public GeneratorBase
//{
//public:
//	UserDefinedTypeGenerator(std::shared_ptr<SolidityGenerator> _mutator):
//		GeneratorBase(std::move(_mutator))
//	{}
//	void setup() override;
//	std::string visit() override;
//	void reset() override {}
//	std::string name() override
//	{
//		return "User defined type generator";
//	}
//};

struct SolidityType
{
	TYPE_ENUM_DECLS(Address, ADDRESS)
	TYPE_ENUM_DECLS(Bool, BOOL)
	TYPE_ENUM_DECLS(
		Bytes,
		BOOST_PP_REPEAT_FROM_TO(1, 34, BYTES_ENUM_ELEM, unused)
	)
	TYPE_ENUM_DECLS(
		Integer,
		BOOST_PP_REPEAT_FROM_TO(0, 32, INTEGER_ENUM_ELEM, INT),
		BOOST_PP_REPEAT_FROM_TO(0, 32, INTEGER_ENUM_ELEM, UINT)
	)
	enum class TypeCategory: size_t
	{
		BOOL = 0,
		ADDRESS,
		INTEGER,
		BYTES,
		TYPEMAX
	};
	using Type = std::variant<Address, Bool, Bytes, Integer>;
	struct TypeStringVisitor
	{
		template <typename T>
		std::string operator()(T const& _type)
		{
			return toString(_type);
		}
	};
	struct TypeIndexVisitor
	{
		template <typename T>
		size_t operator()(T const& _type)
		{
			return static_cast<size_t>(_type);
		}
	};
	SolidityType(TypeCategory _type, std::shared_ptr<RandomEngine> _rand):
		typeCategory(_type),
		randomEngine(std::move(_rand))
	{
		switch (typeCategory)
		{
		case TypeCategory::ADDRESS:
			type = indexedAddressType(0);
			break;
		case TypeCategory::BOOL:
			type = indexedBoolType(0);
			break;
		case TypeCategory::BYTES:
			type = indexedBytesType(GenerationProbability{}.distributionOneToN(size_t(Bytes::BYTES) + 1, randomEngine) - 1);
			break;
		case TypeCategory::INTEGER:
			type = indexedIntegerType(GenerationProbability{}.distributionOneToN(size_t(Integer::UINT256) + 1, randomEngine) - 1);
			break;
		case TypeCategory::TYPEMAX:
			solAssert(false, "");
		}
	}
	void setValueType()
	{
		switch (typeCategory)
		{
		case TypeCategory::ADDRESS:
			type = indexedAddressType(0);
			break;
		case TypeCategory::BOOL:
			type = indexedBoolType(0);
			break;
		case TypeCategory::BYTES:
			type = indexedBytesType(GenerationProbability{}.distributionOneToN(size_t(Bytes::BYTES), randomEngine) - 1);
			break;
		case TypeCategory::INTEGER:
			type = indexedIntegerType(GenerationProbability{}.distributionOneToN(size_t(Integer::UINT256) + 1, randomEngine) - 1);
			break;
		case TypeCategory::TYPEMAX:
			solAssert(false, "");
		}
	}
	void setNonValueType()
	{
		type = indexedBytesType(size_t(Bytes::BYTES));
	}
	virtual ~SolidityType() = default;
	TypeCategory typeCategory;
	std::pair<Type, std::string> type;
	std::shared_ptr<RandomEngine> randomEngine;
};

class ExpressionGenerator: public GeneratorBase
{
public:
	enum Type
	{
		INDEXACCESS = 0ul,
		INDEXRANGEACCESS,
		MEMBERACCESS,
		FUNCTIONCALLOPTIONS,
		FUNCTIONCALL,
		PAYABLECONVERSION,
		METATYPE,
		UNARYPREFIXOP,
		UNARYSUFFIXOP,
		EXPOP,
		MULDIVMODOP,
		ADDSUBOP,
		SHIFTOP,
		BITANDOP,
		BITXOROP,
		BITOROP,
		ORDERCOMPARISON,
		EQUALITYCOMPARISON,
		ANDOP,
		OROP,
		CONDITIONAL,
		ASSIGNMENT,
		NEWEXPRESSION,
		TUPLE,
		INLINEARRAY,
		IDENTIFIER,
		LITERAL,
		ELEMENTARYTYPENAME,
		USERDEFINEDTYPENAME,
		TYPEMAX
	};
	ExpressionGenerator(
		std::shared_ptr<SolidityGenerator> _mutator,
		bool _compileTimeConstantExpressionsOnly = false
	):
		GeneratorBase(std::move(_mutator)),
		m_expressionNestingDepth(0),
		m_compileTimeConstantExpressionsOnly(_compileTimeConstantExpressionsOnly),
		m_type(randomTypeCategory((*rand)()), rand)
	{}
	void setup() override {}
	std::string visit() override;
	void endVisit() override
	{
		m_expressionNestingDepth = 0;
	}
	std::string name() override
	{
		return "Expression Generator";
	}
	std::string typeString()
	{
		return m_type.type.second;
	}
	std::string randomTypeString()
	{
		return SolidityType(randomTypeCategory((*rand)()), rand).type.second;
	}
	std::shared_ptr<SolidityType> randomType()
	{
		return std::make_shared<SolidityType>(
			SolidityType(randomTypeCategory((*rand)()), rand)
		);
	}
	void setType(std::shared_ptr<SolidityType> _type)
	{
		m_type = *_type;
	}
	void setType(SolidityType _type)
	{
		m_type = _type;
	}
	void setValueType()
	{
		m_type.setValueType();
	}
	void setNonValueType()
	{
		m_type.setNonValueType();
	}
private:
	static SolidityType::TypeCategory randomTypeCategory(size_t _pseudoRandomNumber)
	{
		return static_cast<SolidityType::TypeCategory>(
			_pseudoRandomNumber % static_cast<size_t>(SolidityType::TypeCategory::TYPEMAX)
		);
	}
	std::string identifier();
	std::string boolLiteral()
	{
		return GenerationProbability{}.chooseOneOfN(2, rand) ? "true" : "false";
	}
	std::string doubleQuotedStringLiteral();
	std::string hexLiteral();
	std::string numberLiteral();
	std::string addressLiteral();
	std::string literal();
	std::string expression();
	void incrementNestingDepth()
	{
		m_expressionNestingDepth++;
	}
	bool nestingDepthTooHigh()
	{
		return m_expressionNestingDepth > s_maxNumNestedExpressions;
	}
	size_t m_expressionNestingDepth;
	bool m_compileTimeConstantExpressionsOnly;
	SolidityType m_type;
	static constexpr size_t s_maxNumNestedExpressions = 5;
	static constexpr size_t s_maxStringLength = 10;
	static constexpr size_t s_maxHexLiteralLength = 64;
	static constexpr size_t s_maxElementsInTuple = 4;
	static constexpr size_t s_maxElementsInlineArray = 4;
};

class StateVariableDeclarationGenerator: public GeneratorBase
{
public:
	enum Visibility
	{
		PUBLIC = 0,
		PRIVATE,
		INTERNAL,
		VISIBILITYMAX
	};
	StateVariableDeclarationGenerator(std::shared_ptr<SolidityGenerator> _mutator):
		GeneratorBase(std::move(_mutator))
	{}
	void setup() override;
	std::string visit() override;
	std::string name() override
	{
		return "StateVariableDeclarationGenerator";
	}
private:
	std::string identifier()
	{
		return "sv" + std::to_string(GenerationProbability{}.distributionOneToN(s_maxStateVariables, rand));
	}
	std::string visibility();
	static constexpr size_t s_maxStateVariables = 3;
	std::string const m_declarationTemplate =
		std::string(R"(<natSpecString>)") +
		R"(<type> <vis><?constant> constant</constant><?immutable> immutable</immutable> <id> = <value>;)";
};

struct Exports
{
	Exports(std::string& _path): sourceUnitPath(_path),	symbols({}), types({})
	{}
	/// Source unit path
	std::string sourceUnitPath;
	/// Exported symbols
	std::set<std::string> symbols;
	/// Exported user defined types
	std::set<std::string> types;
};

struct ExportedSymbols
{
	ExportedSymbols(): symbols({}), types({})
	{}
	ExportedSymbols& operator+=(ExportedSymbols& _right)
	{
		for (auto item: _right.symbols)
			if (!symbols.count(item))
				symbols.emplace(item);
		for (auto item: _right.types)
			if (!types.count(item))
				types.emplace(item);
		return *this;
	}
	ExportedSymbols& operator+=(std::string& _right)
	{
		if (!symbols.count(_right))
			symbols.emplace(_right);
		if (!types.count(_right))
			types.emplace(_right);
		return *this;
	}
	void removeSymbolsAndTypes()
	{
		symbols.clear();
		types.clear();
	}
	std::string randomSymbol(std::shared_ptr<RandomEngine> _rand);
	std::string randomUserDefinedType(std::shared_ptr<RandomEngine> _rand);
	std::set<std::string> symbols;
	std::set<std::string> types;
};

struct State
{
	virtual ~State() {}
};

struct FunctionState: State
{
	~FunctionState()
	{
		std::cout << "Destroying function state" << std::endl;
		inputParameters.clear();
		returnParameters.clear();
		locals.clear();
	}
	/// Parameter type, name pair
	using ParamType = std::pair<std::shared_ptr<SolidityType>, std::string>;
	enum class Mutability
	{
		PURE,
		VIEW,
		PAYABLE,
		NONPAYABLE,
	};
	enum class Visibility
	{
		EXTERNAL,
		INTERNAL,
		PUBLIC,
		PRIVATE
	};
	enum class Inheritance
	{
		VIRTUAL,
		OVERRIDE,
		VIRTUALOVERRIDE,
		NONE
	};
	Mutability randomMutability(std::shared_ptr<RandomEngine> _rand)
	{
		switch (GenerationProbability{}.distributionOneToN(4, _rand))
		{
		case 1:
			return Mutability::PURE;
		case 2:
			return Mutability::VIEW;
		case 3:
			return Mutability::PAYABLE;
		case 4:
			return Mutability::NONPAYABLE;
		}
		solAssert(false, "");
	}
	Mutability randomFreeFunctionMutability(std::shared_ptr<RandomEngine> _rand)
	{
		switch (GenerationProbability{}.distributionOneToN(3, _rand))
		{
		case 1:
			return Mutability::PURE;
		case 2:
			return Mutability::VIEW;
		case 3:
			return Mutability::NONPAYABLE;
		}
		solAssert(false, "");
	}
	void setName(std::string _name)
	{
		name = _name;
	}
	void setMutability(Mutability _mut)
	{
		mutability = _mut;
	}
	void setVisibility(Visibility _vis)
	{
		visibility = _vis;
	}
	void setParameterTypes(std::vector<ParamType> _paramTypes)
	{
		inputParameters = std::move(_paramTypes);
	}
	void setReturnTypes(std::vector<ParamType> _returnTypes)
	{
		returnParameters = std::move(_returnTypes);
	}
	void addReturn(std::shared_ptr<SolidityType>& _returnType)
	{
		returnParameters.push_back({_returnType, "r" + std::to_string(numReturns++)});
	}
	void addInput(std::shared_ptr<SolidityType>& _inputType)
	{
		inputParameters.push_back({_inputType, "i" + std::to_string(numInputs++)});
	}
	void addVariable(std::shared_ptr<SolidityType>& _variableType)
	{
		locals.push_back({_variableType, "v" + std::to_string(numLocals++)});
	}
	void setInheritance(Inheritance _inh)
	{
		inheritance = _inh;
	}
	bool identifiers()
	{
		return numReturns > 0 || numInputs > 0 || numLocals > 0;
	}
	bool operator==(FunctionState const& _other);
	std::string name;
	Mutability mutability;
	Visibility visibility;
	size_t numInputs;
	size_t numReturns;
	size_t numLocals;
	std::vector<ParamType> inputParameters;
	std::vector<ParamType> returnParameters;
	std::vector<ParamType> locals;
	Inheritance inheritance;
};

struct SourceUnitState: State
{
	SourceUnitState(): exportedSymbols({})
	{}
	~SourceUnitState()
	{
		std::cout << "Destroying source unit state" << std::endl;
		functions.clear();
		exportedSymbols.removeSymbolsAndTypes();
	}
	void exportSymbol(std::string& _symbol)
	{
		exportedSymbols += _symbol;
	}
	void exportSymbols(ExportedSymbols& _symbols)
	{
		exportedSymbols += _symbols;
	}
	void enterFunction(std::shared_ptr<FunctionState> _function)
	{
		exportedSymbols += _function->name;
		functions.emplace_back(_function);
	}
	void leaveFunction()
	{
		functions.pop_back();
	}
	std::shared_ptr<FunctionState> currentFunction()
	{
		if (functions.size() > 0)
			return functions[functions.size() - 1];
		else
			return nullptr;
	}
	bool functionExists(std::shared_ptr<FunctionState> _function)
	{
		for (auto const& f: functions)
			if (f == _function)
				return true;
		return false;
	}
	bool symbols()
	{
		return exportedSymbols.symbols.size() > 0;
	}
	bool userDefinedTypes()
	{
		return exportedSymbols.types.size() > 0;
	}
	ExportedSymbols exportedSymbols;
	std::vector<std::shared_ptr<FunctionState>> functions;
};

struct ImportState
{
	/// Maps a symbol to its alias identifier
	using SymbolAliases = std::map<std::string, std::string>;
	/// A single alias identifier for all symbols
	using UnitAlias = std::string;
	/// An alias is optional, when present it is either
	/// a single identifier or a mapping of symbols to
	/// their respective alias identifiers.
	using Alias = std::optional<std::variant<SymbolAliases, UnitAlias>>;
	ImportState(std::string&& _path, std::set<std::string>&& _symbols, Alias _alias):
		path(_path),
		symbols(std::move(_symbols)),
		aliases(std::move(_alias))
	{}
	/// Import path
	std::string path;
	/// Imported symbols
	std::set<std::string> symbols;
	/// Alias representation
	Alias aliases;
};

struct TestState: State
{
	TestState(std::shared_ptr<RandomEngine> _rand):
		sourceUnitStates({}),
		currentSourceName({}),
		rand(std::move(_rand))
	{}
	~TestState()
	{
		std::cout << "Destroying test state" << std::endl;
	}
	void addSourceUnit(std::string& _path)
	{
		sourceUnitStates.emplace(_path, SourceUnitState{});
		currentSourceName = _path;
	}
	bool empty()
	{
		return sourceUnitStates.empty();
	}
	size_t size()
	{
		return sourceUnitStates.size();
	}
	void print();
	std::string randomPath();
	std::string randomNonCurrentPath();
	std::string currentSourceUnit()
	{
		return currentSourceName;
	}
	SourceUnitState& currentSourceState()
	{
		return sourceUnitStates[currentSourceUnit()];
	}
	void removeSourceStates()
	{
		sourceUnitStates.clear();
	}
	std::map<std::string, SourceUnitState> sourceUnitStates;
	std::string currentSourceName;
	std::shared_ptr<RandomEngine> rand;
};

class TestCaseGenerator: public GeneratorBase
{
public:
	TestCaseGenerator(std::shared_ptr<SolidityGenerator> _mutator):
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
	bool empty()
	{
		return m_numSourceUnits == 0;
	}
	std::string randomPath();
	std::shared_ptr<TestState> testState()
	{
		return state;
	}
	std::string path(size_t _number) const
	{
		return m_sourceUnitNamePrefix + std::to_string(_number) + ".sol";
	}
	std::string path() const
	{
		return m_sourceUnitNamePrefix + std::to_string(m_numSourceUnits) + ".sol";
	}
	void addSourceUnit(std::string& _path)
	{
		state->addSourceUnit(_path);
	}
	/// Number of source units in test input
	size_t m_numSourceUnits;
	/// String prefix of source unit names
	std::string const m_sourceUnitNamePrefix = "su";
	/// Maximum number of source units per test input
	static constexpr unsigned s_maxSourceUnits = 3;
};

class PragmaGenerator: public GeneratorBase
{
public:
	PragmaGenerator(std::shared_ptr<SolidityGenerator> _mutator):
		GeneratorBase(std::move(_mutator))
	{}
	std::string visit() override;
	std::string name() override { return "Pragma generator"; }
};

class ImportGenerator: public GeneratorBase
{
public:
	ImportGenerator(std::shared_ptr<SolidityGenerator> _mutator):
		GeneratorBase(std::move(_mutator))
	{}
	std::string visit() override;
	std::string name() override { return "Import generator"; }
private:
	std::vector<Exports> m_globalExports;
	std::string const m_importPathAs = R"(import "<path>"<?as> as <identifier></as>;)";
	std::string const m_importStar = R"(import * as <identifier> from "<path>";)";
	std::string const m_alias = R"(<symbol><?as> as <alias></as>)";
	std::string const m_importSymAliases = R"(import {<aliases>} from "<path>";)";
	static constexpr size_t s_selfImportInvProb = 101;
};

struct InterfaceFunction
{
//	FunctionMutability mutability;
};

struct InterfaceState
{

};

struct ContractState
{
	ContractState():
		baseContractStates({}),
		functionStates({})
	{}

	void addBaseContract();
	void addFunction();
	std::vector<std::shared_ptr<ContractState>> baseContractStates;
	std::vector<std::shared_ptr<FunctionState>> functionStates;
};

struct Expression
{
	std::string visit()
	{
		return expressionTemplate;
	}
	std::string const expressionTemplate = R"(1)";
};

struct NamedArgument
{
	std::string visit()
	{
		return identifier + ": " + expression.visit();
	}
	std::string identifier;
	Expression expression;
};

struct NamedArgumentList
{
	std::set<NamedArgument> namedArguments;
	std::string const namedTemplate = R"({<commaSepNamedArgs>})";
};

struct CallArgument
{
	using Argument = std::variant<Expression, NamedArgument>;
    Argument argument;
};

struct CallArgumentList
{
	std::vector<CallArgument> callArguments;
};

struct InheritanceSpecifier
{
	std::string name;
	std::optional<CallArgumentList> callArguments;
};

struct InheritanceSpecifierList
{
	std::set<InheritanceSpecifier> inheritanceSpecifier;
};

struct Location
{
	enum class Loc
	{
		MEMORY,
		STORAGE,
		CALLDATA,
		STACK
	};
	Location(Loc _l): loc(_l) {}
	Loc loc;
	std::string visit();
};

class LocationGenerator: public GeneratorBase
{
public:
	LocationGenerator(std::shared_ptr<SolidityGenerator> _mutator):
		GeneratorBase(std::move(_mutator))
	{}
	std::string visit() override;
	std::string name() override
	{
		return "LocationGenerator";
	}
};

struct IntegerWidth
{
	IntegerWidth(unsigned _w)
	{
		width = (8 * _w) % 256;
	}
	std::string visit()
	{
		return width > 0 ? std::to_string(width) : std::string("256");
	}
	unsigned width;
};

struct Statement
{
	virtual ~Statement() {}
	virtual std::string visit() = 0;
};

struct VariableDeclaration
{
	VariableDeclaration(std::shared_ptr<SolidityType> _type, Location _loc, std::string&& _id):
        type(std::move(_type)),
		location(_loc),
		identifier(std::move(_id))
	{}
	std::shared_ptr<SolidityType> type;
	Location location;
	std::string identifier;
	std::string visit();
	std::string const varDeclTemplate = R"(<type> <location> <name>;)";
};

class VariableDeclarationGenerator: public GeneratorBase
{
public:
	VariableDeclarationGenerator(std::shared_ptr<SolidityGenerator> _mutator):
		GeneratorBase(std::move(_mutator))
	{}
	void setup() override;
	std::string name() override
	{
		return "VariableDeclarationGenerator";
	}
	std::string visit() override;
private:
	std::string identifier();
};

struct ParameterListState
{

};

class ParameterListGenerator: public GeneratorBase
{
public:
	ParameterListGenerator(std::shared_ptr<SolidityGenerator> _mutator):
		GeneratorBase(std::move(_mutator))
	{}
	void setup() override;
	std::string visit() override;
	std::string name() override
	{
		return "ParameterListGenerator";
	}
};

struct ParameterList
{
	std::vector<VariableDeclaration> params;
	std::string const parameterListTemplate = R"(<commaSeparatedParams>)";
};

struct VariableDeclarationTuple
{
	std::vector<VariableDeclaration> varDecls;
	std::string const varDeclTupleTemplate =
		R"(<commaStarPre><varDecl><?commaStarPost><commaStar><!commaStarPost><commaSepVarDecls></commaStarPost>)";
};

struct ExpressionStatement: Statement
{
	ExpressionStatement()
	{

	}
	std::string visit() override;
	Expression expression;
	std::string const exprStmtTemplate = R"(<expression>;)";
};

struct VariableDeclarationTupleAssignment: Statement
{
	VariableDeclarationTuple tuple;
	Expression expression;
	std::string visit() override;
	std::string const varDeclTupleAssignTemplate =
		R"(<tuple> = <expression>;)";
};

class SimpleVarDeclGenerator: public GeneratorBase
{
public:
	SimpleVarDeclGenerator(std::shared_ptr<SolidityGenerator> _mutator):
		GeneratorBase(std::move(_mutator))
	{}
	void setup() override;
	std::string visit() override;
	std::string name() override
	{
		return "Simple var decl statement generator";
	}
	std::string const simpleVarDeclTemplate =
		R"(<type> <location> <name><?assign> = <expression></assign>)";
};

struct GeneratorTypeVisitor
{
	template <typename T>
	std::string operator()(T& _value)
	{
		return _value.visit();
	}
};

//struct VariableDeclarationStatement: Statement
//{
//	using VarDeclStmt = std::variant<SimpleVariableDeclaration, VariableDeclarationTupleAssignment>;
//	VariableDeclarationStatement(VarDeclStmt _stmt): stmt(std::move(_stmt))
//	{}
//	VarDeclStmt stmt;
//	std::string visit() override
//	{
//		return std::visit(GeneratorTypeVisitor{}, stmt);
//	}
//};

//struct SimpleStatement: Statement
//{
//	using Stmt = std::variant<VariableDeclarationStatement, ExpressionStatement>;
//	SimpleStatement(Stmt _stmt): statement(std::move(_stmt))
//	{}
//	Stmt statement;
//	std::string visit() override
//	{
//		return std::visit(GeneratorTypeVisitor{}, statement);
//	}
//};

/// Forward declaration
//using StatementTy = std::variant<SimpleStatement, BlockStatement>;

class StatementGenerator: public GeneratorBase
{
public:
	enum class Type: size_t
	{
		BLOCK = 0,
		SIMPLE,
		IF,
		FOR,
		WHILE,
		DOWHILE,
		CONTINUE,
		BREAK,
		TRY,
		RETURN,
		EMIT,
		ASSEMBLY,
		TYPEMAX
	};
	enum class YulStmtType: size_t
	{
		BLOCK = 0,
		VARDECL,
		ASSIGN,
		FUNCTIONCALL,
		IF,
		FOR,
		SWITCH,
		LEAVE,
		BREAK,
		CONTINUE,
		FUNCTIONDEF
	};
	StatementGenerator(std::shared_ptr<SolidityGenerator> _mutator):
		GeneratorBase(std::move(_mutator)),
		m_type(randomType((*rand)())),
		m_statementNestingDepth(0),
		m_numVariables(0),
		m_loop(false)
	{}
	void setup() override;
	std::string visit() override;
	std::string name() override
	{
		return "Block statement generator";
	}
private:
	std::string variableName()
	{
		return "v" + std::to_string(m_numVariables++);
	}
	std::string statement();
	static Type randomType(size_t _randomNumber)
	{
		return static_cast<Type>(
			_randomNumber % static_cast<size_t>(Type::TYPEMAX)
		);
	}
	void incrementNestingDepth()
	{
		m_statementNestingDepth++;
	}
	bool nestingDepthTooHigh()
	{
		return m_statementNestingDepth > s_maxNumNestedStatements;
	}
	std::string simpleStatement();
	std::string blockStatement();
	Type m_type;
	size_t m_statementNestingDepth;
	size_t m_numVariables;
	bool m_loop;
	static size_t constexpr s_maxNumNestedStatements = 5;
};

class EnumDeclaration: public GeneratorBase
{
public:
	EnumDeclaration(std::shared_ptr<SolidityGenerator> _mutator):
		GeneratorBase(std::move(_mutator))
	{}
	void setup() override {}
	std::string visit() override;
	std::string name() override
	{
		return "Enum generator";
	}
private:
	std::string enumName()
	{
		return "E" + std::to_string((*rand)() % s_maxIdentifiers);
	}
	std::string const enumTemplate = R"(enum <name> { <members> })";
	static constexpr size_t s_maxMembers = 5;
	static constexpr size_t s_maxIdentifiers = 4;
};

class ConstantVariableDeclaration: public GeneratorBase
{
public:
	ConstantVariableDeclaration(std::shared_ptr<SolidityGenerator> _mutator):
		GeneratorBase(std::move(_mutator))
	{}
	void setup() override;
	std::string visit() override;
	std::string name() override { return "Constant variable generator"; }
private:
	std::string const constantVarDeclTemplate =
		R"(<type> constant <name> = <expression>;)";
};

class FallbackDefinitionGenerator: public GeneratorBase
{
public:
	FallbackDefinitionGenerator(std::shared_ptr<SolidityGenerator> _mutator):
		GeneratorBase(std::move(_mutator))
	{}
	void setup() override;
	std::string visit() override;
	std::string name() override
	{
		return "FallbackDefinitionGenerator";
	}
};

class FunctionDefinitionGenerator: public GeneratorBase
{
public:
	FunctionDefinitionGenerator(std::shared_ptr<SolidityGenerator> _mutator):
		GeneratorBase(std::move(_mutator))
	{}
	void setup() override;
	std::string visit() override;
	std::string name() override { return "Function generator"; }
	void freeFunctionMode()
	{
		m_freeFunction = true;
	}
	void contractFunctionMode()
	{
		m_freeFunction = false;
	}
	static const std::vector<std::string> s_mutability;
private:
	std::string functionIdentifier();
	std::shared_ptr<TestState> m_state;
	std::shared_ptr<FunctionState> m_functionState;
	bool m_freeFunction;
	static const std::vector<std::string> s_visibility;
	static const std::vector<std::string> s_freeFunctionMutability;
	std::string const m_functionTemplate =
		R"(<natSpecString>)" +
		std::string(R"(function <id> (<paramList>) )") +
		R"(<visibility> <stateMutability> <modInvocation> <virtual> <overrideSpec>)" +
		R"(<?return> returns (<retParamList>)</return>)" +
		R"(<?definition><body><!definition>;</definition>)";
};

class ContractDefinitionGenerator: public GeneratorBase
{
public:
	ContractDefinitionGenerator(std::shared_ptr<SolidityGenerator> _generator):
		GeneratorBase(std::move(_generator))
	{}
	void setup() override;
	std::string visit() override;
	std::string name() override { return "Contract generator"; }
private:
	std::optional<InheritanceSpecifierList> m_inheritanceList;
	const std::string m_contractTemplate =
		R"(<natSpecString>)" +
		std::string(R"(<?abstract>abstract</abstract> contract <id>)") +
		R"(<?inheritance> is <inheritanceSpecifierList></inheritance> { <stateVar> <function> })";
	/// List of inverse probabilities of sub-components
	static constexpr size_t s_abstractInvProb = 10;
	static constexpr size_t s_inheritanceInvProb = 10;
};

class SourceUnitGenerator: public GeneratorBase
{
public:
	SourceUnitGenerator(std::shared_ptr<SolidityGenerator> _mutator):
		GeneratorBase(std::move(_mutator))
	{
		m_sourceState = std::make_shared<SourceUnitState>();
	}
	void setup() override;
	std::string visit() override;
	std::string name() override { return "Source unit generator"; }
private:
	void saveState();
	std::shared_ptr<TestState> m_testState;
	std::shared_ptr<SourceUnitState> m_sourceState;
	static constexpr unsigned s_maxElements = 10;
};

class NatSpecGenerator: public GeneratorBase
{
public:
	enum class TagCategory
	{
		CONTRACT,
		FUNCTION,
		PUBLICSTATEVAR,
		EVENT
	};
	enum class Tag
	{
		TITLE,
		AUTHOR,
		NOTICE,
		DEV,
		PARAM,
		RETURN,
		INHERITDOC
	};
	NatSpecGenerator(std::shared_ptr<SolidityGenerator> _generator): GeneratorBase(std::move(_generator))
	{
		m_nestingDepth = 0;
	}
	void setup() override {}
	std::string visit() override;
	void endVisit() override
	{
		m_nestingDepth = 0;
	}
	std::string name() override { return "NatSpec generator"; }
	void tagCategory(TagCategory _tag)
	{
		m_tag = _tag;
	}
private:
	std::string randomNatSpecString(TagCategory _category);
	Tag randomTag(TagCategory _category);
	TagCategory m_tag;
	size_t m_nestingDepth;
	static std::map<TagCategory, std::vector<Tag>> s_tagLookup;
	static constexpr size_t s_maxTextLength = 8;
	static constexpr size_t s_maxNestedTags = 3;
	std::string const m_tagTemplate = R"(<tag> <random> <recurse>)";
};

struct InterfaceSpecifiers
{
	std::set<std::string> typeNames;
};

struct SourceState {
	unsigned numPragmas;
	unsigned numImports;
	unsigned numContracts;
	unsigned numAbstractContracts;
	unsigned numInterfaces;
	unsigned numLibraries;
	unsigned numGlobalStructs;
	unsigned numGlobalFuncs;
	unsigned numGlobalEnums;
};

struct ProgramState
{
	enum class ContractType
	{
		CONTRACT,
		ABSTRACTCONTRACT,
		INTERFACE,
		LIBRARY
	};

	unsigned numFunctions;
	unsigned numModifiers;
	unsigned numContracts;
	unsigned numLibraries;
	unsigned numInterfaces;
	unsigned numStructs;
	unsigned numEvents;
	bool constructorDefined;
	ContractType contractType;
};

class SolidityGenerator: public std::enable_shared_from_this<SolidityGenerator>
{
public:
	explicit SolidityGenerator(unsigned _seed);

	/// Returns a multi-source test case.
	std::string visit();
	/// Returns the generator of type @param T.
	template <typename T>
	std::shared_ptr<T> generator();
	/// Returns a shared ptr to underlying random
	/// number generator.
	std::shared_ptr<RandomEngine> randomEngine()
	{
		return m_rand;
	}
	std::shared_ptr<TestState> testState()
	{
		return m_state;
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
	void destroyState()
	{
		m_state->removeSourceStates();
	}
	void initialize();
	/// Random number generator
	std::shared_ptr<RandomEngine> m_rand;
	/// Sub generators
	std::set<GeneratorPtr> m_generators;
	/// Test state
	std::shared_ptr<TestState> m_state;
};
}
