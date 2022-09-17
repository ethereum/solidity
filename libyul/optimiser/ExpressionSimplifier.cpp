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
 * Optimiser component that uses the simplification rules to simplify expressions.
 */

#include <libyul/optimiser/ExpressionSimplifier.h>

#include <libyul/AST.h>
#include <libyul/Utilities.h>
#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/backends/evm/EVMMetrics.h>
#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/optimiser/OptimizerUtilities.h>
#include <libyul/optimiser/SimplificationRules.h>

#include <libevmasm/SemanticInformation.h>

using namespace std;
using namespace solidity;
using namespace solidity::yul;

void ExpressionSimplifier::run(OptimiserStepContext& _context, Block& _ast)
{
	ExpressionSimplifier{_context.dialect, _context.expectedExecutionsPerDeployment}(_ast);
}

void ExpressionSimplifier::visit(Expression& _expression)
{
	ASTModifier::visit(_expression);

	while (auto const* match = SimplificationRules::findFirstMatch(
		_expression,
		m_dialect,
		[this](YulString _var) { return variableValue(_var); }
	))
		_expression = match->action().toExpression(debugDataOf(_expression));

	GasMeter gasMeter{
		dynamic_cast<EVMDialect const&>(m_dialect),
		!m_expectedExecutionsPerDeployment,
		m_expectedExecutionsPerDeployment ? *m_expectedExecutionsPerDeployment : 1};

	if (auto* functionCall = get_if<FunctionCall>(&_expression))
		if (optional<evmasm::Instruction> instruction = toEVMInstruction(m_dialect, functionCall->functionName.name))
		{
			for (auto op: evmasm::SemanticInformation::readWriteOperations(*instruction))
				if (op.startParameter && op.lengthParameter)
				{
					Expression& startArgument = functionCall->arguments.at(*op.startParameter);
					Expression const& lengthArgument = functionCall->arguments.at(*op.lengthParameter);
					if (
						knownToBeZero(lengthArgument) &&
						!knownToBeZero(startArgument) &&
						!holds_alternative<FunctionCall>(startArgument)
					)
						startArgument = Literal{debugDataOf(startArgument), LiteralKind::Number, "0"_yulstring, {}};
				}

			// Replace add(X, A) with sub(X, -A) if it would save gas.
			if (*instruction == evmasm::Instruction::ADD)
			{
				Expression const& arg1 = functionCall->arguments.at(0);
				Expression& arg2 = functionCall->arguments.at(1);
				if (auto const* arg2Literal = get_if<Literal>(&arg2))
				{
					// Estimate gas savings.
					bigint const costOfAdd = gasMeter.costs(_expression);
					Identifier const identifierSub = Identifier{
						debugDataOf(functionCall->functionName),
						dynamic_cast<EVMDialect const&>(m_dialect).builtin("sub"_yulstring)->name};
					Literal const arg2Negated = Literal{
						debugDataOf(arg2),
						LiteralKind::Number,
						YulString{(0 - valueOfLiteral(*arg2Literal)).str()},
						{}};
					FunctionCall const functionCallSub
						= FunctionCall{debugDataOf(_expression), identifierSub, {arg1, arg2Negated}};
					bigint const costOfSub = gasMeter.costs(functionCallSub);

					if (costOfSub < costOfAdd)
						*functionCall = functionCallSub;
				}
			}
		}
}

bool ExpressionSimplifier::knownToBeZero(Expression const& _expression) const
{
	if (auto const* literal = get_if<Literal>(&_expression))
		return valueOfLiteral(*literal) == 0;
	else if (auto const* identifier = get_if<Identifier>(&_expression))
		return valueOfIdentifier(identifier->name) == 0;
	else
		return false;
}
