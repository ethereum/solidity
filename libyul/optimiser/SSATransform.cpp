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
 * Optimiser component that turns subsequent assignments to variable declarations
 * and assignments.
 */

#include <libyul/optimiser/SSATransform.h>

#include <libyul/optimiser/NameCollector.h>
#include <libyul/optimiser/NameDispenser.h>
#include <libyul/AST.h>

#include <libsolutil/CommonData.h>

using namespace solidity;
using namespace solidity::yul;
using namespace solidity::langutil;

namespace
{

/**
 * First step of SSA transform: Introduces new SSA variables for each assignment or
 * declaration of a variable to be replaced.
 */
class IntroduceSSA: public ASTModifier
{
public:
	explicit IntroduceSSA(
		NameDispenser& _nameDispenser,
		std::set<YulName> const& _variablesToReplace
	):
		m_nameDispenser(_nameDispenser),
		m_variablesToReplace(_variablesToReplace)
	{ }

	void operator()(Block& _block) override;

private:
	NameDispenser& m_nameDispenser;
	std::set<YulName> const& m_variablesToReplace;
};


void IntroduceSSA::operator()(Block& _block)
{
	util::iterateReplacing(
		_block.statements,
		[&](Statement& _s) -> std::optional<std::vector<Statement>>
		{
			if (std::holds_alternative<VariableDeclaration>(_s))
			{
				VariableDeclaration& varDecl = std::get<VariableDeclaration>(_s);
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
				langutil::DebugData::ConstPtr debugData = varDecl.debugData;
				std::vector<Statement> statements;
				statements.emplace_back(VariableDeclaration{debugData, {}, std::move(varDecl.value)});
				NameWithDebugDataList newVariables;
				for (auto const& var: varDecl.variables)
				{
					YulName oldName = var.name;
					YulName newName = m_nameDispenser.newName(oldName);
					newVariables.emplace_back(NameWithDebugData{debugData, newName});
					statements.emplace_back(VariableDeclaration{
						debugData,
						{NameWithDebugData{debugData, oldName}},
						std::make_unique<Expression>(Identifier{debugData, newName})
					});
				}
				std::get<VariableDeclaration>(statements.front()).variables = std::move(newVariables);
				return { std::move(statements) };
			}
			else if (std::holds_alternative<Assignment>(_s))
			{
				Assignment& assignment = std::get<Assignment>(_s);
				visit(*assignment.value);
				for (auto const& var: assignment.variableNames)
					assertThrow(m_variablesToReplace.count(var.name), OptimizerException, "");

				// Replace "a := v" by "let a_1 := v  a := v"
				// Replace "a, b := v" by "let a_1, b_1 := v  a := a_1 b := b_2"
				langutil::DebugData::ConstPtr debugData = assignment.debugData;
				std::vector<Statement> statements;
				statements.emplace_back(VariableDeclaration{debugData, {}, std::move(assignment.value)});
				NameWithDebugDataList newVariables;
				for (auto const& var: assignment.variableNames)
				{
					YulName oldName = var.name;
					YulName newName = m_nameDispenser.newName(oldName);
					newVariables.emplace_back(NameWithDebugData{debugData, newName});
					statements.emplace_back(Assignment{
						debugData,
						{Identifier{debugData, oldName}},
						std::make_unique<Expression>(Identifier{debugData, newName})
					});
				}
				std::get<VariableDeclaration>(statements.front()).variables = std::move(newVariables);
				return { std::move(statements) };
			}
			else
				visit(_s);
			return {};
		}
	);
}

/**
 * Second step of SSA transform: Introduces new SSA variables at each control-flow join
 * and at the beginning of functions.
 */
class IntroduceControlFlowSSA: public ASTModifier
{
public:
	explicit IntroduceControlFlowSSA(
		NameDispenser& _nameDispenser,
		std::set<YulName> const& _variablesToReplace
	):
		m_nameDispenser(_nameDispenser),
		m_variablesToReplace(_variablesToReplace)
	{ }

