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
using namespace dev::solidity::smt;

SMTLib2Interface::SMTLib2Interface(ReadFile::Callback const& _readFileCallback):
	m_communicator(_readFileCallback)
{
	reset();
}

void SMTLib2Interface::reset()
{
	m_accumulatedOutput.clear();
	m_accumulatedOutput.emplace_back();
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

Expression SMTLib2Interface::newFunction(string _name, Sort _domain, Sort _codomain)
{
	write(
		"(declare-fun |" +
		_name +
		"| (" +
		(_domain == Sort::Int ? "Int" : "Bool") +
		") " +
		(_codomain == Sort::Int ? "Int" : "Bool") +
		")"
	);
	return Expression(std::move(_name), {});
}

Expression SMTLib2Interface::newInteger(string _name)
{
	write("(declare-const |" + _name + "| Int)");
	return Expression(std::move(_name), {});
}

Expression SMTLib2Interface::newBool(string _name)
{
	write("(declare-const |" + _name + "| Bool)");
	return Expression(std::move(_name), {});
}

void SMTLib2Interface::addAssertion(Expression const& _expr)
{
	write("(assert " + _expr.toSExpr() + ")");
}

pair<CheckResult, vector<string>> SMTLib2Interface::check(vector<Expression> const& _expressionsToEvaluate)
{
	string response = m_communicator.communicate(
		boost::algorithm::join(m_accumulatedOutput, "\n") +
		checkSatAndGetValuesCommand(_expressionsToEvaluate)
	);
	CheckResult result;
	// TODO proper parsing
	if (boost::starts_with(response, "sat\n"))
		result = CheckResult::SAT;
	else if (boost::starts_with(response, "unsat\n"))
		result = CheckResult::UNSAT;
	else if (boost::starts_with(response, "unknown\n"))
		result = CheckResult::UNKNOWN;
	else
		result = CheckResult::ERROR;

	vector<string> values;
	if (result != CheckResult::UNSAT && result != CheckResult::ERROR)
		values = parseValues(find(response.cbegin(), response.cend(), '\n'), response.cend());
	return make_pair(result, values);
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
			// TODO they don't have to be ints...
			command += "(declare-const |EVALEXPR_" + to_string(i) + "| Int)\n";
			command += "(assert (= |EVALEXPR_" + to_string(i) + "| " + e.toSExpr() + "))\n";
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
