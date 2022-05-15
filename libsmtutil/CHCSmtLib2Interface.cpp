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

#include <boost/filesystem/operations.hpp>
#include <libsmtutil/CHCSmtLib2Interface.h>

#include <libsolutil/Algorithms.h>
#include <libsolutil/Keccak256.h>

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/process.hpp>

#include <range/v3/view.hpp>
#include <range/v3/algorithm/for_each.hpp>

#include <array>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::frontend;
using namespace solidity::smtutil;

CHCSmtLib2Interface::CHCSmtLib2Interface(
	map<h256, string> const& _queryResponses,
	ReadCallback::Callback _smtCallback,
	SMTSolverChoice _enabledSolvers,
	optional<unsigned> _queryTimeout
):
	CHCSolverInterface(_queryTimeout),
	m_smtlib2(make_unique<SMTLib2Interface>(_queryResponses, _smtCallback, m_queryTimeout)),
	m_queryResponses(move(_queryResponses)),
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
		auto fSort = dynamic_pointer_cast<FunctionSort>(_expr.sort);
		string domain = toSmtLibSort(fSort->domain);
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

tuple<CheckResult, Expression, CHCSolverInterface::CexGraph> CHCSmtLib2Interface::query(Expression const& _block)
{
	string accumulated{};
	swap(m_accumulatedOutput, accumulated);
	solAssert(m_smtlib2, "");
	writeHeader();
	for (auto const& decl: m_smtlib2->userSorts() | ranges::views::values)
		write(decl);
	m_accumulatedOutput += accumulated;

	string queryRule = "(assert\n(forall " + forall() + "\n" +
		"(=> " + _block.name + " false)"
		"))";
	string response = querySolver(
		m_accumulatedOutput +
		queryRule +
		"\n(check-sat)"
	);
	swap(m_accumulatedOutput, accumulated);

	CheckResult result;
	// TODO proper parsing
	if (boost::starts_with(response, "sat"))
		return {CheckResult::UNSATISFIABLE, parseInvariants(response), {}};
	else if (boost::starts_with(response, "unsat"))
		return {CheckResult::SATISFIABLE, Expression (true), parseCounterexample(response)};
	else if (boost::starts_with(response, "unknown"))
		result = CheckResult::UNKNOWN;
	else
		result = CheckResult::ERROR;

	return {result, Expression(true), {}};
}

namespace
{

string readToken(string const& _data, size_t _pos)
{
	string r;
	while (_pos < _data.size())
	{
		char c = _data[_pos++];
		if (iswspace(unsigned(c)) || c == ',' || c == '(' || c == ')')
			break;
		r += c;
	}
	return r;
}

size_t skipWhitespaces(string const& _data, size_t _pos)
{
	while (_pos < _data.size() && iswspace(unsigned(_data[_pos])))
	{
		++_pos;
	}

	return _pos;
}

/// @param _data here is always going to be either
/// - term
/// - term(args)
pair<smtutil::Expression, size_t> parseExpression(string const& _data)
{
	size_t pos = skipWhitespaces(_data, 0);

	string fname = readToken(_data, pos);
	pos += fname.size();

	if (pos >= _data.size() || _data[pos] != '(')
	{
		if (fname == "true" || fname == "false")
			return {Expression(fname, {}, SortProvider::boolSort), pos};
		return {Expression(fname, {}, SortProvider::uintSort), pos};
	}

	smtAssert(_data[pos] == '(');

	vector<Expression> exprArgs;
	do
	{
		++pos;
		auto [arg, size] = parseExpression(_data.substr(pos));
		pos += size;
		exprArgs.emplace_back(move(arg));
		smtAssert(_data[pos] == ',' || _data[pos] == ')');
	} while (_data[pos] == ',');

	smtAssert(_data[pos] == ')');
	++pos;

	if (fname == "const")
		fname = "const_array";

	return {Expression(fname, move(exprArgs), SortProvider::uintSort), pos};
}

/// @param _data here is always going to be either
/// - term
/// - (term arg1 arg2 ... argk), where each arg is an sexpr.
pair<smtutil::Expression, size_t> parseSExpression(string const& _data)
{
	size_t pos = skipWhitespaces(_data, 0);

	vector<Expression> exprArgs;
	string fname;
	if (_data[pos] != '(')
	{
		fname = readToken(_data, pos);
		pos += fname.size();
	}
	else
	{
		++pos;

		string subExpr = _data.substr(pos);
		string target = "(as const ";
		if (boost::starts_with(subExpr, target))
		{
			fname = "const_array";
			pos += target.size();
			auto [symbArg, newPos] = parseSExpression(_data.substr(pos));
			exprArgs.emplace_back(move(symbArg));
			pos += newPos;
			++pos;
		}
		else
		{
			fname = readToken(_data, pos);
			pos += fname.size();
		}

		do
		{
			auto [symbArg, newPos] = parseSExpression(_data.substr(pos));
			exprArgs.emplace_back(move(symbArg));
			pos += newPos;
		} while (_data[pos] != ')');
		++pos;
	}

	if (fname == "")
		fname = "var-decl";
	else if (fname == "const")
		fname = "const_array";

	if (fname == "true" || fname == "false")
		return {Expression(fname, {}, SortProvider::boolSort), pos};

	return {Expression(fname, move(exprArgs), SortProvider::uintSort), pos};
}

smtutil::Expression parseDefineFun(string const& _data)
{
	auto [defineFun, pos] = parseSExpression(_data);

	vector<Expression> newArgs;
	Expression const& curArgs = defineFun.arguments.at(1);
	smtAssert(curArgs.name == "var-decl");
	for (auto&& curArg: curArgs.arguments)
		newArgs.emplace_back(move(curArg));

	Expression predExpr{defineFun.arguments.at(0).name, move(newArgs), SortProvider::boolSort};

	Expression& invExpr = defineFun.arguments.at(3);

	solidity::util::BreadthFirstSearch<Expression*> bfs{{&invExpr}};
	bfs.run([&](auto&& _expr, auto&& _addChild) {
		if (_expr->name == "=")
		{
			smtAssert(_expr->arguments.size() == 2);
			auto check = [](string const& _name) {
				return boost::starts_with(_name, "mapping") && boost::ends_with(_name, "length");
			};
			if (check(_expr->arguments.at(0).name) || check(_expr->arguments.at(1).name))
				*_expr = Expression(true);
		}
		for (auto& arg: _expr->arguments)
			_addChild(&arg);
	});

	Expression eq{"=", {move(predExpr), move(defineFun.arguments.at(3))}, SortProvider::boolSort};

	return eq;
}

}