	void operator()(FunctionDefinition& _function) override;
	void operator()(ForLoop& _forLoop) override;
	void operator()(Switch& _switch) override;
	void operator()(Block& _block) override;

private:
	NameDispenser& m_nameDispenser;
	std::set<YulName> const& m_variablesToReplace;
	/// Variables (that are to be replaced) currently in scope.
	std::set<YulName> m_variablesInScope;
	/// Variables that do not have a specific value.
	util::UniqueVector<YulName> m_variablesToReassign;
};

void IntroduceControlFlowSSA::operator()(FunctionDefinition& _function)
{
	std::set<YulName> varsInScope;
	std::swap(varsInScope, m_variablesInScope);
	util::UniqueVector<YulName> toReassign;
	std::swap(toReassign, m_variablesToReassign);

	for (auto const& param: _function.parameters)
		if (m_variablesToReplace.count(param.name))
		{
			m_variablesInScope.insert(param.name);
			m_variablesToReassign.pushBack(param.name);
		}

	ASTModifier::operator()(_function);

	m_variablesInScope = std::move(varsInScope);
	m_variablesToReassign = std::move(toReassign);
}

void IntroduceControlFlowSSA::operator()(ForLoop& _for)
{
	yulAssert(_for.pre.statements.empty(), "For loop init rewriter not run.");

	for (auto const& var: assignedVariableNames(_for.body) + assignedVariableNames(_for.post))
		if (util::contains(m_variablesInScope,var))
			m_variablesToReassign.pushBack(var);

	(*this)(_for.body);
	(*this)(_for.post);
}

void IntroduceControlFlowSSA::operator()(Switch& _switch)
{
	yulAssert(m_variablesToReassign.empty(), "");

	util::UniqueVector<YulName> toReassign;
	for (auto& c: _switch.cases)
	{
		(*this)(c.body);
		toReassign.pushBack(m_variablesToReassign);
	}

	m_variablesToReassign.pushBack(toReassign);
}

void IntroduceControlFlowSSA::operator()(Block& _block)
{
	util::UniqueVector<YulName> variablesDeclaredHere;
	util::UniqueVector<YulName> assignedVariables;

	util::iterateReplacing(
		_block.statements,
		[&](Statement& _s) -> std::optional<std::vector<Statement>>
		{
			std::vector<Statement> toPrepend;
			for (YulName toReassign: m_variablesToReassign)
			{
				YulName newName = m_nameDispenser.newName(toReassign);
				toPrepend.emplace_back(VariableDeclaration{
					debugDataOf(_s),
					{NameWithDebugData{debugDataOf(_s), newName}},
					std::make_unique<Expression>(Identifier{debugDataOf(_s), toReassign})
				});
				assignedVariables.pushBack(toReassign);
			}
			m_variablesToReassign.clear();

			if (std::holds_alternative<VariableDeclaration>(_s))
			{
				VariableDeclaration& varDecl = std::get<VariableDeclaration>(_s);
				for (auto const& var: varDecl.variables)
					if (m_variablesToReplace.count(var.name))
					{
						variablesDeclaredHere.pushBack(var.name);
						m_variablesInScope.insert(var.name);
					}
			}
			else if (std::holds_alternative<Assignment>(_s))
			{
				Assignment& assignment = std::get<Assignment>(_s);
				for (auto const& var: assignment.variableNames)
					if (m_variablesToReplace.count(var.name))
						assignedVariables.pushBack(var.name);
			}
			else
				visit(_s);

			if (toPrepend.empty())
				return {};
			else
			{
				toPrepend.emplace_back(std::move(_s));
				return {std::move(toPrepend)};
			}
		}
	);

	m_variablesToReassign.pushBack(assignedVariables);
	m_variablesInScope -= variablesDeclaredHere.contents();
	m_variablesToReassign.removeAll(variablesDeclaredHere.contents());
}

/**
 * Third step of SSA transform: Replace the references to variables-to-be-replaced
 * by their current values.
 */
class PropagateValues: public ASTModifier
{
public:
	explicit PropagateValues(std::set<YulName> const& _variablesToReplace):
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
	std::set<YulName> const& m_variablesToReplace;
	std::map<YulName, YulName> m_currentVariableValues;
	std::set<YulName> m_clearAtEndOfBlock;
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

	YulName variable = _varDecl.variables.front().name;
	if (m_variablesToReplace.count(variable))
	{
		// `let a := a_1` - regular declaration of non-SSA variable
		yulAssert(std::holds_alternative<Identifier>(*_varDecl.value), "");
		m_currentVariableValues[variable] = std::get<Identifier>(*_varDecl.value).name;
		m_clearAtEndOfBlock.insert(variable);
	}
	else if (_varDecl.value && std::holds_alternative<Identifier>(*_varDecl.value))
	{
		// `let a_1 := a` - assignment to SSA variable after a branch.
		YulName value = std::get<Identifier>(*_varDecl.value).name;
		if (m_variablesToReplace.count(value))
		{
			// This is safe because `a_1` is not a "variable to replace" and thus
			// will not be re-assigned.
			m_currentVariableValues[value] = variable;
			m_clearAtEndOfBlock.insert(value);
		}
	}
}


void PropagateValues::operator()(Assignment& _assignment)
{
	visit(*_assignment.value);

	if (_assignment.variableNames.size() != 1)
		return;
	YulName name = _assignment.variableNames.front().name;
	if (!m_variablesToReplace.count(name))
		return;

	yulAssert(_assignment.value && std::holds_alternative<Identifier>(*_assignment.value), "");
	m_currentVariableValues[name] = std::get<Identifier>(*_assignment.value).name;
	m_clearAtEndOfBlock.insert(name);
}

void PropagateValues::operator()(ForLoop& _for)
{
	yulAssert(_for.pre.statements.empty(), "For loop init rewriter not run.");

	for (auto const& var: assignedVariableNames(_for.body) + assignedVariableNames(_for.post))
		m_currentVariableValues.erase(var);

	visit(*_for.condition);
	(*this)(_for.body);
	(*this)(_for.post);
}

void PropagateValues::operator()(Block& _block)
{
	std::set<YulName> clearAtParentBlock = std::move(m_clearAtEndOfBlock);
	m_clearAtEndOfBlock.clear();

	ASTModifier::operator()(_block);

	for (auto const& var: m_clearAtEndOfBlock)
		m_currentVariableValues.erase(var);

	m_clearAtEndOfBlock = std::move(clearAtParentBlock);
}

}

void SSATransform::run(OptimiserStepContext& _context, Block& _ast)
{
	std::set<YulName> assignedVariables = assignedVariableNames(_ast);
	IntroduceSSA{_context.dispenser, assignedVariables}(_ast);
	IntroduceControlFlowSSA{_context.dispenser, assignedVariables}(_ast);
	PropagateValues{assignedVariables}(_ast);
}


