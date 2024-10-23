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

#include <libsmtutil/SMTLib2Interface.h>

#include <libsmtutil/SMTLib2Parser.h>

#include <libsolutil/Keccak256.h>

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <range/v3/algorithm/find_if.hpp>

#include <array>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

using namespace solidity;
using namespace solidity::util;
using namespace solidity::frontend;
using namespace solidity::smtutil;

SMTLib2Interface::SMTLib2Interface(
	std::map<h256, std::string> _queryResponses,
	ReadCallback::Callback _smtCallback,
	std::optional<unsigned> _queryTimeout
):
	BMCSolverInterface(_queryTimeout),
	m_queryResponses(std::move(_queryResponses)),
	m_smtCallback(std::move(_smtCallback))
{
	reset();
}

void SMTLib2Interface::reset()
{
	m_commands.clear();
	m_context.clear();
	m_commands.setOption("produce-models", "true");
	if (m_queryTimeout)
		m_commands.setOption("timeout", std::to_string(*m_queryTimeout));
	m_commands.setLogic("ALL");
	m_context.setTupleDeclarationCallback([&](TupleSort const& tupleSort){
		m_commands.declareTuple(
			tupleSort.name,
			tupleSort.members,
			tupleSort.components
				| ranges::views::transform([&](SortPointer const& sort){ return m_context.toSmtLibSort(sort); })
				| ranges::to<std::vector>()
		);
	});
}

void SMTLib2Interface::push()
{
	m_commands.push();
}

void SMTLib2Interface::pop()
{
	m_commands.pop();
}

void SMTLib2Interface::declareVariable(std::string const& _name, SortPointer const& _sort)
{
	smtAssert(_sort);
	if (_sort->kind == Kind::Function)
		declareFunction(_name, _sort);
	else if (!m_context.isDeclared(_name))
	{
		m_context.declare(_name, _sort);
		m_commands.declareVariable(_name, toSmtLibSort(_sort));
	}
}

void SMTLib2Interface::declareFunction(std::string const& _name, SortPointer const& _sort)
{
	smtAssert(_sort);
	smtAssert(_sort->kind == Kind::Function);
	if (!m_context.isDeclared(_name))
	{
		auto const& fSort = std::dynamic_pointer_cast<FunctionSort>(_sort);
		auto domain = toSmtLibSort(fSort->domain);
		std::string codomain = toSmtLibSort(fSort->codomain);
		m_context.declare(_name, _sort);
		m_commands.declareFunction(_name, domain, codomain);
	}
}

void SMTLib2Interface::addAssertion(Expression const& _expr)
{
	m_commands.assertion(toSExpr(_expr));
}

namespace
{
std::vector<std::string> parseValuesFromResponse(std::string const& _response)
{
	std::stringstream ss(_response);
	std::string answer;
	ss >> answer;
	smtSolverInteractionRequire(answer == "sat", "SMT: Parsing model values only possible after sat answer");

	std::vector<SMTLib2Expression> parsedOutput;
	SMTLib2Parser parser(ss);
	try
	{
		while (!parser.isEOF())
			parsedOutput.push_back(parser.parseExpression());
	}
	catch(SMTLib2Parser::ParsingException&)
	{
		smtSolverInteractionRequire(false, "Error during parsing SMT answer");
	}
	smtSolverInteractionRequire(parsedOutput.size() == 1, "SMT: Expected model values as a single s-expression");
	auto const& values = parsedOutput[0];
	smtSolverInteractionRequire(!isAtom(values), "Invalid format of values in SMT answer");
	std::vector<std::string> parsedValues;
	for (auto const& nameValuePair: asSubExpressions(values))
	{
		smtSolverInteractionRequire(!isAtom(nameValuePair), "Invalid format of values in SMT answer");
		smtSolverInteractionRequire(asSubExpressions(nameValuePair).size() == 2, "Invalid format of values in SMT answer");
		auto const& value = asSubExpressions(nameValuePair)[1];
		parsedValues.push_back(value.toString());
	}
	return parsedValues;
}
} // namespace

std::pair<CheckResult, std::vector<std::string>> SMTLib2Interface::check(std::vector<Expression> const& _expressionsToEvaluate)
{
	std::string response = querySolver(dumpQuery(_expressionsToEvaluate));

	CheckResult result;
	// TODO proper parsing
	if (boost::starts_with(response, "sat"))
		result = CheckResult::SATISFIABLE;
	else if (boost::starts_with(response, "unsat"))
		result = CheckResult::UNSATISFIABLE;
	else if (boost::starts_with(response, "unknown"))
		result = CheckResult::UNKNOWN;
	else
		result = CheckResult::ERROR;

	std::vector<std::string> values;
	if (result == CheckResult::SATISFIABLE && !_expressionsToEvaluate.empty())
		values = parseValuesFromResponse(response);
	return std::make_pair(result, values);
}

