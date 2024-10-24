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

#include <libyul/optimiser/ConstantFunctionEvaluator.h>

#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/Exceptions.h>
#include <libyul/AST.h>
#include <libyul/Dialect.h>

#include <libevmasm/Instruction.h>

#include <libsolutil/CommonData.h>
#include <libsolutil/Visitor.h>

#include <range/v3/action/remove.hpp>
#include <range/v3/view/reverse.hpp>
#include <range/v3/view/zip.hpp>

#include <boost/algorithm/string/predicate.hpp>

using namespace solidity;
using namespace solidity::evmasm;
using namespace solidity::yul;
using namespace solidity::yul::tools::interpreter;

using namespace std::literals::string_literals;

void ConstantFunctionEvaluator::run(OptimiserStepContext& _context, Block& _ast)
{
	ConstantFunctionEvaluator(_context.dialect)(_ast);
}


ConstantFunctionEvaluator::ConstantFunctionEvaluator(Dialect const& _dialect):
	m_dialect(_dialect),
	m_rootScope(),
	m_currentScope(&m_rootScope)
{
}

void ConstantFunctionEvaluator::operator()(FunctionDefinition& _function)
{
	ASTModifier::operator()(_function);
	if (_function.parameters.size() > 0) return ;

	InterpreterState state;
	// TODO make these configurable
	state.maxExprNesting = 100;
	state.maxSteps = 10000;
	state.maxTraceSize = 0;
	// This must be limited, because the stack is also used in this optimizer component
	state.maxRecursionDepth = 64;

	std::map<YulName, u256> returnVariables;
	for (auto const& retVar: _function.returnVariables)
	{
		returnVariables[retVar.name] = 0;
	}

	ArithmeticOnlyInterpreter interpreter(
		state,
		m_dialect,
		*m_currentScope,
		/* _callerRecursionDepth=*/ 0,
		returnVariables
	);
	try
	{
		interpreter(_function.body);
	} catch (InterpreterTerminatedGeneric const&)
	{
		// won't replace body
		return ;
	}

	Block newBody;
	newBody.debugData = _function.body.debugData;

	// After the execution, all debug data got swept away. To still maintain
	// useful information, we assign the literal debug data with the debug data
	// of the function itself.
	// One case this assignment is helpful is in the case of function with only
	// one return variable. In this case, it would likely be a solidity
	// constant.
	langutil::DebugData::ConstPtr literalDebugData = _function.debugData;

	for (auto const& retVar: _function.returnVariables)
	{
		Identifier ident;
		ident.name = retVar.name;

		Literal val;
		val.debugData = literalDebugData;
		val.kind = LiteralKind::Number;
		val.value = LiteralValue(interpreter.valueOfVariable(retVar.name));

		Assignment assignment;
		assignment.variableNames = { std::move(ident) };
		assignment.value = { std::make_unique<Expression>(std::move(val)) };

		newBody.statements.push_back(std::move(assignment));
	}
	_function.body = std::move(newBody);
}

void ConstantFunctionEvaluator::operator()(Block& _block)
{
	enterScope(_block);

	for (auto const& statement: _block.statements)
		if (std::holds_alternative<FunctionDefinition>(statement))
		{
			FunctionDefinition const& funDef = std::get<FunctionDefinition>(statement);
			m_currentScope->names.emplace(funDef.name, &funDef);
		}

	for (auto& statement: _block.statements)
	{
		visit(statement);
	}

	leaveScope();
}

void ConstantFunctionEvaluator::enterScope(Block const& _block)
{
	if (!m_currentScope->subScopes.count(&_block))
		m_currentScope->subScopes[&_block] = std::make_unique<Scope>(Scope{
			{},
			{},
			m_currentScope,
		});
	m_currentScope = m_currentScope->subScopes[&_block].get();
}

void ConstantFunctionEvaluator::leaveScope()
{
	m_currentScope = m_currentScope->parent;
	yulAssert(m_currentScope, "");
}

