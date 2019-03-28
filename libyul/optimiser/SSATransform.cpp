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

	// Creates a new variable (and returns its declaration) with value _value
	// and replaces _value by a reference to that new variable.

	auto replaceByNew = [&](SourceLocation _loc, YulString _varName, YulString _type, unique_ptr<Expression>& _value) -> VariableDeclaration
	{
		YulString newName = m_nameDispenser.newName(_varName);
		m_currentVariableValues[_varName] = newName;
		variablesToClearAtEnd.emplace(_varName);
		unique_ptr<Expression> v = make_unique<Expression>(Identifier{_loc, newName});
		_value.swap(v);
		return VariableDeclaration{_loc, {TypedName{_loc, std::move(newName), std::move(_type)}}, std::move(v)};
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
				if (varDecl.variables.size() != 1 || !m_variablesToReplace.count(varDecl.variables.front().name))
					return {};
				vector<Statement> v;
				// Replace "let a := v" by "let a_1 := v  let a := a_1"
				v.emplace_back(replaceByNew(
					varDecl.location,
					varDecl.variables.front().name,
					varDecl.variables.front().type,
					varDecl.value
				));
				v.emplace_back(move(varDecl));
				return std::move(v);
			}
			else if (_s.type() == typeid(Assignment))
			{
				Assignment& assignment = boost::get<Assignment>(_s);
				visit(*assignment.value);
				if (assignment.variableNames.size() != 1)
					return {};
				assertThrow(m_variablesToReplace.count(assignment.variableNames.front().name), OptimizerException, "");
				vector<Statement> v;
				// Replace "a := v" by "let a_1 := v  a := v"
				v.emplace_back(replaceByNew(
					assignment.location,
					assignment.variableNames.front().name,
					{}, // TODO determine type
					assignment.value
				));
				v.emplace_back(move(assignment));
				return std::move(v);
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

