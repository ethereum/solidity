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

#include <libyul/tools/interpreter/Interpreter.h>

#include <libyul/tools/interpreter/EVMInstructionInterpreter.h>

#include <libyul/AST.h>
#include <libyul/Dialect.h>
#include <libyul/Utilities.h>
#include <libyul/backends/evm/EVMDialect.h>

#include <liblangutil/Exceptions.h>

#include <libsolutil/FixedHash.h>
#include <libsolutil/Visitor.h>

#include <range/v3/view/reverse.hpp>

#include <ostream>
#include <variant>

using namespace solidity;
using namespace solidity::yul;
using namespace solidity::yul::tools::interpreter;

using solidity::util::h256;

void Interpreter::run(
	InterpreterState& _state,
	Dialect const& _dialect,
	Block const& _ast
)
{
	Scope scope;
	Interpreter{_state, _dialect, scope, 0}(_ast);
}

ExecutionResult Interpreter::operator()(ExpressionStatement const& _expressionStatement)
{
	EvaluationResult res = evaluate(_expressionStatement.expression, 0);
	if (auto* terminated = std::get_if<ExecutionTerminated>(&res)) return *terminated;
	return ExecutionOk{ ControlFlowState::Default };
}

ExecutionResult Interpreter::operator()(Assignment const& _assignment)
{
	solAssert(_assignment.value, "");
	EvaluationResult evalRes = evaluate(*_assignment.value, _assignment.variableNames.size());
	if (auto* terminated = std::get_if<ExecutionTerminated>(&evalRes)) return *terminated;

	std::vector<u256> const& values = std::get<EvaluationOk>(evalRes).values;
	for (size_t i = 0; i < values.size(); ++i)
	{
		YulName varName = _assignment.variableNames.at(i).name;
		solAssert(m_variables.count(varName), "");
		m_variables[varName] = values.at(i);
	}
	return ExecutionOk { ControlFlowState::Default };
}

ExecutionResult Interpreter::operator()(VariableDeclaration const& _declaration)
{
	std::vector<u256> values(_declaration.variables.size(), 0);
	if (_declaration.value)
	{
		EvaluationResult evalRes = evaluate(*_declaration.value, _declaration.variables.size());
		if (auto* terminated = std::get_if<ExecutionTerminated>(&evalRes)) return *terminated;
		values = std::get<EvaluationOk>(evalRes).values;
	}

	solAssert(values.size() == _declaration.variables.size(), "");
	for (size_t i = 0; i < values.size(); ++i)
	{
		YulName varName = _declaration.variables.at(i).name;
		solAssert(!m_variables.count(varName), "");
		m_variables[varName] = values.at(i);
		m_scope->names.emplace(varName, nullptr);
	}
	return ExecutionOk { ControlFlowState::Default };
}

ExecutionResult Interpreter::operator()(If const& _if)
{
	solAssert(_if.condition, "");
	EvaluationResult conditionRes = evaluate(*_if.condition, 1);
	if (auto* terminated = std::get_if<ExecutionTerminated>(&conditionRes)) return *terminated;

	if (std::get<EvaluationOk>(conditionRes).values.at(0) != 0)
		return (*this)(_if.body);
	return ExecutionOk { ControlFlowState::Default };
}

ExecutionResult Interpreter::operator()(Switch const& _switch)
{
	solAssert(_switch.expression, "");
	solAssert(!_switch.cases.empty(), "");

	EvaluationResult expressionRes = evaluate(*_switch.expression, 1);
	if (auto* terminated = std::get_if<ExecutionTerminated>(&expressionRes)) return *terminated;

	u256 val = std::get<EvaluationOk>(expressionRes).values.at(0);
	for (auto const& c: _switch.cases)
	{
		bool caseMatched = false;
		// Default case has to be last.
		if (!c.value) caseMatched = true;
		else
		{
			EvaluationResult caseRes = evaluate(*c.value, 1);
			if (auto* terminated = std::get_if<ExecutionTerminated>(&caseRes)) return *terminated;
			caseMatched = std::get<EvaluationOk>(caseRes).values.at(0) == val;
		}
		if (caseMatched) return (*this)(c.body);
	}
	return ExecutionOk { ControlFlowState::Default };
}

ExecutionResult Interpreter::operator()(FunctionDefinition const&)
{
	return ExecutionOk{ ControlFlowState::Default };
}

