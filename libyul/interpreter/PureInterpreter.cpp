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

#include <libyul/interpreter/PureInterpreter.h>

#include <libyul/interpreter/PureEVMInstructionInterpreter.h>

#include <libyul/AST.h>
#include <libyul/Dialect.h>
#include <libyul/Utilities.h>
#include <libyul/backends/evm/EVMDialect.h>

#include <liblangutil/Exceptions.h>

#include <libsolutil/CommonData.h>
#include <libsolutil/Exceptions.h>
#include <libsolutil/FixedHash.h>
#include <libsolutil/Visitor.h>

#include <libevmasm/Instruction.h>

using namespace solidity;
using namespace solidity::yul;
using namespace solidity::yul::interpreter;
namespace outcome = BOOST_OUTCOME_V2_NAMESPACE;

using solidity::util::h256;

ExecutionResult PureInterpreter::operator()(ExpressionStatement const& _expressionStatement)
{
	BOOST_OUTCOME_TRY(evaluate(_expressionStatement.expression, 0));
	return ExecutionOk{ControlFlowState::Default};
}

ExecutionResult PureInterpreter::operator()(Assignment const& _assignment)
{
	solAssert(_assignment.value);
	BOOST_OUTCOME_TRY(EvaluationOk evalResult, evaluate(*_assignment.value, _assignment.variableNames.size()));

	std::vector<u256> const& values = std::move(evalResult.values);
	for (size_t i = 0; i < values.size(); ++i)
	{
		YulName varName = _assignment.variableNames.at(i).name;
		auto [_, isNew] = m_variables.insert_or_assign(varName, values.at(i));
		solAssert(!isNew);
	}
	return ExecutionOk{ControlFlowState::Default};
}

ExecutionResult PureInterpreter::operator()(VariableDeclaration const& _declaration)
{
	std::vector<u256> values;
	if (_declaration.value)
	{
		BOOST_OUTCOME_TRY(EvaluationOk evalResult, evaluate(*_declaration.value, _declaration.variables.size()));
		values = std::move(evalResult.values);
	}
	else
	{
		values.assign(_declaration.variables.size(), 0);
	}

	solAssert(values.size() == _declaration.variables.size());
	for (size_t i = 0; i < values.size(); ++i)
	{
		YulName varName = _declaration.variables.at(i).name;
		auto [_, isNew] = m_variables.insert_or_assign(varName, values.at(i));
		solAssert(isNew);
		m_scope->addDeclaredVariable(varName);
	}
	return ExecutionOk{ControlFlowState::Default};
}

ExecutionResult PureInterpreter::operator()(If const& _if)
{
	solAssert(_if.condition);
	BOOST_OUTCOME_TRY(EvaluationOk conditionResult, evaluate(*_if.condition, 1));

	if (conditionResult.values.at(0) != 0)
		return (*this)(_if.body);
	return ExecutionOk{ControlFlowState::Default};
}

ExecutionResult PureInterpreter::operator()(Switch const& _switch)
{
	solAssert(_switch.expression);
	solAssert(!_switch.cases.empty());

	BOOST_OUTCOME_TRY(EvaluationOk expressionResult, evaluate(*_switch.expression, 1));

	u256 expressionValue = expressionResult.values.at(0);
	for (auto const& currentCase: _switch.cases)
	{
		bool caseMatched = false;
		// Default case has to be last.
		if (!currentCase.value) caseMatched = true;
		else
		{
			BOOST_OUTCOME_TRY(EvaluationOk caseResult, evaluate(*currentCase.value, 1));
			caseMatched = caseResult.values.at(0) == expressionValue;
		}
		if (caseMatched)
			return (*this)(currentCase.body);
	}
	return ExecutionOk{ControlFlowState::Default};
}

ExecutionResult PureInterpreter::operator()(FunctionDefinition const&)
{
	return ExecutionOk{ControlFlowState::Default};
}

ExecutionResult PureInterpreter::operator()(ForLoop const& _forLoop)
{
	solAssert(_forLoop.condition);

	enterScope(_forLoop.pre);
	ScopeGuard g([this]{ leaveScope(); });

	{
		BOOST_OUTCOME_TRY(ExecutionOk preResult, execute(_forLoop.pre.statements));
		if (preResult == ExecutionOk{ControlFlowState::Leave})
			return preResult;
	}
	while (true)
	{
		{
			BOOST_OUTCOME_TRY(EvaluationOk conditionResult, evaluate(*_forLoop.condition, 1));
			if (conditionResult.values.at(0) == 0)
				break;
		}

		// Increment step for each loop iteration for loops with
		// an empty body and post blocks to prevent a deadlock.
		if (_forLoop.body.statements.size() == 0 && _forLoop.post.statements.size() == 0)
		{
			BOOST_OUTCOME_TRY(incrementStatementStep());
		}

		{
			BOOST_OUTCOME_TRY(ExecutionOk bodyResult, (*this)(_forLoop.body));
			if (bodyResult == ExecutionOk{ControlFlowState::Leave})
				return bodyResult;

			if (bodyResult == ExecutionOk{ControlFlowState::Break})
				return ExecutionOk{ControlFlowState::Default};
		}

		{
			BOOST_OUTCOME_TRY(ExecutionOk postResult, (*this)(_forLoop.post));
			if (postResult == ExecutionOk{ControlFlowState::Leave})
				return postResult;
		}
	}
	return ExecutionOk{ControlFlowState::Default};
}

