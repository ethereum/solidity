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
#include <boost/range/adaptor/reversed.hpp>
#include <libevmcore/Instruction.h>
#include <libevmcore/Assembly.h>
#include <libsolidity/AST.h>
#include <libsolidity/Compiler.h>
#include <libsolidity/ExpressionCompiler.h>
#include <libsolidity/CompilerUtils.h>

using namespace std;

namespace dev {
namespace solidity {

void Compiler::compileContract(ContractDefinition const& _contract,
							   map<ContractDefinition const*, bytes const*> const& _contracts)
{
	m_context = CompilerContext(); // clear it just in case
	initializeContext(_contract, _contracts);
	appendFunctionSelector(_contract);
	set<Declaration const*> functions = m_context.getFunctionsWithoutCode();
	while (!functions.empty())
	{
		for (Declaration const* function: functions)
			function->accept(*this);
		functions = m_context.getFunctionsWithoutCode();
	}

	// Swap the runtime context with the creation-time context
	swap(m_context, m_runtimeContext);
	initializeContext(_contract, _contracts);
	packIntoContractCreator(_contract, m_runtimeContext);
}

void Compiler::initializeContext(ContractDefinition const& _contract,
								 map<ContractDefinition const*, bytes const*> const& _contracts)
{
	m_context.setCompiledContracts(_contracts);
	m_context.setInheritanceHierarchy(_contract.getLinearizedBaseContracts());
	registerStateVariables(_contract);
}

void Compiler::packIntoContractCreator(ContractDefinition const& _contract, CompilerContext const& _runtimeContext)
{
	// arguments for base constructors, filled in derived-to-base order
	map<ContractDefinition const*, vector<ASTPointer<Expression>> const*> baseArguments;

	// Determine the arguments that are used for the base constructors.
	std::vector<ContractDefinition const*> const& bases = _contract.getLinearizedBaseContracts();
	for (ContractDefinition const* contract: bases)
		for (ASTPointer<InheritanceSpecifier> const& base: contract->getBaseContracts())
		{
			ContractDefinition const* baseContract = dynamic_cast<ContractDefinition const*>(
													base->getName()->getReferencedDeclaration());
			solAssert(baseContract, "");
			if (baseArguments.count(baseContract) == 0)
				baseArguments[baseContract] = &base->getArguments();
		}

	// Call constructors in base-to-derived order.
	// The Constructor for the most derived contract is called later.
	for (unsigned i = 1; i < bases.size(); i++)
	{
		ContractDefinition const* base = bases[bases.size() - i];
		solAssert(base, "");
		FunctionDefinition const* baseConstructor = base->getConstructor();
		if (!baseConstructor)
			continue;
		solAssert(baseArguments[base], "");
		appendBaseConstructorCall(*baseConstructor, *baseArguments[base]);
	}
	if (_contract.getConstructor())
		appendConstructorCall(*_contract.getConstructor());

	eth::AssemblyItem sub = m_context.addSubroutine(_runtimeContext.getAssembly());
	// stack contains sub size
	m_context << eth::Instruction::DUP1 << sub << u256(0) << eth::Instruction::CODECOPY;
	m_context << u256(0) << eth::Instruction::RETURN;

	// note that we have to include the functions again because of absolute jump labels
	set<Declaration const*> functions = m_context.getFunctionsWithoutCode();
	while (!functions.empty())
	{
		for (Declaration const* function: functions)
			function->accept(*this);
		functions = m_context.getFunctionsWithoutCode();
	}
}

void Compiler::appendBaseConstructorCall(FunctionDefinition const& _constructor,
										 vector<ASTPointer<Expression>> const& _arguments)
{
	FunctionType constructorType(_constructor);
	eth::AssemblyItem returnLabel = m_context.pushNewTag();
	for (unsigned i = 0; i < _arguments.size(); ++i)
		compileExpression(*_arguments[i], constructorType.getParameterTypes()[i]);
	m_context.appendJumpTo(m_context.getFunctionEntryLabel(_constructor));
	m_context << returnLabel;
}

void Compiler::appendConstructorCall(FunctionDefinition const& _constructor)
{
	eth::AssemblyItem returnTag = m_context.pushNewTag();
	// copy constructor arguments from code to memory and then to stack, they are supplied after the actual program
	unsigned argumentSize = 0;
	for (ASTPointer<VariableDeclaration> const& var: _constructor.getParameters())
		argumentSize += CompilerUtils::getPaddedSize(var->getType()->getCalldataEncodedSize());

	if (argumentSize > 0)
	{
		m_context << u256(argumentSize);
		m_context.appendProgramSize();
		m_context << u256(CompilerUtils::dataStartOffset); // copy it to byte four as expected for ABI calls
		m_context << eth::Instruction::CODECOPY;
		appendCalldataUnpacker(FunctionType(_constructor).getParameterTypes(), true);
	}
	m_context.appendJumpTo(m_context.getFunctionEntryLabel(_constructor));
	m_context << returnTag;
}

void Compiler::appendFunctionSelector(ContractDefinition const& _contract)
{
	map<FixedHash<4>, FunctionTypePointer> interfaceFunctions = _contract.getInterfaceFunctions();
	map<FixedHash<4>, const eth::AssemblyItem> callDataUnpackerEntryPoints;

	// retrieve the function signature hash from the calldata
	if (!interfaceFunctions.empty())
		CompilerUtils(m_context).loadFromMemory(0, 4, false, true);

	// stack now is: 1 0 <funhash>
	for (auto const& it: interfaceFunctions)
	{
		callDataUnpackerEntryPoints.insert(std::make_pair(it.first, m_context.newTag()));
		m_context << eth::dupInstruction(1) << u256(FixedHash<4>::Arith(it.first)) << eth::Instruction::EQ;
		m_context.appendConditionalJumpTo(callDataUnpackerEntryPoints.at(it.first));
	}
	if (FunctionDefinition const* fallback = _contract.getFallbackFunction())
	{
		eth::AssemblyItem returnTag = m_context.pushNewTag();
		fallback->accept(*this);
		m_context << returnTag;
		appendReturnValuePacker(FunctionType(*fallback).getReturnParameterTypes());
	}
	else
		m_context << eth::Instruction::STOP; // function not found

	for (auto const& it: interfaceFunctions)
	{
		FunctionTypePointer const& functionType = it.second;
		m_context << callDataUnpackerEntryPoints.at(it.first);
		eth::AssemblyItem returnTag = m_context.pushNewTag();
		appendCalldataUnpacker(functionType->getParameterTypes());
		m_context.appendJumpTo(m_context.getFunctionEntryLabel(it.second->getDeclaration()));
		m_context << returnTag;
		appendReturnValuePacker(functionType->getReturnParameterTypes());
	}
}

unsigned Compiler::appendCalldataUnpacker(TypePointers const& _typeParameters, bool _fromMemory)
{
	// We do not check the calldata size, everything is zero-padded.
	unsigned dataOffset = CompilerUtils::dataStartOffset; // the 4 bytes of the function hash signature
	//@todo this can be done more efficiently, saving some CALLDATALOAD calls
	for (TypePointer const& type: _typeParameters)
	{
		unsigned const c_numBytes = type->getCalldataEncodedSize();
		if (c_numBytes > 32)
			BOOST_THROW_EXCEPTION(CompilerError()
								  << errinfo_comment("Type " + type->toString() + " not yet supported."));
		bool const c_leftAligned = type->getCategory() == Type::Category::String;
		bool const c_padToWords = true;
		dataOffset += CompilerUtils(m_context).loadFromMemory(dataOffset, c_numBytes, c_leftAligned,
															  !_fromMemory, c_padToWords);
	}
	return dataOffset;
}

void Compiler::appendReturnValuePacker(TypePointers const& _typeParameters)
{
	//@todo this can be also done more efficiently
	unsigned dataOffset = 0;
	unsigned stackDepth = 0;
	for (TypePointer const& type: _typeParameters)
		stackDepth += type->getSizeOnStack();

	for (TypePointer const& type: _typeParameters)
	{
		CompilerUtils(m_context).copyToStackTop(stackDepth, *type);
		ExpressionCompiler::appendTypeConversion(m_context, *type, *type, true);
		bool const c_padToWords = true;
		dataOffset += CompilerUtils(m_context).storeInMemory(dataOffset, *type, c_padToWords);
		stackDepth -= type->getSizeOnStack();
	}
	// note that the stack is not cleaned up here
	m_context << u256(dataOffset) << u256(0) << eth::Instruction::RETURN;
}

void Compiler::registerStateVariables(ContractDefinition const& _contract)
{
	for (ContractDefinition const* contract: boost::adaptors::reverse(_contract.getLinearizedBaseContracts()))
		for (ASTPointer<VariableDeclaration> const& variable: contract->getStateVariables())
			m_context.addStateVariable(*variable);
}

bool Compiler::visit(VariableDeclaration const& _variableDeclaration)
{
	solAssert(_variableDeclaration.isStateVariable(), "Compiler visit to non-state variable declaration.");

	m_context.startFunction(_variableDeclaration);
	m_breakTags.clear();
	m_continueTags.clear();

	m_context << m_context.getFunctionEntryLabel(_variableDeclaration);
	ExpressionCompiler::appendStateVariableAccessor(m_context, _variableDeclaration);

	return false;
}

bool Compiler::visit(FunctionDefinition const& _function)
{
	//@todo to simplify this, the calling convention could by changed such that
	// caller puts: [retarg0] ... [retargm] [return address] [arg0] ... [argn]
	// although note that this reduces the size of the visible stack

	m_context.startFunction(_function);
	m_returnTag = m_context.newTag();
	m_breakTags.clear();
	m_continueTags.clear();
	m_stackCleanupForReturn = 0;
	m_currentFunction = &_function;
	m_modifierDepth = 0;

	// stack upon entry: [return address] [arg0] [arg1] ... [argn]
	// reserve additional slots: [retarg0] ... [retargm] [localvar0] ... [localvarp]

	unsigned parametersSize = CompilerUtils::getSizeOnStack(_function.getParameters());
	m_context.adjustStackOffset(parametersSize);
	for (ASTPointer<VariableDeclaration const> const& variable: _function.getParameters())
	{
		m_context.addVariable(*variable, parametersSize);
		parametersSize -= variable->getType()->getSizeOnStack();
	}
	for (ASTPointer<VariableDeclaration const> const& variable: _function.getReturnParameters())
		m_context.addAndInitializeVariable(*variable);
	for (VariableDeclaration const* localVariable: _function.getLocalVariables())
		m_context.addAndInitializeVariable(*localVariable);

	appendModifierOrFunctionCode();

	m_context << m_returnTag;

	// Now we need to re-shuffle the stack. For this we keep a record of the stack layout
	// that shows the target positions of the elements, where "-1" denotes that this element needs
	// to be removed from the stack.
	// Note that the fact that the return arguments are of increasing index is vital for this
	// algorithm to work.

	unsigned const c_argumentsSize = CompilerUtils::getSizeOnStack(_function.getParameters());
	unsigned const c_returnValuesSize = CompilerUtils::getSizeOnStack(_function.getReturnParameters());
	unsigned const c_localVariablesSize = CompilerUtils::getSizeOnStack(_function.getLocalVariables());

	vector<int> stackLayout;
	stackLayout.push_back(c_returnValuesSize); // target of return address
	stackLayout += vector<int>(c_argumentsSize, -1); // discard all arguments
	for (unsigned i = 0; i < c_returnValuesSize; ++i)
		stackLayout.push_back(i);
	stackLayout += vector<int>(c_localVariablesSize, -1);

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

bool Compiler::visit(IfStatement const& _ifStatement)
{
	compileExpression(_ifStatement.getCondition());
	eth::AssemblyItem trueTag = m_context.appendConditionalJump();
	if (_ifStatement.getFalseStatement())
		_ifStatement.getFalseStatement()->accept(*this);
	eth::AssemblyItem endTag = m_context.appendJumpToNew();
	m_context << trueTag;
	_ifStatement.getTrueStatement().accept(*this);
	m_context << endTag;
	return false;
}

bool Compiler::visit(WhileStatement const& _whileStatement)
{
	eth::AssemblyItem loopStart = m_context.newTag();
	eth::AssemblyItem loopEnd = m_context.newTag();
	m_continueTags.push_back(loopStart);
	m_breakTags.push_back(loopEnd);

	m_context << loopStart;
	compileExpression(_whileStatement.getCondition());
	m_context << eth::Instruction::ISZERO;
	m_context.appendConditionalJumpTo(loopEnd);

	_whileStatement.getBody().accept(*this);

	m_context.appendJumpTo(loopStart);
	m_context << loopEnd;

	m_continueTags.pop_back();
	m_breakTags.pop_back();
	return false;
}

bool Compiler::visit(ForStatement const& _forStatement)
{
	eth::AssemblyItem loopStart = m_context.newTag();
	eth::AssemblyItem loopEnd = m_context.newTag();
	m_continueTags.push_back(loopStart);
	m_breakTags.push_back(loopEnd);

	if (_forStatement.getInitializationExpression())
		_forStatement.getInitializationExpression()->accept(*this);

	m_context << loopStart;

	// if there is no terminating condition in for, default is to always be true
	if (_forStatement.getCondition())
	{
		compileExpression(*_forStatement.getCondition());
		m_context << eth::Instruction::ISZERO;
		m_context.appendConditionalJumpTo(loopEnd);
	}

	_forStatement.getBody().accept(*this);

	// for's loop expression if existing
	if (_forStatement.getLoopExpression())
		_forStatement.getLoopExpression()->accept(*this);

	m_context.appendJumpTo(loopStart);
	m_context << loopEnd;

	m_continueTags.pop_back();
	m_breakTags.pop_back();
	return false;
}

bool Compiler::visit(Continue const&)
{
	if (!m_continueTags.empty())
		m_context.appendJumpTo(m_continueTags.back());
	return false;
}

bool Compiler::visit(Break const&)
{
	if (!m_breakTags.empty())
		m_context.appendJumpTo(m_breakTags.back());
	return false;
}

bool Compiler::visit(Return const& _return)
{
	//@todo modifications are needed to make this work with functions returning multiple values
	if (Expression const* expression = _return.getExpression())
	{
		solAssert(_return.getFunctionReturnParameters(), "Invalid return parameters pointer.");
		VariableDeclaration const& firstVariable = *_return.getFunctionReturnParameters()->getParameters().front();
		compileExpression(*expression, firstVariable.getType());
		CompilerUtils(m_context).moveToStackVariable(firstVariable);
	}
	for (unsigned i = 0; i < m_stackCleanupForReturn; ++i)
		m_context << eth::Instruction::POP;
	m_context.appendJumpTo(m_returnTag);
	m_context.adjustStackOffset(m_stackCleanupForReturn);
	return false;
}

bool Compiler::visit(VariableDefinition const& _variableDefinition)
{
	if (Expression const* expression = _variableDefinition.getExpression())
	{
		compileExpression(*expression, _variableDefinition.getDeclaration().getType());
		CompilerUtils(m_context).moveToStackVariable(_variableDefinition.getDeclaration());
	}
	return false;
}

bool Compiler::visit(ExpressionStatement const& _expressionStatement)
{
	Expression const& expression = _expressionStatement.getExpression();
	compileExpression(expression);
	CompilerUtils(m_context).popStackElement(*expression.getType());
	return false;
}

bool Compiler::visit(PlaceholderStatement const&)
{
	++m_modifierDepth;
	appendModifierOrFunctionCode();
	--m_modifierDepth;
	return true;
}

void Compiler::appendModifierOrFunctionCode()
{
	solAssert(m_currentFunction, "");
	if (m_modifierDepth >= m_currentFunction->getModifiers().size())
		m_currentFunction->getBody().accept(*this);
	else
	{
		ASTPointer<ModifierInvocation> const& modifierInvocation = m_currentFunction->getModifiers()[m_modifierDepth];

		ModifierDefinition const& modifier = m_context.getFunctionModifier(modifierInvocation->getName()->getName());
		solAssert(modifier.getParameters().size() == modifierInvocation->getArguments().size(), "");
		for (unsigned i = 0; i < modifier.getParameters().size(); ++i)
		{
			m_context.addVariable(*modifier.getParameters()[i]);
			compileExpression(*modifierInvocation->getArguments()[i],
							  modifier.getParameters()[i]->getType());
		}
		for (VariableDeclaration const* localVariable: modifier.getLocalVariables())
			m_context.addAndInitializeVariable(*localVariable);

		unsigned const c_stackSurplus = CompilerUtils::getSizeOnStack(modifier.getParameters()) +
										CompilerUtils::getSizeOnStack(modifier.getLocalVariables());
		m_stackCleanupForReturn += c_stackSurplus;

		modifier.getBody().accept(*this);

		for (unsigned i = 0; i < c_stackSurplus; ++i)
			m_context << eth::Instruction::POP;
		m_stackCleanupForReturn -= c_stackSurplus;
	}
}

void Compiler::compileExpression(Expression const& _expression, TypePointer const& _targetType)
{
	ExpressionCompiler::compileExpression(m_context, _expression, m_optimize);
	if (_targetType)
		ExpressionCompiler::appendTypeConversion(m_context, *_expression.getType(), *_targetType);
}

}
}
