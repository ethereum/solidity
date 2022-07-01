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

#include <libsolutil/BooleanLP.h>
#include <libsolutil/Visitor.h>
#include <liblangutil/Common.h>
#include <libsolutil/CommonIO.h>

#include <variant>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::langutil;
using namespace solidity::smtutil;

struct SMTLib2Expression
{
	variant<string, vector<SMTLib2Expression>> data;

	string toString() const
	{
		return std::visit(GenericVisitor{
			[](string const& _sv) { return string{_sv}; },
			[](vector<SMTLib2Expression> const& _subExpr) {
				vector<string> formatted;
				for (auto const& item: _subExpr)
					formatted.emplace_back(item.toString());
				return "(" + joinHumanReadable(formatted, " ") + ")";
			}
		}, data);
	}
};

class SMTLib2Parser
{
public:
	SMTLib2Parser(istream& _input):
		m_input(_input),
		m_token(static_cast<char>(m_input.get()))
	{}

	SMTLib2Expression parseExpression()
	{
		skipWhitespace();
		if (token() == '(')
		{
			advance();
			skipWhitespace();
			vector<SMTLib2Expression> subExpressions;
			while (token() != 0 && token() != ')')
			{
				subExpressions.emplace_back(parseExpression());
				skipWhitespace();
			}
			solAssert(token() == ')');
			// simulate whitespace because we do not want to read the next token
			// since it might block.
			m_token = ' ';
			return {move(subExpressions)};
		}
		else
			return {parseToken()};
	}

	bool isEOF()
	{
		skipWhitespace();
		return m_input.eof();
	}

private:
	string parseToken()
	{
		string result;

		skipWhitespace();
		bool isPipe = token() == '|';
		if (isPipe)
			advance();
		while (token() != 0)
		{
			char c = token();
			if (isPipe && c == '|')
			{
				advance();
				break;
			}
			else if (!isPipe && (langutil::isWhiteSpace(c) || c == '(' || c == ')'))
				break;
			result.push_back(c);
			advance();
		}
		return result;
	}

	void skipWhitespace()
	{
		while (isWhiteSpace(token()))
			advance();
	}

	char token() const
	{
		return m_token;
	}

	void advance()
	{
		m_token = static_cast<char>(m_input.get());
		if (token() == ';')
			while (token() != '\n' && token() != 0)
				m_token = static_cast<char>(m_input.get());
	}

	istream& m_input;
	char m_token = 0;
};

namespace
{

string const& command(SMTLib2Expression const& _expr)
{
	vector<SMTLib2Expression> const& items = get<vector<SMTLib2Expression>>(_expr.data);
	solAssert(!items.empty());
	solAssert(holds_alternative<string>(items.front().data));
	return get<string>(items.front().data);
}

namespace
{
bool isNumber(string const& _expr)
{
	for (char c: _expr)
		if (!isDigit(c) && c != '.')
			return false;
	return true;
}
}

smtutil::Expression toSMTUtilExpression(SMTLib2Expression const& _expr, map<string, SortPointer>& _variableSorts)
{
	return std::visit(GenericVisitor{
		[&](string const& _atom) {
			if (_atom == "true" || _atom == "false")
				return Expression(_atom == "true");
			else if (isNumber(_atom))
				return Expression(_atom, {}, SortProvider::realSort);
			else
				return Expression(_atom, {}, _variableSorts.at(_atom));
		},
		[&](vector<SMTLib2Expression> const& _subExpr) {
			SortPointer sort;
			vector<smtutil::Expression> arguments;
			string const& op = get<string>(_subExpr.front().data);
			if (op == "let")
			{
				map<string, SortPointer> subSorts;
				solAssert(_subExpr.size() == 3);
				// We change the nesting here:
				// (let ((x1 t1) (x2 t2)) T) -> let(x1(t1), x2(t2), T)
				for (auto const& binding: get<vector<SMTLib2Expression>>(_subExpr.at(1).data))
				{
					auto const& bindingElements = get<vector<SMTLib2Expression>>(binding.data);
					solAssert(bindingElements.size() == 2);
					string const& varName = get<string>(bindingElements.at(0).data);
					Expression replacement = toSMTUtilExpression(bindingElements.at(1), _variableSorts);
#ifdef DEBUG
					cerr << "Binding " << varName << " to " << replacement.toString() << endl;
#endif
					subSorts[varName] = replacement.sort;
					arguments.emplace_back(Expression(varName, {move(replacement)}, replacement.sort));
				}
				for (auto&& [name, value]: subSorts)
					_variableSorts[name] = value;
				arguments.emplace_back(toSMTUtilExpression(_subExpr.at(2), _variableSorts));
				for (auto const& var: subSorts)
					_variableSorts.erase(var.first);
				sort = arguments.back().sort;
			}
			else
			{
				set<string> boolOperators{"and", "or", "not", "=", "<", ">", "<=", ">=", "=>"};
				for (size_t i = 1; i < _subExpr.size(); i++)
					arguments.emplace_back(toSMTUtilExpression(_subExpr[i], _variableSorts));
				sort =
					contains(boolOperators, op) ?
					SortProvider::boolSort :
					arguments.back().sort;
			}
			return Expression(op, move(arguments), move(sort));
		}
	}, _expr.data);
}

class SolSMT
{
public:
	SolSMT(istream& _input): m_input(_input) {}
	void run();
private:

