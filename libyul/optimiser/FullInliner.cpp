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
 * Optimiser component that performs function inlining for arbitrary functions.
 */

#include <libyul/optimiser/FullInliner.h>

#include <libyul/optimiser/ASTCopier.h>
#include <libyul/optimiser/CallGraphGenerator.h>
#include <libyul/optimiser/FunctionCallFinder.h>
#include <libyul/optimiser/NameCollector.h>
#include <libyul/optimiser/Metrics.h>
#include <libyul/optimiser/SSAValueTracker.h>
#include <libyul/optimiser/Semantics.h>
#include <libyul/optimiser/CallGraphGenerator.h>
#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/Exceptions.h>
#include <libyul/AST.h>
#include <libyul/Dialect.h>

#include <libsolutil/CommonData.h>
#include <libsolutil/Visitor.h>

using namespace std;
using namespace solidity;
using namespace solidity::yul;

void FullInliner::run(OptimiserStepContext& _context, Block& _ast)
{
	FullInliner inliner{_ast, _context.dispenser, _context.dialect};
	inliner.run(Pass::InlineTiny);
	inliner.run(Pass::InlineRest);
}

FullInliner::FullInliner(Block& _ast, NameDispenser& _dispenser, Dialect const& _dialect):
	m_ast(_ast),
	m_recursiveFunctions(CallGraphGenerator::callGraph(_ast).recursiveFunctions()),
	m_nameDispenser(_dispenser),
	m_dialect(_dialect)
{

	// Determine constants
	SSAValueTracker tracker;
	tracker(m_ast);
	for (auto const& ssaValue: tracker.values())
		if (ssaValue.second && holds_alternative<Literal>(*ssaValue.second))
			m_constants.emplace(ssaValue.first);

	// Store size of global statements.
	m_functionSizes[YulString{}] = CodeSize::codeSize(_ast);
	map<YulString, size_t> references = ReferencesCounter::countReferences(m_ast);
	for (auto& statement: m_ast.statements)
	{
		if (!holds_alternative<FunctionDefinition>(statement))
			continue;
		FunctionDefinition& fun = std::get<FunctionDefinition>(statement);
		m_functions[fun.name] = &fun;
		if (LeaveFinder::containsLeave(fun))
			m_noInlineFunctions.insert(fun.name);
		// Always inline functions that are only called once.
		if (references[fun.name] == 1)
			m_singleUse.emplace(fun.name);
		updateCodeSize(fun);
	}

	// Check for memory guard.
	vector<FunctionCall*> memoryGuardCalls = FunctionCallFinder::run(
		_ast,
		"memoryguard"_yulstring
	);
	// We will perform less aggressive inlining, if no ``memoryguard`` call is found.
	if (!memoryGuardCalls.empty())
		m_hasMemoryGuard = true;
}

void FullInliner::run(Pass _pass)
{
	m_pass = _pass;

	// Note that the order of inlining can result in very different code.
	// Since AST IDs and thus function names depend on whether or not a contract
	// is compiled together with other source files, a change in AST IDs
	// should have as little an impact as possible. This is the case
	// if we handle inlining in source (and thus, for the IR generator,
	// function name) order.
	// We use stable_sort below to keep the inlining order of two functions
	// with the same depth.
	map<YulString, size_t> depths = callDepths();
	vector<FunctionDefinition*> functions;
	for (auto& statement: m_ast.statements)
		if (holds_alternative<FunctionDefinition>(statement))
			functions.emplace_back(&std::get<FunctionDefinition>(statement));
	std::stable_sort(functions.begin(), functions.end(), [depths](
		FunctionDefinition const* _a,
		FunctionDefinition const* _b
	) {
		return depths.at(_a->name) < depths.at(_b->name);
	});
	for (FunctionDefinition* fun: functions)
	{
		handleBlock(fun->name, fun->body);
		updateCodeSize(*fun);
	}

	for (auto& statement: m_ast.statements)
		if (holds_alternative<Block>(statement))
			handleBlock({}, std::get<Block>(statement));
}

map<YulString, size_t> FullInliner::callDepths() const
{
	CallGraph cg = CallGraphGenerator::callGraph(m_ast);
	cg.functionCalls.erase(""_yulstring);

	// Remove calls to builtin functions.
	for (auto& call: cg.functionCalls)
		for (auto it = call.second.begin(); it != call.second.end();)
			if (m_dialect.builtin(*it))
				it = call.second.erase(it);
			else
				++it;

	map<YulString, size_t> depths;
	size_t currentDepth = 0;

	while (true)
	{
		vector<YulString> removed;
		for (auto it = cg.functionCalls.begin(); it != cg.functionCalls.end();)
		{
			auto const& [fun, callees] = *it;
			if (callees.empty())
			{
				removed.emplace_back(fun);
				depths[fun] = currentDepth;
				it = cg.functionCalls.erase(it);
			}
			else
				++it;
		}

		for (auto& call: cg.functionCalls)
			call.second -= removed;

		currentDepth++;

		if (removed.empty())
			break;
	}

	// Only recursive functions left here.
	for (auto const& fun: cg.functionCalls)
		depths[fun.first] = currentDepth;

	return depths;
}

