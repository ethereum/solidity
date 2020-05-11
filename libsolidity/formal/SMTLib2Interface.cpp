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

#include <libsolutil/Keccak256.h>

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <array>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::frontend;
using namespace solidity::frontend::smt;

SMTLib2Interface::SMTLib2Interface(
	map<h256, string> const& _queryResponses,
	ReadCallback::Callback _smtCallback
):
	m_queryResponses(_queryResponses),
	m_smtCallback(std::move(_smtCallback))
{
	reset();
}

void SMTLib2Interface::reset()
{
	m_accumulatedOutput.clear();
	m_accumulatedOutput.emplace_back();
	m_variables.clear();
	m_userSorts.clear();
	write("(set-option :produce-models true)");
	write("(set-logic ALL)");
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

void SMTLib2Interface::declareVariable(string const& _name, SortPointer const& _sort)
{
	solAssert(_sort, "");
	if (_sort->kind == Kind::Function)
		declareFunction(_name, _sort);
	else if (!m_variables.count(_name))
	{
		m_variables.emplace(_name, _sort);
		write("(declare-fun |" + _name + "| () " + toSmtLibSort(*_sort) + ')');
	}
}

void SMTLib2Interface::declareFunction(string const& _name, SortPointer const& _sort)
{
	solAssert(_sort, "");
	solAssert(_sort->kind == smt::Kind::Function, "");
	// TODO Use domain and codomain as key as well
	if (!m_variables.count(_name))
	{
		auto const& fSort = dynamic_pointer_cast<FunctionSort>(_sort);
		string domain = toSmtLibSort(fSort->domain);
		string codomain = toSmtLibSort(*fSort->codomain);
		m_variables.emplace(_name, _sort);
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

void SMTLib2Interface::addAssertion(smt::Expression const& _expr)
{
	write("(assert " + toSExpr(_expr) + ")");
}

pair<CheckResult, vector<string>> SMTLib2Interface::check(vector<smt::Expression> const& _expressionsToEvaluate)
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

string SMTLib2Interface::toSExpr(smt::Expression const& _expr)
{
	if (_expr.arguments.empty())
		return _expr.name;

	std::string sexpr = "(";
	if (_expr.name == "const_array")
	{
		solAssert(_expr.arguments.size() == 2, "");
		auto sortSort = std::dynamic_pointer_cast<SortSort>(_expr.arguments.at(0).sort);
		solAssert(sortSort, "");
		auto arraySort = dynamic_pointer_cast<ArraySort>(sortSort->inner);
		solAssert(arraySort, "");
		sexpr += "(as const " + toSmtLibSort(*arraySort) + ") ";
		sexpr += toSExpr(_expr.arguments.at(1));
	}
	else if (_expr.name == "tuple_get")
	{
		solAssert(_expr.arguments.size() == 2, "");
		auto tupleSort = dynamic_pointer_cast<TupleSort>(_expr.arguments.at(0).sort);
		unsigned index = std::stoi(_expr.arguments.at(1).name);
		solAssert(index < tupleSort->members.size(), "");
		sexpr += tupleSort->members.at(index) + " " + toSExpr(_expr.arguments.at(0));
	}
	else
	{
		sexpr += _expr.name;
		for (auto const& arg: _expr.arguments)
			sexpr += " " + toSExpr(arg);
	}
	sexpr += ")";
	return sexpr;
}

string SMTLib2Interface::toSmtLibSort(Sort const& _sort)
{
	switch (_sort.kind)
	{
	case Kind::Int:
		return "Int";
	case Kind::Bool:
		return "Bool";
	case Kind::Array:
	{
		auto const& arraySort = dynamic_cast<ArraySort const&>(_sort);
		solAssert(arraySort.domain && arraySort.range, "");
		return "(Array " + toSmtLibSort(*arraySort.domain) + ' ' + toSmtLibSort(*arraySort.range) + ')';
	}
	case Kind::Tuple:
	{
		auto const& tupleSort = dynamic_cast<TupleSort const&>(_sort);
		if (!m_userSorts.count(tupleSort.name))
		{
			m_userSorts.insert(tupleSort.name);
			string decl("(declare-datatypes ((" + tupleSort.name + " 0)) (((" + tupleSort.name);
			solAssert(tupleSort.members.size() == tupleSort.components.size(), "");
			for (unsigned i = 0; i < tupleSort.members.size(); ++i)
				decl += " (" + tupleSort.members.at(i) + " " + toSmtLibSort(*tupleSort.components.at(i)) + ")";
			decl += "))))";
			write(decl);
		}

		return tupleSort.name;
	}
	default:
		solAssert(false, "Invalid SMT sort");
	}
}

string SMTLib2Interface::toSmtLibSort(vector<SortPointer> const& _sorts)
{
	string ssort("(");
	for (auto const& sort: _sorts)
		ssort += toSmtLibSort(*sort) + " ";
	ssort += ")";
	return ssort;
}

void SMTLib2Interface::write(string _data)
{
	solAssert(!m_accumulatedOutput.empty(), "");
	m_accumulatedOutput.back() += move(_data) + "\n";
}

string SMTLib2Interface::checkSatAndGetValuesCommand(vector<smt::Expression> const& _expressionsToEvaluate)
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
			solAssert(e.sort->kind == Kind::Int || e.sort->kind == Kind::Bool, "Invalid sort for expression to evaluate.");
			command += "(declare-const |EVALEXPR_" + to_string(i) + "| " + (e.sort->kind == Kind::Int ? "Int" : "Bool") + ")\n";
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
	h256 inputHash = keccak256(_input);
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
