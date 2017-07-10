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

#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;
using namespace dev;
using namespace dev::solidity::smt;


//namespace
//{

//void createSubprocess(FILE*& _readPipe, FILE*& _writePipe)
//{
//	int pipe_in[2];   /* This is the pipe with wich we write to the child process. */
//	int pipe_out[2];  /* This is the pipe with wich we read from the child process. */

//	solAssert(!pipe(pipe_in) && !pipe(pipe_out), "");

//	/* Attempt to fork and check for errors */
//	pid_t pid = fork();
//	solAssert(pid != -1, "");

//	if (pid)
//	{
//		/* The parent has the non-zero PID. */
//		_readPipe = fdopen(pipe_out[0], "r");
//		_writePipe = fdopen(pipe_in[1], "w");
//		close(pipe_out[1]);
//		close(pipe_in[0]);
//	}
//	else
//	{
//		/* The child has the zero pid returned by fork*/
//		cout << "child" << endl;
//		solAssert(dup2(pipe_out[1], 1) != -1, "");
//		solAssert(dup2(pipe_in[0], 0) != -1, "");
//		solAssert(close(pipe_out[0]) == 0, "");
//		solAssert(close(pipe_out[1]) == 0, "");
//		solAssert(close(pipe_in[0]) == 0, "");
//		solAssert(close(pipe_in[1]) == 0, "");
//		execl("/usr/bin/z3", "z3", "-smt2", "-in", NULL);
//		exit(1); /* Only reached if execl() failed */
//	}
//}

//}

SMTLib2Interface::SMTLib2Interface()
{
	// TODO using pipes did not really work, so trying it the hard way for now.
//	createSubprocess(m_solverWrite, m_solverRead);
//	solAssert(m_solverWrite, "Unable to start Z3");
//	solAssert(m_solverRead, "Unable to start Z3");
//	write("(set-option :produce-models true)\n(set-logic QF_LIA)\n");
	reset();
}

SMTLib2Interface::~SMTLib2Interface()
{
//	if (m_solverWrite)
//	{
//		write("(exit)\n");
//		fflush(m_solverWrite);
//		fclose(m_solverWrite);
//		m_solverWrite = nullptr;
//	}
//	if (m_solverRead)
//	{
//		fclose(m_solverRead);
//		m_solverRead = nullptr;
//	}
}

void SMTLib2Interface::reset()
{
//	write("(reset)\n");
//	write("(set-option :produce-models true)\n");
	m_accumulatedOutput.clear();
	m_accumulatedOutput.emplace_back();
	write("(set-option :produce-models true)\n(set-logic QF_UFLIA)\n");
}

void SMTLib2Interface::push()
{
	m_accumulatedOutput.emplace_back();
	//write("(push)\n");
}

void SMTLib2Interface::pop()
{
	//write("(pop)\n");
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
		")\n"
	);
	return Expression(_name, {});
}

Expression SMTLib2Interface::newInteger(string _name)
{
	write("(declare-const |" + _name + "| Int)\n");
	return Expression(_name, {});
}

Expression SMTLib2Interface::newBool(string _name)
{
	write("(declare-const |" + _name + "| Bool)\n");
	return Expression(_name, {});
}

void SMTLib2Interface::addAssertion(Expression _expr)
{
	write("(assert " + _expr.toSExpr() + ")\n");
}

pair<CheckResult, vector<string>> SMTLib2Interface::check(vector<Expression> const& _expressionsToEvaluate)
{
	string checks;
	if (_expressionsToEvaluate.empty())
		checks = "(check-sat)\n";
	else
	{
		// TODO make sure these are unique
		for (size_t i = 0; i < _expressionsToEvaluate.size(); i++)
		{
			auto const& e = _expressionsToEvaluate.at(i);
			// TODO they don't have to be ints...
			checks += "(declare-const |EVALEXPR_" + to_string(i) + "| Int)\n";
			checks += "(assert (= |EVALEXPR_" + to_string(i) + "| " + e.toSExpr() + "))\n";
		}
		checks += "(check-sat)\n";
		checks += "(get-value (";
		for (size_t i = 0; i < _expressionsToEvaluate.size(); i++)
			checks += "|EVALEXPR_" + to_string(i) + "| ";
		checks += "))\n";
	}
	string response = communicate(boost::algorithm::join(m_accumulatedOutput, "\n") + checks);
	CheckResult result;
	// TODO proper parsing
	if (boost::starts_with(response, "sat\n"))
		result = CheckResult::SAT;
	else if (boost::starts_with(response, "unsat\n"))
		result = CheckResult::UNSAT;
	else if (boost::starts_with(response, "unknown\n"))
		result = CheckResult::UNKNOWN;
	else
		solAssert(false, "Invalid response to check-sat: " + response);

	vector<string> values;
	if (result != CheckResult::UNSAT)
		values = parseValues(find(response.cbegin(), response.cend(), '\n'), response.cend());
	return make_pair(result, values);
}

//string SMTLib2Interface::eval(Expression _expr)
//{
//	write("(get-value (" + _expr.toSExpr() + ")\n");
//	std::string reply = communicate();
//	cout << "<-- Z3: " << reply << endl;
//	// TODO parse
//	return reply;
//}

void SMTLib2Interface::write(string _data)
{
//	cout << " --> Z3: " << _data << endl;
//	solAssert(m_solverWrite, "");
//	solAssert(fputs(_data.c_str(), m_solverWrite) >= 0 || true, "EOF while communicating with Z3.");
//	solAssert(fflush(m_solverWrite) == 0 || true, "");
	solAssert(!m_accumulatedOutput.empty(), "");
	m_accumulatedOutput.back() += move(_data);
}

string SMTLib2Interface::communicate(std::string const& _input)
{
	ofstream("/tmp/z3exchange.smt2") << _input << "(exit)" << endl;
	FILE* solverOutput = popen("z3 -smt2 /tmp/z3exchange.smt2", "r");
	string result;
	array<char, 128> buffer;
	while (!feof(solverOutput))
		if (fgets(buffer.data(), 127, solverOutput) != nullptr)
			result += buffer.data();
	fclose(solverOutput);
	return result;
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
