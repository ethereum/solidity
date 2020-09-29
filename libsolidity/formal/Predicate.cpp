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
	return functor().name.rfind("summary", 0) == 0;
}

bool Predicate::isInterface() const
{
	return functor().name.rfind("interface", 0) == 0;
}

string Predicate::formatSummaryCall(vector<string> const& _args) const
{
	if (programContract())
		return "constructor()";

	solAssert(isSummary(), "");

	auto stateVars = stateVariables();
	solAssert(stateVars.has_value(), "");
	auto const* fun = programFunction();
	solAssert(fun, "");

	/// The signature of a function summary predicate is: summary(error, preStateVars, preInputVars, postStateVars, postInputVars, outputVars).
	/// Here we are interested in preInputVars.
	vector<string>::const_iterator first = _args.begin() + static_cast<int>(stateVars->size()) + 1;
	vector<string>::const_iterator last = first + static_cast<int>(fun->parameters().size());
	solAssert(first >= _args.begin() && first <= _args.end(), "");
	solAssert(last >= _args.begin() && last <= _args.end(), "");
	vector<string> functionArgsCex(first, last);
	vector<string> functionArgs;

	auto const& params = fun->parameters();
	solAssert(params.size() == functionArgsCex.size(), "");
	for (unsigned i = 0; i < params.size(); ++i)
		if (params[i]->type()->isValueType())
			functionArgs.emplace_back(functionArgsCex[i]);
		else
			functionArgs.emplace_back(params[i]->name());

	string fName = fun->isConstructor() ? "constructor" :
		fun->isFallback() ? "fallback" :
		fun->isReceive() ? "receive" :
		fun->name();
	return fName + "(" + boost::algorithm::join(functionArgs, ", ") + ")";

}

vector<string> Predicate::summaryStateValues(vector<string> const& _args) const
{
	/// The signature of a function summary predicate is: summary(error, preStateVars, preInputVars, postStateVars, postInputVars, outputVars).
	/// The signature of an implicit constructor summary predicate is: summary(error, postStateVars).
	/// Here we are interested in postStateVars.

	auto stateVars = stateVariables();
	solAssert(stateVars.has_value(), "");

	vector<string>::const_iterator stateFirst;
	vector<string>::const_iterator stateLast;
	if (auto const* function = programFunction())
	{
		stateFirst = _args.begin() + 1 + static_cast<int>(stateVars->size()) + static_cast<int>(function->parameters().size());
		stateLast = stateFirst + static_cast<int>(stateVars->size());
	}
	else if (programContract())
	{
		stateFirst = _args.begin() + 1;
		stateLast = stateFirst + static_cast<int>(stateVars->size());
	}
	else
		solAssert(false, "");

	solAssert(stateFirst >= _args.begin() && stateFirst <= _args.end(), "");
	solAssert(stateLast >= _args.begin() && stateLast <= _args.end(), "");

	vector<string> stateArgs(stateFirst, stateLast);
	solAssert(stateArgs.size() == stateVars->size(), "");
	return stateArgs;
}

vector<string> Predicate::summaryPostInputValues(vector<string> const& _args) const
{
	/// The signature of a function summary predicate is: summary(error, preStateVars, preInputVars, postStateVars, postInputVars, outputVars).
	/// Here we are interested in postInputVars.
	auto const* function = programFunction();
	solAssert(function, "");

	auto stateVars = stateVariables();
	solAssert(stateVars.has_value(), "");

	auto const& inParams = function->parameters();

	vector<string>::const_iterator first = _args.begin() + 1 + static_cast<int>(stateVars->size()) * 2 + static_cast<int>(inParams.size());
	vector<string>::const_iterator last = first + static_cast<int>(inParams.size());

	solAssert(first >= _args.begin() && first <= _args.end(), "");
	solAssert(last >= _args.begin() && last <= _args.end(), "");

	vector<string> inValues(first, last);
	solAssert(inValues.size() == inParams.size(), "");
	return inValues;
}

vector<string> Predicate::summaryPostOutputValues(vector<string> const& _args) const
{
	/// The signature of a function summary predicate is: summary(error, preStateVars, preInputVars, postStateVars, postInputVars, outputVars).
	/// Here we are interested in outputVars.
	auto const* function = programFunction();
	solAssert(function, "");

	auto stateVars = stateVariables();
	solAssert(stateVars.has_value(), "");

	auto const& inParams = function->parameters();

	vector<string>::const_iterator first = _args.begin() + 1 + static_cast<int>(stateVars->size()) * 2 + static_cast<int>(inParams.size()) * 2;

	solAssert(first >= _args.begin() && first <= _args.end(), "");

	vector<string> outValues(first, _args.end());
	solAssert(outValues.size() == function->returnParameters().size(), "");
	return outValues;
}
