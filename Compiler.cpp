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
#include <libsolidity/AST.h>
#include <libsolidity/Compiler.h>
#include <libsolidity/ExpressionCompiler.h>

using namespace std;

namespace dev {
namespace solidity {

bytes Compiler::compile(ContractDefinition& _contract)
{
	Compiler compiler;
	compiler.compileContract(_contract);
	return compiler.m_context.getAssembledBytecode();
}

void Compiler::compileContract(ContractDefinition& _contract)
{
	m_context = CompilerContext(); // clear it just in case

	//@todo constructor
	//@todo register state variables

	for (ASTPointer<FunctionDefinition> const& function: _contract.getDefinedFunctions())
		m_context.addFunction(*function);

	appendFunctionSelector(_contract.getDefinedFunctions());
	for (ASTPointer<FunctionDefinition> const& function: _contract.getDefinedFunctions())
		function->accept(*this);

	packIntoContractCreator();
}

void Compiler::packIntoContractCreator()
{
	CompilerContext creatorContext;
	eth::AssemblyItem sub = creatorContext.addSubroutine(m_context.getAssembly());
	// stack contains sub size
	creatorContext << eth::Instruction::DUP1 << sub << u256(0) << eth::Instruction::CODECOPY;
	creatorContext << u256(0) << eth::Instruction::RETURN;
	swap(m_context, creatorContext);
}

void Compiler::appendFunctionSelector(vector<ASTPointer<FunctionDefinition>> const& _functions)
{
	// sort all public functions and store them together with a tag for their argument decoding section
	map<string, pair<FunctionDefinition const*, eth::AssemblyItem>> publicFunctions;
	for (ASTPointer<FunctionDefinition> const& f: _functions)
		if (f->isPublic())
			publicFunctions.insert(make_pair(f->getName(), make_pair(f.get(), m_context.newTag())));

	//@todo remove constructor

	if (publicFunctions.size() > 255)
		BOOST_THROW_EXCEPTION(CompilerError() << errinfo_comment("More than 255 public functions for contract."));

	//@todo check for calldatasize?
	// retrieve the first byte of the call data
	m_context << u256(0) << eth::Instruction::CALLDATALOAD << u256(0) << eth::Instruction::BYTE;
	// check that it is not too large
	m_context << eth::Instruction::DUP1 << u256(publicFunctions.size() - 1) << eth::Instruction::LT;
	eth::AssemblyItem returnTag = m_context.appendConditionalJump();

	// otherwise, jump inside jump table (each entry of the table has size 4)
	m_context << u256(4) << eth::Instruction::MUL;
	eth::AssemblyItem jumpTableStart = m_context.pushNewTag();
	m_context << eth::Instruction::ADD << eth::Instruction::JUMP;

	// jump table @todo it could be that the optimizer destroys this
	m_context << jumpTableStart;
	for (pair<string, pair<FunctionDefinition const*, eth::AssemblyItem>> const& f: publicFunctions)
		m_context.appendJumpTo(f.second.second) << eth::Instruction::JUMPDEST;

	m_context << returnTag << eth::Instruction::RETURN;

	for (pair<string, pair<FunctionDefinition const*, eth::AssemblyItem>> const& f: publicFunctions)
	{
		m_context << f.second.second;
		appendFunctionCallSection(*f.second.first);
	}
}

void Compiler::appendFunctionCallSection(FunctionDefinition const& _function)
{
	eth::AssemblyItem returnTag = m_context.pushNewTag();

	appendCalldataUnpacker(_function);

	m_context.appendJumpTo(m_context.getFunctionEntryLabel(_function));
	m_context << returnTag;

	appendReturnValuePacker(_function);
}

void Compiler::appendCalldataUnpacker(FunctionDefinition const& _function)
{
	// We do not check the calldata size, everything is zero-padded.
	unsigned dataOffset = 1;

	//@todo this can be done more efficiently, saving some CALLDATALOAD calls
	for (ASTPointer<VariableDeclaration> const& var: _function.getParameters())
	{
		unsigned const numBytes = var->getType()->getCalldataEncodedSize();
		if (numBytes == 0)
			BOOST_THROW_EXCEPTION(CompilerError()
								  << errinfo_sourceLocation(var->getLocation())
								  << errinfo_comment("Type not yet supported."));
		if (numBytes == 32)
			m_context << u256(dataOffset) << eth::Instruction::CALLDATALOAD;
		else
			m_context << (u256(1) << ((32 - numBytes) * 8)) << u256(dataOffset)
					  << eth::Instruction::CALLDATALOAD << eth::Instruction::DIV;
		dataOffset += numBytes;
	}
}

void Compiler::appendReturnValuePacker(FunctionDefinition const& _function)
{
	//@todo this can be also done more efficiently
	unsigned dataOffset = 0;
	vector<ASTPointer<VariableDeclaration>> const& parameters = _function.getReturnParameters();
	for (unsigned i = 0 ; i < parameters.size(); ++i)
	{
		unsigned numBytes = parameters[i]->getType()->getCalldataEncodedSize();
		if (numBytes == 0)
			BOOST_THROW_EXCEPTION(CompilerError()
								  << errinfo_sourceLocation(parameters[i]->getLocation())
								  << errinfo_comment("Type not yet supported."));
		m_context << eth::dupInstruction(parameters.size() - i);
		if (numBytes == 32)
			m_context << u256(dataOffset) << eth::Instruction::MSTORE;
		else
			m_context << u256(dataOffset) << (u256(1) << ((32 - numBytes) * 8))
					  << eth::Instruction::MUL << eth::Instruction::MSTORE;
		dataOffset += numBytes;
	}
	// note that the stack is not cleaned up here
	m_context << u256(dataOffset) << u256(0) << eth::Instruction::RETURN;
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

	unsigned const numArguments = _function.getParameters().size();
	unsigned const numReturnValues = _function.getReturnParameters().size();
	unsigned const numLocalVariables = _function.getLocalVariables().size();

	for (ASTPointer<VariableDeclaration> const& variable: _function.getParameters() + _function.getReturnParameters())
		m_context.addVariable(*variable);
	for (VariableDeclaration const* localVariable: _function.getLocalVariables())
		m_context.addVariable(*localVariable);
	m_context.initializeLocalVariables(numReturnValues + numLocalVariables);

	_function.getBody().accept(*this);

	m_context << m_returnTag;

	// Now we need to re-shuffle the stack. For this we keep a record of the stack layout
	// that shows the target positions of the elements, where "-1" denotes that this element needs
	// to be removed from the stack.
	// Note that the fact that the return arguments are of increasing index is vital for this
	// algorithm to work.

	vector<int> stackLayout;
	stackLayout.push_back(numReturnValues); // target of return address
	stackLayout += vector<int>(numArguments, -1); // discard all arguments
	for (unsigned i = 0; i < numReturnValues; ++i)
		stackLayout.push_back(i);
	stackLayout += vector<int>(numLocalVariables, -1);

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

	m_context << eth::Instruction::JUMP;

	return false;
}

bool Compiler::visit(IfStatement& _ifStatement)
{
	ExpressionCompiler::compileExpression(m_context, _ifStatement.getCondition());
	eth::AssemblyItem trueTag = m_context.appendConditionalJump();
	if (_ifStatement.getFalseStatement())
		_ifStatement.getFalseStatement()->accept(*this);
	eth::AssemblyItem endTag = m_context.appendJump();
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
	assert(!m_continueTags.empty());
	m_context.appendJumpTo(m_continueTags.back());
	return false;
}

bool Compiler::visit(Break&)
{
	assert(!m_breakTags.empty());
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
		ExpressionCompiler::cleanHigherOrderBitsIfNeeded(*expression->getType(), *firstVariable.getType());
		int stackPosition = m_context.getStackPositionOfVariable(firstVariable);
		m_context << eth::swapInstruction(stackPosition) << eth::Instruction::POP;
	}
	m_context.appendJumpTo(m_returnTag);
	return false;
}

bool Compiler::visit(VariableDefinition& _variableDefinition)
{
	if (Expression* expression = _variableDefinition.getExpression())
	{
		ExpressionCompiler::compileExpression(m_context, *expression);
		ExpressionCompiler::cleanHigherOrderBitsIfNeeded(*expression->getType(),
														 *_variableDefinition.getDeclaration().getType());
		int stackPosition = m_context.getStackPositionOfVariable(_variableDefinition.getDeclaration());
		m_context << eth::swapInstruction(stackPosition) << eth::Instruction::POP;
	}
	return false;
}

bool Compiler::visit(ExpressionStatement& _expressionStatement)
{
	Expression& expression = _expressionStatement.getExpression();
	ExpressionCompiler::compileExpression(m_context, expression);
	if (expression.getType()->getCategory() != Type::Category::VOID)
		m_context << eth::Instruction::POP;
	return false;
}

}
}
