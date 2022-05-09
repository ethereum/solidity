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
	variant<string_view, vector<SMTLib2Expression>> data;

	string toString() const
	{
		return std::visit(GenericVisitor{
			[](string_view const& _sv) { return string{_sv}; },
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
	SMTLib2Parser(string_view const& _data): m_data(_data) {}

	SMTLib2Expression parseExpression()
	{
		skipWhitespace();
		// TODO This does not check if there is trailing data after the closing ')'
		if (token() == '(')
		{
			advance();
			vector<SMTLib2Expression> subExpressions;
			while (token() != 0 && token() != ')')
			{
				subExpressions.emplace_back(parseExpression());
				skipWhitespace();
			}
			if (token() == ')')
				advance();
			return {subExpressions};
		}
		else
			return {parseToken()};
	}

	string_view remainingInput() const
	{
		return m_data.substr(m_pos);
	}

private:
	string_view parseToken()
	{
		skipWhitespace();
		size_t start = m_pos;
		while (m_pos < m_data.size())
		{
			char c = token();
			if (langutil::isWhiteSpace(c) || c == '(' || c == ')')
				break;
			advance();
		}
		return m_data.substr(start, m_pos - start);
	}

	void skipWhitespace()
	{
		while (isWhiteSpace(token()))
			advance();
	}

	char token()
	{
		return m_pos < m_data.size() ? m_data[m_pos] : 0;
	}
	void advance() { m_pos++;}

	size_t m_pos = 0;
	string_view const m_data;
};

namespace
{

string_view command(SMTLib2Expression const& _expr)
{
	vector<SMTLib2Expression> const& items = get<vector<SMTLib2Expression>>(_expr.data);
	solAssert(!items.empty());
	solAssert(holds_alternative<string_view>(items.front().data));
	return get<string_view>(items.front().data);
}

// TODO If we want to return rational here, we need smtutil::Expression to support rationals...
u256 parseRational(string_view _atom)
{
	if (_atom.size() >= 3 && _atom.at(_atom.size() - 1) == '0' && _atom.at(_atom.size() - 2) == '.')
		return parseRational(_atom.substr(0, _atom.size() - 2));
	else
		return u256(_atom);
}

smtutil::Expression toSMTUtilExpression(SMTLib2Expression const& _expr)
{
	return std::visit(GenericVisitor{
		[](string_view const& _atom) {
			if (isDigit(_atom.front()) || _atom.front() == '.')
				return Expression(parseRational(_atom));
			else
				// TODO should be real, but internally, we use ints.
				return Expression(string(_atom), {}, SortProvider::intSort());
		},
		[&](vector<SMTLib2Expression> const& _subExpr) {
			set<string> boolOperators{"and", "or", "not", "=", "<", ">", "<=", ">=", "=>"};
			vector<smtutil::Expression> arguments;
			for (size_t i = 1; i < _subExpr.size(); i++)
				arguments.emplace_back(toSMTUtilExpression(_subExpr[i]));
			string_view op = get<string_view>(_subExpr.front().data);
			return Expression(
				string(op),
				move(arguments),
				contains(boolOperators, op) ?
				SortProvider::boolSort :
				SortProvider::intSort()
			);
		}
	}, _expr.data);
}

}

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		cout << "Usage: solsmt <smtlib2 fil>" << endl;
		return -1;
	}

	string input = readFileAsString(argv[1]);
	string_view inputToParse = input;

	BooleanLPSolver solver;
	while (!inputToParse.empty())
	{
		//cout << line << endl;
		SMTLib2Parser parser(inputToParse);
		//cout << " -> " << parser.parseExpression().toString() << endl;
		SMTLib2Expression expr = parser.parseExpression();
		inputToParse = parser.remainingInput();
		vector<SMTLib2Expression> const& items = get<vector<SMTLib2Expression>>(expr.data);
		string_view cmd = command(expr);
		if (cmd == "set-info")
			continue; // ignore
		else if (cmd == "declare-fun")
		{
			solAssert(items.size() == 4);
			string variableName = string{get<string_view>(items[1].data)};
			solAssert(get<vector<SMTLib2Expression>>(items[2].data).empty());
			string_view type = get<string_view>(items[3].data);
			solAssert(type == "Real" || type == "Bool");
			// TODO should be real, but we call it int...
			SortPointer sort = type == "Real" ? SortProvider::intSort() : SortProvider::boolSort;
			solver.declareVariable(variableName, move(sort));
		}
		else if (cmd == "assert")
		{
			solAssert(items.size() == 2);
			solver.addAssertion(toSMTUtilExpression(items[1]));
		}
		else if (cmd == "set-logic")
		{
			// ignore - could check the actual logic.
		}
		else if (cmd == "check-sat")
		{
			auto&& [result, model] = solver.check({});
			if (result == CheckResult::SATISFIABLE)
				cout << "(sat)" << endl;
			else if (result == CheckResult::UNSATISFIABLE)
				cout << "(unsat)" << endl;
			else
				cout << "(unknown)" << endl;
		}
		else if (cmd == "exit")
			return 0;
		else
			solAssert(false, "Unknown instruction: " + string(cmd));
	}

	return 0;
}
