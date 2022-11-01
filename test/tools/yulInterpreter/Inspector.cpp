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
/**
 * Yul interpreter.
 */

#include <test/tools/yulInterpreter/Inspector.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string.hpp>

using namespace solidity;
using namespace solidity::yul;
using namespace solidity::yul::test;

using namespace std;

namespace
{

void printVariable(YulString const& _name, u256 const& _value)
{
	cout << "\t" << _name.str() << " = " << _value.str();

	if (_value != 0)
		cout << " (" << toCompactHexWithPrefix(_value) << ")";

	cout << endl;
}

}

void InspectedInterpreter::run(
	std::shared_ptr<Inspector> _inspector,
	InterpreterState& _state,
	Dialect const& _dialect,
	Block const& _ast,
	bool _disableExternalCalls,
	bool _disableMemoryTrace
)
{
	Scope scope;
	InspectedInterpreter{_inspector, _state, _dialect, scope, _disableExternalCalls, _disableMemoryTrace}(_ast);
}

Inspector::NodeAction Inspector::queryUser(DebugData const& _data, map<YulString, u256> const& _variables)
{
	if (m_stepMode == NodeAction::RunNode)
	{
		// Output instructions that are being skipped/run
		cout << "Running " << currentSource(_data) << endl;

		return NodeAction::StepThroughNode;
	}

	string input;

	while (true)
	{
		// Output sourcecode about to run.
		cout << "> " << currentSource(_data) << endl;

		// Ask user for action
		cout << endl
			<< "(s)tep/(n)ext/(i)nspect/(p)rint/all (v)ariables?"
			<< endl
			<< "# ";

		cout.flush();

		getline(cin, input);
		boost::algorithm::trim(input);

		// Imitate GDB and repeat last cmd for empty string input.
		if (input == "")
			input = m_lastInput;
		else
			m_lastInput = input;

		if (input == "next" || input == "n")
			return NodeAction::RunNode;
		else if (input == "step" || input == "s")
			return NodeAction::StepThroughNode;
		else if (input == "inspect" || input == "i")
			m_state.dumpTraceAndState(cout, false);
		else if (input == "variables" || input == "v")
		{
			for (auto &&[yulStr, val]: _variables)
				printVariable(yulStr, val);
			cout << endl;
		}
		else if (
			boost::starts_with(input, "print") ||
			boost::starts_with(input, "p")
		)
		{
			size_t whitespacePos = input.find(' ');

			if (whitespacePos == string::npos)
				cout << "Error parsing command! Expected variable name." << endl;

			string const varname = input.substr(whitespacePos + 1);

			vector<string> candidates;

			bool found = false;
			for (auto &&[yulStr, val]: _variables)
				if (yulStr.str() == varname)
				{
					printVariable(yulStr, val);
					found = true;
					break;
				}

			if (!found)
				cout << varname << " not found." << endl;
		}
	}
}

std::string Inspector::currentSource(DebugData const& _data) const
{
	return m_source.substr(
		static_cast<size_t>(_data.nativeLocation.start),
		static_cast<size_t>(_data.nativeLocation.end - _data.nativeLocation.start)
	);
}

u256 InspectedInterpreter::evaluate(Expression const& _expression)
{
	InspectedExpressionEvaluator ev(m_inspector, m_state, m_dialect, *m_scope, m_variables, m_disableExternalCalls, m_disableMemoryTrace);
	ev.visit(_expression);
	return ev.value();
}

std::vector<u256> InspectedInterpreter::evaluateMulti(Expression const& _expression)
{
	InspectedExpressionEvaluator ev(m_inspector, m_state, m_dialect, *m_scope, m_variables, m_disableExternalCalls, m_disableMemoryTrace);
	ev.visit(_expression);
	return ev.values();
}
