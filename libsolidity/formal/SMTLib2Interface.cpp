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

#include <libsolidity/formal/SMTLib2Interface.h>

#include <libsolidity/interface/Exceptions.h>
#include <libsolidity/interface/ReadFile.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/filesystem/operations.hpp>

#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

using namespace std;
using namespace dev;
using namespace dev::solidity;
using namespace dev::solidity::smt;

SMTLib2Interface::SMTLib2Interface(ReadCallback::Callback const& _queryCallback):
	m_queryCallback(_queryCallback)
{
	reset();
}

void SMTLib2Interface::reset()
{
	m_accumulatedOutput.clear();
	m_accumulatedOutput.emplace_back();
	m_constants.clear();
	m_functions.clear();
	write("(set-option :produce-models true)");
	write("(set-logic QF_UFLIA)");
}

void SMTLib2Interface::push()
{
	m_accumulatedOutput.emplace_back();
}

void SMTLib2Interface::pop()
{
	solAssert(!m_accumulatedOutput.empty(), "");
	m_accumulatedOutput.pop_back();
}

void SMTLib2Interface::declareFunction(string _name, vector<Sort> const& _domain, Sort _codomain)
{
	// TODO Use domain and codomain as key as well
	string domain("");
	for (auto const& sort: _domain)
		domain += toSmtLibSort(sort) + ' ';
	if (!m_functions.count(_name))
	{
		m_functions.insert(_name);
		write(
			"(declare-fun |" +
			_name +
			"| (" +
			domain +
			") " +
			(_codomain == Sort::Int ? "Int" : "Bool") +
			")"
		);
	}
}

void SMTLib2Interface::declareInteger(string _name)
{
	if (!m_constants.count(_name))
	{
		m_constants.insert(_name);
		write("(declare-const |" + _name + "| Int)");
	}
}

void SMTLib2Interface::declareBool(string _name)
{
	if (!m_constants.count(_name))
	{
		m_constants.insert(_name);
		write("(declare-const |" + _name + "| Bool)");
	}
}

void SMTLib2Interface::addAssertion(Expression const& _expr)
{
	write("(assert " + toSExpr(_expr) + ")");
}

pair<CheckResult, vector<string>> SMTLib2Interface::check(vector<Expression> const& _expressionsToEvaluate)
{
	string response = querySolver(
		boost::algorithm::join(m_accumulatedOutput, "\n") +
		checkSatAndGetValuesCommand(_expressionsToEvaluate)
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

	vector<string> values;
	if (result == CheckResult::SATISFIABLE && result != CheckResult::ERROR)
		values = parseValues(find(response.cbegin(), response.cend(), '\n'), response.cend());
	return make_pair(result, values);
}

string SMTLib2Interface::toSExpr(Expression const& _expr)
{
	if (_expr.arguments.empty())
		return _expr.name;
	std::string sexpr = "(" + _expr.name;
	for (auto const& arg: _expr.arguments)
		sexpr += " " + toSExpr(arg);
	sexpr += ")";
	return sexpr;
}

string SMTLib2Interface::toSmtLibSort(Sort _sort)
{
	switch (_sort)
	{
	case Sort::Int:
		return "Int";
	case Sort::Bool:
		return "Bool";
	default:
		solAssert(false, "Invalid SMT sort");
	}
}

void SMTLib2Interface::write(string _data)
{
	solAssert(!m_accumulatedOutput.empty(), "");
	m_accumulatedOutput.back() += move(_data) + "\n";
}

string SMTLib2Interface::checkSatAndGetValuesCommand(vector<Expression> const& _expressionsToEvaluate)
{
	string command;
	if (_expressionsToEvaluate.empty())
		command = "(check-sat)\n";
	else
	{
		// TODO make sure these are unique
		for (size_t i = 0; i < _expressionsToEvaluate.size(); i++)
		{
			auto const& e = _expressionsToEvaluate.at(i);
			solAssert(e.sort == Sort::Int || e.sort == Sort::Bool, "Invalid sort for expression to evaluate.");
			command += "(declare-const |EVALEXPR_" + to_string(i) + "| " + (e.sort == Sort::Int ? "Int" : "Bool") + ")\n";
			command += "(assert (= |EVALEXPR_" + to_string(i) + "| " + toSExpr(e) + "))\n";
		}
		command += "(check-sat)\n";
		command += "(get-value (";
		for (size_t i = 0; i < _expressionsToEvaluate.size(); i++)
			command += "|EVALEXPR_" + to_string(i) + "| ";
		command += "))\n";
	}

	return command;
}

vector<string> SMTLib2Interface::parseValues(string::const_iterator _start, string::const_iterator _end)
{
	vector<string> values;
	while (_start < _end)
	{
		auto valStart = find(_start, _end, ' ');
		if (valStart < _end)
			++valStart;
		auto valEnd = find(valStart, _end, ')');
		values.emplace_back(valStart, valEnd);
		_start = find(valEnd, _end, '(');
	}

	return values;
}

string SMTLib2Interface::querySolver(string const& _input)
{
	if (!m_queryCallback)
		BOOST_THROW_EXCEPTION(SolverError() << errinfo_comment("No SMT solver available."));

	ReadCallback::Result queryResult = m_queryCallback(_input);
	if (!queryResult.success)
		BOOST_THROW_EXCEPTION(SolverError() << errinfo_comment(queryResult.responseOrErrorMessage));
	return queryResult.responseOrErrorMessage;
}