u256 ArithmeticOnlyInterpreter::evaluate(Expression const& _expression)
{
	ArithmeticOnlyExpressionEvaluator ev(
		m_state,
		m_dialect,
		*m_scope,
		m_variables,
		m_disableExternalCalls,
		m_disableMemoryTrace,
		m_recursionDepth
	);
	ev.visit(_expression);
	return ev.value();
}

std::vector<u256> ArithmeticOnlyInterpreter::evaluateMulti(Expression const& _expression)
{
	ArithmeticOnlyExpressionEvaluator ev(
		m_state,
		m_dialect,
		*m_scope,
		m_variables,
		m_disableExternalCalls,
		m_disableMemoryTrace,
		m_recursionDepth
	);
	ev.visit(_expression);
	return ev.values();
}


void ArithmeticOnlyExpressionEvaluator::operator()(FunctionCall const& _funCall)
{
	FunctionCallType fnCallType = determineFunctionCallType(_funCall);
	if (fnCallType == FunctionCallType::BuiltinNonArithmetic)
	{
		BOOST_THROW_EXCEPTION(BuiltinNonArithmeticFunctionInvoked());
	}
	ExpressionEvaluator::operator()(_funCall);
}

ArithmeticOnlyExpressionEvaluator::FunctionCallType
ArithmeticOnlyExpressionEvaluator::determineFunctionCallType(FunctionCall const& _funCall)
{
	if (EVMDialect const* dialect = dynamic_cast<EVMDialect const*>(&m_dialect))
	{
		if (BuiltinFunctionForEVM const* fun = dialect->builtin(_funCall.functionName.name))
		{
			if (fun->instruction)
			{
				switch (*fun->instruction)
				{
				// --------------- arithmetic ---------------
				case Instruction::ADD:
				case Instruction::MUL:
				case Instruction::SUB:
				case Instruction::DIV:
				case Instruction::SDIV:
				case Instruction::MOD:
				case Instruction::SMOD:
				case Instruction::EXP:
				case Instruction::NOT:
				case Instruction::LT:
				case Instruction::GT:
				case Instruction::SLT:
				case Instruction::SGT:
				case Instruction::EQ:
				case Instruction::ISZERO:
				case Instruction::AND:
				case Instruction::OR:
				case Instruction::XOR:
				case Instruction::BYTE:
				case Instruction::SHL:
				case Instruction::SHR:
				case Instruction::SAR:
				case Instruction::ADDMOD:
				case Instruction::MULMOD:
				case Instruction::SIGNEXTEND:
					return FunctionCallType::BuiltinArithmetic;
				// --------------- stop ---------------------------
				case Instruction::STOP:
				// --------------- blockchain stuff ---------------
				case Instruction::KECCAK256:
				case Instruction::ADDRESS:
				case Instruction::BALANCE:
				case Instruction::SELFBALANCE:
				case Instruction::ORIGIN:
				case Instruction::CALLER:
				case Instruction::CALLVALUE:
				case Instruction::CALLDATALOAD:
				case Instruction::CALLDATASIZE:
				case Instruction::CALLDATACOPY:
				case Instruction::CODESIZE:
				case Instruction::CODECOPY:
				case Instruction::GASPRICE:
				case Instruction::CHAINID:
				case Instruction::BASEFEE:
				case Instruction::BLOBHASH:
				case Instruction::BLOBBASEFEE:
				case Instruction::EXTCODESIZE:
				case Instruction::EXTCODEHASH:
				case Instruction::EXTCODECOPY:
				case Instruction::RETURNDATASIZE:
				case Instruction::RETURNDATACOPY:
				case Instruction::MCOPY:
				case Instruction::BLOCKHASH:
				case Instruction::COINBASE:
				case Instruction::TIMESTAMP:
				case Instruction::NUMBER:
				case Instruction::PREVRANDAO:
				case Instruction::GASLIMIT:
				// --------------- memory / storage / logs ---------------
				case Instruction::MLOAD:
				case Instruction::MSTORE:
				case Instruction::MSTORE8:
				case Instruction::SLOAD:
				case Instruction::SSTORE:
				case Instruction::PC:
				case Instruction::MSIZE:
				case Instruction::GAS:
				case Instruction::LOG0:
				case Instruction::LOG1:
				case Instruction::LOG2:
				case Instruction::LOG3:
				case Instruction::LOG4:
				case Instruction::TLOAD:
				case Instruction::TSTORE:
				// --------------- calls ---------------
				case Instruction::CREATE:
				case Instruction::CREATE2:
				case Instruction::CALL:
				case Instruction::CALLCODE:
				case Instruction::DELEGATECALL:
				case Instruction::STATICCALL:
				case Instruction::RETURN:
				case Instruction::REVERT:
				case Instruction::INVALID:
				case Instruction::SELFDESTRUCT:
					return FunctionCallType::BuiltinNonArithmetic;

				// --------------- pop only discard value. ------------------
				case Instruction::POP:
					return FunctionCallType::BuiltinArithmetic;

				// --------------- invalid in strict assembly ---------------
				case Instruction::JUMP:
				case Instruction::JUMPI:
				case Instruction::JUMPDEST:
				case Instruction::PUSH0:
				case Instruction::PUSH1:
				case Instruction::PUSH2:
				case Instruction::PUSH3:
				case Instruction::PUSH4:
				case Instruction::PUSH5:
				case Instruction::PUSH6:
				case Instruction::PUSH7:
				case Instruction::PUSH8:
				case Instruction::PUSH9:
				case Instruction::PUSH10:
				case Instruction::PUSH11:
				case Instruction::PUSH12:
				case Instruction::PUSH13:
				case Instruction::PUSH14:
				case Instruction::PUSH15:
				case Instruction::PUSH16:
				case Instruction::PUSH17:
				case Instruction::PUSH18:
				case Instruction::PUSH19:
				case Instruction::PUSH20:
				case Instruction::PUSH21:
				case Instruction::PUSH22:
				case Instruction::PUSH23:
				case Instruction::PUSH24:
				case Instruction::PUSH25:
				case Instruction::PUSH26:
				case Instruction::PUSH27:
				case Instruction::PUSH28:
				case Instruction::PUSH29:
				case Instruction::PUSH30:
				case Instruction::PUSH31:
				case Instruction::PUSH32:
				case Instruction::DUP1:
				case Instruction::DUP2:
				case Instruction::DUP3:
				case Instruction::DUP4:
				case Instruction::DUP5:
				case Instruction::DUP6:
				case Instruction::DUP7:
				case Instruction::DUP8:
				case Instruction::DUP9:
				case Instruction::DUP10:
				case Instruction::DUP11:
				case Instruction::DUP12:
				case Instruction::DUP13:
				case Instruction::DUP14:
				case Instruction::DUP15:
				case Instruction::DUP16:
				case Instruction::SWAP1:
				case Instruction::SWAP2:
				case Instruction::SWAP3:
				case Instruction::SWAP4:
				case Instruction::SWAP5:
				case Instruction::SWAP6:
				case Instruction::SWAP7:
				case Instruction::SWAP8:
				case Instruction::SWAP9:
				case Instruction::SWAP10:
				case Instruction::SWAP11:
				case Instruction::SWAP12:
				case Instruction::SWAP13:
				case Instruction::SWAP14:
				case Instruction::SWAP15:
				case Instruction::SWAP16:
				{
					yulAssert(false, "");
				}
				}
			}
			else
			{
				static std::set<std::string> const NON_INSTRUCTION_FUNC_NAME = {
					"datasize",
					"dataoffset",
					"datacopy",
					"memoryguard",
					"loadimmutable",
					"setimmutable",
					"linkersymbol"
				};
				if (NON_INSTRUCTION_FUNC_NAME.count(fun->name.str()))
				{
					return FunctionCallType::BuiltinNonArithmetic;
				}
				if (boost::algorithm::starts_with(fun->name.str(), "verbatim"))
				{
					return FunctionCallType::BuiltinNonArithmetic;
				}
			}

			yulAssert(false, "Can not determine function call type for function " + fun->name.str());
		}
	}

	return FunctionCallType::InvokeOther;
}
