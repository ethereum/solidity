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

#include <libsmtutil/CHCSmtLib2Interface.h>

#include <libsmtutil/SMTLib2Parser.h>

#include <libsolutil/Keccak256.h>
#include <libsolutil/StringUtils.h>
#include <libsolutil/Visitor.h>

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/algorithm/find_if.hpp>
#include <range/v3/algorithm/sort.hpp>
#include <range/v3/view.hpp>

#include <array>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>

using namespace solidity;
using namespace solidity::util;
using namespace solidity::frontend;
using namespace solidity::smtutil;

CHCSmtLib2Interface::CHCSmtLib2Interface(
	std::map<h256, std::string> _queryResponses,
	ReadCallback::Callback _smtCallback,
	std::optional<unsigned> _queryTimeout
):
	CHCSolverInterface(_queryTimeout),
	m_queryResponses(std::move(_queryResponses)),
	m_smtCallback(std::move(_smtCallback))
{
	reset();
}

void CHCSmtLib2Interface::reset()
{
	m_unhandledQueries.clear();
	m_commands.clear();
	m_context.clear();
	createHeader();
	m_context.setTupleDeclarationCallback([&](TupleSort const& _tupleSort){
		m_commands.declareTuple(
			_tupleSort.name,
			_tupleSort.members,
			_tupleSort.components
				| ranges::views::transform([&](SortPointer const& _sort){ return m_context.toSmtLibSort(_sort); })
				| ranges::to<std::vector>()
		);
	});
}

void CHCSmtLib2Interface::registerRelation(Expression const& _expr)
{
	smtAssert(_expr.sort);
	smtAssert(_expr.sort->kind == Kind::Function);
	if (m_context.isDeclared(_expr.name))
		return;
	auto const& fSort = std::dynamic_pointer_cast<FunctionSort>(_expr.sort);
	smtAssert(fSort->codomain);
	auto domain = toSmtLibSort(fSort->domain);
	std::string codomain = toSmtLibSort(fSort->codomain);
	m_commands.declareFunction(_expr.name, domain, codomain);
	m_context.declare(_expr.name, _expr.sort);
}

void CHCSmtLib2Interface::addRule(Expression const& _expr, std::string const& /*_name*/)
{
	m_commands.assertion("(forall" + forall(_expr) + '\n' + m_context.toSExpr(_expr) + ")\n");
}

CHCSolverInterface::QueryResult CHCSmtLib2Interface::query(Expression const& _block)
{
	std::string query = dumpQuery(_block);
	std::string response = querySolver(query);

	CheckResult result;
	// NOTE: Our internal semantics is UNSAT -> SAFE and SAT -> UNSAFE, which corresponds to usual SMT-based model checking
	// However, with CHC solvers, the meaning is flipped, UNSAT -> UNSAFE and SAT -> SAFE.
	// So we have to flip the answer.
	if (boost::starts_with(response, "sat"))
	{
		auto maybeInvariants = invariantsFromSolverResponse(response);
		return {CheckResult::UNSATISFIABLE, maybeInvariants.value_or(Expression(true)), {}};
	}
	else if (boost::starts_with(response, "unsat"))
		result = CheckResult::SATISFIABLE;
	else if (boost::starts_with(response, "unknown"))
		result = CheckResult::UNKNOWN;
	else
		result = CheckResult::ERROR;

	return {result, Expression(true), {}};
}

void CHCSmtLib2Interface::declareVariable(std::string const& _name, SortPointer const& _sort)
{
	smtAssert(_sort);
	if (!m_context.isDeclared(_name))
		m_context.declare(_name, _sort);
}

std::string CHCSmtLib2Interface::toSmtLibSort(SortPointer const& _sort)
{
	return m_context.toSmtLibSort(_sort);
}

std::vector<std::string> CHCSmtLib2Interface::toSmtLibSort(std::vector<SortPointer> const& _sorts)
{
	return applyMap(_sorts, [this](auto const& sort) { return toSmtLibSort(sort); });
}

