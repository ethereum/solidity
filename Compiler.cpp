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
}

void Compiler::appendFunctionSelector(std::vector<ASTPointer<FunctionDefinition>> const&)
{
	// filter public functions, and sort by name. Then select function from first byte,
	// unpack arguments from calldata, push to stack and jump. Pack return values to
	// output and return.
}

bool Compiler::visit(FunctionDefinition& _function)
{
	//@todo to simplify this, the colling convention could by changed such that
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
	m_context << eth::Instruction::NOT;
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