CHCSolverInterface::CexGraph CHCSmtLib2Interface::parseCounterexample(string const& _result)
{
	CHCSolverInterface::CexGraph cexGraph;

	for (auto&& line: _result | ranges::views::split('\n') | ranges::to<vector<string>>())
	{
		string firstDelimiter = ": ";
		string secondDelimiter = " -> ";

		size_t f = line.find(firstDelimiter);
		if (f != string::npos)
		{
			string id = line.substr(0, f);
			string rest = line.substr(f + firstDelimiter.size());

			size_t s = rest.find(secondDelimiter);
			string pred;
			string adj;
			if (s == string::npos)
				pred = rest;
			else
			{
				pred = rest.substr(0, s);
				adj = rest.substr(s + secondDelimiter.size());
			}

			if (pred == "FALSE")
				pred = "false";

			unsigned iid = unsigned(stoi(id));

			vector<unsigned> children;
			for (auto&& v: adj | ranges::views::split(',') | ranges::to<vector<string>>())
				children.emplace_back(unsigned(stoi(v)));

			auto [expr, size] = parseExpression(pred);

			cexGraph.nodes.emplace(iid, move(expr));
			cexGraph.edges.emplace(iid, move(children));
		}
	}

	return cexGraph;
}

Expression CHCSmtLib2Interface::parseInvariants(string const& _result)
{
	vector<Expression> eqs;
	for (auto&& line: _result | ranges::views::split('\n') | ranges::to<vector<string>>())
	{
		if (!boost::starts_with(line, "(define-fun"))
			continue;

		eqs.emplace_back(parseDefineFun(line));
	}

	Expression conj{"and", move(eqs), SortProvider::boolSort};
	return conj;
}

void CHCSmtLib2Interface::declareVariable(string const& _name, SortPointer const& _sort)
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

string CHCSmtLib2Interface::toSmtLibSort(Sort const& _sort)
{
	if (!m_sortNames.count(&_sort))
		m_sortNames[&_sort] = m_smtlib2->toSmtLibSort(_sort);
	return m_sortNames.at(&_sort);
}

string CHCSmtLib2Interface::toSmtLibSort(vector<SortPointer> const& _sorts)
{
	string ssort("(");
	for (auto const& sort: _sorts)
		ssort += toSmtLibSort(*sort) + " ";
	ssort += ")";
	return ssort;
}

void CHCSmtLib2Interface::writeHeader()
{
	if (m_queryTimeout)
		write("(set-option :timeout " + to_string(*m_queryTimeout) + ")");
	write("(set-logic HORN)\n");
}

string CHCSmtLib2Interface::forall()
{
	string vars("(");
	for (auto const& [name, sort]: m_smtlib2->variables())
	{
		solAssert(sort, "");
		if (sort->kind != Kind::Function)
			vars += " (" + name + " " + toSmtLibSort(*sort) + ")";
	}
	vars += ")";
	return vars;
}

void CHCSmtLib2Interface::declareFunction(string const& _name, SortPointer const& _sort)
{
	smtAssert(_sort);
	smtAssert(_sort->kind == Kind::Function);
	// TODO Use domain and codomain as key as well
	if (!m_variables.count(_name))
	{
		auto fSort = dynamic_pointer_cast<FunctionSort>(_sort);
		smtAssert(fSort->codomain);
		string domain = toSmtLibSort(fSort->domain);
		string codomain = toSmtLibSort(*fSort->codomain);
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
	util::h256 inputHash = util::keccak256(_input);
	if (m_queryResponses.count(inputHash))
		return m_queryResponses.at(inputHash);

	if (m_enabledSolvers.smtlib2 && m_smtCallback)
	{
		auto result = m_smtCallback(ReadCallback::kindString(ReadCallback::Kind::SMTQuery), _input);
		if (result.success)
			return result.responseOrErrorMessage;
	}

	if (m_enabledSolvers.eld)
	{
		auto tempDir = boost::filesystem::temp_directory_path();
		auto queryFileName = tempDir / "query.smt2";

		auto queryFile = boost::filesystem::ofstream(queryFileName);
		queryFile << _input;

		auto eldBin = boost::process::search_path("eld");
		if (!eldBin.empty())
		{
			boost::process::ipstream is;
			boost::process::child eld(
				eldBin,
				"-ssol",
				//"-scex",
				"-cex",
				queryFileName,
				boost::process::std_out > is
			);

			vector<string> data;
			string line;
			while (eld.running() && std::getline(is, line))
				if (!line.empty())
					data.push_back(line);

			eld.wait();

			return boost::join(data, "\n");
		}
	}

	m_unhandledQueries.push_back(_input);
	return "unknown\n";
}
