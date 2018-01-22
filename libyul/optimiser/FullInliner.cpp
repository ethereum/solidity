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
 * Optimiser component that performs function inlining for arbitrary functions.
 */

#include <libyul/optimiser/FullInliner.h>

#include <libyul/optimiser/ASTCopier.h>
#include <libyul/optimiser/ASTWalker.h>
#include <libyul/optimiser/NameCollector.h>
#include <libyul/optimiser/Utilities.h>
#include <libyul/optimiser/Metrics.h>
#include <libyul/optimiser/SSAValueTracker.h>
#include <libyul/Exceptions.h>

#include <libsolidity/inlineasm/AsmData.h>

#include <libdevcore/CommonData.h>
#include <libdevcore/Visitor.h>

#include <boost/range/adaptor/reversed.hpp>

using namespace std;
using namespace dev;
using namespace dev::yul;
using namespace dev::solidity;

FullInliner::FullInliner(Block& _ast):
	m_ast(_ast)
{
	assertThrow(m_ast.statements.size() >= 1, OptimizerException, "");
	assertThrow(m_ast.statements.front().type() == typeid(Block), OptimizerException, "");
	m_nameDispenser.m_usedNames = NameCollector(m_ast).names();

	// Determine constants
	SSAValueTracker tracker;
	tracker(m_ast);
	for (auto const& ssaValue: tracker.values())
		if (ssaValue.second && ssaValue.second->type() == typeid(Literal))
			m_constants.insert(ssaValue.first);

	map<string, size_t> references = ReferencesCounter::countReferences(m_ast);
	for (size_t i = 1; i < m_ast.statements.size(); ++i)
	{
		assertThrow(m_ast.statements.at(i).type() == typeid(FunctionDefinition), OptimizerException, "");
		FunctionDefinition& fun = boost::get<FunctionDefinition>(m_ast.statements.at(i));
		m_functions[fun.name] = &fun;
		// Always inline functions that are only called once.
		if (references[fun.name] == 1)
			m_alwaysInline.insert(fun.name);
		updateCodeSize(fun);
	}
}

void FullInliner::run()
{
	assertThrow(m_ast.statements[0].type() == typeid(Block), OptimizerException, "");

	handleBlock("", boost::get<Block>(m_ast.statements[0]));
	// TODO it might be good to determine a visiting order:
	// first handle functions that are called from many places.
	for (auto const& fun: m_functions)
	{
		handleBlock(fun.second->name, fun.second->body);
		updateCodeSize(*fun.second);
	}
}

void FullInliner::updateCodeSize(FunctionDefinition& fun)
{
	m_functionSizes[fun.name] = CodeSize::codeSize(fun.body);
}

void FullInliner::handleBlock(string const& _currentFunctionName, Block& _block)
{
	InlineModifier{*this, m_nameDispenser, _currentFunctionName}(_block);
}

bool FullInliner::shallInline(FunctionCall const& _funCall, string const& _callSite)
{
	// No recursive inlining
	if (_funCall.functionName.name == _callSite)
		return false;

	FunctionDefinition& calledFunction = function(_funCall.functionName.name);
	if (m_alwaysInline.count(calledFunction.name))
		return true;

	// Constant arguments might provide a means for further optimization, so they cause a bonus.
	bool constantArg = false;
	for (auto const& argument: _funCall.arguments)
		if (argument.type() == typeid(Literal) || (
			argument.type() == typeid(Identifier) &&
			m_constants.count(boost::get<Identifier>(argument).name)
		))
		{
			constantArg = true;
			break;
		}

	size_t size = m_functionSizes.at(calledFunction.name);
	return (size < 10 || (constantArg && size < 50));
}


void InlineModifier::operator()(Block& _block)
{
	function<boost::optional<vector<Statement>>(Statement&)> f = [&](Statement& _statement) -> boost::optional<vector<Statement>> {
		visit(_statement);
		return tryInlineStatement(_statement);
	};
	iterateReplacing(_block.statements, f);
}

