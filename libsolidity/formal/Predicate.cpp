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

#include <libsolidity/formal/Predicate.h>

#include <libsolidity/formal/SMTEncoder.h>

#include <libsolidity/ast/AST.h>

#include <boost/algorithm/string/join.hpp>
#include <utility>

using namespace std;
using namespace solidity;
using namespace solidity::smtutil;
using namespace solidity::frontend;
using namespace solidity::frontend::smt;

map<string, Predicate> Predicate::m_predicates;

Predicate const* Predicate::create(
	SortPointer _sort,
	string _name,
	PredicateType _type,
	EncodingContext& _context,
	ASTNode const* _node
)
{
	smt::SymbolicFunctionVariable predicate{_sort, move(_name), _context};
	string functorName = predicate.currentName();
	solAssert(!m_predicates.count(functorName), "");
	return &m_predicates.emplace(
		std::piecewise_construct,
		std::forward_as_tuple(functorName),
		std::forward_as_tuple(move(predicate), _type, _node)
	).first->second;
}

Predicate::Predicate(
	smt::SymbolicFunctionVariable&& _predicate,
	PredicateType _type,
	ASTNode const* _node
):
	m_predicate(move(_predicate)),
	m_type(_type),
	m_node(_node)
{
}

Predicate const* Predicate::predicate(string const& _name)
{
	return &m_predicates.at(_name);
}

void Predicate::reset()
{
	m_predicates.clear();
}

smtutil::Expression Predicate::operator()(vector<smtutil::Expression> const& _args) const
{
	return m_predicate(_args);
}

smtutil::Expression Predicate::functor() const
{
	return m_predicate.currentFunctionValue();
}

smtutil::Expression Predicate::functor(unsigned _idx) const
{
	return m_predicate.functionValueAtIndex(_idx);
}

void Predicate::newFunctor()
{
	m_predicate.increaseIndex();
}

ASTNode const* Predicate::programNode() const
{
	return m_node;
}

ContractDefinition const* Predicate::programContract() const
{
	if (auto const* contract = dynamic_cast<ContractDefinition const*>(m_node))
		if (!contract->constructor())
			return contract;

	return nullptr;
}

FunctionDefinition const* Predicate::programFunction() const
{
	if (auto const* contract = dynamic_cast<ContractDefinition const*>(m_node))
	{
		if (contract->constructor())
			return contract->constructor();
		return nullptr;
	}

	if (auto const* fun = dynamic_cast<FunctionDefinition const*>(m_node))
		return fun;

	return nullptr;
}

optional<vector<VariableDeclaration const*>> Predicate::stateVariables() const
{
	if (auto const* fun = programFunction())
		return SMTEncoder::stateVariablesIncludingInheritedAndPrivate(*fun);
	if (auto const* contract = programContract())
		return SMTEncoder::stateVariablesIncludingInheritedAndPrivate(*contract);

	auto const* node = m_node;
	while (auto const* scopable = dynamic_cast<Scopable const*>(node))
	{
		node = scopable->scope();
		if (auto const* fun = dynamic_cast<FunctionDefinition const*>(node))
			return SMTEncoder::stateVariablesIncludingInheritedAndPrivate(*fun);
	}

	return nullopt;
}

bool Predicate::isSummary() const
{
	return m_type == PredicateType::ConstructorSummary || m_type == PredicateType::FunctionSummary;
}

bool Predicate::isInterface() const
{
	return m_type == PredicateType::Interface;
}

string Predicate::formatSummaryCall(vector<smtutil::Expression> const& _args) const
{
	if (programContract())
		return "constructor()";

	solAssert(isSummary(), "");

	auto stateVars = stateVariables();
	solAssert(stateVars.has_value(), "");
	auto const* fun = programFunction();
	solAssert(fun, "");

	/// The signature of a function summary predicate is: summary(error, this, cryptoFunctions, txData, preBlockChainState, preStateVars, preInputVars, postBlockchainState, postStateVars, postInputVars, outputVars).
	/// Here we are interested in preInputVars.
	auto first = _args.begin() + 5 + static_cast<int>(stateVars->size());
	auto last = first + static_cast<int>(fun->parameters().size());
	solAssert(first >= _args.begin() && first <= _args.end(), "");
	solAssert(last >= _args.begin() && last <= _args.end(), "");
	auto inTypes = FunctionType(*fun).parameterTypes();
	vector<optional<string>> functionArgsCex = formatExpressions(vector<smtutil::Expression>(first, last), inTypes);
	vector<string> functionArgs;

	auto const& params = fun->parameters();
	solAssert(params.size() == functionArgsCex.size(), "");
	for (unsigned i = 0; i < params.size(); ++i)
		if (params.at(i) && functionArgsCex.at(i))
			functionArgs.emplace_back(*functionArgsCex.at(i));
		else
			functionArgs.emplace_back(params[i]->name());

	string fName = fun->isConstructor() ? "constructor" :
		fun->isFallback() ? "fallback" :
		fun->isReceive() ? "receive" :
		fun->name();
	return fName + "(" + boost::algorithm::join(functionArgs, ", ") + ")";

}

