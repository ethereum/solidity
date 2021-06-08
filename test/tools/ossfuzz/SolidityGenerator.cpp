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
#include <test/tools/ossfuzz/LiteralGeneratorUtil.h>

#include <libsolutil/Whiskers.h>
#include <libsolutil/Visitor.h>

#include <boost/algorithm/string/join.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm/copy.hpp>


using namespace solidity::test::fuzzer;
using namespace solidity::test::fuzzer::mutator;
using namespace solidity::util;
using namespace std;

GeneratorBase::GeneratorBase(SolidityGenerator* _mutator): state(_mutator->testState())
{
	mutator = _mutator;
}

template <typename T>
std::shared_ptr<T> GeneratorBase::generator()
{
	for (auto& g: generators)
		if (std::holds_alternative<std::shared_ptr<T>>(g.first))
			return std::get<std::shared_ptr<T>>(g.first);
	solAssert(false, "");
}

void GeneratorBase::addGenerators(set<pair<GeneratorPtr, unsigned>>&& _generators)
{
	generators = std::move(_generators);
}

string GeneratorBase::visitChildren()
{
	ostringstream os;
	// Randomise visit order
	vector<std::pair<GeneratorPtr, unsigned>> randomisedChildren;
	for (auto const& child: generators)
		randomisedChildren.push_back(child);
	shuffle(randomisedChildren.begin(), randomisedChildren.end(), *uRandDist()->randomEngine);
	for (auto const& child: randomisedChildren)
		if (uRandDist()->likely(child.second + 1))
			for (unsigned i = 0; i < uRandDist()->distributionOneToN(child.second); i++)
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
	set<pair<GeneratorPtr, unsigned>> dependsOn = {{mutator->generator<SourceUnitGenerator>(), s_maxSourceUnits}};
	addGenerators(std::move(dependsOn));
}

string TestCaseGenerator::visit()
{
	return visitChildren();
}

void SourceUnitGenerator::setup()
{
	set<pair<GeneratorPtr, unsigned>> dependsOn = {
		{mutator->generator<ImportGenerator>(), s_maxImports},
		{mutator->generator<PragmaGenerator>(), 1},
		{mutator->generator<ContractGenerator>(), 1},
		{mutator->generator<FunctionGenerator>(), s_maxFreeFunctions}
	};
	addGenerators(std::move(dependsOn));
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
	pragmas.insert(s_abiPragmas[uRandDist()->distributionOneToN(s_abiPragmas.size()) - 1]);
	return boost::algorithm::join(pragmas, "\n") + "\n";
}

void SourceState::resolveImports(map<SolidityTypePtr, string> _importedSymbols)
{
	for (auto const& item: _importedSymbols)
		exports.emplace(item);
}

void SourceState::mergeFunctionState(set<shared_ptr<FunctionState>> _importedFreeFunctions)
{
	freeFunctions += _importedFreeFunctions;
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
		state->sourceUnitState[state->currentPath()]->mergeFunctionState(
			state->sourceUnitState[importPath]->freeFunctions
		);
	}
	return os.str();
}

void ContractGenerator::setup()
{
	set<pair<GeneratorPtr, unsigned>> dependsOn = {{mutator->generator<FunctionGenerator>(), s_maxFunctions}};
	addGenerators(std::move(dependsOn));
}