ExecutionResult Interpreter::operator()(ForLoop const& _forLoop)
{
	solAssert(_forLoop.condition, "");

	enterScope(_forLoop.pre);
	ScopeGuard g([this]{ leaveScope(); });

	for (auto const& statement: _forLoop.pre.statements)
	{
		ExecutionResult execRes = visit(statement);
		if (execRes == ExecutionResult(ExecutionOk { ControlFlowState::Leave }))
			return execRes;
	}
	while (true)
	{
		{
			EvaluationResult conditionRes = evaluate(*_forLoop.condition, 1);
			if (auto* terminated = std::get_if<ExecutionTerminated>(&conditionRes)) return *terminated;
			if (std::get<EvaluationOk>(conditionRes).values.at(0) == 0) break;
		}

		// Increment step for each loop iteration for loops with
		// an empty body and post blocks to prevent a deadlock.
		if (_forLoop.body.statements.size() == 0 && _forLoop.post.statements.size() == 0)
			if (auto terminated = incrementStatementStep()) return *terminated;

		{
			ExecutionResult bodyRes = (*this)(_forLoop.body);
			if (
				std::holds_alternative<ExecutionTerminated>(bodyRes) ||
				bodyRes == ExecutionResult(ExecutionOk{ ControlFlowState::Leave })
			) return bodyRes;

			if (bodyRes == ExecutionResult(ExecutionOk{ ControlFlowState::Break }))
				return ExecutionOk { ControlFlowState::Default };
		}

		{
			ExecutionResult postRes = (*this)(_forLoop.post);
			if (
				std::holds_alternative<ExecutionTerminated>(postRes) ||
				postRes == ExecutionResult(ExecutionOk{ ControlFlowState::Leave })
			) return postRes;
		}
	}
	return ExecutionOk { ControlFlowState::Default };
}

ExecutionResult Interpreter::operator()(Break const&)
{
	return ExecutionOk{ ControlFlowState::Break };
}

ExecutionResult Interpreter::operator()(Continue const&)
{
	return ExecutionOk{ ControlFlowState::Continue };
}

ExecutionResult Interpreter::operator()(Leave const&)
{
	return ExecutionOk{ ControlFlowState::Leave };
}

ExecutionResult Interpreter::operator()(Block const& _block)
{
	enterScope(_block);
	ScopeGuard guard([this] { leaveScope(); });

	// Register functions.
	for (auto const& statement: _block.statements)
		if (std::holds_alternative<FunctionDefinition>(statement))
		{
			FunctionDefinition const& funDef = std::get<FunctionDefinition>(statement);
			m_scope->names.emplace(funDef.name, &funDef);
		}

	for (auto const& statement: _block.statements)
	{
		ExecutionResult statementRes = visit(statement);
		if (statementRes != ExecutionResult(ExecutionOk{ ControlFlowState::Default }))
			return statementRes;
	}
	return ExecutionOk{ ControlFlowState::Default };
}

ExecutionResult Interpreter::visit(Statement const& _st)
{
	if (auto terminated = incrementStatementStep()) return *terminated;
	return std::visit(*this, _st);
}

EvaluationResult Interpreter::operator()(Literal const& _literal)
{
	return EvaluationOk(_literal.value.value());
}

EvaluationResult Interpreter::operator()(Identifier const& _identifier)
{
	solAssert(m_variables.count(_identifier.name), "");
	return EvaluationOk(m_variables.at(_identifier.name));
}