bool FullInliner::shallInline(FunctionCall const& _funCall, YulString _callSite)
{
	// No recursive inlining
	if (_funCall.functionName.name == _callSite)
		return false;

	FunctionDefinition* calledFunction = function(_funCall.functionName.name);
	if (!calledFunction)
		return false;

	if (m_noInlineFunctions.count(_funCall.functionName.name) || recursive(*calledFunction))
		return false;

	// Inline really, really tiny functions
	size_t size = m_functionSizes.at(calledFunction->name);
	if (size <= 1)
		return true;

	// In the first pass, only inline tiny functions.
	if (m_pass == Pass::InlineTiny)
		return false;

	bool aggressiveInlining = true;

	if (
		EVMDialect const* evmDialect = dynamic_cast<EVMDialect const*>(&m_dialect);
		!evmDialect || !evmDialect->providesObjectAccess() || evmDialect->evmVersion() <= langutil::EVMVersion::homestead()
	)
		// No aggressive inlining with the old code transform.
		aggressiveInlining = false;

	// No aggressive inlining, if we cannot perform stack-to-memory.
	if (!m_hasMemoryGuard || m_recursiveFunctions.count(_callSite))
		aggressiveInlining = false;

	if (!aggressiveInlining && m_functionSizes.at(_callSite) > 45)
		return false;

	if (m_singleUse.count(calledFunction->name))
		return true;

	// Constant arguments might provide a means for further optimization, so they cause a bonus.
	bool constantArg = false;
	for (auto const& argument: _funCall.arguments)
		if (holds_alternative<Literal>(argument) || (
			holds_alternative<Identifier>(argument) &&
			m_constants.count(std::get<Identifier>(argument).name)
		))
		{
			constantArg = true;
			break;
		}

	return (size < (aggressiveInlining ? 8 : 6) || (constantArg && size < (aggressiveInlining ? 16 : 12)));
}

void FullInliner::tentativelyUpdateCodeSize(YulString _function, YulString _callSite)
{
	m_functionSizes.at(_callSite) += m_functionSizes.at(_function);
}

void FullInliner::updateCodeSize(FunctionDefinition const& _fun)
{
	m_functionSizes[_fun.name] = CodeSize::codeSize(_fun.body);
}

void FullInliner::handleBlock(YulString _currentFunctionName, Block& _block)
{
	InlineModifier{*this, m_nameDispenser, _currentFunctionName, m_dialect}(_block);
}

bool FullInliner::recursive(FunctionDefinition const& _fun) const
{
	map<YulString, size_t> references = ReferencesCounter::countReferences(_fun);
	return references[_fun.name] > 0;
}

void InlineModifier::operator()(Block& _block)
{
	function<std::optional<vector<Statement>>(Statement&)> f = [&](Statement& _statement) -> std::optional<vector<Statement>> {
		visit(_statement);
		return tryInlineStatement(_statement);
	};
	util::iterateReplacing(_block.statements, f);
}

std::optional<vector<Statement>> InlineModifier::tryInlineStatement(Statement& _statement)
{
	// Only inline for expression statements, assignments and variable declarations.
	Expression* e = std::visit(util::GenericVisitor{
		util::VisitorFallback<Expression*>{},
		[](ExpressionStatement& _s) { return &_s.expression; },
		[](Assignment& _s) { return _s.value.get(); },
		[](VariableDeclaration& _s) { return _s.value.get(); }
	}, _statement);
	if (e)
	{
		// Only inline direct function calls.
		FunctionCall* funCall = std::visit(util::GenericVisitor{
			util::VisitorFallback<FunctionCall*>{},
			[](FunctionCall& _e) { return &_e; }
		}, *e);
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

	// helper function to create a new variable that is supposed to model
	// an existing variable.
	auto newVariable = [&](TypedName const& _existingVariable, Expression* _value) {
		YulString newName = m_nameDispenser.newName(_existingVariable.name);
		variableReplacements[_existingVariable.name] = newName;
		VariableDeclaration varDecl{_funCall.debugData, {{_funCall.debugData, newName, _existingVariable.type}}, {}};
		if (_value)
			varDecl.value = make_unique<Expression>(std::move(*_value));
		else
			varDecl.value = make_unique<Expression>(m_dialect.zeroLiteralForType(varDecl.variables.front().type));
		newStatements.emplace_back(std::move(varDecl));
	};

	for (size_t i = 0; i < _funCall.arguments.size(); ++i)
		newVariable(function->parameters[i], &_funCall.arguments[i]);
	for (auto const& var: function->returnVariables)
		newVariable(var, nullptr);

	Statement newBody = BodyCopier(m_nameDispenser, variableReplacements)(function->body);
	newStatements += std::move(std::get<Block>(newBody).statements);

	std::visit(util::GenericVisitor{
		util::VisitorFallback<>{},
		[&](Assignment& _assignment)
		{
			for (size_t i = 0; i < _assignment.variableNames.size(); ++i)
				newStatements.emplace_back(Assignment{
					_assignment.debugData,
					{_assignment.variableNames[i]},
					make_unique<Expression>(Identifier{
						_assignment.debugData,
						variableReplacements.at(function->returnVariables[i].name)
					})
				});
		},
		[&](VariableDeclaration& _varDecl)
		{
			for (size_t i = 0; i < _varDecl.variables.size(); ++i)
				newStatements.emplace_back(VariableDeclaration{
					_varDecl.debugData,
					{std::move(_varDecl.variables[i])},
					make_unique<Expression>(Identifier{
						_varDecl.debugData,
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
		m_variableReplacements[var.name] = m_nameDispenser.newName(var.name);
	return ASTCopier::operator()(_varDecl);
}

Statement BodyCopier::operator()(FunctionDefinition const&)
{
	assertThrow(false, OptimizerException, "Function hoisting has to be done before function inlining.");
	return {};
}

YulString BodyCopier::translateIdentifier(YulString _name)
{
	if (m_variableReplacements.count(_name))
		return m_variableReplacements.at(_name);
	else
		return _name;
}