string ContractGenerator::visit()
{
	ScopeGuard reset([&]() {
		mutator->generator<FunctionGenerator>()->scope(true);
		state->unindent();
		state->exitContract();
	});
	auto set = [&]() {
		state->indent();
		mutator->generator<FunctionGenerator>()->scope(false);
		state->enterContract();
	};
	ostringstream os;
	string inheritance;
	if (state->sourceUnitState[state->currentPath()]->contractType())
		inheritance = state->currentSourceState()->randomContract();
	string name = state->newContract();
	state->updateContract(name);
	os << "contract " << name;
	if (!inheritance.empty())
	{
		os << " is " << inheritance;
		state->currentContractState()->functions += state->contractState[inheritance]->functions;
	}
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

	// TODO: Detect function state mutability instead of generating blanket
	// impure functions.
	if (outputs.empty())
		return retString + " external";
	else
		return retString + " external returns (" + typeString(outputs) +	")";
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

AssignmentStmtGenerator::AssignOp AssignmentStmtGenerator::assignOp(SolidityTypePtr _type)
{
	enum Type
	{
		SIGNEDINTEGER = 1,
		UNSIGNEDINTEGER,
		BOOL,
		FIXEDBYTES,
		BYTES,
		FUNCTION,
		CONTRACT,
		ADDRESS
	};
	static map<Type, vector<AssignOp>> assignOpLookUp = {
		{SIGNEDINTEGER, {
			AssignOp::ASSIGN,
			AssignOp::ASSIGNBITOR,
			AssignOp::ASSIGNBITXOR,
			AssignOp::ASSIGNBITAND,
			AssignOp::ASSIGNADD,
			AssignOp::ASSIGNSUB,
			AssignOp::ASSIGNMUL,
			AssignOp::ASSIGNDIV,
			AssignOp::ASSIGNMOD
		}},
		{UNSIGNEDINTEGER, {
			AssignOp::ASSIGN,
			AssignOp::ASSIGNBITOR,
			AssignOp::ASSIGNBITXOR,
			AssignOp::ASSIGNBITAND,
			AssignOp::ASSIGNSHL,
			AssignOp::ASSIGNSAR,
			AssignOp::ASSIGNSHR,
			AssignOp::ASSIGNADD,
			AssignOp::ASSIGNSUB,
			AssignOp::ASSIGNMUL,
			AssignOp::ASSIGNDIV,
			AssignOp::ASSIGNMOD
		}},
		{FIXEDBYTES, {
			AssignOp::ASSIGN,
			AssignOp::ASSIGNBITOR,
			AssignOp::ASSIGNBITXOR,
			AssignOp::ASSIGNBITAND
		}},
		{BOOL, {AssignOp::ASSIGN}},
		{BYTES, {AssignOp::ASSIGN}},
		{FUNCTION, {AssignOp::ASSIGN}},
		{CONTRACT, {AssignOp::ASSIGN}},
		{ADDRESS, {AssignOp::ASSIGN}}
	};
	vector<AssignOp> possibleOps;
	if (holds_alternative<shared_ptr<IntegerType>>(_type))
	{
		auto t = get<shared_ptr<IntegerType>>(_type);
		if (t->signedType)
			possibleOps = assignOpLookUp[SIGNEDINTEGER];
		else
			possibleOps = assignOpLookUp[UNSIGNEDINTEGER];
	}
	else if (holds_alternative<shared_ptr<FixedBytesType>>(_type))
	{
		possibleOps = assignOpLookUp[FIXEDBYTES];
	}
	else if (holds_alternative<shared_ptr<BoolType>>(_type) ||
		holds_alternative<shared_ptr<BytesType>>(_type) ||
		holds_alternative<shared_ptr<FunctionType>>(_type) ||
		holds_alternative<shared_ptr<ContractType>>(_type) ||
		holds_alternative<shared_ptr<AddressType>>(_type)
	)
	{
		return AssignOp::ASSIGN;
	}
	else
		solAssert(false, "");

	return possibleOps[uRandDist()->distributionOneToN(possibleOps.size()) - 1];
}

string AssignmentStmtGenerator::assignOp(AssignOp _op)
{
	switch (_op)
	{
	case AssignOp::ASSIGN:
		return " = ";
	case AssignOp::ASSIGNBITOR:
		return " |= ";
	case AssignOp::ASSIGNBITXOR:
		return " ^= ";
	case AssignOp::ASSIGNBITAND:
		return " &= ";
	case AssignOp::ASSIGNSHL:
		return " <<= ";
	case AssignOp::ASSIGNSAR:
	case AssignOp::ASSIGNSHR:
		return " >>= ";
	case AssignOp::ASSIGNADD:
		return " += ";
	case AssignOp::ASSIGNSUB:
		return " -= ";
	case AssignOp::ASSIGNMUL:
		return " *= ";
	case AssignOp::ASSIGNDIV:
		return " /= ";
	case AssignOp::ASSIGNMOD:
		return " %= ";
	default:
		solAssert(false, "");
	}
}

string AssignmentStmtGenerator::visit()
{
	ExpressionGenerator exprGen{state};
	auto lhs = exprGen.randomLValueExpression();
	exprGen.resetNestingDepth();
	if (!lhs.has_value())
		return "\n";
	auto rhs = exprGen.rLValueOrLiteral(lhs.value());
	exprGen.resetNestingDepth();
	if (!rhs.has_value())
		return "\n";
	auto operation = assignOp(lhs.value().first);
	return indentation() + lhs.value().second + assignOp(operation) + rhs.value().second + ";\n";
}

string ExpressionStmtGenerator::visit()
{
	ExpressionGenerator exprGen{state};
	auto randomType = TypeProvider{state}.type();
	pair<SolidityTypePtr, string> randomTypeName = {randomType, {}};
	auto expression = exprGen.rLValueOrLiteral(randomTypeName);
	if (expression.has_value())
		return indentation() + expression.value().second + ";\n";
	else
		return "\n";
}

void IfStmtGenerator::setup()
{
	set<pair<GeneratorPtr, unsigned>> dependsOn = {
		{mutator->generator<BlockStmtGenerator>(), 1},
	};
	addGenerators(std::move(dependsOn));
}

string IfStmtGenerator::conditionalStmt(Condition _cond)
{
	ostringstream condStmt;
	ExpressionGenerator exprGen{state};
	auto boolType = make_shared<BoolType>();
	pair<SolidityTypePtr, string> boolTypeName = {boolType, {}};
	auto expression = exprGen.rLValueOrLiteral(boolTypeName);
	solAssert(expression.has_value(), "");
	if (_cond == Condition::IF)
		condStmt << indentation()
		         << "if ("
		         << expression.value().second
		         << ")\n";
	else if (_cond == Condition::ELSEIF)
		condStmt << indentation()
		         << "else if ("
		         << expression.value().second
		         << ")\n";
	else
		condStmt << indentation()
		         << "else"
		         << "\n";
	// Make sure block stmt generator does not output an unchecked block
	mutator->generator<BlockStmtGenerator>()->unchecked(false);
	ostringstream condBlock;
	condBlock << visitChildren();
	if (condBlock.str().empty())
		condBlock << indentation() << "{ }\n";
	condStmt << condBlock.str();
	return condStmt.str();
}

string IfStmtGenerator::visit()
{
	ostringstream ifStmt;
	auto numConditionStmts = uRandDist()->distributionOneToN(s_maxConditionalStmts);
	ifStmt << conditionalStmt(Condition::IF);
	numConditionStmts--;
	solAssert(numConditionStmts >= 0, "");
	while (numConditionStmts > 0)
	{
		if (numConditionStmts == 1)
		{
			if (uRandDist()->probable(2))
				ifStmt << conditionalStmt(Condition::ELSEIF);
			else
				ifStmt << conditionalStmt(Condition::ELSE);
		}
		else
			ifStmt << conditionalStmt(Condition::ELSEIF);
		numConditionStmts--;
	}
	return ifStmt.str();
}

void WhileStmtGenerator::setup()
{
	set<pair<GeneratorPtr, unsigned>> dependsOn = {
		{mutator->generator<BlockStmtGenerator>(), 1},
	};
	addGenerators(std::move(dependsOn));
}

string WhileStmtGenerator::visit()
{
	ostringstream whileStmt;
	state->enterLoop();
	ScopeGuard exitLoop([&]() { state->exitLoop(); });
	ExpressionGenerator exprGen{state};
	auto boolType = make_shared<BoolType>();
	pair<SolidityTypePtr, string> boolTypeName = {boolType, {}};
	auto expression = exprGen.rLValueOrLiteral(boolTypeName);
	solAssert(expression.has_value(), "");
	bool doWhile = uRandDist()->probable(2);
	if (doWhile)
		whileStmt << indentation()
			<< "do\n";
	else
		whileStmt << indentation()
			<< "while ("
			<< expression.value().second
			<< ")\n";
	// Make sure block stmt generator does not output an unchecked block
	mutator->generator<BlockStmtGenerator>()->unchecked(false);
	ostringstream whileBlock;
	whileBlock << visitChildren();
	if (whileBlock.str().empty())
		whileBlock << indentation() << "{ }\n";
	whileStmt << whileBlock.str();
	if (doWhile)
		whileStmt << indentation()
			<< "while ("
			<< expression.value().second
			<< ")\n";
	return whileStmt.str();
}

void StatementGenerator::setup()
{
	set<pair<GeneratorPtr, unsigned>> dependsOn = {
		{mutator->generator<BlockStmtGenerator>(), 1},
		{mutator->generator<AssignmentStmtGenerator>(), 1},
		{mutator->generator<FunctionCallGenerator>(), 1},
		{mutator->generator<ExpressionStmtGenerator>(), 1},
		{mutator->generator<IfStmtGenerator>(), 2},
		{mutator->generator<WhileStmtGenerator>(), 1},
		{mutator->generator<BreakStmtGenerator>(), 1},
		{mutator->generator<ContinueStmtGenerator>(), 1}
	};
	addGenerators(std::move(dependsOn));
}

string StatementGenerator::visit()
{
	bool unchecked = uRandDist()->probable(s_uncheckedBlockInvProb);
	bool inUnchecked = mutator->generator<BlockStmtGenerator>()->unchecked();
	// Do not generate nested unchecked blocks.
	bool generateUncheckedBlock = unchecked && !inUnchecked;
	if (generateUncheckedBlock)
		mutator->generator<BlockStmtGenerator>()->unchecked(true);

	ostringstream os;
	// Choose random statement type
	auto genIterator = generators.begin();
	auto advanceBy = uRandDist()->distributionOneToN(generators.size()) - 1;
	std::advance(genIterator, advanceBy);
	auto child = *genIterator;
	os << std::visit(GenericVisitor{
		[](auto const& _item) { return _item->generate(); }
	}, child.first);
	if (holds_alternative<shared_ptr<BlockStmtGenerator>>(child.first) &&
		generateUncheckedBlock
	)
	{
		get<shared_ptr<BlockStmtGenerator>>(child.first)->unchecked(false);
		get<shared_ptr<BlockStmtGenerator>>(child.first)->resetInUnchecked();
	}
	return os.str();
}

void BlockStmtGenerator::setup()
{
	set<pair<GeneratorPtr, unsigned>> dependsOn = {{mutator->generator<StatementGenerator>(), s_maxStatements}};
	addGenerators(std::move(dependsOn));
}

string BlockStmtGenerator::visit()
{
	if (nestingTooDeep())
		return indentation() + "{ }\n";
	ScopeGuard decDepth([&]() { decrementNestingDepth(); });
	incrementNestingDepth();
	ostringstream block;
	if (unchecked() && !m_inUnchecked)
	{
		block << indentation() + "unchecked " + "{\n";
		m_inUnchecked = true;
	}
	else
		block << indentation() + "{\n";

	// Create blockscope inside current function state
	auto newBlockScope = make_shared<BlockScope>();
	state->currentFunctionState()->scopes.push_back(
		std::move(newBlockScope)
	);
	state->indent();
	block << visitChildren();
	state->unindent();
	state->currentFunctionState()->scopes.pop_back();
	block << indentation() << "}\n";
	return block.str();
}

void FunctionGenerator::setup()
{
	set<pair<GeneratorPtr, unsigned>> dependsOn = {{mutator->generator<BlockStmtGenerator>(), 1}};
	addGenerators(std::move(dependsOn));
}

string FunctionGenerator::visit()
{
	string visibility;
	string name = state->newFunction();
	state->updateFunction(name, m_freeFunction);
	if (!m_freeFunction)
		visibility = "external";

	// Add I/O
	if (uRandDist()->likely(s_maxInputs + 1))
		for (unsigned i = 0; i < uRandDist()->distributionOneToN(s_maxInputs); i++)
			state->currentFunctionState()->addInput(TypeProvider{state}.type());

	if (uRandDist()->likely(s_maxOutputs + 1))
		for (unsigned i = 0; i < uRandDist()->distributionOneToN(s_maxOutputs); i++)
			state->currentFunctionState()->addOutput(TypeProvider{state}.type());

	ostringstream function;
	function << indentation()
		<< "function "
		<< name
		<< state->currentFunctionState()->params(FunctionState::Params::INPUT)
		<< " "
		<< visibility;
	if (!state->currentFunctionState()->outputs.empty())
		function << " returns"
			<< state->currentFunctionState()->params(FunctionState::Params::OUTPUT);
	ostringstream block;
	// Make sure block stmt generator does not output an unchecked block
	mutator->generator<BlockStmtGenerator>()->unchecked(false);
	block << visitChildren();
	if (m_freeFunction)
		state->currentSourceState()->addFreeFunction(state->currentFunctionState());
	else
		state->currentContractState()->addFunction(state->currentFunctionState());
	// Since visitChildren() may not visit block stmt, we default to an empty
	// block.
	if (block.str().empty())
		block << indentation() << "{ }\n";
	function << "\n" << block.str();
	return function.str();
}

void FunctionGenerator::endVisit()
{
	mutator->generator<BlockStmtGenerator>()->resetNestingDepth();
}

vector<pair<SolidityTypePtr, string>> ExpressionGenerator::liveVariables()
{
	auto liveVariables = state->currentFunctionState()->inputs |
	                     ranges::views::transform([](auto& _item) { return _item; }) |
	                     ranges::to<vector<pair<SolidityTypePtr, string>>>();
	liveVariables += state->currentFunctionState()->outputs |
	                 ranges::views::transform([](auto& _item) { return _item; }) |
	                 ranges::to<vector<pair<SolidityTypePtr, string>>>();
	for (auto const& scope: state->currentFunctionState()->scopes)
		liveVariables += scope->variables |
		                 ranges::views::transform([](auto& _item) { return _item; }) |
		                 ranges::to<vector<pair<SolidityTypePtr, string>>>();
	return liveVariables;
}

vector<pair<SolidityTypePtr, string>> ExpressionGenerator::liveVariables(
	pair<SolidityTypePtr, string>& _typeName
)
{
	auto liveTypedVariables = state->currentFunctionState()->inputs |
	                          ranges::views::filter([&_typeName](auto& _item) {
		                          return _item.first.index() == _typeName.first.index() &&
		                                 _item.second != _typeName.second &&
		                                 visit(TypeComparator{}, _item.first, _typeName.first);
	                          }) |
	                          ranges::to<vector<pair<SolidityTypePtr, string>>>();
	liveTypedVariables += state->currentFunctionState()->outputs |
	                      ranges::views::filter([&_typeName](auto& _item) {
		                      return _item.first.index() == _typeName.first.index() &&
		                             _item.second != _typeName.second &&
		                             visit(TypeComparator{}, _item.first, _typeName.first);
	                      }) |
	                      ranges::to<vector<pair<SolidityTypePtr, string>>>();
	for (auto const& scope: state->currentFunctionState()->scopes)
		liveTypedVariables += scope->variables |
		                      ranges::views::filter([&_typeName](auto& _item) {
			                      return _item.first.index() == _typeName.first.index() &&
			                             _item.second != _typeName.second &&
			                             visit(TypeComparator{}, _item.first, _typeName.first);
		                      }) |
		                      ranges::to<vector<pair<SolidityTypePtr, string>>>();
	return liveTypedVariables;
}

optional<pair<SolidityTypePtr, string>> ExpressionGenerator::randomLValueExpression()
{
	auto liveVars = liveVariables();
	if (liveVars.empty())
		return nullopt;
	auto randomLValue = liveVars[state->uRandDist->distributionOneToN(liveVars.size()) - 1];
	return lValueExpression(randomLValue);
}

optional<pair<SolidityTypePtr, string>> ExpressionGenerator::lValueExpression(
	pair<SolidityTypePtr, string>& _typeName
)
{
	// Filter non-identical variables of the same type.
	auto typedLiveVars = liveVariables(_typeName);

	if (typedLiveVars.empty())
		return nullopt;
	else
		return typedLiveVars[state->uRandDist->distributionOneToN(typedLiveVars.size()) - 1];
}

optional<pair<SolidityTypePtr, string>> ExpressionGenerator::literal(SolidityTypePtr _type)
{
	bool functionType = holds_alternative<shared_ptr<FunctionType>>(_type);
	bool contractType = holds_alternative<shared_ptr<ContractType>>(_type);
	// TODO: Generate literals for contract and function types.
	if (functionType || contractType)
		return nullopt;
	else
	{
		string literalValue = visit(LiteralGenerator{state}, _type);
		return pair(_type, literalValue);
	}
}

optional<pair<SolidityTypePtr, string>> ExpressionGenerator::rLValueOrLiteral(
	pair<SolidityTypePtr, string>& _typeName
)
{
	optional<pair<SolidityTypePtr, string>> rLValue;
	// Try to obtain an RLValue failing which a typed literal.
	rLValue = rOrLValueExpression(_typeName);
	if (!rLValue.has_value())
		rLValue = literal(_typeName.first);
	return rLValue;
}

optional<pair<SolidityTypePtr, string>> ExpressionGenerator::unaryExpression(
	pair<SolidityTypePtr, string>& _typeName,
	string const& _op
)
{
	optional<pair<SolidityTypePtr, string>> rLValue = rLValueOrLiteral(_typeName);
	pair<SolidityTypePtr, string> result;
	if (rLValue.has_value())
	{
		result = rLValue.value();
		result.second = _op + "(" + result.second + ")";
		return result;
	}
	else
		return nullopt;
}

optional<pair<SolidityTypePtr, string>> ExpressionGenerator::binaryExpression(
	pair<SolidityTypePtr, string>& _typeName,
	string const& _op
)
{
	auto left = rLValueOrLiteral(_typeName);
	auto right = rLValueOrLiteral(_typeName);
	if (left.has_value() && right.has_value())
	{
		auto leftResult = left.value();
		auto rightResult = right.value();
		leftResult.second = "(" +
			leftResult.second +
			" " +
			_op +
			" " +
			rightResult.second +
			")";
		return leftResult;
	}
	else
		return nullopt;
}

optional<pair<SolidityTypePtr, string>> ExpressionGenerator::incDecOperation(
	pair<SolidityTypePtr, string>& _typeName,
	string const& _op,
	bool _prefixOp
)
{
	solAssert(
		holds_alternative<shared_ptr<IntegerType>>(_typeName.first),
		"Invalid inc/dec op"
	);

	auto lValue = lValueExpression(_typeName);
	if (!lValue.has_value())
		return nullopt;

	auto lResult = lValue.value();
	if (_prefixOp)
		lResult.second = _op + lResult.second;
	else
		lResult.second += _op;
	return lResult;
}

ExpressionGenerator::RLValueExpr ExpressionGenerator::expressionType(SolidityTypePtr& _typePtr)
{
	vector<RLValueExpr> permittedTypes;

	if (holds_alternative<shared_ptr<BoolType>>(_typePtr))
	{
		permittedTypes = {
			RLValueExpr::VARREF,
			RLValueExpr::LIT,
			RLValueExpr::NOT,
			RLValueExpr::LT,
			RLValueExpr::GT,
			RLValueExpr::LTE,
			RLValueExpr::GTE,
			RLValueExpr::EQ,
			RLValueExpr::NEQ,
			RLValueExpr::AND,
			RLValueExpr::OR
		};
	}
	else if (holds_alternative<shared_ptr<FixedBytesType>>(_typePtr))
	{
		permittedTypes = {
			RLValueExpr::VARREF,
			RLValueExpr::LIT,
			RLValueExpr::BITAND,
			RLValueExpr::BITXOR,
			RLValueExpr::BITOR,
		};
	}
	else if (holds_alternative<shared_ptr<IntegerType>>(_typePtr))
	{
		bool signedType = get<shared_ptr<IntegerType>>(_typePtr)->signedType;
		if (signedType)
			permittedTypes = {
				RLValueExpr::VARREF,
				RLValueExpr::LIT,
				RLValueExpr::PINC,
				RLValueExpr::PDEC,
				RLValueExpr::SINC,
				RLValueExpr::SDEC,
				RLValueExpr::BITNOT,
				RLValueExpr::USUB,
				RLValueExpr::MUL,
				RLValueExpr::DIV,
				RLValueExpr::MOD,
				RLValueExpr::ADD,
				RLValueExpr::BSUB,
				RLValueExpr::BITAND,
				RLValueExpr::BITXOR,
				RLValueExpr::BITOR
			};
		else
			permittedTypes = {
				RLValueExpr::VARREF,
				RLValueExpr::LIT,
				RLValueExpr::PINC,
				RLValueExpr::PDEC,
				RLValueExpr::SINC,
				RLValueExpr::SDEC,
				RLValueExpr::BITNOT,
				RLValueExpr::EXP,
				RLValueExpr::MUL,
				RLValueExpr::DIV,
				RLValueExpr::MOD,
				RLValueExpr::ADD,
				RLValueExpr::BSUB,
				RLValueExpr::SHL,
				RLValueExpr::SHR,
				RLValueExpr::BITAND,
				RLValueExpr::BITXOR,
				RLValueExpr::BITOR
			};
	}
	else
	{
		permittedTypes = {RLValueExpr::VARREF, RLValueExpr::LIT};
	}
	return permittedTypes[state->uRandDist->distributionOneToN(permittedTypes.size()) - 1];
}

optional<pair<SolidityTypePtr, string>> ExpressionGenerator::rOrLValueExpression(pair<SolidityTypePtr, string>& _typeName)
{
	RLValueExpr exprType = expressionType(_typeName.first);

	if (deeplyNested())
		return literal(_typeName.first);

	incrementNestingDepth();

	string op;
	switch (exprType)
	{
	case RLValueExpr::VARREF:
		return lValueExpression(_typeName);
	case RLValueExpr::PINC:
		return incDecOperation(_typeName, "++", true);
	case RLValueExpr::PDEC:
		return incDecOperation(_typeName, "--", true);
	case RLValueExpr::SINC:
		return incDecOperation(_typeName, "++", false);
	case RLValueExpr::SDEC:
		return incDecOperation(_typeName, "--", false);
	case RLValueExpr::NOT:
	{
		// Logical not may only be applied to expressions of boolean type.
		solAssert(
			holds_alternative<shared_ptr<BoolType>>(_typeName.first),
			"Invalid not op"
		);
		op = "!";
		return unaryExpression(_typeName, op);
	}
	case RLValueExpr::BITNOT:
	{
		// Bitwise not may only be applied to integer types.
		solAssert(
			holds_alternative<shared_ptr<IntegerType>>(_typeName.first),
			"Invalid bitnot op"
		);
		op = "~";
		return unaryExpression(_typeName, op);
	}
	case RLValueExpr::USUB:
	{
		// Unary sub may only be applied to signed integer types
		solAssert(
			holds_alternative<shared_ptr<IntegerType>>(_typeName.first) &&
			get<shared_ptr<IntegerType>>(_typeName.first)->signedType,
			"Invalid unary sub op"
		);
		op = "-";
		return unaryExpression(_typeName, op);
	}
	case RLValueExpr::EXP:
	{
		// Exponentiation may only be applied to unsigned integer types
		solAssert(
			holds_alternative<shared_ptr<IntegerType>>(_typeName.first) &&
			!get<shared_ptr<IntegerType>>(_typeName.first)->signedType,
			"Invalid exp op"
		);
		op = "**";
		return binaryExpression(_typeName, op);
	}
	// Arithmetic ops only be applied to integer types
	case RLValueExpr::MUL:
	{
		solAssert(
			holds_alternative<shared_ptr<IntegerType>>(_typeName.first),
			"Invalid mul op"
		);
		op = "*";
		return binaryExpression(_typeName, op);
	}
	case RLValueExpr::DIV:
	{
		solAssert(
			holds_alternative<shared_ptr<IntegerType>>(_typeName.first),
			"Invalid div op"
		);
		op = "/";
		return binaryExpression(_typeName, op);
	}
	case RLValueExpr::MOD:
	{
		solAssert(
			holds_alternative<shared_ptr<IntegerType>>(_typeName.first),
			"Invalid mod op"
		);
		op = "%";
		return binaryExpression(_typeName, op);
	}
	case RLValueExpr::ADD:
	{
		solAssert(
			holds_alternative<shared_ptr<IntegerType>>(_typeName.first),
			"Invalid add op"
		);
		op = "+";
		return binaryExpression(_typeName, op);
	}
	case RLValueExpr::BSUB:
	{
		solAssert(
			holds_alternative<shared_ptr<IntegerType>>(_typeName.first),
			"Invalid sub op"
		);
		op = "-";
		return binaryExpression(_typeName, op);
	}
	case RLValueExpr::SHL:
	{
		// Left shift may only be applied to unsigned integer types.
		solAssert(
			holds_alternative<shared_ptr<IntegerType>>(_typeName.first) &&
			!get<shared_ptr<IntegerType>>(_typeName.first)->signedType,
			"Invalid shl op"
		);
		op = "<<";
		return binaryExpression(_typeName, op);
	}
	case RLValueExpr::SHR:
	{
		// Left shift may only be applied to unsigned integer types.
		solAssert(
			holds_alternative<shared_ptr<IntegerType>>(_typeName.first) &&
			!get<shared_ptr<IntegerType>>(_typeName.first)->signedType,
			"Invalid shr op"
		);
		op = ">>";
		return binaryExpression(_typeName, op);
	}
	case RLValueExpr::BITAND:
	{
		// Bitwise ops may only be applied to integer and fixed bytes types.
		solAssert(
			holds_alternative<shared_ptr<IntegerType>>(_typeName.first) ||
			holds_alternative<shared_ptr<FixedBytesType>>(_typeName.first),
			"Invalid bitand op"
		);
		op = "&";
		return binaryExpression(_typeName, op);
	}
	case RLValueExpr::BITOR:
	{
		// Bitwise ops may only be applied to integer and fixed bytes types.
		solAssert(
			holds_alternative<shared_ptr<IntegerType>>(_typeName.first) ||
			holds_alternative<shared_ptr<FixedBytesType>>(_typeName.first),
			"Invalid bitand op"
		);
		op = "|";
		return binaryExpression(_typeName, op);
	}
	case RLValueExpr::BITXOR:
	{
		// Bitwise ops may only be applied to integer and fixed bytes types.
		solAssert(
			holds_alternative<shared_ptr<IntegerType>>(_typeName.first) ||
			holds_alternative<shared_ptr<FixedBytesType>>(_typeName.first),
			"Invalid bitand op"
		);
		op = "^";
		return binaryExpression(_typeName, op);
	}
	case RLValueExpr::LT:
	{
		// Comparison ops may be applied only if LHS type is boolean.
		solAssert(
			holds_alternative<shared_ptr<BoolType>>(_typeName.first),
			"Invalid lt op"
		);

		// Types being compared could be integer, fixed bytes, address, or contract.
		auto operandType = TypeProvider{state}.type();
		bool opFunctionType = holds_alternative<shared_ptr<FunctionType>>(operandType);
		bool opBoolType = holds_alternative<shared_ptr<BoolType>>(operandType);
		bool opBytesType = holds_alternative<shared_ptr<BytesType>>(operandType);
		if (opFunctionType || opBoolType || opBytesType)
			return nullopt;
		op = "<";
		pair<SolidityTypePtr, string> operandTypeName = {operandType, {}};
		return binaryExpression(operandTypeName, op);
	}
	case RLValueExpr::GT:
	{
		// Comparison ops may be applied only if LHS type is boolean.
		solAssert(
			holds_alternative<shared_ptr<BoolType>>(_typeName.first),
			"Invalid gt op"
		);

		// Types being compared could be integer, fixed bytes, address, or contract.
		auto operandType = TypeProvider{state}.type();
		bool opFunctionType = holds_alternative<shared_ptr<FunctionType>>(operandType);
		bool opBoolType = holds_alternative<shared_ptr<BoolType>>(operandType);
		bool opBytesType = holds_alternative<shared_ptr<BytesType>>(operandType);
		if (opFunctionType || opBoolType || opBytesType)
			return nullopt;
		op = ">";
		pair<SolidityTypePtr, string> operandTypeName = {operandType, {}};
		return binaryExpression(operandTypeName, op);
	}
	case RLValueExpr::LTE:
	{
		// Comparison ops may be applied only if LHS type is boolean.
		solAssert(
			holds_alternative<shared_ptr<BoolType>>(_typeName.first),
			"Invalid lte op"
		);

		// Types being compared could be integer, fixed bytes, address, or contract.
		auto operandType = TypeProvider{state}.type();
		bool opFunctionType = holds_alternative<shared_ptr<FunctionType>>(operandType);
		bool opBoolType = holds_alternative<shared_ptr<BoolType>>(operandType);
		bool opBytesType = holds_alternative<shared_ptr<BytesType>>(operandType);
		if (opFunctionType || opBoolType || opBytesType)
			return nullopt;
		op = "<=";
		pair<SolidityTypePtr, string> operandTypeName = {operandType, {}};
		return binaryExpression(operandTypeName, op);
	}
	case RLValueExpr::GTE:
	{
		// Comparison ops may be applied only if LHS type is boolean.
		solAssert(
			holds_alternative<shared_ptr<BoolType>>(_typeName.first),
			"Invalid gte op"
		);

		// Types being compared could be integer, fixed bytes, address, or contract.
		auto operandType = TypeProvider{state}.type();
		bool opFunctionType = holds_alternative<shared_ptr<FunctionType>>(operandType);
		bool opBoolType = holds_alternative<shared_ptr<BoolType>>(operandType);
		bool opBytesType = holds_alternative<shared_ptr<BytesType>>(operandType);
		if (opFunctionType || opBoolType || opBytesType)
			return nullopt;
		op = ">=";
		pair<SolidityTypePtr, string> operandTypeName = {operandType, {}};
		return binaryExpression(operandTypeName, op);
	}
	case RLValueExpr::EQ:
	{
		// Comparison ops may be applied only if LHS type is boolean.
		solAssert(
			holds_alternative<shared_ptr<BoolType>>(_typeName.first),
			"Invalid eq op"
		);

		// Types being compared could be integer, fixed bytes, address, or contract.
		auto operandType = TypeProvider{state}.type();
		bool opFunctionType = holds_alternative<shared_ptr<FunctionType>>(operandType);
		bool opBoolType = holds_alternative<shared_ptr<BoolType>>(operandType);
		bool opBytesType = holds_alternative<shared_ptr<BytesType>>(operandType);
		if (opFunctionType || opBoolType || opBytesType)
			return nullopt;
		op = "==";
		pair<SolidityTypePtr, string> operandTypeName = {operandType, {}};
		return binaryExpression(operandTypeName, op);
	}
	case RLValueExpr::NEQ:
	{
		// Comparison ops may be applied only if LHS type is boolean.
		solAssert(
			holds_alternative<shared_ptr<BoolType>>(_typeName.first),
			"Invalid neq op"
		);

		// Types being compared could be integer, fixed bytes, address, or contract.
		auto operandType = TypeProvider{state}.type();
		bool opFunctionType = holds_alternative<shared_ptr<FunctionType>>(operandType);
		bool opBoolType = holds_alternative<shared_ptr<BoolType>>(operandType);
		bool opBytesType = holds_alternative<shared_ptr<BytesType>>(operandType);
		if (opFunctionType || opBoolType || opBytesType)
			return nullopt;
		op = "!=";
		pair<SolidityTypePtr, string> operandTypeName = {operandType, {}};
		return binaryExpression(operandTypeName, op);
	}
	case RLValueExpr::AND:
	{
		// Logical ops may be applied only to boolean types.
		solAssert(
			holds_alternative<shared_ptr<BoolType>>(_typeName.first),
			"Invalid and op"
		);

		op = "&&";
		return binaryExpression(_typeName, op);
	}
	case RLValueExpr::OR:
	{
		// Logical ops may be applied only to boolean types.
		solAssert(
			holds_alternative<shared_ptr<BoolType>>(_typeName.first),
			"Invalid or op"
		);

		op = "||";
		return binaryExpression(_typeName, op);
	}
	case RLValueExpr::LIT:
		return literal(_typeName.first);
	default:
		solAssert(false, "");
	}
}

string LiteralGenerator::operator()(shared_ptr<AddressType> const&)
{
	string preChecksumAddress = LiteralGeneratorUtil{}.fixedBytes(
		20,
		(*state->uRandDist->randomEngine)(),
		true
	);
	return string("address(") +
		solidity::util::getChecksummedAddress(preChecksumAddress) +
		")";
}

string LiteralGenerator::operator()(shared_ptr<BoolType> const&)
{
	if (state->uRandDist->probable(2))
		return "true";
	else
		return "false";
}

string LiteralGenerator::operator()(shared_ptr<BytesType> const&)
{
	return "\"" +
		LiteralGeneratorUtil{}.fixedBytes(
			state->uRandDist->distributionOneToN(32),
			(*state->uRandDist->randomEngine)(),
			true
		) +
		"\"";
}

string LiteralGenerator::operator()(shared_ptr<FixedBytesType> const& _type)
{
	bool bytes20 = _type->numBytes == 20;
	string literalString = "0x" +
		LiteralGeneratorUtil{}.fixedBytes(
		_type->numBytes,
		(*state->uRandDist->randomEngine)(),
		true
		);
	if (bytes20)
		return "bytes20(address(" + solidity::util::getChecksummedAddress(literalString) + "))";
	else
		return literalString;
}

string LiteralGenerator::operator()(shared_ptr<ContractType> const&)
{
	solAssert(false, "");
}

string LiteralGenerator::operator()(shared_ptr<FunctionType> const&)
{
	solAssert(false, "");
}

string LiteralGenerator::operator()(shared_ptr<IntegerType> const& _type)
{
	return LiteralGeneratorUtil{}.integerValue(
		(*state->uRandDist->randomEngine)(),
		_type->numBits,
		_type->signedType
	);
}

optional<SolidityTypePtr> TypeProvider::type(SolidityTypePtr _type)
{
	vector<SolidityTypePtr> matchingTypes = state->currentFunctionState()->inputs |
		ranges::views::filter([&_type](auto& _item) {
			return _item.first >= _type;
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
		return make_shared<FunctionType>(true);
	case Type::CONTRACT:
		if (state->sourceUnitState[state->currentPath()]->contractType())
			return state->sourceUnitState[state->currentPath()]->randomContractType();
		return make_shared<BoolType>();
	default:
		solAssert(false, "");
	}
}

string FunctionCallGenerator::lhs(vector<pair<SolidityTypePtr, string>>& _functionReturnTypeNames)
{
	ExpressionGenerator exprGen{state};
	ostringstream callStmtLhs;

	auto assignToVars = _functionReturnTypeNames |
		ranges::views::transform([&exprGen](auto& _item) -> pair<bool, optional<pair<SolidityTypePtr, string>>> {
			auto e = exprGen.lValueExpression(_item);
			exprGen.resetNestingDepth();
			if (e.has_value())
				return {true, e.value()};
			else
				return {false, nullopt};
		});
	bool useExistingVars = ranges::all_of(
		assignToVars,
		[](auto const& _item) -> bool { return _item.first; }
	);

	if (useExistingVars)
	{
		auto vars = assignToVars |
			ranges::views::transform([](auto const& _item) { return _item.second.value().second; }) |
			ranges::to<vector<string>>();
		callStmtLhs << "("
			<< boost::algorithm::join(vars, ",")
			<< ") = ";
	}
	else
	{
		auto newVars = _functionReturnTypeNames |
			ranges::views::transform([&](auto const& _item) -> string {
				state->currentFunctionState()->addLocal(_item.first);
				string varName = state->currentFunctionState()->scopes.back()->variables.back().second;
				return std::visit(
						GenericVisitor{[](auto const& _it) { return _it->toString(); }},
						_item.first
					) +
					" " +
					varName;
			}) |
			ranges::to<vector<string>>();
		callStmtLhs << "("
			<< boost::algorithm::join(newVars, ", ")
			<< ") = ";
	}
	return callStmtLhs.str();
}

optional<string> FunctionCallGenerator::rhs(vector<pair<SolidityTypePtr, string>>& _functionInputTypeNames)
{
	ExpressionGenerator exprGen{state};
	ostringstream callStmtRhs;

	auto inputArguments = _functionInputTypeNames |
		ranges::views::transform([&exprGen](auto& _item) -> pair<bool, optional<pair<SolidityTypePtr, string>>>
		{
			auto e = exprGen.rLValueOrLiteral(_item);
			exprGen.resetNestingDepth();
			if (e.has_value())
				return {true, e.value()};
			else
				return {false, nullopt};
		}) |
		ranges::to<vector<pair<bool, optional<pair<SolidityTypePtr, string>>>>>();
	bool inputArgsValid = ranges::all_of(
		inputArguments,
		[](auto const& _item) -> bool { return _item.first; }
	);

	if (inputArgsValid)
	{
		auto vars = inputArguments |
			ranges::views::transform([](auto const& _item) {
				solAssert(_item.second.has_value(), "");
				return _item.second.value().second;
			}) |
			ranges::to<vector<string>>();
		callStmtRhs << boost::algorithm::join(vars, ",");
		return callStmtRhs.str();
	}
	else
	{
		return nullopt;
	}
}

string FunctionCallGenerator::callStmt(shared_ptr<FunctionState> _callee)
{
	ostringstream callStmtStream;
	string lhsExpr;
	string rhsExpr;
	bool callValid = true;

	// Create arguments only if function contains non-zero input parameters.
	if (!_callee->inputs.empty())
	{
		auto callRhs = rhs(_callee->inputs);
		// Arguments may not be found for function and contract types. In this
		// case, do not make the call.
		if (callRhs.has_value())
			rhsExpr = (_callee->type->functionScope() ? "" : "this.") + _callee->name + "(" + callRhs.value() + ");";
		else
			callValid = false;
	}
	else
		rhsExpr = (_callee->type->functionScope() ? "" : "this.") + _callee->name + "();";

	if (callValid)
	{
		// Create lhs expression only if function outputs non-zero return values.
		if (!_callee->outputs.empty())
			lhsExpr = lhs(_callee->outputs);

		callStmtStream << indentation()
			<< lhsExpr
			<< rhsExpr;
	}

	callStmtStream << "\n";
	return callStmtStream.str();
}

string FunctionCallGenerator::visit()
{
//	// TODO: Generalise call to varargs function
//	for (auto const& f: state->currentFunctionState()->inputs)
//		if (holds_alternative<shared_ptr<FunctionType>>(f.first))
//			return indentation() + f.second + "();\n";

	// Consolidate available functions
	auto availableFunctions = state->currentSourceState()->freeFunctions;
	if (state->insideContract)
		availableFunctions += state->currentContractState()->functions;
	if (availableFunctions.empty())
		return "\n";

	shared_ptr<FunctionState> callee;
	if (availableFunctions.size() > 1)
	{
		for (auto const& i: availableFunctions)
			if (uRandDist()->probable(availableFunctions.size()))
				callee = i;
	}
	else
		callee = *availableFunctions.begin();

	if (callee)
		return callStmt(callee);
	else
		return "\n";
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
	auto engine = make_unique<RandomEngine>(_seed);
	m_urd = make_shared<UniformRandomDistribution>(std::move(engine));
	m_state = make_shared<TestState>(m_urd);
}

SolidityGenerator::~SolidityGenerator()
{
	for (auto& g: m_generators)
		std::visit(GenericVisitor{
			[&](auto const& _item) { return _item->teardown(); }
		}, g);
	m_generators.clear();
	m_urd.reset();
	m_state.reset();
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
	return program;
}
