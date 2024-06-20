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

#include <range/v3/algorithm/find_if.hpp>
#include <range/v3/algorithm/all_of.hpp>
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
	m_smtlib2(std::make_unique<SMTLib2Interface>(_queryResponses, _smtCallback, m_queryTimeout)),
	m_queryResponses(std::move(_queryResponses)),
	m_smtCallback(_smtCallback)
{
	reset();
}

void CHCSmtLib2Interface::reset()
{
	m_accumulatedOutput.clear();
	m_variables.clear();
	m_unhandledQueries.clear();
	m_smtlib2->reset();
}

void CHCSmtLib2Interface::registerRelation(Expression const& _expr)
{
	smtAssert(_expr.sort);
	smtAssert(_expr.sort->kind == Kind::Function);
	if (!m_variables.count(_expr.name))
	{
		auto fSort = std::dynamic_pointer_cast<FunctionSort>(_expr.sort);
		std::string domain = toSmtLibSort(fSort->domain);
		// Relations are predicates which have implicit codomain Bool.
		m_variables.insert(_expr.name);
		write(
			"(declare-fun |" +
			_expr.name +
			"| " +
			domain +
			" Bool)"
		);
	}
}

void CHCSmtLib2Interface::addRule(Expression const& _expr, std::string const& /*_name*/)
{
	write(
		"(assert\n(forall " + forall() + "\n" +
		m_smtlib2->toSExpr(_expr) +
		"))\n\n"
	);
}

std::tuple<CheckResult, Expression, CHCSolverInterface::CexGraph> CHCSmtLib2Interface::query(Expression const& _block)
{
	std::string query = dumpQuery(_block);
	std::string response = querySolver(query);

	CheckResult result;
	// TODO proper parsing
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
	if (_sort->kind == Kind::Function)
		declareFunction(_name, _sort);
	else if (!m_variables.count(_name))
	{
		m_variables.insert(_name);
		write("(declare-var |" + _name + "| " + toSmtLibSort(_sort) + ')');
	}
}

std::string CHCSmtLib2Interface::toSmtLibSort(SortPointer _sort)
{
	return m_smtlib2->toSmtLibSort(_sort);
}

std::string CHCSmtLib2Interface::toSmtLibSort(std::vector<SortPointer> const& _sorts)
{
	return m_smtlib2->toSmtLibSort(_sorts);
}

std::string CHCSmtLib2Interface::forall()
{
	std::string vars("(");
	for (auto const& [name, sort]: m_smtlib2->variables())
	{
		solAssert(sort, "");
		if (sort->kind != Kind::Function)
			vars += " (" + name + " " + toSmtLibSort(sort) + ")";
	}
	vars += ")";
	return vars;
}

void CHCSmtLib2Interface::declareFunction(std::string const& _name, SortPointer const& _sort)
{
	smtAssert(_sort);
	smtAssert(_sort->kind == Kind::Function);
	// TODO Use domain and codomain as key as well
	if (!m_variables.count(_name))
	{
		auto fSort = std::dynamic_pointer_cast<FunctionSort>(_sort);
		smtAssert(fSort->codomain);
		std::string domain = toSmtLibSort(fSort->domain);
		std::string codomain = toSmtLibSort(fSort->codomain);
		m_variables.insert(_name);
		write(
			"(declare-fun |" +
			_name +
			"| " +
			domain +
			" " +
			codomain +
			")"
		);
	}
}

void CHCSmtLib2Interface::write(std::string _data)
{
	m_accumulatedOutput += std::move(_data) + "\n";
}

std::string CHCSmtLib2Interface::querySolver(std::string const& _input)
{
	util::h256 inputHash = util::keccak256(_input);
	if (m_queryResponses.count(inputHash))
		return m_queryResponses.at(inputHash);

	if (m_smtCallback)
	{
		setupSmtCallback();
		auto result = m_smtCallback(ReadCallback::kindString(ReadCallback::Kind::SMTQuery), _input);
		if (result.success)
			return result.responseOrErrorMessage;
	}

	m_unhandledQueries.push_back(_input);
	return "unknown\n";
}

