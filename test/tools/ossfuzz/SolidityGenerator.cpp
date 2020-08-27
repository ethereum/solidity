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

#include <boost/preprocessor.hpp>
#include <boost/range/adaptor/map.hpp>

#include <algorithm>

using namespace solidity::test::fuzzer;
using namespace solidity::util;
using namespace std;
using PrngUtil = solidity::test::fuzzer::GenerationProbability;

GeneratorBase::GeneratorBase(std::shared_ptr<SolidityGenerator> _mutator)
{
	mutator = std::move(_mutator);
	rand = mutator->randomEngine();
	state = mutator->testState();
}

string GeneratorBase::visitChildren()
{
	ostringstream os;
	// Randomise visit order
	vector<GeneratorPtr> randomisedChildren;
	for (auto child: generators)
		randomisedChildren.push_back(child);
	shuffle(randomisedChildren.begin(), randomisedChildren.end(), *rand);
	std::cout << "Visiting children" << std::endl;
	for (auto child: randomisedChildren)
	{
		std::cout << "Visiting " << std::visit(NameVisitor{}, child) << std::endl;
		os << std::visit(GeneratorVisitor{}, child);
	}

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
		addSourceUnit(sourcePath);
		m_numSourceUnits++;
		os << visitChildren();
	}
	return os.str();
}