std::set<std::string> CHCSmtLib2Interface::collectVariableNames(Expression const& _expr) const
{
	std::set<std::string> names;
	auto dfs = [&](Expression const& _current, auto _recurse) -> void
	{
		if (_current.arguments.empty())
		{
			if (m_context.isDeclared(_current.name))
				names.insert(_current.name);
		}
		else
			for (auto const& arg: _current.arguments)
				_recurse(arg, _recurse);
	};
	dfs(_expr, dfs);
	return names;
}

std::string CHCSmtLib2Interface::forall(Expression const& _expr)
{
	auto varNames = collectVariableNames(_expr);
	std::string vars("(");
	for (auto const& name: varNames)
	{
		auto sort = m_context.getDeclaredSort(name);
		if (sort->kind != Kind::Function)
			vars += " (" + name + " " + toSmtLibSort(sort) + ")";
	}
	vars += ")";
	return vars;
}

std::string CHCSmtLib2Interface::querySolver(std::string const& _input)
{
	util::h256 inputHash = util::keccak256(_input);
	if (m_queryResponses.count(inputHash))
		return m_queryResponses.at(inputHash);

	if (m_smtCallback)
	{
		auto result = m_smtCallback(ReadCallback::kindString(ReadCallback::Kind::SMTQuery), _input);
		if (result.success)
			return result.responseOrErrorMessage;
	}

	m_unhandledQueries.push_back(_input);
	return "unknown\n";
}

std::string CHCSmtLib2Interface::dumpQuery(Expression const& _expr)
{
	return m_commands.toString() + createQueryAssertion(_expr.name) + '\n' + "(check-sat)" + '\n';
}

void CHCSmtLib2Interface::createHeader()
{
	if (m_queryTimeout)
		m_commands.setOption("timeout", std::to_string(*m_queryTimeout));
	m_commands.setLogic("HORN");
}

std::string CHCSmtLib2Interface::createQueryAssertion(std::string _name) {
	return "(assert\n(forall ((UNUSED Bool))\n(=> " + std::move(_name) + " false)))";
}

namespace
{
bool isNumber(std::string const& _expr)
{
	return ranges::all_of(_expr, [](char c) { return isDigit(c) || c == '.'; });
}

bool isBitVectorHexConstant(std::string const& _string)
{
	if (_string.substr(0, 2) != "#x")
		return false;
	if (_string.find_first_not_of("0123456789abcdefABCDEF", 2) != std::string::npos)
		return false;
	return true;
}

bool isBitVectorConstant(std::string const& _string)
{
	if (_string.substr(0, 2) != "#b")
		return false;
	if (_string.find_first_not_of("01", 2) != std::string::npos)
		return false;
	return true;
}
}

void CHCSmtLib2Interface::ScopedParser::addVariableDeclaration(std::string _name, solidity::smtutil::SortPointer _sort)
{
	m_localVariables.emplace(std::move(_name), std::move(_sort));
}

std::optional<SortPointer> CHCSmtLib2Interface::ScopedParser::lookupKnownTupleSort(std::string const& _name) const
{
	return m_context.getTupleType(_name);
}

SortPointer CHCSmtLib2Interface::ScopedParser::toSort(SMTLib2Expression const& _expr)
{
	if (isAtom(_expr))
	{
		auto const& name = asAtom(_expr);
		if (name == "Int")
			return SortProvider::sintSort;
		if (name == "Bool")
			return SortProvider::boolSort;
		auto tupleSort = lookupKnownTupleSort(name);
		if (tupleSort)
			return tupleSort.value();
	}
	else
	{
		auto const& args = asSubExpressions(_expr);
		if (asAtom(args[0]) == "Array")
		{
			smtAssert(args.size() == 3);
			auto domainSort = toSort(args[1]);
			auto codomainSort = toSort(args[2]);
			return std::make_shared<ArraySort>(std::move(domainSort), std::move(codomainSort));
		}
		if (args.size() == 3 && isAtom(args[0]) && asAtom(args[0]) == "_" && isAtom(args[1])
			&& asAtom(args[1]) == "int2bv")
			return std::make_shared<BitVectorSort>(std::stoul(asAtom(args[2])));
	}
	smtAssert(false, "Unknown sort encountered");
}

