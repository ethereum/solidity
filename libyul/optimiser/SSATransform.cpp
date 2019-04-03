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
/**
 * Optimiser component that turns subsequent assignments to variable declarations
 * and assignments.
 */

#include <libyul/optimiser/SSATransform.h>

#include <libyul/optimiser/NameCollector.h>
#include <libyul/optimiser/NameDispenser.h>
#include <libyul/AsmData.h>

#include <libdevcore/CommonData.h>

using namespace std;
using namespace dev;
using namespace langutil;
using namespace yul;

void SSATransform::operator()(Identifier& _identifier)
{
	if (m_currentVariableValues.count(_identifier.name))
		_identifier.name = m_currentVariableValues[_identifier.name];
}

void SSATransform::operator()(ForLoop& _for)
{
	// This will clear the current value in case of a reassignment inside the
	// init part, although the new variable would still be in scope inside the whole loop.
	// This small inefficiency is fine if we move the pre part of all for loops out
	// of the for loop.
	(*this)(_for.pre);

	Assignments assignments;
	assignments(_for.body);
	assignments(_for.post);
	for (auto const& var: assignments.names())
		m_currentVariableValues.erase(var);

	visit(*_for.condition);
	(*this)(_for.body);
	(*this)(_for.post);
}


void SSATransform::operator()(Block& _block)
{
	set<YulString> variablesToClearAtEnd;

	// Creates a new variable and stores it in the current variable value map.
	auto newVariable = [&](YulString _varName) -> YulString
	{
		YulString newName = m_nameDispenser.newName(_varName);
		m_currentVariableValues[_varName] = newName;
		variablesToClearAtEnd.emplace(_varName);
		return newName;
	};

	iterateReplacing(
		_block.statements,
		[&](Statement& _s) -> boost::optional<vector<Statement>>
		{
			if (_s.type() == typeid(VariableDeclaration))
			{
				VariableDeclaration& varDecl = boost::get<VariableDeclaration>(_s);
				if (varDecl.value)
					visit(*varDecl.value);

				bool needToReplaceSome = false;
				for (auto const& var: varDecl.variables)
					if (m_variablesToReplace.count(var.name))
						needToReplaceSome = true;
				if (!needToReplaceSome)
					return {};

				// Replace "let a := v" by "let a_1 := v  let a := a_1"
				// Replace "let a, b := v" by "let a_1, b_1 := v  let a := a_1 let b := b_2"
				auto loc = varDecl.location;
				vector<Statement> statements;
				statements.emplace_back(VariableDeclaration{loc, {}, std::move(varDecl.value)});
				TypedNameList newVariables;
				for (auto const& var: varDecl.variables)
				{
					YulString newName = newVariable(var.name);
					YulString oldName = var.name;
					newVariables.emplace_back(TypedName{loc, newName, {}});
					statements.emplace_back(VariableDeclaration{
						loc,
						{TypedName{loc, oldName, {}}},
						make_unique<Expression>(Identifier{loc, newName})
					});
				}
				boost::get<VariableDeclaration>(statements.front()).variables = std::move(newVariables);
				return std::move(statements);
			}
			else if (_s.type() == typeid(Assignment))
			{
				Assignment& assignment = boost::get<Assignment>(_s);
				visit(*assignment.value);
				for (auto const& var: assignment.variableNames)
					assertThrow(m_variablesToReplace.count(var.name), OptimizerException, "");

				// Replace "a := v" by "let a_1 := v  a := v"
				// Replace "a, b := v" by "let a_1, b_1 := v  a := a_1 b := b_2"
				auto loc = assignment.location;
				vector<Statement> statements;
				statements.emplace_back(VariableDeclaration{loc, {}, std::move(assignment.value)});
				TypedNameList newVariables;
				for (auto const& var: assignment.variableNames)
				{
					YulString newName = newVariable(var.name);
					YulString oldName = var.name;
					newVariables.emplace_back(TypedName{loc, newName, {}});
					statements.emplace_back(Assignment{
						loc,
						{Identifier{loc, oldName}},
						make_unique<Expression>(Identifier{loc, newName})
					});
				}
				boost::get<VariableDeclaration>(statements.front()).variables = std::move(newVariables);
				return std::move(statements);
			}
			else
				visit(_s);
			return {};
		}
	);
	for (auto const& var: variablesToClearAtEnd)
		m_currentVariableValues.erase(var);
}

void SSATransform::run(Block& _ast, NameDispenser& _nameDispenser)
{
	Assignments assignments;
	assignments(_ast);
	SSATransform{_nameDispenser, assignments.names()}(_ast);
}

