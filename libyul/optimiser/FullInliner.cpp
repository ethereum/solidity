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
#include <libyul/AsmData.h>

#include <libdevcore/CommonData.h>
#include <libdevcore/Visitor.h>

#include <boost/range/adaptor/reversed.hpp>

using namespace std;
using namespace dev;
using namespace yul;
using namespace dev::solidity;

FullInliner::FullInliner(Block& _ast, NameDispenser& _dispenser):
	m_ast(_ast), m_nameDispenser(_dispenser)
{
	// Determine constants
	SSAValueTracker tracker;
	tracker(m_ast);
	for (auto const& ssaValue: tracker.values())
		if (ssaValue.second && ssaValue.second->type() == typeid(Literal))
			m_constants.emplace(ssaValue.first);

	// Store size of global statements.
	m_functionSizes[YulString{}] = CodeSize::codeSize(_ast);
	map<YulString, size_t> references = ReferencesCounter::countReferences(m_ast);
	for (auto& statement: m_ast.statements)
	{
		if (statement.type() != typeid(FunctionDefinition))
			continue;
		FunctionDefinition& fun = boost::get<FunctionDefinition>(statement);
		m_functions[fun.name] = &fun;
		// Always inline functions that are only called once.
		if (references[fun.name] == 1)
			m_singleUse.emplace(fun.name);
		updateCodeSize(fun);
	}
}

void FullInliner::run()
{
	for (auto& statement: m_ast.statements)
		if (statement.type() == typeid(Block))
			handleBlock({}, boost::get<Block>(statement));

	// TODO it might be good to determine a visiting order:
	// first handle functions that are called from many places.
	for (auto const& fun: m_functions)
	{
		handleBlock(fun.second->name, fun.second->body);
		updateCodeSize(*fun.second);
	}
}

void FullInliner::updateCodeSize(FunctionDefinition const& _fun)
{
	m_functionSizes[_fun.name] = CodeSize::codeSize(_fun.body);
}

void FullInliner::handleBlock(YulString _currentFunctionName, Block& _block)
{
	InlineModifier{*this, m_nameDispenser, _currentFunctionName}(_block);
}

bool FullInliner::shallInline(FunctionCall const& _funCall, YulString _callSite)
{
	// No recursive inlining
	if (_funCall.functionName.name == _callSite)
		return false;

	FunctionDefinition* calledFunction = function(_funCall.functionName.name);
	if (!calledFunction)
		return false;

	// Inline really, really tiny functions
	size_t size = m_functionSizes.at(calledFunction->name);
	if (size <= 1)
		return true;

	// Do not inline into already big functions.
	if (m_functionSizes.at(_callSite) > 45)
		return false;

	if (m_singleUse.count(calledFunction->name))
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

	return (size < 6 || (constantArg && size < 12));
}

void FullInliner::tentativelyUpdateCodeSize(YulString _function, YulString _callSite)
{
	m_functionSizes.at(_callSite) += m_functionSizes.at(_function);
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
	map<YulString, YulString> variableReplacements;

	FunctionDefinition* function = m_driver.function(_funCall.functionName.name);
	assertThrow(!!function, OptimizerException, "Attempt to inline invalid function.");

	m_driver.tentativelyUpdateCodeSize(function->name, m_currentFunction);

	static Expression const zero{Literal{{}, LiteralKind::Number, YulString{"0"}, {}}};

	// helper function to create a new variable that is supposed to model
	// an existing variable.
	auto newVariable = [&](TypedName const& _existingVariable, Expression* _value) {
		YulString newName = m_nameDispenser.newName(_existingVariable.name, function->name);
		variableReplacements[_existingVariable.name] = newName;
		VariableDeclaration varDecl{_funCall.location, {{_funCall.location, newName, _existingVariable.type}}, {}};
		if (_value)
			varDecl.value = make_shared<Expression>(std::move(*_value));
		else
			varDecl.value = make_shared<Expression>(zero);
		newStatements.emplace_back(std::move(varDecl));
	};

	for (size_t i = 0; i < _funCall.arguments.size(); ++i)
		newVariable(function->parameters[i], &_funCall.arguments[i]);
	for (auto const& var: function->returnVariables)
		newVariable(var, nullptr);

	Statement newBody = BodyCopier(m_nameDispenser, function->name, variableReplacements)(function->body);
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
						variableReplacements.at(function->returnVariables[i].name)
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
						variableReplacements.at(function->returnVariables[i].name)
					})
				});
		}
		// nothing to be done for expression statement
	}, _statement);
	return newStatements;
}

Statement BodyCopier::operator()(VariableDeclaration const& _varDecl)
{
	for (auto const& var: _varDecl.variables)
		m_variableReplacements[var.name] = m_nameDispenser.newName(var.name, m_varNamePrefix);
	return ASTCopier::operator()(_varDecl);
}

Statement BodyCopier::operator()(FunctionDefinition const& _funDef)
{
	assertThrow(false, OptimizerException, "Function hoisting has to be done before function inlining.");
	return _funDef;
}

YulString BodyCopier::translateIdentifier(YulString _name)
{
	if (m_variableReplacements.count(_name))
		return m_variableReplacements.at(_name);
	else
		return _name;
}