std::string CHCSmtLib2Interface::dumpQuery(Expression const& _expr)
{
	std::stringstream s;

	s
		<< createHeaderAndDeclarations()
		<< m_accumulatedOutput << std::endl
		<< createQueryAssertion(_expr.name) << std::endl
		<< "(check-sat)" << std::endl;

	return s.str();
}

std::string CHCSmtLib2Interface::createHeaderAndDeclarations() {
	std::stringstream s;
	if (m_queryTimeout)
		s << "(set-option :timeout " + std::to_string(*m_queryTimeout) + ")\n";
	s << "(set-logic HORN)" << std::endl;

	for (auto const& decl: m_smtlib2->userSorts() | ranges::views::values)
		s << decl << std::endl;

	return s.str();
}

std::string CHCSmtLib2Interface::createQueryAssertion(std::string name) {
	return "(assert\n(forall " + forall() + "\n" + "(=> " + name + " false)))";
}

class SMTLibTranslationContext
{
	CHCSmtLib2Interface const& m_chcInterface;
	std::map<std::string, SortPointer> m_knownVariables;

	static bool isNumber(std::string const& _expr)
	{
		return ranges::all_of(_expr, [](char c) { return isDigit(c) || c == '.'; });
	}


public:
	explicit SMTLibTranslationContext(CHCSmtLib2Interface const& _chcInterface) : m_chcInterface(_chcInterface)
	{
		// fill user defined sorts and constructors
		auto const& userSorts = m_chcInterface.smtlib2Interface()->userSorts();
		for (auto const& declaration: userSorts | ranges::views::values)
		{
			std::stringstream ss(declaration);
			SMTLib2Parser parser(ss);
			auto expr = parser.parseExpression();
			smtAssert(parser.isEOF());
			smtAssert(!isAtom(expr));
			auto const& args = asSubExpressions(expr);
			smtAssert(args.size() == 3);
			smtAssert(isAtom(args[0]) && asAtom(args[0]) == "declare-datatypes");
			// args[1] is the name of the type
			// args[2] is the constructor with the members
			smtAssert(!isAtom(args[2]) && asSubExpressions(args[2]).size() == 1 && !isAtom(asSubExpressions(args[2])[0]));
			auto const& constructors = asSubExpressions(asSubExpressions(args[2])[0]);
			smtAssert(constructors.size() == 1);
			auto const& constructor = constructors[0];
			// constructor is a list: name + members
			smtAssert(!isAtom(constructor));
			auto const& constructorArgs = asSubExpressions(constructor);
			for (unsigned i = 1u; i < constructorArgs.size(); ++i)
			{
				auto const& carg = constructorArgs[i];
				smtAssert(!isAtom(carg) && asSubExpressions(carg).size() == 2);
				auto const& nameSortPair = asSubExpressions(carg);
				m_knownVariables.emplace(asAtom(nameSortPair[0]), toSort(nameSortPair[1]));
			}
		}
	}

	void addVariableDeclaration(std::string name, SortPointer sort)
	{
		smtAssert(m_knownVariables.find(name) == m_knownVariables.end());
		m_knownVariables.emplace(std::move(name), std::move(sort));
	}

	std::optional<SortPointer> lookupKnownTupleSort(std::string const& name) {
		auto const& userSorts = m_chcInterface.sortNames();
		std::string quotedName = "|" + name + "|";
		auto it = ranges::find_if(userSorts, [&](auto const& entry) { return entry.second == quotedName; });
		if (it != userSorts.end() && it->first->kind == Kind::Tuple)
		{
			auto tupleSort = std::dynamic_pointer_cast<TupleSort>(it->first);
			smtAssert(tupleSort);
			return tupleSort;
		}
		return {};
	}