vector<optional<string>> Predicate::summaryStateValues(vector<smtutil::Expression> const& _args) const
{
	/// The signature of a function summary predicate is: summary(error, this, cryptoFunctions, txData, preBlockchainState, preStateVars, preInputVars, postBlockchainState, postStateVars, postInputVars, outputVars).
	/// The signature of the summary predicate of a contract without constructor is: summary(error, this, cryptoFunctions, txData, preBlockchainState, postBlockchainState, preStateVars, postStateVars).
	/// Here we are interested in postStateVars.
	auto stateVars = stateVariables();
	solAssert(stateVars.has_value(), "");

	vector<smtutil::Expression>::const_iterator stateFirst;
	vector<smtutil::Expression>::const_iterator stateLast;
	if (auto const* function = programFunction())
	{
		stateFirst = _args.begin() + 5 + static_cast<int>(stateVars->size()) + static_cast<int>(function->parameters().size()) + 1;
		stateLast = stateFirst + static_cast<int>(stateVars->size());
	}
	else if (programContract())
	{
		stateFirst = _args.begin() + 6 + static_cast<int>(stateVars->size());
		stateLast = stateFirst + static_cast<int>(stateVars->size());
	}
	else
		solAssert(false, "");

	solAssert(stateFirst >= _args.begin() && stateFirst <= _args.end(), "");
	solAssert(stateLast >= _args.begin() && stateLast <= _args.end(), "");

	vector<smtutil::Expression> stateArgs(stateFirst, stateLast);
	solAssert(stateArgs.size() == stateVars->size(), "");
	auto stateTypes = applyMap(*stateVars, [&](auto const& _var) { return _var->type(); });
	return formatExpressions(stateArgs, stateTypes);
}

vector<optional<string>> Predicate::summaryPostInputValues(vector<smtutil::Expression> const& _args) const
{
	/// The signature of a function summary predicate is: summary(error, this, cryptoFunctions, txData, preBlockchainState, preStateVars, preInputVars, postBlockchainState, postStateVars, postInputVars, outputVars).
	/// Here we are interested in postInputVars.
	auto const* function = programFunction();
	solAssert(function, "");

	auto stateVars = stateVariables();
	solAssert(stateVars.has_value(), "");

	auto const& inParams = function->parameters();

	auto first = _args.begin() + 5 + static_cast<int>(stateVars->size()) * 2 + static_cast<int>(inParams.size()) + 1;
	auto last = first + static_cast<int>(inParams.size());

	solAssert(first >= _args.begin() && first <= _args.end(), "");
	solAssert(last >= _args.begin() && last <= _args.end(), "");

	vector<smtutil::Expression> inValues(first, last);
	solAssert(inValues.size() == inParams.size(), "");
	auto inTypes = FunctionType(*function).parameterTypes();
	return formatExpressions(inValues, inTypes);
}

vector<optional<string>> Predicate::summaryPostOutputValues(vector<smtutil::Expression> const& _args) const
{
	/// The signature of a function summary predicate is: summary(error, this, cryptoFunctions, txData, preBlockchainState, preStateVars, preInputVars, postBlockchainState, postStateVars, postInputVars, outputVars).
	/// Here we are interested in outputVars.
	auto const* function = programFunction();
	solAssert(function, "");

	auto stateVars = stateVariables();
	solAssert(stateVars.has_value(), "");

	auto const& inParams = function->parameters();

	auto first = _args.begin() + 5 + static_cast<int>(stateVars->size()) * 2 + static_cast<int>(inParams.size()) * 2 + 1;

	solAssert(first >= _args.begin() && first <= _args.end(), "");

	vector<smtutil::Expression> outValues(first, _args.end());
	solAssert(outValues.size() == function->returnParameters().size(), "");
	auto outTypes = FunctionType(*function).returnParameterTypes();
	return formatExpressions(outValues, outTypes);
}

