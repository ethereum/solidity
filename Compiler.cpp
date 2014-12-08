/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Solidity compiler.
 */

#include <algorithm>
#include <libevmcore/Instruction.h>
#include <libevmcore/Assembly.h>
#include <libsolidity/AST.h>
#include <libsolidity/Compiler.h>
#include <libsolidity/ExpressionCompiler.h>
#include <libsolidity/CompilerUtils.h>

using namespace std;

namespace dev {
namespace solidity {

void Compiler::compileContract(ContractDefinition& _contract, vector<MagicVariableDeclaration const*> const& _magicGlobals)
{
	m_context = CompilerContext(); // clear it just in case

	for (MagicVariableDeclaration const* variable: _magicGlobals)
		m_context.addMagicGlobal(*variable);

	for (ASTPointer<FunctionDefinition> const& function: _contract.getDefinedFunctions())
		if (function->getName() != _contract.getName()) // don't add the constructor here
			m_context.addFunction(*function);
	registerStateVariables(_contract);

	appendFunctionSelector(_contract);
	for (ASTPointer<FunctionDefinition> const& function: _contract.getDefinedFunctions())
		if (function->getName() != _contract.getName()) // don't add the constructor here
			function->accept(*this);

	packIntoContractCreator(_contract);
}

void Compiler::packIntoContractCreator(ContractDefinition const& _contract)
{
	CompilerContext runtimeContext;
	swap(m_context, runtimeContext);

	registerStateVariables(_contract);

	FunctionDefinition* constructor = nullptr;
	for (ASTPointer<FunctionDefinition> const& function: _contract.getDefinedFunctions())
		if (function->getName() == _contract.getName())
		{
			constructor = function.get();
			break;
		}
	if (constructor)
	{
		eth::AssemblyItem returnTag = m_context.pushNewTag();
		m_context.addFunction(*constructor); // note that it cannot be called due to syntactic reasons
		//@todo copy constructor arguments from calldata to memory prior to this
		//@todo calling other functions inside the constructor should either trigger a parse error
		//or we should copy them here (register them above and call "accept") - detecting which
		// functions are referenced / called needs to be done in a recursive way.
		appendCalldataUnpacker(*constructor, true);
		m_context.appendJumpTo(m_context.getFunctionEntryLabel(*constructor));
		constructor->accept(*this);
		m_context << returnTag;
	}

	eth::AssemblyItem sub = m_context.addSubroutine(runtimeContext.getAssembly());
	// stack contains sub size
	m_context << eth::Instruction::DUP1 << sub << u256(0) << eth::Instruction::CODECOPY;
	m_context << u256(0) << eth::Instruction::RETURN;
}

void Compiler::appendFunctionSelector(ContractDefinition const& _contract)
{
	vector<FunctionDefinition const*> interfaceFunctions = _contract.getInterfaceFunctions();
	vector<eth::AssemblyItem> callDataUnpackerEntryPoints;

	if (interfaceFunctions.size() > 255)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("More than 255 public functions for contract."));

	// retrieve the first byte of the call data, which determines the called function
	// @todo This code had a jump table in a previous version which was more efficient but also
	// error prone (due to the optimizer and variable length tag addresses)
	m_context << u256(1) << u256(0) // some constants
			  << eth::dupInstruction(1) << eth::Instruction::CALLDATALOAD
			  << eth::dupInstruction(2) << eth::Instruction::BYTE
			  << eth::dupInstruction(2);

	// stack here: 1 0 <funid> 0, stack top will be counted up until it matches funid
	for (unsigned funid = 0; funid < interfaceFunctions.size(); ++funid)
	{
		callDataUnpackerEntryPoints.push_back(m_context.newTag());
		m_context << eth::dupInstruction(2) << eth::dupInstruction(2) << eth::Instruction::EQ;
		m_context.appendConditionalJumpTo(callDataUnpackerEntryPoints.back());
		m_context << eth::dupInstruction(4) << eth::Instruction::ADD;
		//@todo avoid the last ADD (or remove it in the optimizer)
	}
	m_context << eth::Instruction::STOP; // function not found