EvaluationResult Interpreter::operator()(FunctionCall const& _funCall)
{
	std::vector<std::optional<LiteralKind>> const* literalArguments = nullptr;
	if (BuiltinFunction const* builtin = m_dialect.builtin(_funCall.functionName.name))
		if (!builtin->literalArguments.empty())
			literalArguments = &builtin->literalArguments;
	EvaluationResult argsRes = evaluateArgs(_funCall.arguments, literalArguments);
	if (auto* terminated = std::get_if<ExecutionTerminated>(&argsRes)) return *terminated;

	std::vector<u256> argsValues = std::get<EvaluationOk>(argsRes).values;

	if (EVMDialect const* dialect = dynamic_cast<EVMDialect const*>(&m_dialect))
	{
		if (BuiltinFunctionForEVM const* fun = dialect->builtin(_funCall.functionName.name))
		{
			EVMInstructionInterpreter interpreter(dialect->evmVersion(), m_state);
			EVMInstructionInterpretedResult const value = interpreter.evalBuiltin(*fun, _funCall.arguments, argsValues);
			if (auto* terminated = std::get_if<ExecutionTerminated>(&value)) return *terminated;
			return EvaluationOk(std::get<u256>(value));
		}
	}

	Scope* scope = m_scope;
	for (; scope; scope = scope->parent)
		if (scope->names.count(_funCall.functionName.name))
			break;
	yulAssert(scope, "");

	FunctionDefinition const* fun = scope->names.at(_funCall.functionName.name);
	yulAssert(fun, "Function not found.");
	yulAssert(argsValues.size() == fun->parameters.size(), "");
	std::map<YulName, u256> variables;
	for (size_t i = 0; i < fun->parameters.size(); ++i)
		variables[fun->parameters.at(i).name] = argsValues.at(i);
	for (size_t i = 0; i < fun->returnVariables.size(); ++i)
		variables[fun->returnVariables.at(i).name] = 0;

	std::unique_ptr<Interpreter> interpreter = makeInterpreterCopy(std::move(variables));
	ExecutionResult funcBodyRes = (*interpreter)(fun->body);
	if (auto* terminated = std::get_if<ExecutionTerminated>(&funcBodyRes)) return *terminated;

	std::vector<u256> returnedValues;
	for (auto const& retVar: fun->returnVariables)
		returnedValues.emplace_back(interpreter->valueOfVariable(retVar.name));
	return EvaluationOk(returnedValues);
}

EvaluationResult Interpreter::visit(Expression const& _st)
{
	if (auto terminated = incrementExpressionStep()) return *terminated;
	return std::visit(*this, _st);
}

EvaluationResult Interpreter::evaluate(Expression const& _expression, size_t _numReturnVars)
{
	EvaluationResult res = visit(_expression);
	if (auto* resOk = std::get_if<EvaluationOk>(&res))
		yulAssert(resOk->values.size() == _numReturnVars, "");

	return res;
}

EvaluationResult Interpreter::evaluateArgs(
	std::vector<Expression> const& _expr,
	std::vector<std::optional<LiteralKind>> const* _literalArguments
)
{
	std::vector<u256> values;
	size_t i = 0;
	/// Function arguments are evaluated in reverse.
	for (auto const& expr: _expr | ranges::views::reverse)
	{
		if (!_literalArguments || !_literalArguments->at(_expr.size() - i - 1))
		{
			EvaluationResult exprRes = visit(expr);
			if (auto* terminated = std::get_if<ExecutionTerminated>(&exprRes)) return *terminated;
			std::vector<u256> const& exprValues = std::get<EvaluationOk>(exprRes).values;
			yulAssert(exprValues.size() == 1, "");
			values.push_back(exprValues.at(0));
		}
		else
		{
			if (std::get<Literal>(expr).value.unlimited())
			{
				yulAssert(std::get<Literal>(expr).kind == LiteralKind::String);
				values.push_back(0xdeadbeef);
			}
			else
				values.push_back(std::get<Literal>(expr).value.value());
		}

		++i;
	}
	std::reverse(values.begin(), values.end());
	return EvaluationOk(values);
}

void Interpreter::enterScope(Block const& _block)
{
	if (!m_scope->subScopes.count(&_block))
		m_scope->subScopes[&_block] = std::make_unique<Scope>(Scope{
			{},
			{},
			m_scope
		});
	m_scope = m_scope->subScopes[&_block].get();
}

void Interpreter::leaveScope()
{
	for (auto const& [var, funDeclaration]: m_scope->names)
		if (!funDeclaration)
			m_variables.erase(var);
	m_scope = m_scope->parent;
	yulAssert(m_scope, "");
}

std::optional<ExecutionTerminated> Interpreter::incrementStatementStep()
{
	m_state.numSteps++;
	if (m_state.config.maxSteps > 0 && m_state.numSteps >= m_state.config.maxSteps)
		return StepLimitReached();

	// Checking recursion depth here because we are sure that a statement
	// inside the body evaluated.
	if (m_state.config.maxRecursionDepth > 0 && m_recursionDepth > m_state.config.maxRecursionDepth)
		return RecursionDepthLimitReached();

	// Reset m_expressionNestingLevel, preparing for new expression.
	m_expressionNestingLevel = 0;
	return std::nullopt;
}

std::optional<ExecutionTerminated> Interpreter::incrementExpressionStep()
{
	m_expressionNestingLevel++;
	if (m_state.config.maxExprNesting > 0 && m_expressionNestingLevel > m_state.config.maxExprNesting)
		return ExpressionNestingLimitReached();
	return std::nullopt;
}
