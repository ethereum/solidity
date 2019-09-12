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

namespace
{

/**
 * First step of SSA transform: Introduces new SSA variables for each assignment or
 * declaration of a variable to be replaced.
 */
class IntroduceSSA: public ASTModifier
{
public:
	explicit IntroduceSSA(NameDispenser& _nameDispenser, set<YulString> const& _variablesToReplace):
		m_nameDispenser(_nameDispenser), m_variablesToReplace(_variablesToReplace)
	{ }

	void operator()(Block& _block) override;

private:
	NameDispenser& m_nameDispenser;
	/// This is a set of all variables that are assigned to anywhere in the code.
	/// Variables that are only declared but never re-assigned are not touched.
	set<YulString> const& m_variablesToReplace;
};


void IntroduceSSA::operator()(Block& _block)
{
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
					YulString oldName = var.name;
					YulString newName = m_nameDispenser.newName(oldName);
					newVariables.emplace_back(TypedName{loc, newName, {}});
					statements.emplace_back(VariableDeclaration{
						loc,
						{TypedName{loc, oldName, {}}},
						make_unique<Expression>(Identifier{loc, newName})
					});
				}
				boost::get<VariableDeclaration>(statements.front()).variables = std::move(newVariables);
				return { std::move(statements) };
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
					YulString oldName = var.name;
					YulString newName = m_nameDispenser.newName(oldName);
					newVariables.emplace_back(TypedName{loc, newName, {}});
					statements.emplace_back(Assignment{
						loc,
						{Identifier{loc, oldName}},
						make_unique<Expression>(Identifier{loc, newName})
					});
				}
				boost::get<VariableDeclaration>(statements.front()).variables = std::move(newVariables);
				return { std::move(statements) };
			}
			else
				visit(_s);
			return {};
		}
	);
}

/**
 * Second step of SSA transform: Replace the references to variables-to-be-replaced
 * by their current values.
 */
class PropagateValues: public ASTModifier
{
public:
	explicit PropagateValues(set<YulString> const& _variablesToReplace):
		m_variablesToReplace(_variablesToReplace)
	{ }

	void operator()(Identifier& _identifier) override;
	void operator()(VariableDeclaration& _varDecl) override;
	void operator()(Assignment& _assignment) override;
	void operator()(ForLoop& _for) override;
	void operator()(Block& _block) override;

private:
	/// This is a set of all variables that are assigned to anywhere in the code.
	/// Variables that are only declared but never re-assigned are not touched.
	set<YulString> const& m_variablesToReplace;
	map<YulString, YulString> m_currentVariableValues;
	set<YulString> m_clearAtEndOfBlock;
};

void PropagateValues::operator()(Identifier& _identifier)
{
	if (m_currentVariableValues.count(_identifier.name))
		_identifier.name = m_currentVariableValues[_identifier.name];
}

void PropagateValues::operator()(VariableDeclaration& _varDecl)
{
	ASTModifier::operator()(_varDecl);

	if (_varDecl.variables.size() != 1)
		return;
	YulString name = _varDecl.variables.front().name;
	if (!m_variablesToReplace.count(name))
		return;

	yulAssert(_varDecl.value->type() == typeid(Identifier), "");
	m_currentVariableValues[name] = boost::get<Identifier>(*_varDecl.value).name;
	m_clearAtEndOfBlock.insert(name);
}


void PropagateValues::operator()(Assignment& _assignment)
{
	visit(*_assignment.value);

	if (_assignment.variableNames.size() != 1)
		return;
	YulString name = _assignment.variableNames.front().name;
	if (!m_variablesToReplace.count(name))
		return;

	yulAssert(_assignment.value->type() == typeid(Identifier), "");
	m_currentVariableValues[name] = boost::get<Identifier>(*_assignment.value).name;
	m_clearAtEndOfBlock.insert(name);
}

void PropagateValues::operator()(ForLoop& _for)
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

void PropagateValues::operator()(Block& _block)
{
	set<YulString> clearAtParentBlock = std::move(m_clearAtEndOfBlock);
	m_clearAtEndOfBlock.clear();

	ASTModifier::operator()(_block);

	for (auto const& var: m_clearAtEndOfBlock)
		m_currentVariableValues.erase(var);

	m_clearAtEndOfBlock = std::move(clearAtParentBlock);
}

}

void SSATransform::run(Block& _ast, NameDispenser& _nameDispenser)
{
	Assignments assignments;
	assignments(_ast);
	IntroduceSSA{_nameDispenser, assignments.names()}(_ast);
	PropagateValues{assignments.names()}(_ast);
}


