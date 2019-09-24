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

#include <libsolidity/formal/CHCSmtLib2Interface.h>

#include <libdevcore/Keccak256.h>

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem/operations.hpp>

#include <array>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>

using namespace std;
using namespace dev;
using namespace dev::solidity;
using namespace dev::solidity::smt;

CHCSmtLib2Interface::CHCSmtLib2Interface(map<h256, string> const& _queryResponses):
	m_smtlib2(make_shared<SMTLib2Interface>(_queryResponses)),
	m_queryResponses(_queryResponses)
{
	reset();
}

void CHCSmtLib2Interface::reset()
{
	m_accumulatedOutput.clear();
	m_variables.clear();
}

void CHCSmtLib2Interface::registerRelation(smt::Expression const& _expr)
{
	solAssert(_expr.sort, "");
	solAssert(_expr.sort->kind == smt::Kind::Function, "");
	if (!m_variables.count(_expr.name))
	{
		auto fSort = dynamic_pointer_cast<FunctionSort>(_expr.sort);
		string domain = m_smtlib2->toSmtLibSort(fSort->domain);
		// Relations are predicates which have implicit codomain Bool.
		m_variables.insert(_expr.name);
		write(
			"(declare-rel |" +
			_expr.name +
			"| " +
			domain +
			")"
		);
	}
}

void CHCSmtLib2Interface::addRule(smt::Expression const& _expr, std::string const& _name)
{
	write(
		"(rule (! " +
		m_smtlib2->toSExpr(_expr) +
		" :named " +
		_name +
		"))"
	);
}

pair<CheckResult, vector<string>> CHCSmtLib2Interface::query(smt::Expression const& _block)
{
	string accumulated{};
	swap(m_accumulatedOutput, accumulated);
	for (auto const& var: m_smtlib2->variables())
		declareVariable(var.first, var.second);
	m_accumulatedOutput += accumulated;

	string response = querySolver(
		m_accumulatedOutput +
		"\n(query " + _block.name + " :print-certificate true)"
	);

	CheckResult result;
	// TODO proper parsing
	if (boost::starts_with(response, "sat\n"))
		result = CheckResult::SATISFIABLE;
	else if (boost::starts_with(response, "unsat\n"))
		result = CheckResult::UNSATISFIABLE;
	else if (boost::starts_with(response, "unknown\n"))
		result = CheckResult::UNKNOWN;
	else
		result = CheckResult::ERROR;

	// TODO collect invariants or counterexamples.
	return make_pair(result, vector<string>{});
}

void CHCSmtLib2Interface::declareVariable(string const& _name, SortPointer const& _sort)
{
	solAssert(_sort, "");
	if (_sort->kind == Kind::Function)
		declareFunction(_name, _sort);
	else if (!m_variables.count(_name))
	{
		m_variables.insert(_name);
		write("(declare-var |" + _name + "| " + m_smtlib2->toSmtLibSort(*_sort) + ')');
	}
}

void CHCSmtLib2Interface::declareFunction(string const& _name, SortPointer const& _sort)
{
	solAssert(_sort, "");
	solAssert(_sort->kind == smt::Kind::Function, "");
	// TODO Use domain and codomain as key as well
	if (!m_variables.count(_name))
	{
		auto fSort = dynamic_pointer_cast<FunctionSort>(_sort);
		solAssert(fSort->codomain, "");
		string domain = m_smtlib2->toSmtLibSort(fSort->domain);
		string codomain = m_smtlib2->toSmtLibSort(*fSort->codomain);
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

void CHCSmtLib2Interface::write(string _data)
{
	m_accumulatedOutput += move(_data) + "\n";
}

string CHCSmtLib2Interface::querySolver(string const& _input)
{
	h256 inputHash = dev::keccak256(_input);
	if (m_queryResponses.count(inputHash))
		return m_queryResponses.at(inputHash);
	else
	{
		m_unhandledQueries.push_back(_input);
		return "unknown\n";
	}
}