ExecutionResult PureInterpreter::operator()(Break const&)
{
	return ExecutionOk{ControlFlowState::Break};
}

ExecutionResult PureInterpreter::operator()(Continue const&)
{
	return ExecutionOk{ControlFlowState::Continue};
}

ExecutionResult PureInterpreter::operator()(Leave const&)
{
	return ExecutionOk{ControlFlowState::Leave};
}

ExecutionResult PureInterpreter::operator()(Block const& _block)
{
	enterScope(_block);
	ScopeGuard guard([this] { leaveScope(); });

	return execute(_block.statements);
}

ExecutionResult PureInterpreter::execute(std::vector<Statement> const& _statements)
{
	for (auto const& statement: _statements)
	{
		BOOST_OUTCOME_TRY(ExecutionOk statementRes, visit(statement));
		if (statementRes != ExecutionOk{ControlFlowState::Default})
			return statementRes;
	}
	return ExecutionOk{ControlFlowState::Default};
}

ExecutionResult PureInterpreter::visit(Statement const& _statement)
{
	BOOST_OUTCOME_TRY(incrementStatementStep());
	return std::visit(*this, _statement);
}

EvaluationResult PureInterpreter::operator()(Literal const& _literal)
{
	return EvaluationOk(_literal.value.value());
}

EvaluationResult PureInterpreter::operator()(Identifier const& _identifier)
{
	auto it = m_variables.find(_identifier.name);
	solAssert(it != m_variables.end());
	return EvaluationOk(it->second);
}

EvaluationResult PureInterpreter::operator()(FunctionCall const& _functionCall)
{
	if (std::optional<BuiltinHandle> builtinHandle = m_dialect.findBuiltin(_functionCall.functionName.name.str()))
		if (
			auto const& args = m_dialect.builtin(*builtinHandle).literalArguments;
			!args.empty()
		)
			return UnlimitedLiteralEncountered();
	BOOST_OUTCOME_TRY(EvaluationOk argsRes, evaluateArgs(_functionCall.arguments));

	std::vector<u256> const& argsValues = argsRes.values;

	if (EVMDialect const* dialect = dynamic_cast<EVMDialect const*>(&m_dialect))
	{
		if (std::optional<BuiltinHandle> builtinHandle = dialect->findBuiltin(_functionCall.functionName.name.str()))
		{
			auto const& function = dialect->builtin(*builtinHandle);
			return PureEVMInstructionInterpreter::eval(dialect->evmVersion(), function, argsValues);
		}
	}

	FunctionDefinition const& functionDefinition = m_scope->getFunction(_functionCall.functionName.name);

	yulAssert(argsValues.size() == functionDefinition.parameters.size());
	VariableValuesMap variables;
	for (size_t i = 0; i < functionDefinition.parameters.size(); ++i)
		variables[functionDefinition.parameters.at(i).name] = argsValues.at(i);
	for (size_t i = 0; i < functionDefinition.returnVariables.size(); ++i)
		variables[functionDefinition.returnVariables.at(i).name] = 0;

	PureInterpreter interpreter = makeInterpreterCopy(std::move(variables));

	BOOST_OUTCOME_TRY(m_state.addTrace<FunctionCallTrace>(functionDefinition, argsValues));

	BOOST_OUTCOME_TRY(interpreter(functionDefinition.body));

	std::vector<u256> returnedValues;
	returnedValues.reserve(functionDefinition.returnVariables.size());
	for (auto const& retVar: functionDefinition.returnVariables)
		returnedValues.emplace_back(interpreter.valueOfVariable(retVar.name));

	BOOST_OUTCOME_TRY(m_state.addTrace<FunctionReturnTrace>(functionDefinition, returnedValues));

	return EvaluationOk(std::move(returnedValues));
}

EvaluationResult PureInterpreter::visit(Expression const& _expression)
{
	BOOST_OUTCOME_TRY(incrementExpressionStep());
	return std::visit(*this, _expression);
}

EvaluationResult PureInterpreter::evaluate(Expression const& _expression, size_t _numReturnVars)
{
	BOOST_OUTCOME_TRY(EvaluationOk result, visit(_expression));
	yulAssert(result.values.size() == _numReturnVars);
	return result;
}

EvaluationResult PureInterpreter::evaluateArgs(
	std::vector<Expression> const& _arguments
)
{
	std::vector<u256> values(_arguments.size());

	/// Function arguments are evaluated in reverse.
	for (size_t i = _arguments.size(); i-- > 0; )
	{
		auto const& currentArgument = _arguments[i];
		BOOST_OUTCOME_TRY(EvaluationOk exprRes, evaluate(currentArgument, 1));
		values[i] = exprRes.values.at(0);
	}
	return EvaluationOk(std::move(values));
}

void PureInterpreter::enterScope(Block const& _block)
{
	m_scope = m_scope->createSubscope(_block);
}

void PureInterpreter::leaveScope()
{
	m_scope->cleanupVariables(m_variables);
	m_scope = m_scope->parent();
	yulAssert(m_scope);
}

outcome::result<void, ExecutionTerminated> PureInterpreter::incrementStatementStep()
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
	return outcome::success();
}

outcome::result<void, ExecutionTerminated> PureInterpreter::incrementExpressionStep()
{
	m_expressionNestingLevel++;
	if (m_state.config.maxExprNesting > 0 && m_expressionNestingLevel > m_state.config.maxExprNesting)
		return ExpressionNestingLimitReached();
	return outcome::success();
}