std::string SMTLib2Interface::toSmtLibSort(SortPointer _sort)
{
	return m_context.toSmtLibSort(std::move(_sort));
}

std::vector<std::string> SMTLib2Interface::toSmtLibSort(std::vector<SortPointer> const& _sorts)
{
	std::vector<std::string> ssorts;
	ssorts.reserve(_sorts.size());
	for (auto const& sort: _sorts)
		ssorts.push_back(toSmtLibSort(sort));
	return ssorts;
}

std::string SMTLib2Interface::toSExpr(solidity::smtutil::Expression const& _expr)
{
	return m_context.toSExpr(_expr);
}

std::string SMTLib2Interface::checkSatAndGetValuesCommand(std::vector<Expression> const& _expressionsToEvaluate)
{
	std::string command;
	if (_expressionsToEvaluate.empty())
		command = "(check-sat)\n";
	else
	{
		// TODO make sure these are unique
		for (size_t i = 0; i < _expressionsToEvaluate.size(); i++)
		{
			auto const& e = _expressionsToEvaluate.at(i);
			smtAssert(e.sort->kind == Kind::Int || e.sort->kind == Kind::Bool, "Invalid sort for expression to evaluate.");
			command += "(declare-const |EVALEXPR_" + std::to_string(i) + "| " + (e.sort->kind == Kind::Int ? "Int" : "Bool") + ")\n";
			command += "(assert (= |EVALEXPR_" + std::to_string(i) + "| " + toSExpr(e) + "))\n";
		}
		command += "(check-sat)\n";
		command += "(get-value (";
		for (size_t i = 0; i < _expressionsToEvaluate.size(); i++)
			command += "|EVALEXPR_" + std::to_string(i) + "| ";
		command += "))\n";
	}

	return command;
}

std::string SMTLib2Interface::querySolver(std::string const& _input)
{
	h256 inputHash = keccak256(_input);
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

std::string SMTLib2Interface::dumpQuery(std::vector<Expression> const& _expressionsToEvaluate)
{
	return m_commands.toString() + '\n' + checkSatAndGetValuesCommand(_expressionsToEvaluate);
}


void SMTLib2Commands::push() {
	m_frameLimits.push_back(m_commands.size());
}

void SMTLib2Commands::pop() {
	smtAssert(!m_commands.empty());
	auto limit = m_frameLimits.back();
	m_frameLimits.pop_back();
	while (m_commands.size() > limit)
		m_commands.pop_back();
}

std::string SMTLib2Commands::toString() const {
	return boost::algorithm::join(m_commands, "\n");
}

void SMTLib2Commands::clear() {
	m_commands.clear();
	m_frameLimits.clear();
}

void SMTLib2Commands::assertion(std::string _expr) {
	m_commands.push_back("(assert " + std::move(_expr) + ')');
}

void SMTLib2Commands::setOption(std::string _name, std::string _value)
{
	m_commands.push_back("(set-option :" + std::move(_name) + ' ' + std::move(_value) + ')');
}

void SMTLib2Commands::setLogic(std::string _logic)
{
	m_commands.push_back("(set-logic " + std::move(_logic) + ')');
}

void SMTLib2Commands::declareVariable(std::string _name, std::string _sort)
{
	m_commands.push_back("(declare-fun |" + std::move(_name) + "| () " + std::move(_sort) + ')');
}

void SMTLib2Commands::declareFunction(std::string const& _name, std::vector<std::string> const& _domain, std::string const& _codomain)
{
	std::stringstream ss;
	ss << "(declare-fun |" << _name << "| " << '(' << boost::join(_domain, " ")
		<< ')' << ' ' << _codomain << ')';
	m_commands.push_back(ss.str());
}

void SMTLib2Commands::declareTuple(
	std::string const& _name,
	std::vector<std::string> const& _memberNames,
	std::vector<std::string> const& _memberSorts
)
{
	auto quotedName = '|' + _name + '|';
	std::stringstream ss;
	ss << "(declare-datatypes ((" << quotedName << " 0)) (((" << quotedName;
	for (auto && [memberName, memberSort]: ranges::views::zip(_memberNames, _memberSorts))
		ss << " (|" << memberName << "| " << memberSort << ")";
	ss << "))))";
	auto declaration = ss.str();
	m_commands.push_back(ss.str());
}