smtutil::Expression CHCSmtLib2Interface::ScopedParser::parseQuantifier(
	std::string const& _quantifierName,
	std::vector<SMTLib2Expression> const& _varList,
	SMTLib2Expression const& _coreExpression)
{
	std::vector<std::pair<std::string, SortPointer>> boundVariables;
	for (auto const& sortedVar: _varList)
	{
		smtAssert(!isAtom(sortedVar));
		auto varSortPair = asSubExpressions(sortedVar);
		smtAssert(varSortPair.size() == 2);
		boundVariables.emplace_back(asAtom(varSortPair[0]), toSort(varSortPair[1]));
	}
	for (auto const& [var, sort]: boundVariables)
	{
		smtAssert(m_localVariables.count(var) == 0); // TODO: deal with shadowing?
		m_localVariables.emplace(var, sort);
	}
	auto core = toSMTUtilExpression(_coreExpression);
	for (auto const& [var, sort]: boundVariables)
	{
		smtAssert(m_localVariables.count(var) != 0);
		m_localVariables.erase(var);
	}
	return Expression(_quantifierName, {core}, SortProvider::boolSort); // TODO: what about the bound variables?
}

smtutil::Expression CHCSmtLib2Interface::ScopedParser::toSMTUtilExpression(SMTLib2Expression const& _expr)
{
	return std::visit(
		GenericVisitor{
			[&](std::string const& _atom)
			{
				if (_atom == "true" || _atom == "false")
					return smtutil::Expression(_atom == "true");
				else if (isNumber(_atom))
					return smtutil::Expression(_atom, {}, SortProvider::sintSort);
				else if (isBitVectorHexConstant(_atom))
					return smtutil::Expression(_atom, {}, std::make_shared<BitVectorSort>((_atom.size() - 2) * 4));
				else if (isBitVectorConstant(_atom))
					return smtutil::Expression(_atom, {}, std::make_shared<BitVectorSort>(_atom.size() - 2));
				else if (auto it = m_localVariables.find(_atom); it != m_localVariables.end())
					return smtutil::Expression(_atom, {}, it->second);
				else if (m_context.isDeclared(_atom))
					return smtutil::Expression(_atom, {}, m_context.getDeclaredSort(_atom));
				else if (auto maybeTupleType = m_context.getTupleType(_atom); maybeTupleType.has_value())
				{
					// 0-ary tuple type, can happen
					return smtutil::Expression(_atom, {}, std::make_shared<TupleSort>(_atom, std::vector<std::string>{}, std::vector<SortPointer>{}));
				}
				else
					smtAssert(false, "Unhandled atomic SMT expression");
			},
			[&](std::vector<SMTLib2Expression> const& _subExpr)
			{
				SortPointer sort;
				std::vector<smtutil::Expression> arguments;
				if (isAtom(_subExpr.front()))
				{
					std::string const& op = asAtom(_subExpr.front());
					if (op == "!")
					{
						// named term, we ignore the name
						smtAssert(_subExpr.size() > 2);
						return toSMTUtilExpression(_subExpr[1]);
					}
					if (op == "exists" || op == "forall")
					{
						smtAssert(_subExpr.size() == 3);
						smtAssert(!isAtom(_subExpr[1]));
						return parseQuantifier(op, asSubExpressions(_subExpr[1]), _subExpr[2]);
					}
					for (size_t i = 1; i < _subExpr.size(); i++)
						arguments.emplace_back(toSMTUtilExpression(_subExpr[i]));
					if (auto tupleSort = lookupKnownTupleSort(op); tupleSort)
					{
						auto sortSort = std::make_shared<SortSort>(tupleSort.value());
						return Expression::tuple_constructor(Expression(sortSort), arguments);
					}
					if (m_context.isDeclared(op))
						return smtutil::Expression(op, std::move(arguments), m_context.getDeclaredSort(op));
					if (auto maybeTupleAccessor = m_context.getTupleAccessor(op); maybeTupleAccessor.has_value())
					{
						auto accessor = maybeTupleAccessor.value();
						return smtutil::Expression("dt_accessor_" + accessor.first, std::move(arguments), accessor.second);
					}
					else
					{
						std::set<std::string> boolOperators{"and", "or", "not", "=", "<", ">", "<=", ">=", "=>"};
						sort = contains(boolOperators, op) ? SortProvider::boolSort : arguments.back().sort;
						return smtutil::Expression(op, std::move(arguments), std::move(sort));
					}
					smtAssert(false, "Unhandled case in expression conversion");
				}
				else
				{
					// check for const array
					if (_subExpr.size() == 2 and !isAtom(_subExpr[0]))
					{
						auto const& typeArgs = asSubExpressions(_subExpr.front());
						if (typeArgs.size() == 3 && typeArgs[0].toString() == "as"
							&& typeArgs[1].toString() == "const")
						{
							auto arraySort = toSort(typeArgs[2]);
							auto sortSort = std::make_shared<SortSort>(arraySort);
							return smtutil::Expression::
								const_array(Expression(sortSort), toSMTUtilExpression(_subExpr[1]));
						}
						if (typeArgs.size() == 3 && typeArgs[0].toString() == "_"
							&& typeArgs[1].toString() == "int2bv")
						{
							auto bvSort = std::dynamic_pointer_cast<BitVectorSort>(toSort(_subExpr[0]));
							smtAssert(bvSort);
							return smtutil::Expression::int2bv(toSMTUtilExpression(_subExpr[1]), bvSort->size);
						}
						if (typeArgs.size() == 4 && typeArgs[0].toString() == "_"
							&& typeArgs[1].toString() == "extract")
							return smtutil::Expression(
								"extract",
								{toSMTUtilExpression(typeArgs[2]), toSMTUtilExpression(typeArgs[3])},
								SortProvider::bitVectorSort // TODO: Compute bit size properly?
							);
					}
					smtAssert(false, "Unhandled case in expression conversion");
				}
			}
		},
		_expr.data
	);
}