boost::optional<vector<Statement>> InlineModifier::tryInlineStatement(Statement& _statement)
{
	// Only inline for expression statements, assignments and variable declarations.
	Expression* e = boost::apply_visitor(GenericFallbackReturnsVisitor<Expression*, ExpressionStatement, Assignment, VariableDeclaration>(
		[](ExpressionStatement& _s) { return &_s.expression; },
		[](Assignment& _s) { return _s.value.get(); },
		[](VariableDeclaration& _s) { return _s.value.get(); }
	), _statement);
	if (e)
	{
		// Only inline direct function calls.
		FunctionCall* funCall = boost::apply_visitor(GenericFallbackReturnsVisitor<FunctionCall*, FunctionCall&>(
			[](FunctionCall& _e) { return &_e; }
		), *e);
		if (funCall && m_driver.shallInline(*funCall, m_currentFunction))
			return performInline(_statement, *funCall);
	}
	return {};
}

vector<Statement> InlineModifier::performInline(Statement& _statement, FunctionCall& _funCall)
{
	vector<Statement> newStatements;
	map<string, string> variableReplacements;

	FunctionDefinition& function = m_driver.function(_funCall.functionName.name);

	// helper function to create a new variable that is supposed to model
	// an existing variable.
	auto newVariable = [&](TypedName const& _existingVariable, Expression* _value) {
		string newName = m_nameDispenser.newName(function.name + "_" + _existingVariable.name);
		variableReplacements[_existingVariable.name] = newName;
		VariableDeclaration varDecl{_funCall.location, {{_funCall.location, newName, _existingVariable.type}}, {}};
		if (_value)
			varDecl.value = make_shared<Expression>(std::move(*_value));
		newStatements.emplace_back(std::move(varDecl));
	};

	for (size_t i = 0; i < _funCall.arguments.size(); ++i)
		newVariable(function.parameters[i], &_funCall.arguments[i]);
	for (auto const& var: function.returnVariables)
		newVariable(var, nullptr);

	Statement newBody = BodyCopier(m_nameDispenser, function.name + "_", variableReplacements)(function.body);
	newStatements += std::move(boost::get<Block>(newBody).statements);

	boost::apply_visitor(GenericFallbackVisitor<Assignment, VariableDeclaration>{
		[&](Assignment& _assignment)
		{
			for (size_t i = 0; i < _assignment.variableNames.size(); ++i)
				newStatements.emplace_back(Assignment{
					_assignment.location,
					{_assignment.variableNames[i]},
					make_shared<Expression>(Identifier{
						_assignment.location,
						variableReplacements.at(function.returnVariables[i].name)
					})
				});
		},
		[&](VariableDeclaration& _varDecl)
		{
			for (size_t i = 0; i < _varDecl.variables.size(); ++i)
				newStatements.emplace_back(VariableDeclaration{
					_varDecl.location,
					{std::move(_varDecl.variables[i])},
					make_shared<Expression>(Identifier{
						_varDecl.location,
						variableReplacements.at(function.returnVariables[i].name)
					})
				});
		}
		// nothing to be done for expression statement
	}, _statement);
	return newStatements;
}

string InlineModifier::newName(string const& _prefix)
{
	return m_nameDispenser.newName(_prefix);
}

Statement BodyCopier::operator()(VariableDeclaration const& _varDecl)
{
	for (auto const& var: _varDecl.variables)
		m_variableReplacements[var.name] = m_nameDispenser.newName(m_varNamePrefix + var.name);
	return ASTCopier::operator()(_varDecl);
}

Statement BodyCopier::operator()(FunctionDefinition const& _funDef)
{
	assertThrow(false, OptimizerException, "Function hoisting has to be done before function inlining.");
	return _funDef;
}

string BodyCopier::translateIdentifier(string const& _name)
{
	if (m_variableReplacements.count(_name))
		return m_variableReplacements.at(_name);
	else
		return _name;
}