vector<optional<string>> Predicate::formatExpressions(vector<smtutil::Expression> const& _exprs, vector<TypePointer> const& _types) const
{
	solAssert(_exprs.size() == _types.size(), "");
	vector<optional<string>> strExprs;
	for (unsigned i = 0; i < _exprs.size(); ++i)
		strExprs.push_back(expressionToString(_exprs.at(i), _types.at(i)));
	return strExprs;
}

optional<string> Predicate::expressionToString(smtutil::Expression const& _expr, TypePointer _type) const
{
	if (smt::isNumber(*_type))
	{
		solAssert(_expr.sort->kind == Kind::Int, "");
		solAssert(_expr.arguments.empty(), "");
		// TODO assert that _expr.name is a number.
		return _expr.name;
	}
	if (smt::isBool(*_type))
	{
		solAssert(_expr.sort->kind == Kind::Bool, "");
		solAssert(_expr.arguments.empty(), "");
		solAssert(_expr.name == "true" || _expr.name == "false", "");
		return _expr.name;
	}
	if (smt::isFunction(*_type))
	{
		solAssert(_expr.arguments.empty(), "");
		return _expr.name;
	}
	if (smt::isArray(*_type))
	{
		auto const& arrayType = dynamic_cast<ArrayType const&>(*_type);
		solAssert(_expr.name == "tuple_constructor", "");
		auto const& tupleSort = dynamic_cast<TupleSort const&>(*_expr.sort);
		solAssert(tupleSort.components.size() == 2, "");

		unsigned long length;
		try
		{
			length = stoul(_expr.arguments.at(1).name);
		}
		catch(out_of_range const&)
		{
			return {};
		}
		// Limit this counterexample size to 1k.
		// Some OSs give you "unlimited" memory through swap and other virtual memory,
		// so purely relying on bad_alloc being thrown is not a good idea.
		// In that case, the array allocation might cause OOM and the program is killed.
		if (length >= 1024)
			return {};
		try
		{
			vector<string> array(length);
			if (!fillArray(_expr.arguments.at(0), array, arrayType))
				return {};
			return "[" + boost::algorithm::join(array, ", ") + "]";
		}
		catch (bad_alloc const&)
		{
			// Solver gave a concrete array but length is too large.
		}
	}
	if (smt::isNonRecursiveStruct(*_type))
	{
		auto const& structType = dynamic_cast<StructType const&>(*_type);
		solAssert(_expr.name == "tuple_constructor", "");
		auto const& tupleSort = dynamic_cast<TupleSort const&>(*_expr.sort);
		auto members = structType.structDefinition().members();
		solAssert(tupleSort.components.size() == members.size(), "");
		solAssert(_expr.arguments.size() == members.size(), "");
		vector<string> elements;
		for (unsigned i = 0; i < members.size(); ++i)
		{
			optional<string> elementStr = expressionToString(_expr.arguments.at(i), members[i]->type());
			elements.push_back(members[i]->name() + (elementStr.has_value() ?  ": " + elementStr.value() : ""));
		}
		return "{" + boost::algorithm::join(elements, ", ") + "}";
	}

	return {};
}

bool Predicate::fillArray(smtutil::Expression const& _expr, vector<string>& _array, ArrayType const& _type) const
{
	// Base case
	if (_expr.name == "const_array")
	{
		auto length = _array.size();
		optional<string> elemStr = expressionToString(_expr.arguments.at(1), _type.baseType());
		if (!elemStr)
			return false;
		_array.clear();
		_array.resize(length, *elemStr);
		return true;
	}

	// Recursive case.
	if (_expr.name == "store")
	{
		if (!fillArray(_expr.arguments.at(0), _array, _type))
			return false;
		optional<string> indexStr = expressionToString(_expr.arguments.at(1), TypeProvider::uint256());
		if (!indexStr)
			return false;
		// Sometimes the solver assigns huge lengths that are not related,
		// we should catch and ignore those.
		unsigned long index;
		try
		{
			index = stoul(*indexStr);
		}
		catch (out_of_range const&)
		{
			return true;
		}
		optional<string> elemStr = expressionToString(_expr.arguments.at(2), _type.baseType());
		if (!elemStr)
			return false;
		if (index < _array.size())
			_array.at(index) = *elemStr;
		return true;
	}

	// Special base case, not supported yet.
	if (_expr.name.rfind("(_ as-array") == 0)
	{
		// Z3 expression representing reinterpretation of a different term as an array
		return false;
	}

	solAssert(false, "");
}
