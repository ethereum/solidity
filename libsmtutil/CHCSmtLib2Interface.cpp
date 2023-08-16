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

#include <libsolutil/Keccak256.h>

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/predicate.hpp>

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
	std::map<h256, std::string> const& _queryResponses,
	ReadCallback::Callback _smtCallback,
	SMTSolverChoice _enabledSolvers,
	std::optional<unsigned> _queryTimeout
):
	CHCSolverInterface(_queryTimeout),
	m_smtlib2(std::make_unique<SMTLib2Interface>(_queryResponses, _smtCallback, m_queryTimeout)),
	m_queryResponses(std::move(_queryResponses)),
	m_smtCallback(_smtCallback),
	m_enabledSolvers(_enabledSolvers)
{
	reset();
}

void CHCSmtLib2Interface::reset()
{
	m_accumulatedOutput.clear();
	m_variables.clear();
	m_unhandledQueries.clear();
	m_sortNames.clear();
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
		result = CheckResult::UNSATISFIABLE;
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
		write("(declare-var |" + _name + "| " + toSmtLibSort(*_sort) + ')');
	}
}

std::string CHCSmtLib2Interface::toSmtLibSort(Sort const& _sort)
{
	if (!m_sortNames.count(&_sort))
		m_sortNames[&_sort] = m_smtlib2->toSmtLibSort(_sort);
	return m_sortNames.at(&_sort);
}

std::string CHCSmtLib2Interface::toSmtLibSort(std::vector<SortPointer> const& _sorts)
{
	std::string ssort("(");
	for (auto const& sort: _sorts)
		ssort += toSmtLibSort(*sort) + " ";
	ssort += ")";
	return ssort;
}

std::string CHCSmtLib2Interface::forall()
{
	std::string vars("(");
	for (auto const& [name, sort]: m_smtlib2->variables())
	{
		solAssert(sort, "");
		if (sort->kind != Kind::Function)
			vars += " (" + name + " " + toSmtLibSort(*sort) + ")";
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
		std::string codomain = toSmtLibSort(*fSort->codomain);
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

	smtAssert(m_enabledSolvers.smtlib2 || m_enabledSolvers.eld);
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
	return "(assert\n(forall " + forall() + "\n" +	"(=> " + name + " false)))";
}