void SourceUnitGenerator::setup()
{
	addGenerators(
		{
			mutator->generator<PragmaGenerator>(),
			mutator->generator<ImportGenerator>(),
			mutator->generator<ConstantVariableDeclaration>(),
			mutator->generator<EnumDeclaration>(),
			mutator->generator<FunctionDefinitionGenerator>(),
			mutator->generator<ContractDefinitionGenerator>()
		}
	);
	mutator->generator<FunctionDefinitionGenerator>()->freeFunctionMode();
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
	m_state = make_shared<TestState>(m_rand);
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

using MP = solidity::test::fuzzer::GenerationProbability;

const std::vector<std::string> FunctionDefinitionGenerator::s_visibility = {
	"public",
	"private",
	"external",
	"internal"
};

const vector<string> FunctionDefinitionGenerator::s_mutability = {
	"payable",
	"view",
	"pure",
	"" // non payable
};

map<NatSpecGenerator::TagCategory, vector<NatSpecGenerator::Tag>> NatSpecGenerator::s_tagLookup = {
	{
		NatSpecGenerator::TagCategory::CONTRACT,
		{
			NatSpecGenerator::Tag::TITLE,
			NatSpecGenerator::Tag::AUTHOR,
			NatSpecGenerator::Tag::NOTICE,
			NatSpecGenerator::Tag::DEV,
		}
	},
	{
		NatSpecGenerator::TagCategory::FUNCTION,
		{
			NatSpecGenerator::Tag::NOTICE,
			NatSpecGenerator::Tag::DEV,
			NatSpecGenerator::Tag::PARAM,
			NatSpecGenerator::Tag::RETURN,
			NatSpecGenerator::Tag::INHERITDOC
		}
	},
	{
		NatSpecGenerator::TagCategory::PUBLICSTATEVAR,
		{
			NatSpecGenerator::Tag::NOTICE,
			NatSpecGenerator::Tag::DEV,
			NatSpecGenerator::Tag::RETURN,
			NatSpecGenerator::Tag::INHERITDOC
		}
	},
	{
		NatSpecGenerator::TagCategory::EVENT,
		{
			NatSpecGenerator::Tag::NOTICE,
			NatSpecGenerator::Tag::DEV,
			NatSpecGenerator::Tag::PARAM
		}
	}
};

const vector<string> FunctionDefinitionGenerator::s_freeFunctionMutability = {
	"view",
	"pure",
	"" // non payable
};

string GenerationProbability::generateRandomAsciiString(size_t _length, std::shared_ptr<RandomEngine> _rand)
{
	vector<char> s{};
	for (size_t i = 0; i < _length * 2; i++)
		s.push_back(
			static_cast<char>(Distribution(0x21, 0x7e)(*_rand))
		);
	return string(s.begin(), s.end());
}

string GenerationProbability::generateRandomHexString(size_t _length, std::shared_ptr<RandomEngine> _rand)
{
	static char const* hexDigit = "0123456789abcdefABCDEF";
	vector<char> s{};
	for (size_t i = 0; i < _length * 2; i++)
		s.push_back(hexDigit[distributionOneToN(22, _rand) - 1]);
	return string(s.begin(), s.end());
}

pair<GenerationProbability::NumberLiteral, string> GenerationProbability::generateRandomNumberLiteral(
	size_t _length,
	std::shared_ptr<RandomEngine> _rand
)
{
//	static char const* hexDigit = "0123456789abcdefABCDEF";
	static char const* decimalDigit = "0123456789";
	vector<char> s{};
	for (size_t i = 0; i < _length; i++)
		s.push_back(decimalDigit[distributionOneToN(10, _rand) - 1]);
	if (s[0] == '0')
		s[0] = decimalDigit[distributionOneToN(9, _rand)];
	return pair(NumberLiteral::DECIMAL, string(s.begin(), s.end()));
}

string ExportedSymbols::randomSymbol(shared_ptr<RandomEngine> _rand)
{
	auto it = symbols.begin();
	auto idx = (*_rand)() % symbols.size();
	for (size_t i = 0; i < idx; i++)
		it++;
	return *it;
}

string ExportedSymbols::randomUserDefinedType(shared_ptr<RandomEngine> _rand)
{
	auto it = types.begin();
	auto idx = (*_rand)() % types.size();
	for (size_t i = 0; i < idx; i++)
		it++;
	return *it;
}

bool FunctionState::operator==(const FunctionState& _other)
{
	if (_other.inputParameters.size() != inputParameters.size())
		return false;
	else
	{
		unsigned index = 0;
		for (auto const& type: _other.inputParameters)
			if (type.first->type != inputParameters[index++].first->type)
				return false;
		return name == _other.name;
	}
}

string TestState::randomPath()
{
	solAssert(!empty(), "Solc custom mutator: Null test state");
	for (auto iter = sourceUnitStates.begin(); iter != sourceUnitStates.end(); )
	{
		// Choose this element equally at random
		if (MP{}.chooseOneOfN(sourceUnitStates.size(), rand))
			return iter->first;
		// If not chosen, increment iterator checking if this
		// is the last element. If it is not the last element
		// continue, otherwise choose it.
		else if (++iter == sourceUnitStates.end())
			return (--iter)->first;
	}
	solAssert(false, "Solc custom mutator: No source path chosen");
}

GeneratorPtr GeneratorBase::randomGenerator()
{
	solAssert(generators.size() > 0, "Invalid hierarchy");

	auto it = generators.begin();
	auto idx = (*rand)() % generators.size();
	for (size_t i = 0; i < idx; i++)
		it++;
	return *it;
}

string TestCaseGenerator::randomPath()
{
	solAssert(!empty(), "Solc custom mutator: Invalid source unit");
	return path(MP{}.distributionOneToN(m_numSourceUnits, rand) - 1);
}

string ExpressionGenerator::doubleQuotedStringLiteral()
{
	string s = MP{}.generateRandomAsciiString(
		MP{}.distributionOneToN(s_maxStringLength, rand),
		rand
	);
	return s;
}

string ExpressionGenerator::hexLiteral()
{
	size_t lengthInBytes = std::visit(SolidityType::TypeIndexVisitor{}, m_type.type.first) + 1;
	string s = MP{}.generateRandomHexString(
		MP{}.distributionOneToN(lengthInBytes, rand),
		rand
	);
	return "hex\"" + s + "\"";
}

string ExpressionGenerator::numberLiteral()
{
	size_t lengthInBytes = std::visit(SolidityType::TypeIndexVisitor{}, m_type.type.first) + 1;
	if (lengthInBytes > 32)
		lengthInBytes -= 32;
	auto [n, s] = MP{}.generateRandomNumberLiteral(
		MP{}.distributionOneToN(lengthInBytes, rand),
		rand
	);
	if (n == MP::NumberLiteral::HEX)
		return "hex\"" + s + "\"";
	else
		return s;
}

string ExpressionGenerator::addressLiteral()
{
	string addr = "0x" + MP{}.generateRandomHexString(20, rand);
	return getChecksummedAddress(addr);
}

string ExpressionGenerator::literal()
{
	string lit;
	switch (m_type.typeCategory)
	{
	case SolidityType::TypeCategory::ADDRESS:
		lit = addressLiteral();
		break;
	case SolidityType::TypeCategory::BOOL:
		lit = boolLiteral();
		break;
	case SolidityType::TypeCategory::BYTES:
		lit = hexLiteral();
		break;
	case SolidityType::TypeCategory::INTEGER:
		lit = numberLiteral();
		break;
	case SolidityType::TypeCategory::TYPEMAX:
		solAssert(false, "");
	}
	return typeString() + "(" + lit + ")";
}

string ExpressionGenerator::identifier()
{
	vector<string> ids;
	// TODO: Implement typed identifiers
//	if (state->currentSourceState().symbols())
//		for (auto &item: state->currentSourceState().exportedSymbols.symbols)
//			ids.push_back(item);
	if (auto f = state->currentSourceState().currentFunction(); f && f->identifiers())
	{
		for (auto& item: f->inputParameters)
			if (item.first->typeCategory == m_type.typeCategory)
				ids.push_back(item.second);
		for (auto& item: f->returnParameters)
			if (item.first->typeCategory == m_type.typeCategory)
				ids.push_back(item.second);
		for (auto& item: f->locals)
			if (item.first->typeCategory == m_type.typeCategory)
				ids.push_back(item.second);
	}
	if (ids.size() > 0)
		return ids[MP{}.distributionOneToN(ids.size(), rand) - 1];
	else
		return "";
}

string ExpressionGenerator::expression()
{
	if (nestingDepthTooHigh())
		return literal();

	incrementNestingDepth();

	string expr;
	switch (MP{}.distributionOneToN(Type::TYPEMAX, rand) - 1)
	{
	case Type::INDEXACCESS:
		// TODO: Implement index access
		expr = literal();
//		expr = Whiskers(R"(<baseExpr>[<indexExpr>])")
//			("baseExpr", expression())
//			("indexExpr", expression())
//			.render();
		break;
	case Type::INDEXRANGEACCESS:
		// TODO: Implement index range access
		expr = literal();
//		expr = Whiskers(R"(<baseExpr>[<startExpr>:<endExpr>])")
//			("baseExpr", expression())
//			("startExpr", expression())
//			("endExpr", expression())
//			.render();
		break;
	case Type::METATYPE:
		// TODO: Implement metatype
		expr = literal();
//		expr = Whiskers(R"(type(<typeName>))")
//			("typeName", randomTypeString())
//			.render();
		break;
	case Type::BITANDOP:
		if (m_type.typeCategory == SolidityType::TypeCategory::INTEGER)
			expr = expression() + " & " + expression();
		else
			expr = literal();
		break;
	case Type::BITOROP:
		if (m_type.typeCategory == SolidityType::TypeCategory::INTEGER)
			expr = expression() + " | " + expression();
		else
			expr = literal();
		break;
	case Type::BITXOROP:
		if (m_type.typeCategory == SolidityType::TypeCategory::INTEGER)
			expr = expression() + " ^ " + expression();
		else
			expr = literal();
		break;
	case Type::ANDOP:
		if (m_type.typeCategory == SolidityType::TypeCategory::BOOL)
			expr = expression() + " && " + expression();
		else
			expr = literal();
		break;
	case Type::OROP:
		if (m_type.typeCategory == SolidityType::TypeCategory::BOOL)
			expr = expression() + " || " + expression();
		else
			expr = literal();
		break;
	case Type::NEWEXPRESSION:
		// TODO: Implement new expression
		expr = literal();
//		expr = Whiskers(R"(new <typeName>)")
//			("typeName", randomTypeString())
//			.render();
		break;
	case Type::CONDITIONAL:
	{
		SolidityType oldType = m_type;
		setType(SolidityType(SolidityType::TypeCategory::BOOL, rand));
		string condition = expression();
		setType(oldType);
		expr = condition + " ? " + expression() + " : " + expression();
		break;
	}
	case Type::ASSIGNMENT:
	{
		// TODO: Implement lvalue expressions
		auto id = identifier();
		if (!id.empty())
			expr = id + " = " + expression();
		else
			expr = literal();
		break;
	}
	case Type::INLINEARRAY:
	{
		// TODO: Implement inline array expressions
		expr = literal();
//		vector<string> exprs{};
//		size_t numElementsInTuple = MP{}.distributionOneToN(s_maxElementsInlineArray, rand);
//		for (size_t i = 0; i < numElementsInTuple; i++)
//			exprs.push_back(expression());
//		expr = Whiskers(R"([<inlineArrayExpression>])")
//			("inlineArrayExpression", boost::algorithm::join(exprs, ", "))
//			.render();
		break;
	}
	case Type::IDENTIFIER:
	{
		auto id = identifier();
		if (id.empty())
			expr = literal();
		else
			expr = id;
		break;
	}
	case Type::LITERAL:
		expr = literal();
		break;
	case Type::TUPLE:
	{
		// TODO: Implement tuple
		expr = literal();
//		vector<string> exprs{};
//		size_t numElementsInTuple = MP{}.distributionOneToN(s_maxElementsInTuple, rand);
//		for (size_t i = 0; i < numElementsInTuple; i++)
//			exprs.push_back(expression());
//		expr = Whiskers(R"((<tupleExpression>))")
//			("tupleExpression", boost::algorithm::join(exprs, ", "))
//			.render();
		break;
	}
	default:
		expr = literal();
	}
	// Typed expression
	return typeString() + "(" + expr + ")";
}

string ExpressionGenerator::visit()
{
	string expr{};
	if (m_compileTimeConstantExpressionsOnly)
		// TODO: Reference identifiers that point to
		// compile time constant expressions.
		return literal();
	else
		return expression();
}

string StateVariableDeclarationGenerator::visibility()
{
	switch (MP{}.distributionOneToN(Visibility::VISIBILITYMAX, rand) - 1)
	{
	case Visibility::INTERNAL:
		return "internal";
	case Visibility::PRIVATE:
		return "private";
	case Visibility::PUBLIC:
		return "public";
	default:
		solAssert(false, "");
	}
}

void StateVariableDeclarationGenerator::setup()
{
	addGenerators(
		{
			mutator->generator<ExpressionGenerator>(),
			mutator->generator<NatSpecGenerator>()
		}
	);
}

string StateVariableDeclarationGenerator::visit()
{
	string id = identifier();
	string vis = visibility();
	bool immutable = false;
	bool constant = MP{}.chooseOneOfN(2, rand);
	if (!constant)
		immutable = MP{}.chooseOneOfN(2, rand);
	solAssert(!(constant && immutable), "State variable cannot be both constant and immutable");
	// Immutables cannot have a non-value type
	if (immutable)
		generator<ExpressionGenerator>()->setValueType();
	string expr = generator<ExpressionGenerator>()->visit();
	string type = generator<ExpressionGenerator>()->typeString();
	// TODO: Actually restrict this setting to public state variables only
	generator<NatSpecGenerator>()->tagCategory(NatSpecGenerator::TagCategory::PUBLICSTATEVAR);
	string natSpecString = generator<NatSpecGenerator>()->visit();
	return Whiskers(m_declarationTemplate)
		("natSpecString", natSpecString)
		("type", type)
		("vis", vis)
		("constant", constant)
		("immutable", immutable)
		("id", id)
		("value", expr)
		.render();
}

//void UserDefinedTypeGenerator::setup()
//{
//	addGenerators({mutator->generator<FunctionTypeGenerator>()});
//}
//
//string UserDefinedTypeGenerator::visit()
//{
//	switch (MP{}.distributionOneToN(2, rand))
//	{
//	case 1:
//		if (state->currentSourceState().userDefinedTypes())
//			return state->currentSourceState().exportedSymbols.randomUserDefinedType(rand);
//		else
//			return "uint";
//	case 2:
//		return generator<FunctionTypeGenerator>()->visit();
//	}
//	solAssert(false, "");
//}

string Location::visit()
{
	switch (loc)
	{
	case Location::Loc::CALLDATA:
		return "calldata";
	case Location::Loc::MEMORY:
		return "memory";
	case Location::Loc::STORAGE:
		return "storage";
	case Location::Loc::STACK:
		return "";
	}
}

string LocationGenerator::visit()
{
	return "";
	// TODO: Implement locations
	switch (MP{}.distributionOneToN(4, rand))
	{
	case 1:
		return Location(Location::Loc::MEMORY).visit();
	case 2:
		return Location(Location::Loc::STORAGE).visit();
	case 3:
		return Location(Location::Loc::CALLDATA).visit();
	case 4:
		return Location(Location::Loc::STACK).visit();
	}
	solAssert(false, "");
}

void SimpleVarDeclGenerator::setup()
{
	addGenerators(
		{
			mutator->generator<ExpressionGenerator>(),
			mutator->generator<LocationGenerator>()
		}
	);
}

string SimpleVarDeclGenerator::visit()
{
	return Whiskers(simpleVarDeclTemplate)
		("type", generator<ExpressionGenerator>()->typeString())
		("location", generator<LocationGenerator>()->visit())
		("name", "v")
		("assign", true)
		("expression", generator<ExpressionGenerator>()->visit())
		.render();

}

string ExpressionStatement::visit()
{
	// TODO: Implement expression generation
	return Whiskers(exprStmtTemplate)("expression", "1").render();
}

void StatementGenerator::setup()
{
	addGenerators(
		{
			mutator->generator<ExpressionGenerator>(),
			mutator->generator<LocationGenerator>(),
		}
	);
}

string StatementGenerator::simpleStatement()
{
	bool variableDecl = MP{}.chooseOneOfN(2, rand);
	string stmt;
	if (variableDecl)
	{
		if (auto f = state->currentSourceState().currentFunction(); f)
		{
			if (f->numReturns > 0)
			{
				auto t = f->returnParameters[
					MP{}.distributionOneToN(f->returnParameters.size(), rand) - 1];
				generator<ExpressionGenerator>()->setType(t.first);
				stmt = t.second + " = " + generator<ExpressionGenerator>()->visit() + ";\n";
			}
			else
			{
				auto t = generator<ExpressionGenerator>()->randomType();
				f->addVariable(t);
				generator<ExpressionGenerator>()->setType(*t);
				stmt = t->type.second +
				       " " +
				       generator<LocationGenerator>()->visit() +
				       " " +
				       "v" + to_string(f->numLocals - 1) +
				       " = " +
				       generator<ExpressionGenerator>()->visit() +
				       ";" +
				       "\n";
			}
		}
		else
			stmt = generator<ExpressionGenerator>()->visit() + ";\n";
	}
	else
		stmt = generator<ExpressionGenerator>()->visit() + ";" + "\n";
	return stmt;
}

string StatementGenerator::blockStatement()
{
	static size_t constexpr maxBlockStmts = 3;
	string stmt = "{";
	if (auto f = state->currentSourceState().currentFunction(); f && f->returnParameters.size() > 0)
		// Always assign value to return parameters
		for (auto t: f->returnParameters)
		{
			generator<ExpressionGenerator>()->setType(t.first);
			stmt = t.second + " = " + generator<ExpressionGenerator>()->visit() + ";\n";
		}
	// Add pseudo randomly generated statements
	for (size_t i = 0; i < MP{}.distributionOneToN(maxBlockStmts, rand); i++)
		stmt += statement();
	stmt += "}";
	return stmt;
}

string StatementGenerator::statement()
{
	if (nestingDepthTooHigh())
		return simpleStatement();

	incrementNestingDepth();

	string stmt;
	switch (randomType((*rand)()))
	{
	case Type::BLOCK:
	{
		stmt = blockStatement();
		break;
	}
	case Type::SIMPLE:
		stmt = simpleStatement();
		break;
	case Type::IF:
		generator<ExpressionGenerator>()->setType(
			{SolidityType::TypeCategory::BOOL, rand}
		);
		stmt = "if(" + generator<ExpressionGenerator>()->visit() + ")";
		stmt += blockStatement();
		break;
	case Type::FOR:
	{
		bool loop = m_loop;
		m_loop = true;
		stmt = "for(" + simpleStatement();
		std::cout << stmt << std::endl;
		generator<ExpressionGenerator>()->setType(
			{SolidityType::TypeCategory::BOOL, rand}
		);
		stmt += generator<ExpressionGenerator>()->visit() +
			"; " +
			generator<ExpressionGenerator>()->visit() +
			")";
		std::cout << stmt << std::endl;
		stmt += blockStatement();
		m_loop = loop;
		break;
	}
	case Type::WHILE:
	{
		bool loop = m_loop;
		m_loop = true;
		generator<ExpressionGenerator>()->setType(
			{SolidityType::TypeCategory::BOOL, rand}
		);
		stmt = "while(" + generator<ExpressionGenerator>()->visit() + ")";
		stmt += blockStatement();
		m_loop = loop;
		break;
	}
	case Type::DOWHILE:
	{
		bool loop = m_loop;
		m_loop = true;
		stmt += "do" + blockStatement();
		generator<ExpressionGenerator>()->setType(
			{SolidityType::TypeCategory::BOOL, rand}
		);
		stmt += "while(" + generator<ExpressionGenerator>()->visit() + ");";
		m_loop = loop;
		break;
	}
	case Type::CONTINUE:
		if (m_loop)
			stmt = "continue;";
		break;
	case Type::BREAK:
		if (m_loop)
			stmt = "break;";
		break;
	case Type::TRY:
		// TODO: Implement try
		break;
	case Type::RETURN:
	{
		if (state->currentSourceState().currentFunction())
		{
			size_t numReturns = state->currentSourceState().currentFunction()->numReturns;
			if (numReturns > 0)
			{
				stmt = "return (";
				string sep{};
				for (size_t i = 0; i < numReturns; i++)
				{
					stmt += sep + "r" + to_string(i);
					if (sep.empty())
						sep = ", ";
				}
				stmt += ");\n";
			}
		}
		break;
	}
	case Type::EMIT:
		// TODO
		break;
	case Type::ASSEMBLY:
		stmt = "assembly {}";
		break;
	default:
		stmt = simpleStatement();
		break;
	}
	return stmt;
}

string StatementGenerator::visit()
{
	static size_t constexpr maxStatements = 5;
	ostringstream os;
	os << "{" << std::endl;
	for (size_t i = 0; i < maxStatements; i++)
		os << statement();
	os << "}";
	return os.str();
}

string VariableDeclaration::visit()
{
	return Whiskers(varDeclTemplate)
		("type", "uint"/*type->visit()*/)
		("location", location.visit())
		("name", identifier)
		.render();
}

string VariableDeclarationGenerator::identifier()
{
	string id = "v" + to_string(MP{}.distributionOneToN(10, rand));
	return id;
}

void VariableDeclarationGenerator::setup()
{
	addGenerators(
		{
			mutator->generator<LocationGenerator>(),
			mutator->generator<ExpressionGenerator>()
		}
	);
}

string VariableDeclarationGenerator::visit()
{
	string type = generator<ExpressionGenerator>()->typeString();
	string location = generator<LocationGenerator>()->visit();
	return type + " " + location + " " + identifier();
}

void ParameterListGenerator::setup()
{
	addGenerators({mutator->generator<VariableDeclarationGenerator>()});
}

string ParameterListGenerator::visit()
{
	size_t numParameters = MP{}.distributionOneToN(4, rand);
	ostringstream out;
	string sep{};
	for (size_t i = 0; i < numParameters; i++)
	{
		out << sep << generator<VariableDeclarationGenerator>()->visit();
		if (sep.empty())
			sep = ", ";
	}
	return out.str();
}

string FunctionDefinitionGenerator::functionIdentifier()
{
	switch (MP{}.distributionOneToN(3, rand))
	{
	case 1:
		return "f" + to_string(MP{}.distributionOneToN(10, rand));
	case 2:
		return "fallback";
	case 3:
		return "receive";
	}
	solAssert(false, "Invalid function identifier");
}

void FunctionDefinitionGenerator::setup()
{
	addGenerators(
		{
			mutator->generator<ParameterListGenerator>(),
			mutator->generator<ExpressionGenerator>(),
			mutator->generator<NatSpecGenerator>(),
		    mutator->generator<StatementGenerator>()
		}
	);
}

string FunctionDefinitionGenerator::visit()
{
	m_functionState = make_shared<FunctionState>();
	state->currentSourceState().enterFunction(m_functionState);
	string identifier = functionIdentifier();
	if (!state->currentSourceState().exportedSymbols.symbols.count(identifier))
		state->currentSourceState().exportedSymbols.symbols.insert(identifier);
	else
		return "";
	m_functionState->setName(identifier);
	string modInvocation = "";
	string virtualise = m_freeFunction ? "" : MP{}.chooseOneOfNStrings({"virtual", ""}, rand);
	string override = "";
	string visibility = m_freeFunction ? "" : MP{}.chooseOneOfNStrings(s_visibility, rand);
	string mutability = m_freeFunction ?
		MP{}.chooseOneOfNStrings(s_freeFunctionMutability, rand) :
		MP{}.chooseOneOfNStrings(s_mutability, rand);

	size_t numInputs = MP{}.distributionOneToN(4, rand) - 1;
	string sep{};
	string inputs{};
	for (size_t i = 0; i < numInputs; i++)
	{
		string location;
		auto inp = generator<ExpressionGenerator>()->randomType();
		if (inp->type.second == "bytes")
			location = MP{}.chooseOneOfN(2, rand) ? "memory" : "calldata";
		m_functionState->addInput(inp);
		inputs += sep + inp->type.second + " " + location + " " + "i" + to_string(m_functionState->numInputs - 1);
		if (sep.empty())
			sep = ", ";
	}
	sep.clear();
	size_t numReturns = MP{}.distributionOneToN(4, rand) - 1;
	string returns{};
	for (size_t i = 0; i < numReturns; i++)
	{
		string location;
		auto r = generator<ExpressionGenerator>()->randomType();
		if (r->type.second == "bytes")
			location = MP{}.chooseOneOfN(2, rand) ? "memory" : "calldata";
		m_functionState->addReturn(r);
		returns += sep + r->type.second + " " + location + " " + "r" + to_string(m_functionState->numReturns - 1);
		if (sep.empty())
			sep = ", ";
	}

	generator<NatSpecGenerator>()->tagCategory(NatSpecGenerator::TagCategory::FUNCTION);
	string natSpecString = generator<NatSpecGenerator>()->visit();
	if (
		(visibility == "internal" || visibility == "private") &&
		mutability == "payable"
	)
		visibility = "public";
	if (visibility == "private" && virtualise == "virtual")
		visibility = "public";
	state->currentSourceState().leaveFunction();
	return Whiskers(m_functionTemplate)
		("natSpecString", natSpecString)
		("id", identifier)
		("paramList", inputs)
		("visibility", visibility)
		("stateMutability", mutability)
		("modInvocation", modInvocation)
		("virtual", virtualise)
		("overrideSpec", override)
		("return", !returns.empty())
		("retParamList", returns)
		("definition", true)
		("body", generator<StatementGenerator>()->visit())
		.render();
}

string EnumDeclaration::visit()
{
	string name = enumName();
	if (!state->currentSourceState().exportedSymbols.types.count(name))
		state->currentSourceState().exportedSymbols.types.insert(name);
	else
		return "";

	string members{};
	string sep{};
	for (size_t i = 0; i < MP{}.distributionOneToN(s_maxMembers, rand); i++)
	{
		members += sep + "M" + to_string(i);
		if (sep.empty())
			sep = ", ";
	}
	return Whiskers(enumTemplate)
		("name", name)
		("members", members)
		.render();
}

//void FunctionTypeGenerator::setup()
//{
//	addGenerators(
//		{
//			mutator->generator<TypeGenerator>()
//		}
//	);
//}
//
//string FunctionTypeGenerator::visit()
//{
//	string visibility = MP{}.chooseOneOfNStrings(s_visibility, rand);
//	size_t numParams = MP{}.distributionOneToN(4, rand) - 1;
//	size_t numReturns = MP{}.distributionOneToN(4, rand) - 1;
//	string sep{};
//	string params{};
//	for (size_t i = 0; i < numParams; i++)
//	{
//		params += sep + generator<TypeGenerator>()->visit();
//		if (sep.empty())
//			sep = ", ";
//	}
//	sep = {};
//	string returns{};
//	for (size_t i = 0; i < numReturns; i++)
//	{
//		returns += sep + generator<TypeGenerator>()->visit();
//		if (sep.empty())
//			sep = ", ";
//	}
//	return Whiskers(m_functionTypeTemplate)
//		("paramList", params)
//		("visibility", visibility)
//		("stateMutability", MP{}.chooseOneOfNStrings(FunctionDefinitionGenerator::s_mutability, rand))
//		("return", !returns.empty())
//		("retParamList", returns)
//		.render();
//}

void ConstantVariableDeclaration::setup()
{
	addGenerators(
		{
			mutator->generator<ExpressionGenerator>()
		}
	);
}

string ConstantVariableDeclaration::visit()
{
	// TODO: Set compileTimeConstantExpressionsOnly in
	// ExpressionGenerator to true
	string type = generator<ExpressionGenerator>()->typeString();
	return Whiskers(constantVarDeclTemplate)
		("type", type)
		("name", "c")
		("expression", type + "(" + generator<ExpressionGenerator>()->visit() + ")")
		.render();
}

void ContractDefinitionGenerator::setup()
{
	addGenerators(
		{
			mutator->generator<StateVariableDeclarationGenerator>(),
			mutator->generator<FunctionDefinitionGenerator>(),
			mutator->generator<NatSpecGenerator>()
		}
	);
}

string ContractDefinitionGenerator::visit()
{
	mutator->generator<FunctionDefinitionGenerator>()->contractFunctionMode();
	string stateVar = generator<StateVariableDeclarationGenerator>()->visit();
	string func = generator<FunctionDefinitionGenerator>()->visit();
	generator<NatSpecGenerator>()->tagCategory(NatSpecGenerator::TagCategory::CONTRACT);
	string natSpecString = generator<NatSpecGenerator>()->visit();
	mutator->generator<FunctionDefinitionGenerator>()->freeFunctionMode();
	// TODO: Implement inheritance
	return Whiskers(m_contractTemplate)
		("natSpecString", natSpecString)
		("abstract", MP{}.chooseOneOfN(s_abstractInvProb, rand))
		("id", "Cx")
		("inheritance", false)
		("inheritanceSpecifierList", "X")
		("stateVar", stateVar)
		("function", func)
		.render();
}

void TestState::print()
{
	std::cout << "Printing test state" << std::endl;
	for (auto const& item: sourceUnitStates)
		std::cout << "Path: " << item.first << std::endl;
}

string TestState::randomNonCurrentPath()
{
	solAssert(size() >= 2, "Solc custom mutator: Invalid test state");
	string fallBackPath{};
	for (auto const& item: sourceUnitStates)
	{
		string iterPath = item.first;
		if (iterPath != currentSourceName)
		{
			// Fallback to first encountered non current path
			fallBackPath = iterPath;
			if (MP{}.chooseOneOfN(size() - 1, rand))
				return iterPath;
		}
	}
	return fallBackPath;
}

string ImportGenerator::visit()
{
	state->print();
	/*
	 * Case 1: No source units defined
	 * Case 2: One source unit defined
	 * Case 3: At least two source units defined
	 */
	// No import
	if (state->empty())
		return {};
	// Self import with a small probability
	else if (state->size() == 1)
	{
		if (MP{}.chooseOneOfN(s_selfImportInvProb, rand))
			return Whiskers(m_importPathAs)
				("path", state->randomPath())
				("as", false)
				.render();
		else
			return {};
	}
	// Import pseudo randomly choosen source unit
	else
	{
		string importPath = state->randomNonCurrentPath();
		auto importedSymbols = state->sourceUnitStates[importPath].exportedSymbols.symbols;
		state->currentSourceState().exportedSymbols.symbols.insert(
			importedSymbols.begin(),
			importedSymbols.end()
		);
		return Whiskers(m_importPathAs)
			("path", importPath)
			("as", false)
			.render();
	}
}

NatSpecGenerator::Tag NatSpecGenerator::randomTag(TagCategory _category)
{
	return s_tagLookup[_category][MP{}.distributionOneToN(s_tagLookup[_category].size(), rand) - 1];
}

string NatSpecGenerator::randomNatSpecString(TagCategory _category)
{
	if (m_nestingDepth > s_maxNestedTags)
		return {};
	else
	{
		m_nestingDepth++;
		string tag{};
		switch (randomTag(_category))
		{
		case Tag::TITLE:
			tag = "@title";
			break;
		case Tag::AUTHOR:
			tag = "@author";
			break;
		case Tag::NOTICE:
			tag = "@notice";
			break;
		case Tag::DEV:
			tag = "@dev";
			break;
		case Tag::PARAM:
			tag = "@param";
			break;
		case Tag::RETURN:
			tag = "@return";
			break;
		case Tag::INHERITDOC:
			tag = "@inheritdoc";
			break;
		}
		return Whiskers(m_tagTemplate)
			("tag", tag)
			("random", MP{}.generateRandomAsciiString(s_maxTextLength, rand))
			("recurse", randomNatSpecString(_category))
			.render();
	}
}

string NatSpecGenerator::visit()
{
// TODO: Enable Natspec strings once we have better precision
#if 1
	return "";
#else
	reset();
	return Whiskers(R"(<nl>/// <natSpecString><nl>)")
		("natSpecString", randomNatSpecString(m_tag))
		("nl", "\n")
		.render();
#endif
}

string SolidityGenerator::generateTestProgram()
{
	createGenerators();
	for (auto &g: m_generators)
		std::visit(AddDependenciesVisitor{}, g);
	string program = generator<TestCaseGenerator>()->visit();
	destroyGenerators();
	destroyState();
	return program;
}
