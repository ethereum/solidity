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
	addGenerators({
		{mutator->generator<FunctionGenerator>(), s_maxFunctions}
	});
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

	return possibleOps[uRandDist->distributionOneToN(possibleOps.size()) - 1];
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
	if (!lhs.has_value())
		return "\n";
	auto rhs = exprGen.expression(lhs.value());
	if (!rhs.has_value())
		return "\n";
	auto operation = assignOp(lhs.value().first);
	return indentation() + lhs.value().second + assignOp(operation) + rhs.value().second + ";\n";
}

void StatementGenerator::setup()
{
	addGenerators({
		{mutator->generator<BlockStmtGenerator>(), 1},
		{mutator->generator<AssignmentStmtGenerator>(), 1},
		{mutator->generator<FunctionCallGenerator>(), 1}
	});
}

string StatementGenerator::visit()
{
	bool unchecked = uRandDist->probable(s_uncheckedBlockInvProb);
	bool inUnchecked = mutator->generator<BlockStmtGenerator>()->unchecked();
	// Do not generate nested unchecked blocks.
	bool generateUncheckedBlock = unchecked && !inUnchecked;
	if (generateUncheckedBlock)
		mutator->generator<BlockStmtGenerator>()->unchecked(true);

	ostringstream os;
	// Randomise visit order
	vector<std::pair<GeneratorPtr, unsigned>> randomisedChildren;
	for (auto const& child: generators)
		randomisedChildren.push_back(child);
	shuffle(randomisedChildren.begin(), randomisedChildren.end(), *uRandDist->randomEngine);
	for (auto const& child: randomisedChildren)
		if (uRandDist->likely(child.second + 1))
		{
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
		}
	return os.str();
}

void BlockStmtGenerator::setup()
{
	addGenerators({
		{mutator->generator<StatementGenerator>(), s_maxStatements},
	});
}

string BlockStmtGenerator::visit()
{
	if (nestingTooDeep())
		return "\n";
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
	state->currentFunctionState()->scopes.push_back(
		make_shared<BlockScope>()
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
	addGenerators({{mutator->generator<BlockStmtGenerator>(), 1}});
}

string FunctionGenerator::visit()
{
	string visibility;
	string name = state->newFunction();
	state->updateFunction(name, m_freeFunction);
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

optional<pair<SolidityTypePtr, string>> ExpressionGenerator::randomLValueExpression()
{
	LValueExpr exprType = static_cast<LValueExpr>(
		state->uRandDist->distributionOneToN(static_cast<size_t>(LValueExpr::TYPEMAX) - 1)
	);
	switch (exprType)
	{
	case LValueExpr::VARREF:
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
		if (liveVariables.empty())
			return nullopt;
		else
			return liveVariables[state->uRandDist->distributionOneToN(liveVariables.size()) - 1];
	}
	default:
		solAssert(false, "");
	}
}

optional<pair<SolidityTypePtr, string>> ExpressionGenerator::lValueExpression(
	pair<SolidityTypePtr, string> _typeName
)
{
	// Filter non-identical variables of the same type.
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
	if (liveTypedVariables.empty())
		return nullopt;
	else
		return liveTypedVariables[state->uRandDist->distributionOneToN(liveTypedVariables.size()) - 1];
}

pair<SolidityTypePtr, string> ExpressionGenerator::literal(SolidityTypePtr _type)
{
	string literalValue = visit(LiteralGenerator{state}, _type);
	return pair(_type, literalValue);
}

optional<pair<SolidityTypePtr, string>> ExpressionGenerator::expression(pair<SolidityTypePtr, string> _typeName)
{
	auto varRef = lValueExpression(_typeName);
	if (!varRef.has_value())
	{
		// TODO: Generate literals for contract and function types.
		if (!(holds_alternative<shared_ptr<FunctionType>>(_typeName.first) || holds_alternative<shared_ptr<ContractType>>(_typeName.first)))
			return literal(_typeName.first);
		else
			return nullopt;
	}
	else
		return varRef.value();
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
		return make_shared<FunctionType>(true);
	case Type::CONTRACT:
		if (state->sourceUnitState[state->currentPath()]->contractType())
			return state->sourceUnitState[state->currentPath()]->randomContractType();
		return make_shared<BoolType>();
	default:
		solAssert(false, "");
	}
}

string FunctionCallGenerator::lhs(vector<pair<SolidityTypePtr, string>> _functionReturnTypeNames)
{
	ExpressionGenerator exprGen{state};
	ostringstream callStmtLhs;

	auto assignToVars = _functionReturnTypeNames |
		ranges::views::transform([&exprGen](auto const& _item) -> pair<bool, optional<pair<SolidityTypePtr, string>>> {
			auto e = exprGen.lValueExpression(_item);
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

optional<string> FunctionCallGenerator::rhs(vector<pair<SolidityTypePtr, string>> _functionInputTypeNames)
{
	ExpressionGenerator exprGen{state};
	ostringstream callStmtRhs;

	auto inputArguments = _functionInputTypeNames |
		ranges::views::transform([&exprGen](auto const& _item) -> pair<bool, optional<pair<SolidityTypePtr, string>>>
		{
			auto e = exprGen.expression(_item);
			if (e.has_value())
				return {true, e.value()};
			else
				return {false, nullopt};
		});
	bool inputArgsValid = ranges::all_of(
		inputArguments,
		[](auto const& _item) -> bool { return _item.first; }
	);

	if (inputArgsValid)
	{
		auto vars = inputArguments |
			ranges::views::transform([](auto const& _item) { return _item.second.value().second; }) |
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
			if (uRandDist->probable(availableFunctions.size()))
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