	istream& m_input;
	bool m_doExit = false;
	bool m_printSuccess = false;
	map<string, SortPointer> m_variableSorts;
	BooleanLPSolver m_solver;
};

void SolSMT::run()
{
	SMTLib2Parser parser(m_input);

	while (!m_doExit && !parser.isEOF())
	{
		SMTLib2Expression expr = parser.parseExpression();
#ifdef DEBUG
		cerr << " -> " << expr.toString() << endl;
#endif
		vector<SMTLib2Expression> const& items = get<vector<SMTLib2Expression>>(expr.data);
		string const& cmd = command(expr);
		if (cmd == "set-info")
		{
			// ignore
		}
		else if (cmd == "set-option")
		{
			solAssert(items.size() >= 2);
			string const& option = get<string>(items[1].data);
			if (option == ":print-success")
				m_printSuccess = (get<string>(items[2].data) == "true");
//			else if (option == ":produce-models")
//				produceModels = (get<string>(items[2].data) == "true");
			// ignore the rest
		}
		else if (cmd == "declare-fun")
		{
			solAssert(items.size() == 4);
			string variableName = string{get<string>(items[1].data)};
			solAssert(get<vector<SMTLib2Expression>>(items[2].data).empty());
			string const& type = get<string>(items[3].data);
			solAssert(type == "Real" || type == "Bool");
			SortPointer sort = type == "Real" ? SortProvider::realSort : SortProvider::boolSort;
			m_variableSorts[variableName] = sort;
			m_solver.declareVariable(variableName, move(sort));
		}
		else if (cmd == "define-fun")
		{
#ifdef DEBUG
			cerr << "Ignoring 'define-fun'" << endl;
#endif
		}
		else if (cmd == "assert")
		{
			solAssert(items.size() == 2);
			m_solver.addAssertion(toSMTUtilExpression(items[1], m_variableSorts));
		}
		else if (cmd == "push")
		{
			// TODO what is the meaning of the numeric argument?
			solAssert(items.size() == 2);
			m_solver.push();
		}
		else if (cmd == "pop")
		{
			// TODO what is the meaning of the numeric argument?
			solAssert(items.size() == 2);
			m_solver.pop();
		}
		else if (cmd == "set-logic")
		{
			// ignore - could check the actual logic.
		}
		else if (cmd == "check-sat")
		{
			auto&& [result, model] = m_solver.check({});
			if (result == CheckResult::SATISFIABLE)
				cout << "sat" << endl;
			else if (result == CheckResult::UNSATISFIABLE)
				cout << "unsat" << endl;
			else
				cout << "unknown" << endl;
			// do not print "success"
			continue;
		}
		else if (cmd == "exit")
			m_doExit = true;
		else
			solAssert(false, "Unknown instruction: " + string(cmd));
		if (m_printSuccess)
			cout << "success" << endl;
	}
}

}

int main(int argc, char** argv)
{
	if (argc != 2 && argc != 1)
	{
		cout << "Usage: solsmt [<smtlib2 file>]" << endl;
		return -1;
	}

	optional<ifstream> input;
	if (argc == 2)
		input = ifstream(argv[1]);
	SolSMT{argc == 1 ? cin : *input}.run();

	return 0;
}