#define precondition(CONDITION) if (!(CONDITION)) return {}
std::optional<smtutil::Expression> CHCSmtLib2Interface::invariantsFromSolverResponse(std::string const& _response) const
{
	std::stringstream ss(_response);
	std::string answer;
	ss >> answer;
	precondition(answer == "sat");
	SMTLib2Parser parser(ss);
	precondition(!parser.isEOF()); // There has to be a model
	std::vector<SMTLib2Expression> parsedOutput;
	try
	{
		while (!parser.isEOF())
			parsedOutput.push_back(parser.parseExpression());
	}
	catch(SMTLib2Parser::ParsingException&)
	{
		return {};
	}
	smtAssert(parser.isEOF());
	precondition(!parsedOutput.empty());
	auto& commands = parsedOutput.size() == 1 ? asSubExpressions(parsedOutput[0]) : parsedOutput;
	std::vector<Expression> definitions;
	for (auto& command: commands)
	{
		auto& args = asSubExpressions(command);
		precondition(args.size() == 5);
		// args[0] = "define-fun"
		// args[1] = predicate name
		// args[2] = formal arguments of the predicate
		// args[3] = return sort
		// args[4] = body of the predicate's interpretation
		precondition(isAtom(args[0]) && asAtom(args[0]) == "define-fun");
		precondition(isAtom(args[1]));
		precondition(!isAtom(args[2]));
		precondition(isAtom(args[3]) && asAtom(args[3]) == "Bool");
		auto& interpretation = args[4];
		inlineLetExpressions(interpretation);
		ScopedParser scopedParser(m_context);
		auto const& formalArguments = asSubExpressions(args[2]);
		std::vector<Expression> predicateArgs;
		for (auto const& formalArgument: formalArguments)
		{
			precondition(!isAtom(formalArgument));
			auto const& nameSortPair = asSubExpressions(formalArgument);
			precondition(nameSortPair.size() == 2);
			precondition(isAtom(nameSortPair[0]));
			SortPointer varSort = scopedParser.toSort(nameSortPair[1]);
			scopedParser.addVariableDeclaration(asAtom(nameSortPair[0]), varSort);
			// FIXME: Why Expression here?
			Expression arg = scopedParser.toSMTUtilExpression(nameSortPair[0]);
			predicateArgs.push_back(arg);
		}

		auto parsedInterpretation = scopedParser.toSMTUtilExpression(interpretation);

		// Hack to make invariants more stable across operating systems
		if (parsedInterpretation.name == "and" || parsedInterpretation.name == "or")
			ranges::sort(parsedInterpretation.arguments, [](Expression const& first, Expression const& second) {
				return first.name < second.name;
			});

		Expression predicate(asAtom(args[1]), predicateArgs, SortProvider::boolSort);
		definitions.push_back(predicate == parsedInterpretation);
	}
	return Expression::mkAnd(std::move(definitions));
}
#undef precondition