	SortPointer toSort(SMTLib2Expression const& expr)
	{
		if (isAtom(expr))
		{
			auto const& name = asAtom(expr);
			if (name == "Int")
				return SortProvider::sintSort;
			if (name == "Bool")
				return SortProvider::boolSort;
			auto tupleSort = lookupKnownTupleSort(name);
			if (tupleSort)
				return tupleSort.value();
		} else {
			auto const& args = asSubExpressions(expr);
			if (asAtom(args[0]) == "Array")
			{
				smtAssert(args.size() == 3);
				auto domainSort = toSort(args[1]);
				auto codomainSort = toSort(args[2]);
				return std::make_shared<ArraySort>(std::move(domainSort), std::move(codomainSort));
			}
			if (args.size() == 3 && isAtom(args[0]) && asAtom(args[0]) == "_" && isAtom(args[1]) && asAtom(args[1]) == "int2bv")
				return std::make_shared<BitVectorSort>(std::stoul(asAtom(args[2])));
		}
		smtAssert(false, "Unknown sort encountered");
	}

	smtutil::Expression parseQuantifier(
		std::string const& quantifierName,
		std::vector<SMTLib2Expression> const& varList,
		SMTLib2Expression const& coreExpression
	)
	{
		std::vector<std::pair<std::string, SortPointer>> boundVariables;
		for (auto const& sortedVar: varList)
		{
			smtAssert(!isAtom(sortedVar));
			auto varSortPair = asSubExpressions(sortedVar);
			smtAssert(varSortPair.size() == 2);
			boundVariables.emplace_back(asAtom(varSortPair[0]), toSort(varSortPair[1]));
		}
		for (auto const& [var, sort] : boundVariables)
		{
			smtAssert(m_knownVariables.find(var) == m_knownVariables.end()); // TODO: deal with shadowing?
			m_knownVariables.emplace(var, sort);
		}
		auto core = toSMTUtilExpression(coreExpression);
		for (auto const& [var, sort] : boundVariables)
		{
			smtAssert(m_knownVariables.find(var) != m_knownVariables.end());
			m_knownVariables.erase(var);
		}
		return Expression(quantifierName, {core}, SortProvider::boolSort); // TODO: what about the bound variables?

	}

	smtutil::Expression toSMTUtilExpression(SMTLib2Expression const& _expr)
	{
		return std::visit(
			GenericVisitor{
				[&](std::string const& _atom)
				{
					if (_atom == "true" || _atom == "false")
						return smtutil::Expression(_atom == "true");
					else if (isNumber(_atom))
						return smtutil::Expression(_atom, {}, SortProvider::sintSort);
					else if (auto it = m_knownVariables.find(_atom); it != m_knownVariables.end())
						return smtutil::Expression(_atom, {}, it->second);
					else // assume this is a predicate with sort bool; TODO: Context should be aware of predicates!
						return smtutil::Expression(_atom, {}, SortProvider::boolSort);
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
						if (auto it = m_knownVariables.find(op); it != m_knownVariables.end())
							return smtutil::Expression(op, std::move(arguments), it->second);
						else
						{
							std::set<std::string>
								boolOperators{"and", "or", "not", "=", "<", ">", "<=", ">=", "=>"};
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
};


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
//		inlineLetExpressions(interpretation);
		SMTLibTranslationContext context(*this);
		auto const& formalArguments = asSubExpressions(args[2]);
		std::vector<Expression> predicateArgs;
		for (auto const& formalArgument: formalArguments)
		{
			precondition(!isAtom(formalArgument));
			auto const& nameSortPair = asSubExpressions(formalArgument);
			precondition(nameSortPair.size() == 2);
			precondition(isAtom(nameSortPair[0]));
			SortPointer varSort = context.toSort(nameSortPair[1]);
			context.addVariableDeclaration(asAtom(nameSortPair[0]), varSort);
			Expression arg = context.toSMTUtilExpression(nameSortPair[0]);
			predicateArgs.push_back(arg);
		}

		auto parsedInterpretation = context.toSMTUtilExpression(interpretation);

		Expression predicate(asAtom(args[1]), predicateArgs, SortProvider::boolSort);
		definitions.push_back(predicate == parsedInterpretation);
	}
	return Expression::mkAnd(std::move(definitions));
}
#undef precondition