	for (unsigned funid = 0; funid < interfaceFunctions.size(); ++funid)
	{
		FunctionDefinition const& function = *interfaceFunctions[funid];
		m_context << callDataUnpackerEntryPoints[funid];
		eth::AssemblyItem returnTag = m_context.pushNewTag();
		appendCalldataUnpacker(function);
		m_context.appendJumpTo(m_context.getFunctionEntryLabel(function));
		m_context << returnTag;
		appendReturnValuePacker(function);
	}
}

unsigned Compiler::appendCalldataUnpacker(FunctionDefinition const& _function, bool _fromMemory)
{
	// We do not check the calldata size, everything is zero-padded.
	unsigned dataOffset = 1;
	eth::Instruction load = _fromMemory ? eth::Instruction::MLOAD : eth::Instruction::CALLDATALOAD;

	//@todo this can be done more efficiently, saving some CALLDATALOAD calls
	for (ASTPointer<VariableDeclaration> const& var: _function.getParameters())
	{
		unsigned const numBytes = var->getType()->getCalldataEncodedSize();
		if (numBytes == 0 || numBytes > 32)
			BOOST_THROW_EXCEPTION(CompilerError()
								  << errinfo_sourceLocation(var->getLocation())
								  << errinfo_comment("Type " + var->getType()->toString() + " not yet supported."));
		if (numBytes == 32)
			m_context << u256(dataOffset) << load;
		else
			m_context << (u256(1) << ((32 - numBytes) * 8)) << u256(dataOffset)
					  << load << eth::Instruction::DIV;
		dataOffset += numBytes;
	}
	return dataOffset;
}

void Compiler::appendReturnValuePacker(FunctionDefinition const& _function)
{
	//@todo this can be also done more efficiently
	unsigned dataOffset = 0;
	vector<ASTPointer<VariableDeclaration>> const& parameters = _function.getReturnParameters();
	for (unsigned i = 0; i < parameters.size(); ++i)
	{
		Type const& paramType = *parameters[i]->getType();
		unsigned numBytes = paramType.getCalldataEncodedSize();
		if (numBytes == 0 || numBytes > 32)
			BOOST_THROW_EXCEPTION(CompilerError()
								  << errinfo_sourceLocation(parameters[i]->getLocation())
								  << errinfo_comment("Type " + paramType.toString() + " not yet supported."));
		m_context << eth::dupInstruction(parameters.size() - i);
		if (numBytes != 32)
			m_context << (u256(1) << ((32 - numBytes) * 8)) << eth::Instruction::MUL;
		m_context << u256(dataOffset) << eth::Instruction::MSTORE;
		dataOffset += numBytes;
	}
	// note that the stack is not cleaned up here
	m_context << u256(dataOffset) << u256(0) << eth::Instruction::RETURN;
}

void Compiler::registerStateVariables(ContractDefinition const& _contract)
{
	//@todo sort them?
	for (ASTPointer<VariableDeclaration> const& variable: _contract.getStateVariables())
		m_context.addStateVariable(*variable);
}

bool Compiler::visit(FunctionDefinition& _function)
{
	//@todo to simplify this, the calling convention could by changed such that
	// caller puts: [retarg0] ... [retargm] [return address] [arg0] ... [argn]
	// although note that this reduces the size of the visible stack

	m_context.startNewFunction();
	m_returnTag = m_context.newTag();
	m_breakTags.clear();
	m_continueTags.clear();

	m_context << m_context.getFunctionEntryLabel(_function);

	// stack upon entry: [return address] [arg0] [arg1] ... [argn]
	// reserve additional slots: [retarg0] ... [retargm] [localvar0] ... [localvarp]

	for (ASTPointer<VariableDeclaration const> const& variable: _function.getParameters())
		m_context.addVariable(*variable);
	for (ASTPointer<VariableDeclaration const> const& variable: _function.getReturnParameters())
		m_context.addAndInitializeVariable(*variable);
	for (VariableDeclaration const* localVariable: _function.getLocalVariables())
		m_context.addAndInitializeVariable(*localVariable);

	_function.getBody().accept(*this);

	m_context << m_returnTag;

	// Now we need to re-shuffle the stack. For this we keep a record of the stack layout
	// that shows the target positions of the elements, where "-1" denotes that this element needs
	// to be removed from the stack.
	// Note that the fact that the return arguments are of increasing index is vital for this
	// algorithm to work.

	unsigned argumentsSize = 0;
	for (ASTPointer<VariableDeclaration const> const& variable: _function.getParameters())
		argumentsSize += variable->getType()->getSizeOnStack();
	unsigned returnValuesSize = 0;
	for (ASTPointer<VariableDeclaration const> const& variable: _function.getReturnParameters())
		returnValuesSize += variable->getType()->getSizeOnStack();
	unsigned localVariablesSize = 0;
	for (VariableDeclaration const* localVariable: _function.getLocalVariables())
		localVariablesSize += localVariable->getType()->getSizeOnStack();

	vector<int> stackLayout;
	stackLayout.push_back(returnValuesSize); // target of return address
	stackLayout += vector<int>(argumentsSize, -1); // discard all arguments
	for (unsigned i = 0; i < returnValuesSize; ++i)
		stackLayout.push_back(i);
	stackLayout += vector<int>(localVariablesSize, -1);

	while (stackLayout.back() != int(stackLayout.size() - 1))
		if (stackLayout.back() < 0)
		{
			m_context << eth::Instruction::POP;
			stackLayout.pop_back();
		}
		else
		{
			m_context << eth::swapInstruction(stackLayout.size() - stackLayout.back() - 1);
			swap(stackLayout[stackLayout.back()], stackLayout.back());
		}
	//@todo assert that everything is in place now

	m_context << eth::Instruction::JUMP;

	return false;
}

bool Compiler::visit(IfStatement& _ifStatement)
{
	ExpressionCompiler::compileExpression(m_context, _ifStatement.getCondition());
	eth::AssemblyItem trueTag = m_context.appendConditionalJump();
	if (_ifStatement.getFalseStatement())
		_ifStatement.getFalseStatement()->accept(*this);
	eth::AssemblyItem endTag = m_context.appendJumpToNew();
	m_context << trueTag;
	_ifStatement.getTrueStatement().accept(*this);
	m_context << endTag;
	return false;
}

bool Compiler::visit(WhileStatement& _whileStatement)
{
	eth::AssemblyItem loopStart = m_context.newTag();
	eth::AssemblyItem loopEnd = m_context.newTag();
	m_continueTags.push_back(loopStart);
	m_breakTags.push_back(loopEnd);

	m_context << loopStart;
	ExpressionCompiler::compileExpression(m_context, _whileStatement.getCondition());
	m_context << eth::Instruction::ISZERO;
	m_context.appendConditionalJumpTo(loopEnd);

	_whileStatement.getBody().accept(*this);

	m_context.appendJumpTo(loopStart);
	m_context << loopEnd;

	m_continueTags.pop_back();
	m_breakTags.pop_back();
	return false;
}

bool Compiler::visit(Continue&)
{
	if (!m_continueTags.empty())
		m_context.appendJumpTo(m_continueTags.back());
	return false;
}

bool Compiler::visit(Break&)
{
	if (!m_breakTags.empty())
		m_context.appendJumpTo(m_breakTags.back());
	return false;
}

bool Compiler::visit(Return& _return)
{
	//@todo modifications are needed to make this work with functions returning multiple values
	if (Expression* expression = _return.getExpression())
	{
		ExpressionCompiler::compileExpression(m_context, *expression);
		VariableDeclaration const& firstVariable = *_return.getFunctionReturnParameters().getParameters().front();
		ExpressionCompiler::appendTypeConversion(m_context, *expression->getType(), *firstVariable.getType());

		CompilerUtils(m_context).moveToStackVariable(firstVariable);
	}
	m_context.appendJumpTo(m_returnTag);
	return false;
}

bool Compiler::visit(VariableDefinition& _variableDefinition)
{
	if (Expression* expression = _variableDefinition.getExpression())
	{
		ExpressionCompiler::compileExpression(m_context, *expression);
		ExpressionCompiler::appendTypeConversion(m_context,
												 *expression->getType(),
												 *_variableDefinition.getDeclaration().getType());
		CompilerUtils(m_context).moveToStackVariable(_variableDefinition.getDeclaration());
	}
	return false;
}

bool Compiler::visit(ExpressionStatement& _expressionStatement)
{
	Expression& expression = _expressionStatement.getExpression();
	ExpressionCompiler::compileExpression(m_context, expression);
	CompilerUtils(m_context).popStackElement(*expression.getType());
	return false;
}

}
}