namespace
{

struct LetBindings
{
	using BindingRecord = std::vector<SMTLib2Expression>;
	std::unordered_map<std::string, BindingRecord> bindings;
	std::vector<std::string> varNames;
	std::vector<std::size_t> scopeBounds;

	bool has(std::string const& varName) { return bindings.find(varName) != bindings.end(); }

	SMTLib2Expression& operator[](std::string const& varName)
	{
		auto it = bindings.find(varName);
		smtAssert(it != bindings.end());
		smtAssert(!it->second.empty());
		return it->second.back();
	}

	void pushScope() { scopeBounds.push_back(varNames.size()); }

	void popScope()
	{
		smtAssert(!scopeBounds.empty());
		auto bound = scopeBounds.back();
		while (varNames.size() > bound)
		{
			auto const& varName = varNames.back();
			auto it = bindings.find(varName);
			smtAssert(it != bindings.end());
			auto& record = it->second;
			record.pop_back();
			if (record.empty())
				bindings.erase(it);
			varNames.pop_back();
		}
		scopeBounds.pop_back();
	}

	void addBinding(std::string name, SMTLib2Expression expression)
	{
		auto it = bindings.find(name);
		if (it == bindings.end())
			bindings.insert({name, {std::move(expression)}});
		else
			it->second.push_back(std::move(expression));
		varNames.push_back(std::move(name));
	}
};

void inlineLetExpressions(SMTLib2Expression& _expr, LetBindings& _bindings)
{
	if (isAtom(_expr))
	{
		auto const& atom = asAtom(_expr);
		if (_bindings.has(atom))
			_expr = _bindings[atom];
		return;
	}
	auto& subexprs = asSubExpressions(_expr);
	smtAssert(!subexprs.empty());
	auto const& first = subexprs.at(0);
	if (isAtom(first) && asAtom(first) == "let")
	{
		smtAssert(subexprs.size() == 3);
		smtAssert(!isAtom(subexprs[1]));
		auto& bindingExpressions = asSubExpressions(subexprs[1]);
		// process new bindings
		std::vector<std::pair<std::string, SMTLib2Expression>> newBindings;
		for (auto& binding: bindingExpressions)
		{
			smtAssert(!isAtom(binding));
			auto& bindingPair = asSubExpressions(binding);
			smtAssert(bindingPair.size() == 2);
			smtAssert(isAtom(bindingPair.at(0)));
			inlineLetExpressions(bindingPair.at(1), _bindings);
			newBindings.emplace_back(asAtom(bindingPair.at(0)), bindingPair.at(1));
		}
		_bindings.pushScope();
		for (auto&& [name, expr]: newBindings)
			_bindings.addBinding(std::move(name), std::move(expr));

		newBindings.clear();

		// get new subexpression
		inlineLetExpressions(subexprs.at(2), _bindings);
		// remove the new bindings
		_bindings.popScope();

		// update the expression
		auto tmp = std::move(subexprs.at(2));
		_expr = std::move(tmp);
		return;
	}
	else if (isAtom(first) && (asAtom(first) == "forall" || asAtom(first) == "exists"))
	{
		// A little hack to ensure quantified variables are not substituted because of some outer let definition:
		// We define the current binding of the variable to itself, before we recurse in to subterm
		smtAssert(subexprs.size() == 3);
		_bindings.pushScope();
		for (auto const& sortedVar: asSubExpressions(subexprs.at(1)))
		{
			auto const& varNameExpr = asSubExpressions(sortedVar).at(0);
			_bindings.addBinding(asAtom(varNameExpr), varNameExpr);
		}
		inlineLetExpressions(subexprs.at(2), _bindings);
		_bindings.popScope();
		return;
	}

	// not a let expression, just process all arguments recursively
	for (auto& subexpr: subexprs)
		inlineLetExpressions(subexpr, _bindings);
}
}

void CHCSmtLib2Interface::inlineLetExpressions(SMTLib2Expression& expr)
{
	LetBindings bindings;
	::inlineLetExpressions(expr, bindings);
}
